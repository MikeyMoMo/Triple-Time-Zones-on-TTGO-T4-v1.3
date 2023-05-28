// Original author: (must be left in any copied or modified code)
//  Mike Morrow (GitHub MikeyMoMo) 5/16/2023

#define myVersion "v.12.00"  // Version for T4 v1.3

#include <SPI.h>
#include <TFT_eSPI.h>
#include "Adafruit_GFX.h"
#include "esp_sntp.h"

#include "Preferences.h"
Preferences preferences;

#include "OpenFontRender.h"
OpenFontRender ofr;

// >>>>>>======== Here begins the customization changes.========<<<<<<

//  >>> *** The lengths must all be equal to the longest one *** <<<
const char * Zulu  = "UTC0                  ";
const char * TopTZ = "PST8PDT,M3.2.0,M11.1.0";  // Top city name
const char * BotTZ = "PHT-8                 ";  // Bottom city name
//  >>> *** *** *** *** *** *** *** *** *** *** *** *** *** ***  <<<

const char * TopCityname = "Benicia";
const char * BotCityname = "Bangui";

// WiFi credentials.  Change to yours for SNTP connect and epoch fetch.
const char * ssid      = "LallaveWifi";
const char * wifipw    = "Lallave@Family7";

#define changeClockShowPin 38  // Change which of the 2 clocks to show
#define changeACShowingPin 37  // Change how the analog display looks

// >>>>>>>>>>============  End of user changes  ============<<<<<<<<<<

unsigned int acCustom;  // default is to show all, fetched, later from preferences.
// This is interpreted as a bit-coded integer.
// acCustom settings:
//  0 bits on -- do not show second hand, do not show digital time
//  1 bit on  -- show short digital time if the 2 bit is off
bool showShortTime;
unsigned int showShortTimeBit = 0;  // bit shift amount for this option
//  2 bit on  -- show long digital time, even if 1 bit is on
bool showLongTime;
unsigned int showLongTimeBit = 1;  // bit shift amount for this option
//  4 bit on  -- show second hand
bool showSecondHand;
unsigned int showSecondHandBit = 2;  // bit shift amount for this option

time_t now, UTC;
struct tm timeinfo;
int iYear, TopOffset, BotOffset, tempOffset;

int  prevHour = -1, prevSecond = -1, i;
long int firstHeapSize = 0, lastHeapSize;
char charWork[100];  // For various conversions from text to integer.
// Oops, we did not include time.h and NTPClient.h.  Whatever will we do?!?!?!

char TopDST[10];
char BotDST[10];

#include <WiFi.h>

// Setup file for ESP32 and TTGO T4 v1.3 SPI bus TFT
// Define TFT_eSPI object with the size of the screen:
//  240 pixels width and 320 pixels height.

//Use this: #include <User_Setups/Setup22_TTGO_T4_v1.3.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite clockSprite = TFT_eSprite(&tft);
TFT_eSprite scrollSprite = TFT_eSprite(&tft);
int scrollSpriteW, scrollSpriteH;

// Setting PWM properties, do not change this!
const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 4;
// Startup TFT backlight intensity on a scale of 0 to 255.
const int ledBacklightFull = 255;
//For orientations 1 & 3, we have to swap the width and height
#undef TFT_WIDTH
#define TFT_WIDTH 320
#undef TFT_HEIGHT
#define TFT_HEIGHT 240
#define RGB565(r,g,b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))
#define RGB888(r,g,b) ((r << 16) | (g << 8) | b)
#define DarkerRed RGB565(150,0,0)
#define SecondHand RGB565(200,128,128)
#define DarkPurple RGB565(118,0,100)
#define DarkBlue RGB565(0, 0, 80)
//                          0    1    2    3    4    5       Hours
int hourlyBrilliance[] = { 60,  50,  30,  30,  20,  20,   //  0- 5
                           //6   7    8    9   10   11
                           30,  50,  70,  80,  80, 100,   //  6-11
                           //12  13   14   15   16   17
                           160, 160, 160, 160, 160, 160,  // 12-17
                           //18   19    20    21    22  23
                           160,  160,  160,  160,  120, 80   // 18-23
                         };
const int xCenter = TFT_WIDTH  / 2;
const int yCenter = TFT_HEIGHT / 2;

// Analog Clock face definitions.
char   *numbers[12] = {"6", "5", "4", "3", "2", "1", "12", "11", "10", "9", "8", "7"};
int    myX1, myY1, myX2, myY2, backX, backY;
int    plus90BulgeX, plus90BulgeY;
int    minus90BulgeX, minus90BulgeY;
int    startMillis;
double angle;
int    radius;
bool   digitalClock = 0;
bool   analogClock = 1;
bool   whichClock = digitalClock;  // Later, save it for restarts.
int    currHour, currMinute, currSecond;

const int displayLine1 =   0;  // subtracting 42
const int displayLine2 =   8;  // was 50
const int displayLine3 =  48;  // was 90
const int displayLine4 =  98;  // was 140
const int displayLine5 = 138;  // was 180

const bool NOscrollUpdate = false;
const bool OKscrollUpdate = true;

/***************************************************************************/
void setup()
/***************************************************************************/
{
  int *a = 0;

  Serial.begin(115200); delay(1000);

  Serial.println(F("\n\nThis is the Triple TZ on T4 v1.3."));
  String asdf = String(__FILE__);
  asdf = asdf.substring(asdf.lastIndexOf("\\") + 1, asdf.length());
  Serial.print("Running from: "); Serial.println(asdf);
  Serial.println("Compiled on: " + String(__DATE__) + " at " + String(__TIME__));

  initDisplay();

  radius = min(tft.width(), tft.height()) / 2 - 1;
  Serial.printf("Analog clock radius is %i\r\n", radius);
  // This must be allocated in PSRAM since ESP folks screwed up allocation.  There is a 32 bit
  //  and an 8 bit allocation but no 16 bit so you get double what you need and it blows the stack!

  scrollSpriteW = tft.width() * 3.5;
  scrollSpriteH = 42;
  a = (int*)scrollSprite.createSprite(scrollSpriteW, scrollSpriteH); // 1st sprite for scrolling info
  if (a == 0) {
    Serial.println("scrollSprite creation failed.  Cannot continue.");
    while (1);
  }
  scrollSprite.fillSprite(DarkBlue);
  scrollSprite.setScrollRect(0, 0, scrollSpriteW, scrollSpriteH, DarkBlue);
  scrollSprite.setTextColor(TFT_WHITE, DarkBlue);
  scrollSprite.setTextColor(TFT_WHITE);

  a = (int*)clockSprite.createSprite(tft.width(), tft.height());  // Leave room for 1st sprite
  if (a == 0) {
    Serial.println("clockSprite creation failed.  Cannot continue.");
    while (1);
  }
  Serial.printf("createclockSprite dispWidth x dispHeight returned: %p\r\n", a);

  // Display What after putting up header
  displayW_Header(displayLine3 + 42, "Start WiFi");
  startWifi();

  // Display What after putting up header
  displayW_Header(displayLine3 + 42, "Fetch NTP Time");
  sntp_set_sync_interval(86400000);  // 1 day in ms.
  sntp_set_time_sync_notification_cb(timeSyncCallback);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", Zulu, 1); tzset();
  displayW_Header(displayLine3 + 42, "Waiting for correct time");
  Serial.println("Waiting for correct time...");

  strftime(charWork, sizeof(charWork), "%Y", localtime(&now));
  iYear = atoi(charWork);
  int iLooper = 0;
  while (iYear < 2023) {
    time(&UTC);
    delay(1000);
    strftime (charWork, 100, "%Y", localtime(&UTC));
    iYear = atoi(charWork);
    Serial.println(localtime(&UTC), "UTC %a %m-%d-%Y %T");
    if (iLooper++ > 20) {
      Serial.println("Cannot get time set. Rebooting.");
      ESP.restart();
    }
  }
  displayW_Header(displayLine3 + 42, "Determining zone offsets");
  Serial.println("\r\nDetermining zone offsets.");
  fetchOffsets();
  time(&UTC);
  now = UTC + BotOffset;  // Assuming, for now, that the bottom clock is local time.
  strftime (charWork, 100, "%H", localtime(&now));  // Get current (24) hour
  prevHour = atoi(charWork);
  Serial.printf("Setting display brightness for hour %i to %i\r\n",
                prevHour, hourlyBrilliance[prevHour]);
  ledcWrite(pwmLedChannelTFT, hourlyBrilliance[prevHour]);  // Set display brightness for local hour.
  pinMode(changeClockShowPin, INPUT);
  pinMode(changeACShowingPin, INPUT);

  lastHeapSize = esp_get_free_heap_size();
  firstHeapSize = lastHeapSize;
  Serial.printf("Starting Heap Size: %i\r\n",  lastHeapSize);

  // The begin() method opens a “storage space” with a defined namespace.
  // The false argument means that we’ll use it in read/write mode.
  // Use true to open or create the namespace in read-only mode.
  // Name the "folder" we will use and set for read/write.
  preferences.begin("TripleTime", false);
  whichClock = preferences.getInt("defaultShow", digitalClock);
  acCustom   = preferences.getInt("defaultAC", 2);  // Default of 2 shows all on analog clock face.
  preferences.end();
  Serial.printf("Preferences for whichClock is %i, for acCustom is %i\r\n",
                whichClock, acCustom);
  decodeAC_Bits();
  delay(1000);
  tft.fillScreen(TFT_BLACK);
  //  Hello(); delay(2000);  // Show sign on message with version number.
}
/***************************************************************************/
void loop()
/***************************************************************************/
{
  if (digitalRead(changeClockShowPin) == LOW) {   // Pressed?
    delay(50);
    if (digitalRead(changeClockShowPin) == LOW)   // Still pressed?
      whichClock = !whichClock;                   // Do the button action.
    // Wait for unpress.
    while (digitalRead(changeClockShowPin) == LOW); // Wait for unpress
    // Use true to open or create the namespace in read-only mode.
    // Name the "folder" we will use and set for read/write.
    Serial.printf("Default clock now %i\r\n", whichClock);
    if (whichClock == digitalClock) {
      tft.fillScreen(TFT_BLACK);
      ofr.setFontColor(TFT_WHITE);
      updateDigitalDisplay();
    } else {
      tft.fillScreen(TFT_BLACK);
      updateAnalogClock(currHour, currMinute, currSecond, now);
    }
  }
  if (whichClock == analogClock) {  // Only change it if it is showing!
    if (digitalRead(changeACShowingPin) == LOW) {  // Pressed?
      delay(50);
      if (digitalRead(changeACShowingPin) == LOW)     // Still pressed?
        acCustom++;
      // 2 & 3 are essentially identical.  Same with 6 and 7.  So, skipping them.
      /*
        //showshorttime, showlongtime, showsecond hand are the 3 columns.
        //2 & 3 and 6 & 7 are the same because of the way decodeAC_Bits is written.
        0 F F F
        1 T F F
        2 F T F  // Same as 3
        3 F T F
        4 F F T
        5 T F T
        6 F T T  // Same as 7
        7 F T T
      */
      if (acCustom == 2) acCustom = 3;  // Do the button action (with fixup)
      if (acCustom > 6) acCustom  = 0;  // Do the button action (with fixup)
      // Wait for unpress.
      while (digitalRead(changeACShowingPin) == LOW); // Wait for unpress
      Serial.printf("acCustom now set to %i\r\n", acCustom);
      decodeAC_Bits();
    }
  }
  if (whichClock == digitalClock) scrollIt(OKscrollUpdate);
  time(&UTC);
  now = UTC + BotOffset;  // Assuming, for now, that the bottom clock is local time.
  strftime (charWork, 100, "%H", localtime(&now));  // Get current hour
  if (prevHour != atoi(charWork)) {
    prevHour = atoi(charWork);
    Serial.println("\r\nDetermining zone offsets for new hour.");
    fetchOffsets();
    Serial.printf("Setting display brightness for hour %i to %i\r\n",
                  prevHour, hourlyBrilliance[prevHour]);
    ledcWrite(pwmLedChannelTFT, hourlyBrilliance[prevHour]);
  }
  now = UTC + BotOffset;  // Assuming, for now, that the bottom clock is local time.
  strftime (charWork, 100, "%S", localtime(&now));
  currSecond = atoi(charWork);
  if (whichClock == digitalClock) scrollIt(OKscrollUpdate);

  //  if (prevSecond == currSecond) return;  // Can't do this old and trusted method since... scrolling.
  prevSecond = currSecond;
  if (lastHeapSize > esp_get_free_heap_size()) {
    lastHeapSize = esp_get_free_heap_size();
    Serial.print(localtime(&now), "%a %m-%d-%Y %T %Z - ");
    Serial.printf("Orig Heap %i, now %i\r\n", firstHeapSize, lastHeapSize);
    if (lastHeapSize < 50000) ESP.restart();
  }
  // Check if default clock to show has been changed and save it, if so.
  if (currMinute % 10 == 0 && currSecond == 0) {
    //    Serial.printf("currMinute %i, currSecond %i\r\n", currMinute, currSecond);
    preferences.begin("TripleTime", false);
    if (whichClock != preferences.getInt("defaultShow", digitalClock)) {
      Serial.print(localtime(&now), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Saving new clock face choice %i\r\n", whichClock);
      preferences.putInt("defaultShow", whichClock);
    }
    if (acCustom != preferences.getInt("defaultAC", 2)) {
      Serial.printf("acCustom %i, pref setting %i\r\n", acCustom, preferences.getInt("defaultAC", 2));
      Serial.print(localtime(&now), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Saving analog show choice %i\r\n", acCustom);
      preferences.putInt("defaultAC", acCustom);
    }
    preferences.end();
  }

  strftime (charWork, 100, "%M", localtime(&now));
  currMinute = atoi(charWork);
  strftime (charWork, 100, "%I", localtime(&now));
  currHour = atoi(charWork);

  // Here is where all of the work is done.
  if (whichClock == digitalClock) updateDigitalDisplay();
  else updateAnalogClock(currHour, currMinute, currSecond, now);

  if (currSecond == 0) {
    if (currMinute == 0) {  // The second is 0. See if the minute is 0 also.
      //      Serial.printf("currSecond %i, currMinute %i\r\n", currSecond, currMinute);
      // This takes 1.6 seconds to complete.  That guarantees that the new second will be upon us.
      for (i = 0; i < 4; i++) {
        tft.invertDisplay(true); delay(200);
        tft.invertDisplay(false); delay(200);
      }
      //      Serial.printf("It is ");
      Serial.print(localtime(&UTC), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Orig Heap %i, now %i\r\n", firstHeapSize, lastHeapSize);
    }
  }
}

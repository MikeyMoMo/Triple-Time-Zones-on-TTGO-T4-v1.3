
// Original author: (must be left in any copied or modified code)
//  Mike Morrow (GitHub MikeyMoMo) 5/16/2023

#define myName "Triple sNTP Time "  //  Version for T4 v1.3
#define myVersion "v.18.16"         //  My version number.
String OTAhostname = "ESP32 Triple Time";  // For OTA identification.

#include <SPI.h>
#include <TFT_eSPI.h>
#include "Adafruit_GFX.h"
#include "esp_sntp.h"
#include <ArduinoJson.h>
#include "TimeLib.h"
#include <ArduinoOTA.h>

#include "Preferences.h"
Preferences preferences;

#include "OpenFontRender.h"
OpenFontRender ofr;

// >>>>>>======== Here begins the customization changes.========<<<<<<

//  >>> *** The lengths must all be equal to the longest one *** <<<
//  This is because of a well known but ignored error in a library.
const char * Zulu  = "UTC0                  ";  // DO NOT CHANGE THIS ONE!!!!!
const char * TopTZ = "PST8PDT,M3.2.0,M11.1.0";  // Top    city time zone
const char * BotTZ = "PHT-8                 ";  // Bottom city time zone (normally local time)
//  >>> *** *** *** *** *** *** *** *** *** *** *** *** *** ***  <<<
const char * TopCityname = "Benicia";  // Top    city name
const char * BotCityname = "Bangui";   // Bottom city name

// WiFi credentials.  Change to yours for SNTP connect and epoch fetch.
//const char * ssid      = "LallaveWifi";
//const char * wifipw    = "Lallave@Family7";
const char* myPortalName = "TTZPortal";  // Connect to 192.168.1.4 to set SSID/PW

#define changeClockShowPin 38  // Change which of the 2 clocks to show
#define changeACShowingPin 37  // Change how the analog display looks

// >>>>>>>>>>============  End of user changes  ============<<<<<<<<<<

#include "WiFiManager.h"    // https://github.com/tzapu/WiFiManager
WiFiManager wifiManager;

int useAPIkey = 0;
unsigned int acCustom;  // default is to show all, fetched, later from preferences.
// If the exchange rate is available (> 0.) it will be shown if either digitial time is shown.
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
bool newRate = false;  // New rate fetched.  Maybe need to resize scrollSprite
unsigned int showSecondHandBit = 2;  // bit shift amount for this option
int  iYear, TopOffset, BotOffset, tempOffset;
int  prevHour = -1, currMinute = -1, prevSecond = -1, i;
int  currHour, currSecond = -1, currDay, prevDay = -1;
long int firstHeapSize = 0, lastHeapSize;

time_t workTime, UTC;
struct tm * timeinfo;

char charWork[200];   // For various conversions from numbers to char for use in strcat.
char charWork2[100];  // Used when charWork alread has stuff in it.
//Oops, we did not include time.h and NTPClient.h.  Whatever will we do?!?!?!
//#include <time.h>       /* time_t, struct tm, time, localtime */

char TopDST[10];
char BotDST[10];

#include <WiFi.h>
#include <HTTPClient.h>

const int  maxAPIkeys = 4;
const char * apiArray[] = {  // Obtain one or more free keys from apilayer.com
  "api key 1",
  "api key 2",
  "api key 3",
  "api key 4",
};

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
int hourlyBrilliance[] = { 70,  60,  50,  40,  30,  30,   //  0- 5
                           //6   7    8    9   10   11
                           50,  60,  70,  80,  80, 100,   //  6-11
                           //12  13   14   15   16   17
                           160, 160, 160, 160, 160, 160,  // 12-17
                           //18   19    20    21    22  23
                           160,  160,  160,  160,  120, 80   // 18-23
                         };
const int xCenter = TFT_WIDTH  / 2;
const int yCenter = TFT_HEIGHT / 2;

// Analog Clock face definitions.
const  char *numbers[12] = {"6", "5", "4", "3", "2", "1", "12", "11", "10", "9", "8", "7"};
int    myX1, myY1, myX2, myY2, backX, backY;
int    plus90BulgeX, plus90BulgeY;
int    minus90BulgeX, minus90BulgeY;
int    startMillis;
double angle;
float  PHP_Rate = 0.;
long unsigned lastXRateFetchTime = 0;
char   readingTime[50];  // For current exchange rate reading.
int    radius;
bool   digitalClock = 0;
bool   analogClock = 1;
bool   whichClock = digitalClock;  // Later, save it for restarts.

//const int displayLine1 =  42;  // When the sprite is at the top
//const int displayLine2 =  50;
//const int displayLine3 =  90;
//const int displayLine4 = 140;
//const int displayLine5 = 180;
const int displayLine1 =   0;  // subtracting 42 for when the scrollSprite is in use.
const int displayLine2 =   8;  // was 50
const int displayLine3 =  48;  // was 90
const int displayLine4 =  98;  // was 140
const int displayLine5 = 138;  // was 180

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

  a = (int*)clockSprite.createSprite(tft.width(), tft.height());  // Leave room for 1st sprite
  if (a == 0) {
    Serial.println("clockSprite creation failed.  Cannot continue.");
    while (1);
  }
  Serial.printf("createclockSprite dispWidth x dispHeight returned: %p\r\n", a);

  scrollSpriteW = tft.width() * 2.6;  // A little too big to leave a little slack space till 2nd copy.
  scrollSpriteH = 42;
  a = (int*)scrollSprite.createSprite(scrollSpriteW, scrollSpriteH); // 1st sprite for scrolling info
  if (a == 0) {
    Serial.println("scrollSprite creation failed.  Cannot continue.");
    while (1);
  }
  scrollSprite.fillSprite(DarkBlue);
  scrollSprite.setTextColor(TFT_WHITE, DarkBlue);

  // Display What after putting up header
  displayW_Header(displayLine3 + 42, "Start WiFi");
//  startWifi();
  startWiFiManager();

  displayW_Header(displayLine3 + 42, "Start OTA");
  initOTA();

  // Display What after putting up header
  displayW_Header(displayLine3 + 42, "Fetch NTP Time");
  sntp_set_sync_interval(86400000);  // 1 day in ms.
  sntp_set_time_sync_notification_cb(timeSyncCallback);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", Zulu, 1); tzset();
  displayW_Header(displayLine3 + 42, "Waiting for correct time");
  Serial.println("Waiting for correct time...");

  strftime(charWork, sizeof(charWork), "%Y", localtime(&workTime));
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
  deduceOffsets();
  time(&UTC);
  workTime = UTC + BotOffset;  // Assuming, for now, that the bottom clock is local time.
  strftime (charWork, 100, "%H", localtime(&workTime));  // Get current (24) hour
  prevHour = atoi(charWork);
  Serial.printf("Setting display brightness for hour %02i to %i\r\n",
                prevHour, hourlyBrilliance[prevHour]);
  ledcWrite(pwmLedChannelTFT, hourlyBrilliance[prevHour]);  // Set display brightness for local hour.
  pinMode(changeClockShowPin, INPUT);
  pinMode(changeACShowingPin, INPUT);

  time(&UTC);  // Get UTC epoch number.+
  workTime = UTC + BotOffset;
  timeinfo = localtime(&workTime);  // Load up the timeinfo struct for immediate use.
  currMinute = timeinfo->tm_min;
  currHour   = timeinfo->tm_hour;
  currDay    = timeinfo->tm_yday;
  currSecond = timeinfo->tm_sec;

  getXchangeRate();

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
  if (whichClock == digitalClock)
    tft.fillRect(0, 0, tft.width(), scrollSpriteH, DarkBlue);
}
/***************************************************************************/
void loop()
/***************************************************************************/
{
  ArduinoOTA.handle();
  time(&UTC);
  workTime = UTC + BotOffset;
  // The old way,
  //  strftime (charWork, sizeof(charWork), "%M", localtime(&workTime)); currMinute = atoi(charWork);
  //  strftime (charWork, sizeof(charWork), "%I", localtime(&workTime)); currHour   = atoi(charWork);
  //  strftime (charWork, sizeof(charWork), "%j", localtime(&workTime)); currDay    = atoi(charWork);
  //  strftime (charWork, sizeof(charWork), "%S", localtime(&workTime)); currSecond = atoi(charWork);

  // Let's not be concerned with atoi this way.
  timeinfo = localtime(&workTime);  // Load up the timeinfo struct for immediate use.
  currMinute = timeinfo->tm_min;
  currHour   = timeinfo->tm_hour;
  currDay    = timeinfo->tm_yday;
  currSecond = timeinfo->tm_sec;

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
      // So there won't be a black band on top during the first few seconds.
      tft.fillRect(0, 0, tft.width(), scrollSpriteH, DarkBlue);
      updateDigitalDisplay();  // Do the dirty!
    } else {
      tft.fillScreen(TFT_BLACK);  // Out with the old...
      updateAnalogClock(currHour, currMinute, currSecond, workTime);  // In with the new!
    }
  }
  // I needed to avoid taking the second button input if the screen it affects is not showing so
  //  I check that analogClock is showing before I even check the button.
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

  if (whichClock == digitalClock) scrollIt();  // One more shot at a smooth scroll on top.
  time(&UTC);
  workTime = UTC + BotOffset;  // Assuming, for now, that the bottom clock is local time.
  strftime (charWork, sizeof(charWork), "%H", localtime(&workTime));  // Get current hour

  if (prevHour != atoi(charWork)) {
    prevHour = atoi(charWork);
    Serial.println("\r\nDetermining zone offsets for new hour.");
    if (whichClock == digitalClock) scrollIt();  // One more shot at a smooth scroll on top.
    deduceOffsets();
    if (whichClock == digitalClock) scrollIt();  // One more shot at a smooth scroll on top.
    Serial.printf("Setting display brightness for hour %i to %i\r\n",
                  prevHour, hourlyBrilliance[prevHour]);
    ledcWrite(pwmLedChannelTFT, hourlyBrilliance[prevHour]);
  }

  workTime = UTC + BotOffset;  // Assuming, for now, that the bottom clock is local time.

  //  strftime (charWork, sizeof(charWork), "%M", localtime(&workTime));  // Get current minute
  //  currMinute = atoi(charWork);  // Initialize prev min

  if (whichClock == digitalClock) scrollIt();

  if (lastHeapSize > esp_get_free_heap_size()) {
    lastHeapSize = esp_get_free_heap_size();
    Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
    Serial.printf("Orig Heap %i, now %i\r\n", firstHeapSize, lastHeapSize);
    if (lastHeapSize < 50000) ESP.restart();
  }
  // Check if default clock to show has been changed and save it, if so.
  if (currMinute % 10 == 0 && currSecond == 0) {
    //    Serial.printf("currMinute %i, currSecond %i\r\n", currMinute, currSecond);
    preferences.begin("TripleTime", false);
    if (whichClock != preferences.getInt("defaultShow", digitalClock)) {
      Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Saving new clock face choice %i\r\n", whichClock);
      preferences.putInt("defaultShow", whichClock);
    }
    if (acCustom != preferences.getInt("defaultAC", 2)) {
      Serial.printf("acCustom %i, pref setting %i\r\n", acCustom, preferences.getInt("defaultAC", 2));
      Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Saving analog show choice %i\r\n", acCustom);
      preferences.putInt("defaultAC", acCustom);
    }
    preferences.end();
  }

  // Here is where all of the work is done.
  if (whichClock == digitalClock) updateDigitalDisplay();
  else updateAnalogClock(currHour, currMinute, currSecond, workTime);

  if (currSecond == 0) {
    if (currMinute == 0) {  // The second is 0. See if the minute is 0 also.
      // This takes 1.6 seconds to complete.  That guarantees that the new second will be upon us.
      for (i = 0; i < 4; i++) {
        tft.invertDisplay(true); delay(200);
        tft.invertDisplay(false); delay(200);
      }
      Serial.print(localtime(&UTC), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Orig Heap %i, now %i\r\n", firstHeapSize, lastHeapSize);
    }
  }
  
  if (currMinute % 10 == 0 && currSecond == 0) getXchangeRate();
  if (prevDay != currDay) useAPIkey = 0;  // May be end of month so reset it.
  prevSecond = currSecond; currMinute = currMinute; prevDay = currDay;
}

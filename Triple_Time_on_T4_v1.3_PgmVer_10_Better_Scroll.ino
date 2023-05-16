// Original author: (must be left in any copied or modified code)
//  Mike Morrow (GitHub MikeyMoMo) 5/16/2023

#define myVersion "v.10.00"  // Version for T4 v1.3

#include <SPI.h>
#include <TFT_eSPI.h>
#include "Adafruit_GFX.h"
#include "Preferences.h"
Preferences preferences;
#include "OpenFontRender.h"
OpenFontRender ofr;

//  >>> *** The lengths must all be equal to the longest one *** <<<
const char * Zulu  = "UTC0                  ";
const char * TopTZ = "PST8PDT,M3.2.0,M11.1.0";
const char * BotTZ = "PHT-8                 ";
//  >>> *** *** *** *** *** *** *** *** *** *** *** *** *** ***  <<<

const char * TopCityname = "Benicia";
const char * BotCityname = "Bangui";

// WiFi credentials.  Change to yours for NTP connect and time download.
const char * ssid      = "LallaveWifi";
const char * wifipw    = "Lallave@Family7";

time_t now, UTC;
struct tm timeinfo;
int iYear, TopOffset, BotOffset, tempOffset;

int  prevHour = -1, prevSecond = -1, i;
long int firstHeapSize = 0, lastHeapSize;
char charWork[100];  // For various conversions from text to integer.
// Oops, we did not include time.h and NTPClient.h.  Whatever will we do?!?!?!

// >>>>>>>>>>>=============  End of changes  =============<<<<<<<<<<<

char TopDST[10];
char BotDST[10];

// Only update the TZ hourly to get the current offset and DST setting.
// That's really more than necessary but not such a big deal.
// I was doing it every second.
// It totally saves me from having to figure out DST for myself.
// I did it once and it was NOT pretty!

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
#define switchPin 38
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

  scrollSpriteW = tft.width() * 3.4;
  scrollSpriteH = 42;
  a = (int*)scrollSprite.createSprite(scrollSpriteW, scrollSpriteH); // 1st sprite for scrolling info
  if (a == 0) {
    Serial.println("scrollSprite creation failed.  Cannot continue.");
    while (1);
  }

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
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", Zulu, 1); tzset();
  displayW_Header(displayLine3 + 42, "Waiting for correct time");
  Serial.println("Waiting for correct time...");

  strftime (charWork, 100, "%Y", localtime(&now));
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
  pinMode(switchPin, INPUT);

  lastHeapSize = esp_get_free_heap_size();
  firstHeapSize = lastHeapSize;
  Serial.printf("Starting Heap Size: %i\r\n",  lastHeapSize);

  // The begin() method opens a “storage space” with a defined namespace.
  // The false argument means that we’ll use it in read/write mode.
  // Use true to open or create the namespace in read-only mode.
  // Name the "folder" we will use and set for read/write.
  preferences.begin("TripleTime", false);
  whichClock = preferences.getInt("defaultShow", digitalClock);
  preferences.end();
  delay(1000);
  tft.fillScreen(TFT_BLACK);
  //  Hello(); delay(2000);  // Show sign on message with version number.
}
/***************************************************************************/
void loop()
/***************************************************************************/
{
  if (digitalRead(switchPin) == LOW) {  // Pressed?
    delay(50);
    if (digitalRead(switchPin) == LOW)   // Still pressed?
      whichClock = !whichClock;
    while (digitalRead(switchPin) == LOW); // Wait for unpress
    // Use true to open or create the namespace in read-only mode.
    // Name the "folder" we will use and set for read/write.
    Serial.printf("Default clock now %i\r\n", whichClock);
    if (whichClock == digitalClock) updateDigitalDisplay();
    else updateAnalogClock(currHour, currMinute, currSecond, now);
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
  strftime (charWork, 100, "%M", localtime(&now));
  currMinute = atoi(charWork);
  strftime (charWork, 100, "%I", localtime(&now));
  currHour = atoi(charWork);

  // Check if default clock to show has been changed and save it, if so.
  if (currMinute % 10 == 0 && currSecond == 0) {
    preferences.begin("TripleTime", false);
    if (whichClock != preferences.getInt("defaultShow", digitalClock)) {
      Serial.printf("Saving new clock face choice %i\r\n", whichClock);
      preferences.begin("TripleTime", false);
      preferences.putInt("defaultShow", whichClock);
      preferences.end();
    }
  }
  if (whichClock == digitalClock) updateDigitalDisplay();
  else updateAnalogClock(currHour, currMinute, currSecond, now);

  if (currSecond == 0) {
    if (currMinute == 0) {  // The second is 0. See if the minute is 0 also.
      // Serial.printf("currSecond %i, currMinute %i\r\n", currSecond, currMinute);
      // This takes 1.6 seconds to complete.  That guarantees that the new second will be upon us.
      for (i = 0; i < 4; i++) {
        tft.invertDisplay(true); delay(200);
        tft.invertDisplay(false); delay(200);
      }
      Serial.printf("It is ");
      Serial.print(localtime(&UTC), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Orig Heap %i, now %i\r\n", firstHeapSize, lastHeapSize);
    }
  }
}

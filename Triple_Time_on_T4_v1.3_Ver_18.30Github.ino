#define CONFIG4MIKE
// Original author: (must be left in any copied or modified code)
//  Mike Morrow (GitHub MikeyMoMo) 5/16/2023

// This is written for a 240/320 display.  I am using a TTGO T4 v1.3 but
//  others will work.  Just put in the correct TFT_eSPI config file.

// If defined, do a smoother movement of the second hand.
//  If not defined, jump the second hand one seconds at a time.
//  It probably does not matter but the minute hand is "sweep" always.
#define DO_SWEEP_HANDS  

// If defined, show the modern wedge hands (slower but looks nice).
#define WEDGE_HANDS  

#define myName "Triple sNTP Time "         //  Version for T4 v1.3
#define myVersion "v.18.30"                //  My version number.
String OTAhostname = "ESP32 Triple Time";  // For OTA identification/upload.

#include <SPI.h>           // Hardware management of the display
#include <TFT_eSPI.h>      // Software management of the display
#include "Adafruit_GFX.h"  // Stuff from Adafruit.  Buy from then if you can.
#include "esp_sntp.h"      // Get UTC epoch here.
#include <ArduinoJson.h>   // For unscrambling the returned XRate packet.
#include "TimeLib.h"       // Nice utilities for time manipulation/display.
#include <ArduinoOTA.h>    // Yes, OTA is here.

#include "Preferences.h"   // Remember stuff that's worth remembering.
Preferences preferences;

// SLOW but nice TTF font renderer in multiple sizes.  When I found the
//  convenience of it, I switched the entire program over to it but it
//  took too much time and I had to switch some time-sensitive parts back
//  to drawString.  Wish I could use it everywhere.  Still a little buggy
//  but getting better with time.
#include "OpenFontRender.h"  
OpenFontRender ofr;

// >>>>>>======== Here begins the customization changes.========<<<<<<

#define ulResetXRateTime 64800  // 3/4 day.  After that, X data is stale. Clear.
//  >>> *** The lengths must all be equal to the longest one *** <<<
//  This is because of a well known but ignored error in a library that
//   causes a memory leak if these are different sizes.  FIX IT!!!
const char * cZulu  = "UTC0                  ";  // DO NOT CHANGE THIS ONE!!!
const char * cTopTZ = "PST8PDT,M3.2.0,M11.1.0";  // Top    city time zone
const char * cBotTZ = "PHT-8                 ";  // Bottom city time zone (normally local time)
//  >>> *** *** *** *** *** *** *** *** *** *** *** *** *** ***  <<<
const char * cTopCity name = "Benicia";  // Top    city name
const char * cBotCityname = "Bangui";   // Bottom city name

// There are 3 WiFi startup routines herewith.  Choose the one you like
//  and configure it to connect to what you use.  There is plenty of
//  documentation, online, for all 3 connect options.
// WiFi credentials.  Change to yours for sNTP connect and epoch fetch.
//const char * ssid      = "LallaveWifi";
//const char * wifipw    = "Lallave@Family7";
const char * ssid      = "MikeysWAP";
const char * wifipw    = "Noogly99";

// The following is for use in StartWiFiManager to name the captive portal.
const char* cMyPortalName = "TTZPortal";  // Connect to 192.168.1.4 to set SSID/PW

#define changeClockShowPin 38  // Change which of the 2 clocks to show
#define changeACShowingPin 37  // Change how the analog display looks
#define changeACbackground 39  // Change the background color of the Analog Clock

// >>>>>>>>>>============  End of user changes  ============<<<<<<<<<<

//#include "WiFiManager.h"    // https://github.com/tzapu/WiFiManager
//WiFiManager wifiManager;
#include "WiFiMulti.h"    // https://github.com/tzapu/WiFiManager
WiFiMulti wifiMulti;
int iUseAPIkey = 0;
unsigned int uiAC_Custom;  // default is to show all, fetched, later from preferences.
// If the exchange rate is available (> 0.) it will be shown if either digitial time is shown.
// This is interpreted as a bit-coded integer.
// uiAC_Custom settings:
//  0 bits on -- do not show second hand, do not show digital time
//  1 bit on  -- show short digital time if the 2 bit is off
bool bShowShortTime;
unsigned int bShowShortTimeBit = 0;  // bit shift amount for this option
//  2 bit on  -- show long digital time, even if 1 bit is on
bool bShowLongTime;
unsigned int bShowLongTimeBit = 1;  // bit shift amount for this option
//  4 bit on  -- show second hand
bool bShowSecondHand;
bool bNewRate = false;  // New rate fetched.  Maybe need to resize scrollSprite
unsigned int uiShowSecondHandBit = 2;  // bit shift amount for this option
unsigned long secStartMillis;  // millis() when this second started.
int  iYear, iTopOffset, iBotOffset, iTempOffset;
int  iPrevHour = -1, iCurrMinute = -1, iPrevSecond = -1, i;
int  iCurrHour, iCurrSecond = -1, iCurrDay, iPrevDay = -1;
long int liFirstHeapSize = 0, lLastHeapSize;

time_t workTime, UTC;
struct tm * timeinfo;

char cCharWork[200];   // For various conversions from numbers to char for use in strcat.
char cCharWork2[100];  // Used when cCharWork alread has stuff in it.

// Standard time or daylight savings time indicator for zones of interest.
char cTopDST[10];  
char cBotDST[10];

#include <WiFi.h>
#include <HTTPClient.h>

// Trying to setup the failover for overuse for both of us.
const int iMaxAPIkeys = 4;  // 2 are recommended.  3 is overkill.

const char * cAPI_Array[] = {
// These come from apilayer.com.  250 fetchs/month are free.
//  You really only need 2.  This code is made to be shared between
//  two people with #define's to say which keys and configuration settings
//  to use. So, there are 4.  2 for each of us.  You only need 1 but 2 are
//  good to have in case of testing or problems.
#if defined CONFIG4MIKE
  "api key 1",  // Mike 1
  "api key 2",  // Mike 2
  "api key 3",  // Joe 1
  "api key 4",  // Joe 2
#else
  "api key 4",  // Joe 2
  "api key 3",  // Joe 1
  "api key 2",  // Mike 2
  "api key 1",  // Mike 1
#endif
};

// Setup file for ESP32 and TTGO T4 v1.3 SPI bus TFT
// Define TFT_eSPI object with the size of the screen:
//  240 pixels width and 320 pixels height.

//Use this: #include <User_Setups/Setup22_TTGO_T4_v1.3.h>
TFT_eSPI tft = TFT_eSPI();

// Sprites are used to eliminate flickering.
TFT_eSprite clockSprite = TFT_eSprite(&tft);
TFT_eSprite scrollSprite = TFT_eSprite(&tft);
int iScrollSpriteW, iScrollSpriteH;

// Setting PWM properties, do not change these 3 lines, please, unless
//  you really know what you are doing!
const int iPWM_Freq = 5000;
const int iPWM_Resolution = 8;
const int iPWM_LedChannelTFT = 4;

// Startup TFT backlight intensity on a scale of 0 to 255.
const int ledBacklightFull = 255;

#define RGB565(r,g,b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))
#define RGB888(r,g,b) ((r << 16) | (g << 8) | b)
#define DarkerRed RGB565(150,0,0)
//#define SecondHand RGB565(200,128,128)
#define DarkPurple RGB565(118,0,100)
#define DarkBlue RGB565(0, 0, 80)
bool bChangeACBG = false;
#if defined CONFIG4MIKE
// Time-controlled display brightness.
//                           0    1    2    3    4    5            Hours
int ihourlyBrilliance[] = { 70,  60,  50,  40,  30,  30,      //  0- 5
                            //6   7    8    9   10   11
                            50,  60,  70,  80,  80, 100,      //  6-11
                            //12  13   14   15   16   17
                            160, 160, 160, 160, 160, 160,     // 12-17
                            //18   19    20    21    22  23
                            160,  160,  160,  160,  120, 80   // 18-23
#else
//                          0    1    2    3    4    5            Hours
int ihourlyBrilliance[] = { 70,  60,  50,  40,  30,  30,      //  0- 5
                            //6   7    8    9   10   11
                            50,  60,  70,  80,  80, 100,      //  6-11
                            //12  13   14   15   16   17
                            160, 160, 160, 160, 160, 160,     // 12-17
                            //18   19    20    21    22  23
                            160,  160,  160,  160,  120, 80   // 18-23
#endif
                          };

// Analog Clock face definitions.
bool   bDigitalClock = 0;
bool   bAnalogClock = 1;
bool   bWhichClock = bDigitalClock;  // Later, save it for restarts.

// Don't ask!
const  char *numbers[12] = {"6", "5", "4", "3", "2", "1", "12", "11", "10", "9", "8", "7"};

int iXCenter, iYCenter;

const  int iDisplayLine1 =   0;  // subtracting 42 for when the scrollSprite is in use.
const  int iDisplayLine2 =   8;  // was 50
const  int iDisplayLine3 =  48;  // was 90
const  int iDisplayLine4 =  98;  // was 140
const  int iDisplayLine5 = 138;  // was 180

int    iMyX1, iMyY1, iMyX2, iMyY2, iBackX, iBackY;
int    iPlus90BulgeX, iPlus90BulgeY;
int    iMinus90BulgeX, iMinus90BulgeY;
int    iStartMillis;
int    iRadius;

double dAngle;
float  fPHP_Rate = 0.;
long   unsigned luLastXRateFetchTime = 0;

char   caReadingTime[50];  // For current exchange rate reading.

/***************************************************************************/
void setup()
/***************************************************************************/
{
  Serial.begin(115200); delay(1000);

  Serial.println(F("\n\nThis is the Triple TZ on T4 v1.3."));
  String asdf = String(__FILE__);
  asdf = asdf.substring(asdf.lastIndexOf("\\") + 1, asdf.length());
  Serial.print("Running from: "); Serial.println(asdf);
  Serial.println("Compiled on: " + String(__DATE__) + " at " + String(__TIME__));

  initDisplay();

  iScrollSpriteW = tft.width() * 2;  // Will be reallocated, later.
  iScrollSpriteH = 42;

  // Display What after putting up header
  //startWiFiManager();
  //startWiFi();
  displayW_Header("Start WiFi");       StartWiFiMulti();
  displayW_Header("Start OTA");        initOTA();
  displayW_Header("Allocate Sprites"); allocateSprites();
  displayW_Header("Fetch NTP Time");   initTime();

  strftime (cCharWork, 100, "%H", localtime(&workTime));  // Get current (24) hour
  iPrevHour = atoi(cCharWork);
  Serial.printf("Setting display brightness for hour %02i to %i\r\n",
                iPrevHour, ihourlyBrilliance[iPrevHour]);
  ledcWrite(iPWM_LedChannelTFT, ihourlyBrilliance[iPrevHour]);  // Set display brightness for local hour.
  pinMode(changeClockShowPin, INPUT);
  pinMode(changeACShowingPin, INPUT);
  pinMode(changeACbackground, INPUT);

/  time(&UTC);  // Get UTC epoch number.+
#if defined CONFIG4MIKE
  workTime = UTC + iBotOffset;  // Assuming, for now, that the bottom clock is local time.
#else
  workTime = UTC + iTopOffset;  // Assuming, for now, that the bottom clock is local time.
#endif
  timeinfo = localtime(&workTime);  // Load up the timeinfo struct for immediate use.
  iCurrMinute = timeinfo->tm_min;
  iCurrHour   = timeinfo->tm_hour;
  iCurrDay    = timeinfo->tm_yday;
  iCurrSecond = timeinfo->tm_sec;

  lLastHeapSize = esp_get_free_heap_size();
  liFirstHeapSize = lLastHeapSize;
  Serial.printf("Starting Heap Size: %i\r\n",  lLastHeapSize);

  // The begin() method opens a “storage space” with a defined namespace.
  // The false argument means that we’ll use it in read/write mode.
  // Use true to open or create the namespace in read-only mode.
  // Name the "folder" we will use and set for read/write.
  preferences.begin("TripleTime", false);
  bWhichClock = preferences.getInt("defaultShow", bDigitalClock);
  // Default of 2 shows all on analog clock face.
  uiAC_Custom   = preferences.getInt("defaultAC", 2);
  preferences.end();
  Serial.printf("Preferences for bWhichClock is %i, for uiAC_Custom is %i\r\n",
                bWhichClock, uiAC_Custom);

  //  getXchangeRate();  // Prime the pump.

  decodeAC_Bits();  // See which display variation on the Analog Clock display.

  tft.fillScreen(TFT_BLACK);  // Let's go!
}
/***************************************************************************/
void loop()
/***************************************************************************/
{
  ArduinoOTA.handle();
  time(&UTC);
#if defined CONFIG4MIKE
  workTime = UTC + iBotOffset;
#else
  workTime = UTC + iTopOffset;  // Use top as local time for Joe.
#endif

  // Load up the timeinfo struct for immediate use.
  timeinfo = localtime(&workTime);
  iCurrMinute = timeinfo->tm_min;
  iCurrHour   = timeinfo->tm_hour;
  iCurrDay    = timeinfo->tm_yday;
  iCurrSecond = timeinfo->tm_sec;
  if (iPrevSecond != iCurrSecond) secStartMillis = millis();
  if ((timeinfo->tm_wday == 0) && (iCurrHour == 4) &&
      (iCurrMinute == 0) && (iCurrSecond == 0))
    ESP.restart();

  // Does the user wish to change the analog clock background color?
  if (digitalRead(changeACbackground) == LOW) {      // Pressed?
    delay(50);
    if (digitalRead(changeACbackground) == LOW)      // Still pressed?
      bChangeACBG = true;
    while (digitalRead(changeACbackground) == LOW);  // Wait for unpress
  }

  if (digitalRead(changeClockShowPin) == LOW) {   // Pressed?
    delay(50);
    if (digitalRead(changeClockShowPin) == LOW)   // Still pressed?
      bWhichClock = !bWhichClock;                   // Do the button action.
    // Wait for unpress.
    while (digitalRead(changeClockShowPin) == LOW); // Wait for unpress
    // Use true to open or create the namespace in read-only mode.
    // Name the "folder" we will use and set for read/write.
    Serial.printf("Default clock now %i\r\n", bWhichClock);
    if (bWhichClock == bDigitalClock) {
      tft.fillScreen(TFT_BLACK);
      ofr.setFontColor(TFT_WHITE);
      // So there won't be a black band on top during the first few seconds.
      tft.fillRect(0, 0, tft.width(), iScrollSpriteH, DarkBlue);
      updateDigitalDisplay();  // Do the dirty!
    } else {
      tft.fillScreen(TFT_BLACK);  // Out with the old...
      updateAnalogClock(workTime);  // In with the new!
    }
  }
  // I needed to avoid taking the second button input if the screen it affects
  //  is not showing so
  //  I check that bAnalogClock is showing before I even check the button.
  if (bWhichClock == bAnalogClock) {  // Only change it if it is showing!
    if (digitalRead(changeACShowingPin) == LOW) {  // Pressed?
      delay(50);
      if (digitalRead(changeACShowingPin) == LOW)     // Still pressed?
        uiAC_Custom++;
      // 2 & 3 are essentially identical.  Same with 6 and 7.  So, skipping them.
      /*
        // bShowShortTime, bShowLongTime, showsecond hand are the 3 columns.
        // 2 & 3 and 6 & 7 are the same because of the way
        //  decodeAC_Bits is written.
        0 F F F
        1 T F F
        2 F T F  // Same as 3
        3 F T F
        4 F F T
        5 T F T
        6 F T T  // Same as 7
        7 F T T
      */
      if (uiAC_Custom == 2) uiAC_Custom = 3;  // Skip duplicate
      if (uiAC_Custom == 7) uiAC_Custom = 0;  // Skip duplicate
      // Wait for unpress.
      while (digitalRead(changeACShowingPin) == LOW); // Wait for unpress
      Serial.printf("uiAC_Custom now set to %i\r\n", uiAC_Custom);
      decodeAC_Bits();
    }
  }

  if (bWhichClock == bDigitalClock) scrollIt();  // One more shot at a smooth scroll on top.
  time(&UTC);
#if defined CONFIG4MIKE
  workTime = UTC + iBotOffset;  // Assuming, for now, that the bottom clock is local time.
#else
  workTime = UTC + iTopOffset;  // Assuming, for now, that the bottom clock is local time.
#endif
  strftime (cCharWork, sizeof(cCharWork), "%H", localtime(&workTime));  // Get current hour

  if (iPrevHour != atoi(cCharWork)) {
    iPrevHour = atoi(cCharWork);
    Serial.println("\r\nDetermining zone offsets for new hour.");
    if (bWhichClock == bDigitalClock) scrollIt();  // One more shot at a smooth scroll on top.
    deduceOffsets();
    if (bWhichClock == bDigitalClock) scrollIt();  // One more shot at a smooth scroll on top.
    Serial.printf("Setting display brightness for hour %i to %i\r\n",
                  iPrevHour, ihourlyBrilliance[iPrevHour]);
    if (bWhichClock == bDigitalClock) scrollIt();  // One more shot at a smooth scroll on top.
    ledcWrite(iPWM_LedChannelTFT, ihourlyBrilliance[iPrevHour]);
  }

  if (bWhichClock == bDigitalClock) scrollIt();  // One more shot at a smooth scroll on top.
#if defined CONFIG4MIKE
  workTime = UTC + iBotOffset;  // Assuming, for now, that the bottom clock is local time.
#else
  workTime = UTC + iTopOffset;  // Assuming, for now, that the bottom clock is local time.
#endif

  //  strftime (cCharWork, sizeof(cCharWork), "%M", localtime(&workTime));  // Get current minute
  //  iCurrMinute = atoi(cCharWork);  // Initialize prev min

  if (bWhichClock == bDigitalClock) scrollIt();

  if (lLastHeapSize > esp_get_free_heap_size()) {
    lLastHeapSize = esp_get_free_heap_size();
    if (bWhichClock == bDigitalClock) scrollIt();  // One more shot at a smooth scroll on top.
    Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
    if (bWhichClock == bDigitalClock) scrollIt();  // One more shot at a smooth scroll on top.
    Serial.printf("Orig Heap %i, now %i\r\n", liFirstHeapSize, lLastHeapSize);
    if (lLastHeapSize < 50000)
      ESP.restart();
    else if (bWhichClock == bDigitalClock) scrollIt(); // One more shot at a smooth scroll on top.
  }
  // Check if default clock to show has been changed and save it, if so.
  if (iCurrMinute % 10 == 0 && iCurrSecond == 0) {
    //    Serial.printf("iCurrMinute %i, iCurrSecond %i\r\n", iCurrMinute, iCurrSecond);
    preferences.begin("TripleTime", false);
    if (bWhichClock != preferences.getInt("defaultShow", bDigitalClock)) {
      Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Saving new clock face choice %i\r\n", bWhichClock);
      preferences.putInt("defaultShow", bWhichClock);
    }
    if (bWhichClock == bDigitalClock) scrollIt();  // One more shot at a smooth scroll on top.
    if (uiAC_Custom != preferences.getInt("defaultAC", 2)) {
      Serial.printf("uiAC_Custom %i, pref setting %i\r\n", uiAC_Custom, preferences.getInt("defaultAC", 2));
      Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Saving analog show choice %i\r\n", uiAC_Custom);
      preferences.putInt("defaultAC", uiAC_Custom);
    }
    preferences.end();
  }

  // Here is where all of the work is done.
  if (bWhichClock == bDigitalClock)
    updateDigitalDisplay();
  else
    updateAnalogClock(workTime);

  if (iCurrSecond == 0) {
    if (iCurrMinute == 0) {  // The second is 0. See if the minute is 0 also.
      // This takes 1.6 seconds to complete.
      //  That guarantees that the new second will be upon us.
      for (i = 0; i < 4; i++) {
        tft.invertDisplay(true); delay(200);
        tft.invertDisplay(false); delay(200);
      }
      Serial.print(localtime(&UTC), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Orig Heap %i, now %i\r\n",
                    liFirstHeapSize, lLastHeapSize);
    }
  }
  if (iCurrMinute % 10 == 0 && iCurrSecond == 0) getXchangeRate();
  if (iPrevDay != iCurrDay) iUseAPIkey = 0;  // May be end of month so reset it.
  iPrevSecond = iCurrSecond; iCurrMinute = iCurrMinute; iPrevDay = iCurrDay;
}

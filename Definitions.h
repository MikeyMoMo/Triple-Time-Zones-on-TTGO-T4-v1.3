// Setup file for ESP32 and TTGO T4 v1.3 SPI bus TFT
// Define TFT_eSPI object with the size of the screen:
//  240 pixels width and 320 pixels height.

//Use this: #include <User_Setups/Setup22_TTGO_T4_v1.3.h>
// Others will work.  Just put in the correct TFT_eSPI config file.

// If defined, do a smoother movement of the second hand.
//  If not defined, jump the second hand one seconds at a time.
//  It probably does not matter but the minute hand is "sweep" always.
#define DO_SWEEP_HANDS
// If defined, will put color under the three graphs.  Otherwise, they
//  will only be line graphs instead of area graphs.
#define FILL_GRAPH
// If defined, show the modern wedge hands (slower but looks nice).
#define WEDGE_HANDS

#define DROP_DATE_EVERY 50  // How many days to skip on the graph before showing date.

// Note: The color lines for the date will always be drawn.  This controls the rest
//       of the color lines up to the spot.
#define COLOR_FILL_GRAPH    // If defined, draw colored lines up to the dot.

#define myName "Triple sNTP Time "         // Version for T4 v1.3
String OTAhostname = "ESP32 Triple Time";  // For OTA identification/upload.

#include <SPI.h>           // Hardware management of the display
#include <TFT_eSPI.h>      // Software management of the display
#include "Adafruit_GFX.h"  // Stuff from Adafruit.  Buy from then if you can.
#include "esp_sntp.h"      // Get UTC epoch here.
#include <ArduinoJson.h>   // For unscrambling the returned XRate packet.
#include "TimeLib.h"       // Nice utilities for time manipulation/display.
#include <ArduinoOTA.h>    // Yes, OTA is here.
#include "Free_Fonts.h"    // Include the header file attached to this sketch
//#include "Time_Zones.h"

#include "Preferences.h"   // Remember stuff that's worth remembering.
Preferences preferences;
#define RO_MODE true   // Preferences calls Read-only mode
#define RW_MODE false  // Preferences calls Read-write mode

// SLOW but nice TTF font renderer in multiple sizes.  When I found the
//  convenience of it, I switched the entire program over to it but it
//  took too much time and I had to switch some time-sensitive parts back
//  to drawString.  Wish I could use it everywhere.  Still a little buggy
//  but getting better with time.
#include "OpenFontRender.h"
OpenFontRender ofr;

// >>>>>>======== Here begins the customization changes.========<<<<<<

// 3/4 day.  After that, X data is stale. Clear.
#define ulResetXRateTime 64800
#define iXRateFetchCheckInterval 10  // In minutes.  Used in modulo.
// It is silly to fetch all night long while sleeping.  Just pause it
//  for a while.  It will stop with the hour shown at night and start
//  with the hour shown in the morning.  Right now, you can't stop after
//  midnight.  I have the code for that somewhere.  Will find it soon.
//unsigned long ulLastXRateFetchEpoch; // Epoch time of last fetch.

// To NOT pause XRate fething and continue getting XRate every 3 hours,
//  search for: "if (!doFetch) {" if the void getXchangeRate() routine 
//  and comment out the entire little routine or
//  add "doFetch = true;" just above it.
// Or, you could possibly set start to 24 and stop to 24. Not tested!
#define fetchStartHour 10  // Will start to fetch at 10:00 
#define fetchStopHour  23  // Will not fetch starting at 23:00

int XRateMon, XRateDay, XRateYr;

//  >>> *** The lengths must all be equal to the longest one *** <<<
//  This is because of a well known but ignored error in a library that
//   causes a memory leak if these are different sizes.  FIX IT!!!
const char * cZulu  = "UTC0                  ";  // DO NOT CHANGE THIS ONE!!!
const char * cTopTZ = "PST8PDT,M3.2.0,M11.1.0";  // Top    city time zone
// Bottom city time zone (normally local time)
const char * cBotTZ = "PHT-8                 ";  // Bottom city time zone
//  >>> *** *** *** *** *** *** *** *** *** *** *** *** *** ***  <<<
const char * cTopCityname = "Benicia";  // Top    city name
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
// Connect to 192.168.1.4 to set SSID/PW
const char* cMyPortalName = "TTZPortal";  

#define changeClockShowPin 38  // Change which of the 2 clocks to show
#define changeACShowingPin 37  // Change how the analog display looks
#define changeACbackground 39  // Change the background color of the 
                               //  Analog Clock

// >>>>>>>>>>============  End of user changes  ============<<<<<<<<<<

//#include "WiFiManager.h"    // https://github.com/tzapu/WiFiManager
//WiFiManager wifiManager;
#include "WiFiMulti.h"    // https://github.com/tzapu/WiFiManager
WiFiMulti wifiMulti;
int iUseAPIkey = 0;
// default is to show all, fetched, later from preferences.
unsigned int uiAC_Custom;
// If the exchange rate is available (> 0.) it will be shown if either
//  digitial time is shown.  This is interpreted as a bit-coded integer.
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
int  iYear, iMonth, iDay, iTopOffset, iBotOffset, iTempOffset;
int  iPrevHour = -1, iCurrMinute = -1, iPrevSecond = -1, i;
int  iCurrHour, iCurrSecond = -1, iCurrDay, iCurrDOW, iPrevDay = -1;
//long int liFirstHeapSize = 0, lLastHeapSize;
int txtLen;
int maxTxtLen1 = 0, maxTxtLen2 = 0, maxTxtLen3 = 0;
int iSavDatum;
time_t workTime, UTC, XR_UTC;
struct tm * timeinfo;

// For various conversions from numbers to char for use in strcat.
char cCharWork[200];
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
  // The code fetches every 3 hours.  That's 8 times a day or 248 times per
  //  month.  That's why a backup is good in case there is some problem and
  //  you go over on your first key.  It will automatically switch to the
  //  backup key when it detects that the primary key is exhausted.
#if defined CONFIG4MIKE
  "",  // LaoagMikey api key
  "",  // BanguiMikey api key (Mike Staples)
  "",  // Joe 1 api key
  "",  // Joe 2 api key
#else
  "",  // Joe 2 api key
  "",  // Joe 1 api key
  "",  // BanguiMikey api key (Mike Staples)
  "",  // LaoagMikey api key
#endif
};

TFT_eSPI tft = TFT_eSPI();

// Sprites are used to eliminate flickering.
TFT_eSprite clockSprite   = TFT_eSprite(&tft);
TFT_eSprite scrollSprite  = TFT_eSprite(&tft);
TFT_eSprite digitalSprite = TFT_eSprite(&tft);
TFT_eSprite XRateSprite   = TFT_eSprite(&tft);

int iScrollSpriteW, iScrollSpriteH;

// Setting PWM properties, do not change these 3 lines, please, unless
//  you really know what you are doing!
const int iPWM_Freq = 5000;
const int iPWM_Resolution = 8;
//const int iPWM_LedChannelTFT = 4;

// Startup TFT backlight intensity on a scale of 0 to 255.
const int ledBacklightFull = 255;

#define RGB565(r,g,b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))
#define RGB888(r,g,b) ((r << 16) | (g << 8) | b)
#define DarkerRed RGB565(150,0,0)
#define DarkerGreen RGB565(0, 80, 0);
//#define SecondHand RGB565(200,128,128)
#define DarkPurple RGB565(118,0,100)
#define DarkBlue RGB565(0, 0, 80)
bool bChangeACBG = false;
#if defined CONFIG4MIKE
// Time-controlled display brightness.
//                           0    1    2    3    4    5            Hours
int ihourlyBrilliance[] = { 60,  40,  40,  30,  30,  30,      //  0- 5
                            //6   7    8    9   10   11
                            50,  70,  90, 100, 140, 150,      //  6-11
                            //12  13   14   15   16   17
                            160, 160, 160, 160, 160, 160,     // 12-17
                            //18  19   20   21   22  23
                            140, 120, 100,  90,  80, 70   // 18-23
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
int   iDigitalClock = 0;
int   iAnalogClock = 1;
int   iXGraph = 2;
int   iMaxShow = 3;  // Until the graphing is ready.
int   iWhichClock = iDigitalClock;  // Later, save it for restarts.
char* clockNames[] = {"Multi-time Digital Clock",
                      "Analog Clock",
                      "Exchange Rate Graph"
                     };
// Don't ask!
const  char *numbers[12] = {"6", "5", "4", "3", "2", "1",
                            "12", "11", "10", "9", "8", "7"
                           };

int iXCenter, iYCenter;

const  int iDisplayLine1 =   0;
const  int iDisplayLine2 =   8;
const  int iDisplayLine3 =  48;
const  int iDisplayLine4 =  98;
const  int iDisplayLine5 = 138;

int    iMyX1, iMyY1, iMyX2, iMyY2, iBackX, iBackY;
int    iPlus90BulgeX, iPlus90BulgeY;
int    iMinus90BulgeX, iMinus90BulgeY;
int    iRadius;

double dAngle;
float  fPHP_Rate = 0.;
long   unsigned ulLastXRateFetchEpoch = 0;  // Epoch time of last fetch.
long   unsigned iStartMillis;

char   caReadingTime[50];  // For current exchange rate reading.
int    iTemp;
char   cTemp;
String sTemp, sVer, sFetchesLeft;

// Funny thing.  I started keeping 365 of these.  It would take some fancy
//  code to get it down to 288 that I actually want to keep.  So I keep 365
//  but only graph the last 288 (or less until it is full).  Some day, I might
//  fix this.  Who knows... (Don't take bets that I will!)
#define      XRateHistLen 365  // How many historical XRates to keep.
float        XRateHist[XRateHistLen];   // One XRate per day.
unsigned int XRateJulian[XRateHistLen];  // Julian Date based on 1/1/4713 BCE.
int totXRatesaved = 0;  // How many daily XRates are in the array, now.
float pixelsPerHundredthV;
const int graphFloorMargin = 20;   // X axis here.
const int graphLeftMargin  = 40;   // Y axis here from the left.
// When the graph screen is selected, I only calculate and show it once.  
//  But if XRate fetch runs, it steals the screen so this bool has to be 
//  set to tell the graph routine that it needs to refresh the screen.  
//  It is set other places, too, to remind the graph production routine 
//  to refresh the screen.  After the graph is shown once, it is deactivated 
//  by setting this bool to false.  It won't  show again until someone else 
//  tells him to.  We are a cooperative lot!
bool refreshGraph = true;  // Show the XRate history graph one time.

// SSDays (Short Short Day) is used for the calendar column headers only.
String SSDays[7] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};
//String LDays[7] = {"Sunday", "Monday", "TueSDays", "WedneSDays", "ThurSDays",
//                  "Friday", "Saturday"
//                 };
String SDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
                  };
String Months[12] = {"January", "February", "March", "April", "May", "June",
                     "July", "August", "September", "October", "November",
                     "December"
                    };
String SMonths[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
                     };

#define RGB565(r,g,b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))
#define DarkerGreen RGB565(0,100,0)

//TL_DATUM 0 // Top left (default)
//TC_DATUM 1 // Upper center
//TR_DATUM 2 // Top right
//ML_DATUM 3 // Left midfielder
//CL_DATUM 3 // Center left, as above
//MC_DATUM 4 // Central center
//CC_DATUM 4 // Center to center, as above
//MR_DATUM 5 // Right midfielder
//CR_DATUM 5 // Center right, as above
//BL_DATUM 6 // Bottom left
//BC_DATUM 7 // Bottom center
//BR_DATUM 8 // Bottom right
// (line on which the character ‘A’ would sit)
//L_BASELINE 9 // Base line of the left character 
//C_BASELINE 10 // Base line of the central character
//R_BASELINE 11 // Base line of the right character

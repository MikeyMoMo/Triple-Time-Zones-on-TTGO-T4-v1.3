#define CONFIG4MIKE
// Original author: (must be left in any copied or modified code)
//  Mike Morrow (GitHub: MikeyMoMo) 5/16/2024

#include "Definitions.h"
/***************************************************************************/
void setup()
/***************************************************************************/
{
  Serial.begin(115200); delay(1000);

  Serial.println(F("\n\nThis is the Triple TZ on T4 v1.3."));
  printVers();
  //  String asdf = String(__FILE__);
  //  asdf = asdf.substring(asdf.lastIndexOf("\\") + 1, asdf.length());
  //  Serial.print("Running from: "); Serial.println(asdf);
  //  Serial.println("Compiled on: " + String(__DATE__) + " at " +
  //                 String(__TIME__));

  // Get the number of entries in the Time Zones table.  In case on-the-fly
  //  zone changing via buttons is ever implemented.

  //  Serial.printf("Table size:\t%i\r\n", sizeof(ENVName));
  //  Serial.printf("Element size:\t%i\r\n", sizeof(*ENVName));
  //  int t = sizeof(ENVName) / sizeof(*ENVName);
  //  Serial.printf("Table elements: %i\r\n", t);
  //  for (int i = 0; i < t; i++) Serial.println(ENVName[i]);

  initDisplay();

  iScrollSpriteW = tft.width() * 2;  // Will be reallocated, later.
  iScrollSpriteH = 42;

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //disable brownout detector

  //displayW_Header("startWiFiManager"); startWiFiManager();
  //displayW_Header("startWiFi");        startWiFi();
  displayW_Header("Start WiFi");         StartWiFiMulti();
  displayW_Header("Start OTA");          initOTA();
  displayW_Header("Allocate Sprites");   allocateSprites();
  displayW_Header("Fetch NTP Time");     initTime();

  // Get current (24) hour
  strftime (cCharWork, 100, "%H", localtime(&workTime));
  iPrevHour = atoi(cCharWork);
  Serial.printf("Setting display brightness for hour %02i to %i\r\n",
                iPrevHour, ihourlyBrilliance[iPrevHour]);
  // Set display brightness for local hour.
  ledcWrite(iPWM_LedChannelTFT, ihourlyBrilliance[iPrevHour]);
  pinMode(changeClockShowPin, INPUT);
  pinMode(changeACShowingPin, INPUT);
  pinMode(changeACbackground, INPUT);

  time(&UTC);  // Get UTC epoch number.
#if defined CONFIG4MIKE
  // Assuming, for now, that the bottom clock is local time.
  workTime = UTC + iBotOffset;
#else
  // Assuming, for now, that the bottom clock is local time.
  workTime = UTC + iTopOffset;
#endif
  // Load up the timeinfo struct for immediate use.
  timeinfo = localtime(&workTime);
  iCurrMinute = timeinfo->tm_min;
  iCurrHour   = timeinfo->tm_hour;
  iCurrDay    = timeinfo->tm_yday;
  iCurrSecond = timeinfo->tm_sec;

  //  lLastHeapSize = esp_get_free_heap_size();
  //  liFirstHeapSize = lLastHeapSize;
  //  Serial.printf("Starting Heap Size: %i\r\n",  lLastHeapSize);
  Serial.printf("Starting Heap Size: %i\r\n", esp_get_free_heap_size());

  /* Testing */

  for (i = 0; i < XRateHistLen; i++) {
    XRateHist[i] = 0.; XRateJulian[i] = 0;
  }

  // Install initial XRate History conditions (zeros)
  //  preferences.begin("TripleTime", RW_MODE);
  //  preferences.putBytes("XRateHist", XRateHist, sizeof(XRateHist));
  //  preferences.putBytes("XRateEpock", XRateJulian, sizeof(XRateJulian));
  //
  //  When through testing
  //  preferences.remove("XRateHist");
  //  preferences.remove("XRateEpock");
  //
  //  preferences.end();

  Serial.printf("XRateHist array is %i bytes, Epoch array is %i bytes\r\n",
                sizeof(XRateHist), sizeof(XRateJulian));

  /* End Testing */

  // The begin() method opens a “storage space” with a defined namespace.
  // The false argument means that we’ll use it in read/write mode.
  // Use true to open or create the namespace in read-only mode.
  // Name the "folder" we will use and set for read/write.
  preferences.begin("TripleTime", RO_MODE);
  iWhichClock = preferences.getInt("defaultShow", iDigitalClock);
  // Default of 2 shows all on analog clock face.
  uiAC_Custom   = preferences.getInt("defaultAC", 2);
  if (preferences.isKey("XRateHist")) {
    Serial.println("Reading XRate History table.");
    preferences.getBytes("XRateHist", XRateHist, sizeof(XRateHist));
    preferences.getBytes("XRateEpock", XRateJulian, sizeof(XRateJulian));
  }
  Serial.printf("Preferences for iWhichClock is %i, "
                "for uiAC_Custom is %i\r\n", iWhichClock, uiAC_Custom);
  Serial.printf("There are %i entries left in TripleTime preferences storage.\r\n",
                preferences.freeEntries());
  preferences.end();

  /* Testing */

  Serial.println("Current non-zero XRate history table entries.");
  for (i = 0; i < XRateHistLen; i++) {
    if (XRateHist[i] != 0.) {
      JulianToGregorian(XRateJulian[i]);  // Sets XRateMon, XRateDay & XRateYr.
      Serial.printf("XRate for element %i value %.3f, Julian %u, %02i/%02i/%02i\r\n",
                    i, XRateHist[i], XRateJulian[i], XRateMon, XRateDay, XRateYr);
    }
  }

  /* End Testing */

  //  getXchangeRate();  // Prime the pump. 0

  decodeAC_Bits();  // Which display variation for the Analog Clock display.

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
    if (digitalRead(changeClockShowPin) == LOW) {  // Still pressed?
      iWhichClock++;  // Change to next screen.
      if (iWhichClock == iMaxShow) iWhichClock = 0; // Reset when more are added
    }
    // Wait for unpress.
    while (digitalRead(changeClockShowPin) == LOW); // Wait for unpress
    // Use true to open or create the namespace in read-only mode.
    // Name the "folder" we will use and set for read/write.
    Serial.printf("Default clock now %i\r\n", iWhichClock);
    if (iWhichClock == iDigitalClock) {
      tft.fillScreen(TFT_BLACK);
      ofr.setFontColor(TFT_WHITE);
      // So there won't be a black band on top during the first few seconds.
      tft.fillRect(0, 0, tft.width(), iScrollSpriteH, DarkBlue);
      updateDigitalDisplay();  // Do the dirty!
    } else if (iWhichClock ==  iAnalogClock) {
      tft.fillScreen(TFT_BLACK);  // Out with the old...
      updateAnalogClock(workTime);  // In with the new!
    } else if (iWhichClock == iXGraph) {
      tft.fillScreen(TFT_BLACK);  // Out with the old...
      showXGraph();
    }
  }
  // I needed to avoid taking the second button input if the screen it
  //  affects is not showing so
  //  I check that bAnalogClock is showing before I even check the button.
  if (iWhichClock == iAnalogClock) {  // Only change it if it is showing!
    if (digitalRead(changeACShowingPin) == LOW) {  // Pressed?
      delay(50);
      if (digitalRead(changeACShowingPin) == LOW)     // Still pressed?
        uiAC_Custom++;
      // 2 & 3 are essentially identical.  Also 6 and 7.  So, skipping them.
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

  // One more shot at a smooth scroll on top.
  if (iWhichClock == iDigitalClock) scrollIt();
  time(&UTC);
#if defined CONFIG4MIKE
  // Assuming, for now, that the bottom clock is local time.
  workTime = UTC + iBotOffset;
#else
  // Assuming, for now, that the bottom clock is local time.
  workTime = UTC + iTopOffset;
#endif
  // Get current hour
  strftime (cCharWork, sizeof(cCharWork), "%H", localtime(&workTime));

  if (iPrevHour != atoi(cCharWork)) {
    iPrevHour = atoi(cCharWork);
    Serial.println("\r\nDetermining zone offsets for new hour.");
    // One more shot at a smooth scroll on top.
    if (iWhichClock == iDigitalClock) scrollIt();
    deduceOffsets();
    // One more shot at a smooth scroll on top.
    if (iWhichClock == iDigitalClock) scrollIt();
    Serial.printf("Setting display brightness for hour %02i to %i\r\n",
                  iPrevHour, ihourlyBrilliance[iPrevHour]);
    // One more shot at a smooth scroll on top.
    if (iWhichClock == iDigitalClock) scrollIt();
    ledcWrite(iPWM_LedChannelTFT, ihourlyBrilliance[iPrevHour]);
  }

  // One more shot at a smooth scroll on top.
  if (iWhichClock == iDigitalClock) scrollIt();
#if defined CONFIG4MIKE
  // Assuming, for now, that the bottom clock is local time.
  workTime = UTC + iBotOffset;
#else
  // Assuming, for now, that the bottom clock is local time.
  workTime = UTC + iTopOffset;
#endif

  // Get current minute
  //  strftime (cCharWork, sizeof(cCharWork), "%M", localtime(&workTime));
  //  iCurrMinute = atoi(cCharWork);  // Initialize prev min

  if (iWhichClock == iDigitalClock) scrollIt();

  //  if (lLastHeapSize > esp_get_free_heap_size()) {
  //    lLastHeapSize = esp_get_free_heap_size();
  //    // One more shot at a smooth scroll on top.
  //    if (iWhichClock == iDigitalClock) scrollIt();
  //    Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
  //    // One more shot at a smooth scroll on top.
  //    if (iWhichClock == iDigitalClock) scrollIt();
  //    //    Serial.printf("Orig Heap %i, now %i\r\n", liFirstHeapSize,
  //            lLastHeapSize);
  //    if (lLastHeapSize < 50000)
  //      ESP.restart();
  //    // One more shot at a smooth scroll on top.
  //    else if (iWhichClock == iDigitalClock) scrollIt();
  //  }

  // Another shot at a smooth scroll on top.
  if (iWhichClock == iDigitalClock) scrollIt();

  // Check if default clock to show has been changed and save it, if so.
  if (iCurrMinute % 10 == 0 && iCurrSecond == 0) {
    //    Serial.printf("iCurrMinute %i, iCurrSecond %i\r\n",
    //                  iCurrMinute, iCurrSecond);
    preferences.begin("TripleTime", RW_MODE);
    if (iWhichClock != preferences.getInt("defaultShow", iDigitalClock)) {
      Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Saving new clock face choice %i\r\n", iWhichClock);
      preferences.putInt("defaultShow", iWhichClock);
    }
    // Another shot at a smooth scroll on top.
    if (iWhichClock == iDigitalClock) scrollIt();
    if (uiAC_Custom != preferences.getInt("defaultAC", 2)) {
      Serial.printf("uiAC_Custom %i, pref setting %i\r\n",
                    uiAC_Custom, preferences.getInt("defaultAC", 2));
      Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Saving analog show choice %i\r\n", uiAC_Custom);
      preferences.putInt("defaultAC", uiAC_Custom);
    }
    preferences.end();
  }

  // Here is where all of the work is done.
  if (iWhichClock == iDigitalClock)
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
      // Serial.print(localtime(&UTC), "%a %m-%d-%Y %T %Z\r\n");
      // Serial.printf("Orig Heap %i, now %i\r\n",
      //               liFirstHeapSize, lLastHeapSize);
    }
  }
  // Avoid congestion  // Here is the first check 10 or 20 minute check delay
  if ((iCurrMinute - 2) % iXRateFetchCheckInterval == 0 && iCurrSecond == 12)
    getXchangeRate();
  // If new day, may be end of month so reset it.
  if (iPrevDay != iCurrDay) iUseAPIkey = 0;
  iPrevSecond = iCurrSecond; iCurrMinute = iCurrMinute; iPrevDay = iCurrDay;
}

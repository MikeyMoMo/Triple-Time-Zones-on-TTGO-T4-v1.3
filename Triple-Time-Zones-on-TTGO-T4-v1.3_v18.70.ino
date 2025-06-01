#define CONFIG4MIKE
// Original author: (must be left in any copied or modified code)
//  Mike Morrow (GitHub: MikeyMoMo) 5/16/2024

// This is written for a display size of 240x320 in landscape orientation.

#include "Definitions.h"
/***************************************************************************/
void setup()
/***************************************************************************/
{
  Serial.begin(115200); delay(2000);

  Serial.println(F("\n\nThis is the Triple TZ on T4 v1.3."));
  printVers();
  // Get the number of entries in the Time Zones table.  In case on-the-fly
  //  zone changing via buttons is ever implemented.

  //  Serial.printf("Table size:\t%i\r\n", sizeof(ENVName));
  //  Serial.printf("Element size:\t%i\r\n", sizeof(*ENVName));
  //  int t = sizeof(ENVName) / sizeof(*ENVName);
  //  Serial.printf("Time ENV Table elements: %i\r\n", t);
  //  for (int i = 0; i < t; i++) Serial.println(ENVName[i]);

  initDisplay();

  iScrollSpriteW = tft.width() * 2;  // Will be reallocated, later.
  iScrollSpriteH = 42;

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //disable brownout detector

  //displayW_Header("startWiFiManager"); startWiFiManager();
  //displayW_Header("startWiFi");        startWiFi();
  displayW_Header("Start WiFiMulti");    startWiFiMulti();
  displayW_Header("Start OTA");          initOTA();
  displayW_Header("Allocate Sprites");   allocateSprites();
  displayW_Header("Fetch NTP Time");     initTime();

  // Get current (24) hour
  strftime (cCharWork, 100, "%H", localtime(&workTime));
  iPrevHour = atoi(cCharWork);
  Serial.printf("Setting display brightness for hour %02i to %i\r\n",
                iPrevHour, ihourlyBrilliance[iPrevHour]);
  // Set display brightness for local hour.
  //  ledcWrite(iPWM_LedChannelTFT, ihourlyBrilliance[iPrevHour]);
  ledcWrite(TFT_BL, ihourlyBrilliance[iPrevHour]);
  pinMode(changeClockShowPin, INPUT_PULLUP);
  pinMode(changeACShowingPin, INPUT_PULLUP);
  pinMode(changeACbackground, INPUT_PULLUP);

  time(&UTC);  // Get UTC epoch number.
#if defined CONFIG4MIKE
  // Assuming, for now, that the bottom clock is local time for Mike.
  workTime = UTC + iBotOffset;
#else
  // Assuming, for now, that the top clock is local time for Joe.
  workTime = UTC + iTopOffset;
#endif
  // Load up the timeinfo struct for immediate use.
  timeinfo = localtime(&workTime);
  iCurrMinute = timeinfo->tm_min;
  iCurrHour   = timeinfo->tm_hour;
  iCurrDay    = timeinfo->tm_yday;
  iCurrDOW    = timeinfo->tm_wday;
  iCurrSecond = timeinfo->tm_sec;

  //  lLastHeapSize = esp_get_free_heap_size();
  //  liFirstHeapSize = lLastHeapSize;
  //  Serial.printf("Starting Heap Size: %i\r\n",  lLastHeapSize);
  Serial.printf("Starting Heap Size: %i\r\n", esp_get_free_heap_size());

  // The begin() method opens a “storage space” with a defined namespace.
  // The false argument means that we’ll use it in read/write mode.
  // Use true to open or create the namespace in read-only mode.
  // Name the "folder" we will use and set for read/write.
  preferences.begin("TripleTime", RO_MODE);

  // Get the two values for XRate continuance across reboot.
  fPHP_Rate = preferences.getFloat("XRateValue", 0);
  ulLastXRateFetchEpoch = preferences.getUInt("XRateEpoch", 0);
  iWhichClock = preferences.getInt("defaultShow", iDigitalClock);
  // Default of 2 shows all on analog clock face.
  uiAC_Custom   = preferences.getInt("defaultAC", 2);
  if (preferences.isKey("XRateHist")) {
    Serial.println("Reading XRate History table.");
    preferences.getBytes("XRateHist", XRateHist, sizeof(XRateHist));
    preferences.getBytes("XRateJulian", XRateJulian, sizeof(XRateJulian));
  }
  /* Temporary Fixup */
  //  for (i = 0; i < XRateHistLen; i++) {
  //    XRateHist[i] = -1.; XRateJulian[i] = 0;
  //  }
  //  // Serial.printf("Element 4: %f, %i\r\n", XRateHist[4], XRateJulian[4]);
  //
  //  //  XRateHist[0] = -1.; XRateJulian[0] = 0;
  //  //  XRateHist[1] = -1.; XRateJulian[1] = 0;
  //  //  XRateHist[2] = -1.; XRateJulian[2] = 0;
  //  //  XRateHist[3] = -1.; XRateJulian[3] = 0;
  //  //  XRateHist[4] = -1.; XRateJulian[4] = 0;
  //  //  XRateHist[5] = -1.; XRateJulian[5] = 0;
  //  //  XRateHist[6] = -1.; XRateJulian[6] = 0;
  //
  //  XRateHist[251] = 58.87;  XRateJulian[251] = 2460658;
  //  XRateHist[252] = 58.87;  XRateJulian[252] = 2460659;
  //  XRateHist[253] = 58.87;  XRateJulian[253] = 2460660;
  //  XRateHist[254] = 58.556; XRateJulian[254] = 2460661;
  //  XRateHist[255] = 58.492; XRateJulian[255] = 2460662;
  //  XRateHist[256] = 58.660; XRateJulian[256] = 2460663;
  //  XRateHist[257] = 58.00;  XRateJulian[257] = 2460664;
  //  XRateHist[258] = 57.918; XRateJulian[258] = 2460665;
  //  XRateHist[259] = 58.87;  XRateJulian[259] = 2460666;
  //  XRateHist[260] = 58.87;  XRateJulian[260] = 2460667;
  //  XRateHist[261] = 58.87;  XRateJulian[261] = 2460668;
  //  XRateHist[262] = 58.556; XRateJulian[262] = 2460669;
  //  XRateHist[263] = 58.492; XRateJulian[263] = 2460670;
  //  XRateHist[264] = 58.66;  XRateJulian[264] = 2460671;
  //  XRateHist[265] = 58.;    XRateJulian[265] = 2460672;
  //  XRateHist[266] = 57.918; XRateJulian[266] = 2460673;
  //  XRateHist[267] = 57.918; XRateJulian[267] = 2460674;
  //  XRateHist[268] = 57.918; XRateJulian[268] = 2460675;
  //  XRateHist[269] = 57.859; XRateJulian[269] = 2460676;
  //  XRateHist[270] = 58.077; XRateJulian[270] = 2460677;
  //  XRateHist[271] = 58.196; XRateJulian[271] = 2460678;
  //  XRateHist[272] = 58.021; XRateJulian[272] = 2460679;
  //  XRateHist[273] = 58.202; XRateJulian[273] = 2460680;
  //  XRateHist[274] = 58.208; XRateJulian[274] = 2460681;
  //  XRateHist[275] = 58.208; XRateJulian[275] = 2460682;
  //  XRateHist[276] = 58.059; XRateJulian[276] = 2460683;
  //  XRateHist[277] = 58.15;  XRateJulian[277] = 2460684;
  //  XRateHist[278] = 58.547; XRateJulian[278] = 2460685;
  //  XRateHist[279] = 58.491; XRateJulian[279] = 2460686;
  //  XRateHist[280] = 58.68;  XRateJulian[280] = 2460687;
  //  XRateHist[281] = 59.009; XRateJulian[281] = 2460688;
  //  XRateHist[282] = 59.009; XRateJulian[282] = 2460689;
  //  XRateHist[283] = 58.66;  XRateJulian[283] = 2460690;
  //  XRateHist[284] = 58.728; XRateJulian[284] = 2460691;
  //  XRateHist[285] = 58.473; XRateJulian[285] = 2460692;
  //  XRateHist[286] = 58.603; XRateJulian[286] = 2460693;
  //  XRateHist[287] = 58.404; XRateJulian[287] = 2460694;
  //  XRateHist[288] = 58.545; XRateJulian[288] = 2460695;
  //  XRateHist[289] = 58.443; XRateJulian[289] = 2460697;
  //  XRateHist[290] = 58.353; XRateJulian[290] = 2460698;
  //  XRateHist[291] = 58.563; XRateJulian[291] = 2460699;
  //  XRateHist[292] = 58.646; XRateJulian[292] = 2460700;
  //  XRateHist[293] = 58.306; XRateJulian[293] = 2460701;
  //  XRateHist[294] = 58.25;  XRateJulian[294] = 2460702;
  //  XRateHist[295] = 58.25;  XRateJulian[295] = 2460703;
  //  XRateHist[296] = 58.425; XRateJulian[296] = 2460705;
  //  XRateHist[297] = 58.445; XRateJulian[297] = 2460706;
  //  XRateHist[298] = 58.333; XRateJulian[298] = 2460707;
  //  XRateHist[299] = 58.431; XRateJulian[299] = 2460708;
  //  XRateHist[300] = 58.421; XRateJulian[300] = 2460709;
  //  XRateHist[301] = 58.421; XRateJulian[301] = 2460710;
  //  XRateHist[302] = 58.494; XRateJulian[302] = 2460711;
  //  XRateHist[303] = 58.015; XRateJulian[303] = 2460712;
  //  XRateHist[304] = 57.878; XRateJulian[304] = 2460713;
  //  XRateHist[305] = 58.06;  XRateJulian[305] = 2460714;
  //  XRateHist[306] = 58.129; XRateJulian[306] = 2460715;
  //  XRateHist[307] = 58.06;  XRateJulian[307] = 2460716;
  //  XRateHist[308] = 58.06;  XRateJulian[308] = 2460717;
  //  XRateHist[309] = 58.12;  XRateJulian[309] = 2460718;
  //  XRateHist[310] = 58.148; XRateJulian[310] = 2460719;
  //  XRateHist[311] = 58.199; XRateJulian[311] = 2460720;
  //  XRateHist[312] = 57.871; XRateJulian[312] = 2460721;
  //  XRateHist[313] = 57.793; XRateJulian[313] = 2460722;
  //  XRateHist[314] = 57.695; XRateJulian[314] = 2460723;
  //  XRateHist[315] = 57.695; XRateJulian[315] = 2460724;
  //  XRateHist[316] = 57.737; XRateJulian[316] = 2460725;
  //  XRateHist[317] = 58.025; XRateJulian[317] = 2460726;
  //  XRateHist[318] = 58.169; XRateJulian[318] = 2460727;
  //  XRateHist[319] = 58.127; XRateJulian[319] = 2460728;
  //  XRateHist[320] = 57.877; XRateJulian[320] = 2460729;
  //  XRateHist[321] = 57.881; XRateJulian[321] = 2460730;
  //  XRateHist[322] = 57.877; XRateJulian[322] = 2460731;
  //  XRateHist[323] = 57.875; XRateJulian[323] = 2460732;
  //  XRateHist[324] = 57.853; XRateJulian[324] = 2460733;
  //  XRateHist[325] = 57.899; XRateJulian[325] = 2460734;
  //  XRateHist[326] = 57.885; XRateJulian[326] = 2460735;
  //  XRateHist[327] = 57.94;  XRateJulian[327] = 2460736;
  //  XRateHist[328] = 57.94;  XRateJulian[328] = 2460737;
  //  XRateHist[329] = 57.965; XRateJulian[329] = 2460738;
  //  XRateHist[330] = 57.806; XRateJulian[330] = 2460739;
  //  XRateHist[331] = 57.684; XRateJulian[331] = 2460740;
  //  XRateHist[332] = 57.288; XRateJulian[332] = 2460741;
  //  XRateHist[333] = 57.187; XRateJulian[333] = 2460742;
  //  XRateHist[334] = 57.332; XRateJulian[334] = 2460743;
  //  XRateHist[335] = 57.205; XRateJulian[335] = 2460744;
  //  XRateHist[336] = 57.205; XRateJulian[336] = 2460745;
  //  XRateHist[337] = 57.469; XRateJulian[337] = 2460746;
  //  XRateHist[338] = 57.457; XRateJulian[338] = 2460747;
  //  XRateHist[339] = 57.39;  XRateJulian[339] = 2460748;
  //  XRateHist[340] = 57.415; XRateJulian[340] = 2460749;
  //  XRateHist[341] = 57.359; XRateJulian[341] = 2460750;
  //  XRateHist[342] = 57.229; XRateJulian[342] = 2460751;
  //  XRateHist[343] = 57.269; XRateJulian[343] = 2460752;
  //  XRateHist[344] = 57.26;  XRateJulian[344] = 2460753;
  //  XRateHist[345] = 57.229; XRateJulian[345] = 2460754;
  //  XRateHist[346] = 57.174; XRateJulian[346] = 2460755;
  //  XRateHist[347] = 57.265; XRateJulian[347] = 2460756;
  //  XRateHist[348] = 57.341; XRateJulian[348] = 2460757;
  //  XRateHist[349] = 57.339; XRateJulian[349] = 2460758;
  //  XRateHist[350] = 57.335; XRateJulian[350] = 2460759;
  //  XRateHist[351] = 57.335; XRateJulian[351] = 2460760;
  //  XRateHist[352] = 57.352; XRateJulian[352] = 2460761;
  //  XRateHist[353] = 57.602; XRateJulian[353] = 2460762;
  //  XRateHist[354] = 57.757; XRateJulian[354] = 2460763;
  //  XRateHist[355] = 57.36;  XRateJulian[355] = 2460764;
  //  XRateHist[356] = 57.378; XRateJulian[356] = 2460765;
  //  XRateHist[357] = 57.279; XRateJulian[357] = 2460766;
  //  XRateHist[358] = 57.271; XRateJulian[358] = 2460767;
  //  XRateHist[359] = 57.218; XRateJulian[359] = 2460768;
  //  XRateHist[360] = 57.238; XRateJulian[360] = 2460769;
  //  XRateHist[361] = 56.907; XRateJulian[361] = 2460770;
  //  XRateHist[362] = 57.27;  XRateJulian[362] = 2460771;
  //  XRateHist[363] = 57.385; XRateJulian[363] = 2460772;
  //  XRateHist[364] = 57.385; XRateJulian[364] = 2460773;
  /* End Temporary Fixup */

  Serial.printf("Preferences for iWhichClock is %i, "
                "for uiAC_Custom is %i\r\n", iWhichClock, uiAC_Custom);
  Serial.printf("There are %i entries left in preferences storage.\r\n",
                preferences.freeEntries());
  preferences.end();

#if defined CONFIG4MIKE
  sprintf(caReadingTime, "As of %02i:%02i %s, ",
          iCurrHour, iCurrMinute, cBotDST);
#else
  sprintf(caReadingTime, "As of %02i:%02i %s, ",
          iCurrHour, iCurrMinute, cTopDST);
#endif

  countXRate(true, true);  // Print out all entries and total.

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
  iCurrDOW    = timeinfo->tm_wday;
  iCurrSecond = timeinfo->tm_sec;
  if (iPrevSecond != iCurrSecond) secStartMillis = millis();
  if ((timeinfo->tm_wday == 0) && (iCurrHour == 4) &&
      (iCurrMinute == 0) && (iCurrSecond == 0)) {
    Serial.printf("%02i:%02i:%02i Doing weekly reboot now.",
                  iCurrHour, iCurrMinute, iCurrSecond);

    // Save the last XRate quote and time before the reboot.
    preferences.begin("TripleTime", RW_MODE);
    preferences.putFloat("XRateValue", fPHP_Rate);
    preferences.putUInt("XRateEpoch", ulLastXRateFetchEpoch);
    preferences.end();

    ESP.restart();
  }

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
      // Reset iMaxShow when more are added
      if (iWhichClock == iMaxShow) iWhichClock = 0;
      Serial.printf("iWhichClock now %i (%s)\r\n",
                    iWhichClock, clockNames[iWhichClock]);
    }
    // Wait for unpress.
    while (digitalRead(changeClockShowPin) == LOW); // Wait for unpress
    Serial.printf("Default clock now %s\r\n", clockNames[iWhichClock]);
    if (iWhichClock == iDigitalClock) {
      tft.fillScreen(TFT_BLACK);
      ofr.setFontColor(TFT_WHITE);
      // So there won't be a black band on top during the first few seconds.
      tft.fillRect(0, 0, tft.width(), iScrollSpriteH, DarkBlue);
      updateDigitalDisplay();  // Do the dirty!
    } else if (iWhichClock ==  iAnalogClock) {
      tft.fillScreen(TFT_BLACK);  // Out with the old...
      Serial.println("Showing Analog Clock");
      updateAnalogClock(workTime);  // In with the new!
    } else if (iWhichClock == iXGraph) {
      Serial.println("Going to iXGraph.");
      tft.fillScreen(TFT_BLACK);  // Out with the old...
      Serial.println("Showing XRate Graph");
      showXGraph();
    }
  }
  // I needed to avoid taking the second button input if the screen it
  //  affects is not showing so...
  //  I check that bAnalogClock is showing before I check the button.
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
      if (uiAC_Custom == 6) uiAC_Custom = 7;  // Skip duplicate
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
    // For board definitions 2.x.x
    //    ledcWrite(iPWM_LedChannelTFT, ihourlyBrilliance[iPrevHour]);
    // For board definitions 3.x.x
    ledcWrite(TFT_BL, ihourlyBrilliance[iPrevHour]);
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

  // Another shot at a smooth scroll on top.
  if (iWhichClock == iDigitalClock) scrollIt();

  // Check if default clock to show has been changed and save it, if so.
  if (iCurrMinute % 10 == 0 && iCurrSecond == 0)  // Only on the 10 minute.
  {
    preferences.begin("TripleTime", RW_MODE);
    if (iWhichClock != preferences.getInt("defaultShow", iDigitalClock))
    {
      Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Saving new clock face choice %i\r\n", iWhichClock);
      preferences.putInt("defaultShow", iWhichClock);
    }
    // Another shot at a smooth scroll for the top sprite.
    if (iWhichClock == iDigitalClock) scrollIt();
    if (uiAC_Custom != preferences.getInt("defaultAC", 2)) {
      Serial.printf("uiAC_Custom %i, pref setting %i\r\n",
                    uiAC_Custom, preferences.getInt("defaultAC", 2));
      Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
      // Another shot at a smooth scroll for the top sprite.
      if (iWhichClock == iDigitalClock) scrollIt();
      Serial.printf("Saving analog show choice %i\r\n", uiAC_Custom);
      preferences.putInt("defaultAC", uiAC_Custom);
    }
    preferences.end();
  }

  // <<<********************************>>>
  // Here is where all of the work is done.
  // <<<********************************>>>
  if (iWhichClock == iDigitalClock)
  {
    refreshGraph = true;  // Reenable updating of the graph screen.
    updateDigitalDisplay();
  }
  else if (iWhichClock == iAnalogClock) {
    refreshGraph = true;  // Reenable updating of the graph screen.
    updateAnalogClock(workTime);
  }
  else if (iWhichClock == iXGraph) showXGraph();

  if (iCurrSecond == 0) {
    if (iCurrMinute == 0) {  // The second is 0. See if the minute is 0 also.
      // This takes 1.6 seconds to complete.
      //  That guarantees that the new second will be upon us.
      for (i = 0; i < 4; i++) {  // Do HourDance #1
        tft.invertDisplay(true); delay(200);
        tft.invertDisplay(false); delay(200);
      }
    }
  }
  // Avoid congestion. Here is the first check 10 or 20 minute check delay.
  if ((iCurrMinute - 2) % iXRateFetchCheckInterval == 0 && iCurrSecond == 12)
    getXchangeRate();
  // If new day, may be end of month so reset it.
  if (iPrevDay != iCurrDay) iUseAPIkey = 0;
  iPrevSecond = iCurrSecond; iCurrMinute = iCurrMinute; iPrevDay = iCurrDay;
}

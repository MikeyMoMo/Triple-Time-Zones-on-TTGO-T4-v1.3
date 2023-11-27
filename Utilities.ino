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
//L_BASELINE 9 // Base line of the left character (line on which the character ‘A’ would sit)
//C_BASELINE 10 // Base line of the central character
//R_BASELINE 11 // Base line of the right character

/***************************************************************************/
void decodeAC_Bits()
/***************************************************************************/
{
  // This unnecessarily complex.  I just felt like doing it this way to
  //  sharpen my skill at decoding bit coded stuff.  It should have just
  //  been a switch case.  Others have done worse!
  if (uiAC_Custom & 0x1 << bShowShortTimeBit) {  // See if short time requested
    bShowShortTime = true;
  } else {
    bShowShortTime = false;
  }
  if (uiAC_Custom & 0x1 << bShowLongTimeBit) {
    //    Serial.println("Turning on bShowLongTime, off bShowShortTime");
    bShowLongTime = true;
    bShowShortTime = false;  // Override short time bit
  } else {
    bShowLongTime = false;
  }
  if (uiAC_Custom & 0x1 << uiShowSecondHandBit)
    bShowSecondHand = true;
  else
    bShowSecondHand = false;
}
/***************************************************************************/
void initDisplay()
/***************************************************************************/
{
  // Note: Currently, font size 28 is used for everything except for notificaiton
  //       screens.  Size 38 is used for notifications screens and is reset after
  //       use, at routine end.
  tft.init(); // Initialize the screen.
  tft.setRotation(3);       // Power on top.  1 for power at bottom
  iXCenter = tft.width() / 2;
  iYCenter = tft.height() / 2;
  clockSprite.setTextColor(TFT_WHITE);
  clockSprite.setTextWrap(false, false);
  scrollSprite.setTextWrap(false, false);
  ofr.setDrawer(clockSprite);
  if (ofr.loadFont(BritanicBoldTTF, sizeof(BritanicBoldTTF))) {
    Serial.println("Render loadFont error for BritanicBoldTTF. InitDisplay 1");
    while (1);
  }
  ofr.setFontSize(28);
  Serial.printf("Text height for size 28 is %i\r\n", ofr.getTextHeight("AB8Myp"));

  tft.fillScreen(TFT_BLACK);
#if defined TFT_BL
  ledcSetup(iPWM_LedChannelTFT, iPWM_Freq, iPWM_Resolution);
  ledcAttachPin(TFT_BL, iPWM_LedChannelTFT); // TFT_BL, 0 - 15
  ledcWrite(iPWM_LedChannelTFT, 200);
#endif
  tft.invertDisplay(false); // Where it is true or false.  False is "normal" on this display.
  ofr.setFontColor(TFT_WHITE, DarkerRed);  // Foreground color, Background color

  tft.loadFont(TimesNewRoman32Bold);

  scrollSprite.loadFont(TimesNewRoman32Bold);
  clockSprite.loadFont(BritanicBold28);
}
/***************************************************************************/
void displayW_Header(String what)
/***************************************************************************/
{
  int txtLen;

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  txtLen = tft.textWidth("Dual NTP Time");
  tft.drawString("Dual NTP Time", iXCenter - txtLen / 2, iDisplayLine1);

  txtLen = tft.textWidth(myVersion);
  tft.drawString(myVersion, iXCenter - txtLen / 2, iDisplayLine3);

  txtLen = tft.textWidth(what);
  tft.drawString(what, iXCenter - txtLen / 2, iDisplayLine5);
  Serial.println(what);
}
/***************************************************************************/
void deduceOffsets()
/***************************************************************************/
{
  // Only update the TZ hourly to get the current offset and DST setting.
  // That's really more than necessary but not such a big deal.
  // I was doing it every second.  However, it only takes 32 ms., so... no biggy.
  // It totally saves me from having to figure out DST for myself.
  // I did it once and it was NOT pretty and only applied to the U.S.!
  // I put in the time env string and activate it then see what conditions it creates.
  //  It gives me complete time, the time type (standard or DST) and the seconds offset.
  //  SO much easier than trying to do all of this myself.  It takes care of changing the
  //  time from DST to standard time and I just use that number.  Sweet!
  //  I just have to be sure that the time is valid and I do that by being sure that the
  //  computed year is less than or equal to my birth year (2023).

  iStartMillis = millis();

  Serial.print(cTopCityname);
  setenv("TZ", cTopTZ, 1); tzset();  // Anybody but me see how silly that 1 is?
  time(&workTime);
  strftime (cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
  iYear = atoi(cCharWork);
  //  Serial.println(localtime(&workTime), " initial set %a, %d-%m-%Y %T %Z %z");
  while (iYear < 2023) {
    delay(1000);
    time(&workTime);
    strftime (cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
    iYear = atoi(cCharWork);
    Serial.print(cTopCityname);
    Serial.println(localtime(&workTime), " waiting %a, %d-%m-%Y %T %Z %z");
  }
  //  Serial.println(localtime(&workTime), " after waiting %a, %d-%m-%Y %T %Z %z");
  Serial.print(localtime(&workTime), " %a, %d-%m-%Y %T %Z %z");

  strftime (cTopDST, 10, "%Z", localtime(&workTime));
  strftime (cCharWork, sizeof(cCharWork), "%z", localtime(&workTime));
  iTempOffset = atoi(cCharWork);
  iTopOffset = (iTempOffset / 100) * 3600 + iTempOffset % 100 * 60;
  //  Serial.print(cTopCityname);
  Serial.printf(" offset = %+i\r\n", iTopOffset);

  Serial.print(cBotCityname);
  setenv("TZ", cBotTZ, 1); tzset();
  time(&workTime);
  strftime (cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
  iYear = atoi(cCharWork);
  //  Serial.println(localtime(&workTime), " initial set %a, %d-%m-%Y %T %Z %z");
  while (iYear < 2023) {
    delay(1000);
    time(&workTime);
    strftime (cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
    iYear = atoi(cCharWork);
    Serial.print(cBotCityname);
    Serial.println(localtime(&workTime), " waiting %a, %d-%m-%Y %T %Z %z");
  }
  //  Serial.println(localtime(&workTime), " after waiting %a, %d-%m-%Y %T %Z %z");
  Serial.print(localtime(&workTime), " %a, %d-%m-%Y %T %Z %z");
  strftime (cBotDST, 10, "%Z", localtime(&workTime));  // Characters of time zone and DST indicator.
  strftime (cCharWork, sizeof(cCharWork), "%z", localtime(&workTime));  // Seconds offset from UTC.
  iTempOffset = atoi(cCharWork);
  iBotOffset = (iTempOffset / 100) * 3600 + iTempOffset % 100 * 60;
  //  Serial.print(cBotCityname);
  Serial.printf(" offset = %+i\r\n", iBotOffset);

  // This must be done and must be last.  The local time(s) are based off of this.  They are
  //  not kept separately, only created when needed by adding the offset to UTC.
  Serial.print("UTC");
  setenv("TZ", cZulu, 1); tzset();
  strftime (cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
  time(&UTC);
  iYear = atoi(cCharWork);
  //  Serial.println(localtime(&UTC), "Zulu initial set %a, %d-%m-%Y %T %Z %z");
  while (iYear < 2023) {
    delay(1000);
    time(&UTC);  // was now.  Not sure if it matters.  Maybe only on Dec 31/Jan 1st.
    strftime (cCharWork, sizeof(cCharWork), "%Y", localtime(&UTC));
    iYear = atoi(cCharWork);
    Serial.println(localtime(&UTC), "cZulu waiting %a, %d-%m-%Y %T %Z %z");
  }
  //  Serial.println(localtime(&UTC), " after waiting %a, %d-%m-%Y %T %Z %z");
  Serial.println(localtime(&UTC), " %a, %d-%m-%Y %T %Z %z");

  // It has been taking about 32-33ms to run this routine.
  //  Serial.printf("Total time for computing offsets %lu ms.\r\n", millis() - iStartMillis);
}
//***************************************************************************/
void startWiFiManager()
//***startWiFiManager********************************************************/
{

  //  wifiManager.resetSettings(); // wipe settings -- emergency use only.

  //  wifiManager.setTimeout(60);  // 30 second connect timeout then reboot.
  //  wifiManager.disconnect();
  //  //  wifiManager.mode(WIFI_STA); // switch off AP
  //  if (!wifiManager.autoConnect(cMyPortalName, "BigLittleClock")) {
  //    Serial.println(F("failed to connect and hit timeout"));
  //    //Reboot and try again.
  //#ifdef ESP32
  //    ESP.restart();
  //#else
  //    ESP.reset();
  //#endif
  //  }
  //  // Set callback that gets called when connecting to previous WiFi fails,
  //  //  and enters Access Point mode
  //  wifiManager.setAPCallback(configModeCallback);
  //
  //  //Fetches ssid and pass and tries to connect
  //  //If it does not connect it starts an access point with the specified name
  //  //here  "ESPWiFiMgr"
  //  //and goes into a blocking loop awaiting configuration
  //  wifiManager.setDebugOutput(false);  // Quieter on the Serial Monitor
  //  if (!wifiManager.autoConnect(cMyPortalName)) {
  //    Serial.println(F("Failed to connect and hit timeout."));
  //    //Reset and try again, or maybe put it to deep sleep
  //#ifdef ESP32
  //    ESP.restart();
  //#else
  //    ESP.reset();
  //#endif
  //  }
  //  // If you get here you have connected to the WiFi
  //  Serial.println(F("Now connected. To work, we go..."));
  //  delay(2000);
}
/***************************************************************************/
//void configModeCallback (WiFiManager * myWiFiManager)
/***************************************************************************/
//{
//  Serial.println(F("Entered config mode..."));
//  Serial.println(WiFi.softAPIP());
//  //if you used auto generated SSID, print it
//  Serial.println(myWiFiManager->getConfigPortalSSID());
//  int yPos = 10;
//  tft.fillScreen(DarkerRed);
//  tft.setTextColor(TFT_WHITE, DarkerRed);  // Foreground color, Background color
//  tft.drawString("Need SSID", iXCenter, yPos);
//  tft.drawString("Connect to", iXCenter, yPos + 43);
//  tft.drawString(cMyPortalName, iXCenter, yPos + 85);
//}
/***************************************************************************/
void  startWiFi()
/***************************************************************************/
{
  //  WiFi.begin(ssid, wifipw);
  //  Serial.println("Connecting Wifi");
  //  int waitCt = 0;
  //  while (WiFi.status() != WL_CONNECTED) {
  //    Serial.print(".");
  //    delay(500);
  //    if (waitCt++ > 20) ESP.restart();
  //  }
  //  Serial.println("Connected!");
  //  Serial.print("Host:\t\t");     Serial.println(WiFi.SSID());
  //  Serial.print("IP Address:\t"); Serial.println(WiFi.localIP());
  //  Serial.print("Wifi RSSI =\t"); Serial.println(WiFi.RSSI());
  //  String myMACAddress = WiFi.macAddress();
  //  Serial.print("MAC Address =\t"); Serial.println(myMACAddress);
  //  //           1111111
  //  // 01234567890123456
  //  // 84:CC:A8:47:53:98
  //  String subS = myMACAddress.substring(   0,  2) + myMACAddress.substring(3,  5)
  //                + myMACAddress.substring( 6,  8) + myMACAddress.substring(9, 11)
  //                + myMACAddress.substring(12, 14) + myMACAddress.substring(15);
  //  Serial.print("Scrubbed MAC:\t");
  //  Serial.println(subS);  // String of MAC address without the ":" characters.
}
/***************************************************************************/
void StartWiFiMulti()
/***************************************************************************/
{
//  wifiMulti.addAP("Converge2G",  "Lallave@Family7");
  wifiMulti.addAP("MikeysWAP",   "Noogly99");
  //  wifiMulti.addAP("DownUnder",  "PCCRules");
  //  wifiMulti.addAP("RaPhaEl",    "thinkpositive-0829");
  //  wifiMulti.addAP("APort",      "helita1943");
  Serial.print("Connecting Wifi...");
  wifiMulti.run(); delay(15000);
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.print("Trying again...");
    wifiMulti.run();
    delay(15000);
  }
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.print("\r\nHost:\t\t"); Serial.println(WiFi.SSID());
    Serial.print("IP Address:\t"); Serial.println(WiFi.localIP());
    Serial.print("Wifi RSSI =\t"); Serial.println(WiFi.RSSI());
    String myMACAddress = WiFi.macAddress();
    Serial.print("MAC Address =\t"); Serial.println(myMACAddress);
    //           1111111
    // 01234567890123456
    // 84:CC:A8:47:53:98
    String subS
      = myMACAddress.substring( 0,  2)
        + myMACAddress.substring( 3,  5)
        + myMACAddress.substring( 6,  8)
        + myMACAddress.substring( 9, 11)
        + myMACAddress.substring(12, 14)
        + myMACAddress.substring(15);
    Serial.print("Scrubbed MAC:\t");
    Serial.println(subS);  // String of MAC address without the ":" characters.
    delay(500);
  }
  if (wifiMulti.run() != WL_CONNECTED)
#ifdef ESP32
    ESP.restart();
#else
    ESP.reset();
#endif
}
/***************************************************************************/
void timeSyncCallback(struct timeval *tv)
/***************************************************************************/
{
  //  struct timeval {  // Instantiated as "*tv"
  //   time_t      tv_sec;   // Number of whole seconds of elapsed time
  //   long int    tv_usec;  // Number of microseconds of rest of elapsed time minus tv_sec.
  //                             Always less than one million
  //};
  Serial.println("\n----Time Sync-----");
  Serial.printf("Time sync at %u ms.\r\nUTC Epoch: ", millis());
  Serial.println(tv->tv_sec);
  Serial.println(ctime(&tv->tv_sec));
  delay(1000);
}
/****************************************************************************/
void getXchangeRate()
/****************************************************************************/
{
  // return;  // Uncomment to deactivate XRate fetching.
  bool iFetchOK;
  unsigned long ulEntryEpoch = UTC;
  static bool bFirstXPass = true;
  static unsigned long ulLastXRateFetchEpoch = 0;  // 0 forces it to initialize.
  static unsigned long ulXRateFetchInterval = 10800;  // 3 hours in seconds
  static int iLastEntryMin = -1;
  //  Serial.printf("iLastEntryMin %i, iCurrMinute %i\r\n",
  //                iLastEntryMin, iCurrMinute);
  if (iLastEntryMin == iCurrMinute) return;  // Once per 10 minute gate.
  iLastEntryMin = iCurrMinute;

  time(&UTC);
  if (luLastXRateFetchTime + ulResetXRateTime < UTC) {
    Serial.println("Stale XRate cleared.");
    fPHP_Rate = 0.;  // Data too old. Clear it.
  }

  int currFontSize = ofr.getFontSize();
  ofr.setFontSize(38);

  Serial.printf("\r\n%02i:%02i:%02i Entering X Rate fetch routine for rate update.\r\n",
                iCurrHour, iCurrMinute, iCurrSecond);

  if (bFirstXPass) {
    bFirstXPass = false;
    Serial.println("This is the initial pass, not fetching X Rate this time.");
  } else {
    Serial.printf("UTC epoch now = %lu, ", ulEntryEpoch);  // UTC epoch now
    Serial.printf("elapsed seconds: %lu/%lu.\r\n",
                  ulEntryEpoch - ulLastXRateFetchEpoch, ulXRateFetchInterval);
    // 1 minute of slack added, just to be sure.
    // This is the once per 3 hour (currently) gate.
    if ((ulEntryEpoch - ulLastXRateFetchEpoch + 60) > ulXRateFetchInterval) {
      Serial.println("\r\nI will try an X Rate fetch now.");
      iFetchOK = xRateWorker(1);  // Try to get the data from apilayer server.
      if (!iFetchOK) {
        Serial.println("\r\nFirst X Rate fetch failed, waiting 5 seconds for a retry. (try 2)");
        delay(5000);  // Wait 5 seconds and try a second time.
        iFetchOK = xRateWorker(2);  // Try to get the data from apilayer server again.
      }
      if (!iFetchOK) {
        Serial.println("\r\nSecond X Rate fetch failed, waiting 5 seconds for final retry. (try 3)");
        delay(5000);  // Wait 5 seconds and try a second time.
        iFetchOK = xRateWorker(3);  // Try to get the data from apilayer server for the last time.
      }
      if (iFetchOK) {
        ulLastXRateFetchEpoch = ulEntryEpoch;  // Got a live one, update timer.
        Serial.println("\r\nXRate fetch successful.");
        int yPos = 35;
        clockSprite.fillSprite(RGB565(0, 80, 0));
        ofr.setFontColor(TFT_WHITE, RGB565(0, 80, 0));  // Foreground White, Background Dark Green
        //                                             ("Maximum length msg.");
        ofr.setCursor(iXCenter, yPos);       ofr.cprintf("Exchange Rate");
        ofr.setCursor(iXCenter, yPos + 70);  ofr.cprintf("fetch succeeded!");
        ofr.setCursor(iXCenter, yPos + 140); ofr.cprintf("Sweet!!");
        clockSprite.pushSprite(0, 0);
        bNewRate = true;                     // Resize sooner than next minute.
        for (i = 0; i < 2; i++) {
          tft.invertDisplay(true); delay(200);
          tft.invertDisplay(false); delay(200);
        }
        delay(2000);
      } else {
        Serial.println("\r\nThird X Rate fetch failed.  Will try again in 10 minutes.");
        int yPos = 35;
        clockSprite.fillSprite(DarkerRed);
        ofr.setFontColor(TFT_YELLOW, DarkerRed);  // Foreground color, Background color
        //                                             ("Maximum length msg.");
        ofr.setCursor(iXCenter, yPos);       ofr.cprintf("XRate fetch");
        ofr.setCursor(iXCenter, yPos + 70);  ofr.cprintf("failed. Retry");
        ofr.setCursor(iXCenter, yPos + 140); ofr.cprintf("in 10 minutes.");
        clockSprite.pushSprite(0, 0);
        for (i = 0; i < 2; i++) {
          tft.invertDisplay(true); delay(200);
          tft.invertDisplay(false); delay(200);
        }
        delay(2000);
      }
    } else {
      Serial.println("Not fetching rate yet. Not long enough since last fetch.");
    }
  }
  ofr.setFontSize(currFontSize);
  if (bWhichClock == bDigitalClock)
    tft.fillRect(0, 0, tft.width(), iScrollSpriteH, DarkBlue);
}
/****************************************************************************/
bool xRateWorker(int iTry)
/****************************************************************************/
{
  /* Returned JSON packet looks something like this:
    {
    "success": true,
    "timestamp": 1651599013,
    "base": "USD",
    "date": "2022-05-03",
    "rates": {
    "PHP": 52.511497
    }
    }
  */
  static String sServerPath;
  bool  bPktValidity = false;  // True if good exchange rate fetched
  int iHttpResponseCode = 429;  // To get it into the loop. This is the code for API key exhausted.

  StaticJsonDocument<200> doc;  // Allocate 200 bytes on the stack.
  //Get just PHP compared to USD.  Returns about 136 bytes.
  //https://api.apilayer.com/exchangerates_data/latest?symbols=PHP
  // &base=USD&apikey=k5MJFkvlen6ebpAvKRpUlbbBd7uPAzAC
  //Get every rate they have in one go
  //https://api.apilayer.com/exchangerates_data/latest
  // &apikey=k5MJFkvlen6ebpAvKRpUlbbBd7uPAzAC
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    while (iUseAPIkey < iMaxAPIkeys && iHttpResponseCode == 429) {
      sServerPath = "https://api.apilayer.com/exchangerates_data/latest?"
                    "symbols=PHP&base=USD&apikey=";
      sServerPath = sServerPath + cAPI_Array[iUseAPIkey];
      Serial.printf("Fetching exchange rate with API key %i with the following URI:\r\n%s\r\n",
                    iUseAPIkey + 1, sServerPath.c_str());

      sprintf(cCharWork, "Try %i, Key %i", iTry, iUseAPIkey + 1);
      Serial.println(cCharWork);

      int yPos = 35;
      clockSprite.fillSprite(RGB565(0, 80, 0));
      ofr.setFontColor(TFT_WHITE, RGB565(0, 80, 0));  // Foreground color, Background color
      ofr.setCursor(iXCenter, yPos);       ofr.cprintf("Attempting to");
      ofr.setCursor(iXCenter, yPos + 70);  ofr.cprintf("fetch X rate.");
      ofr.setCursor(iXCenter, yPos + 140); ofr.cprintf(cCharWork);
      clockSprite.pushSprite(0, 0);

      http.setTimeout(30000);         // Yeah, maybe...
      http.setConnectTimeout(30000);  // Yeah, maybe...
      Serial.printf("%lu - Sending message to X Rate server.\r\n", millis());
      http.begin(sServerPath.c_str());
      iHttpResponseCode = http.GET();
      Serial.printf("%lu - End sending message to X Rate server.\r\n", millis());
      Serial.printf("HTTP.GET response code: %i\r\n", iHttpResponseCode);
      if (iHttpResponseCode == 429) iUseAPIkey++;
    }

    if (iHttpResponseCode == HTTPC_ERROR_CONNECTION_REFUSED) {
      Serial.println("http.GET request connection refused.");
      return false;
    } else if (iHttpResponseCode == 200) {
      String payload = http.getString();
      Serial.println("Returned data:"); Serial.print(payload);
      http.end();  // Free resources
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return false;
      }
      bPktValidity = doc["success"];
      // It is called ternary!
      //  Compare ? True return action : False return action
      Serial.print(bPktValidity ? "V" : "Inv");
      Serial.println("alid packet received.");
      if (!bPktValidity) {
        Serial.println("Rate packet not valid. Not decoded further.");
        //        fPHP_Rate = 0.;
        return bPktValidity;
      }
      unsigned long packetTime = doc["timestamp"];
      //      long int os = iBotOffset;
#if defined CONFIG4MIKE
      long int os = iBotOffset;  // Assuming, for now, that the bottom clock is local time.
#else
      long int os = iTopOffset;  // Assuming, for now, that the bottom clock is local time.
#endif
      // Really!  Get rid of this.  Convert to what's used elsewhere in this pgm.
      //  Then, remove the #include for timeLib.h.  Not done yet as of 6/24/23.
      tmElements_t tm;
      packetTime += os;
      breakTime(packetTime, tm);
      Serial.printf("Currency Conversion packet time: "
                    "%02i/%02i/%02i %02i:%02i:%02i\r\n",
                    tm.Month, tm.Day, tm.Year + 1970,
                    tm.Hour, tm.Minute, tm.Second);
      fPHP_Rate = doc["rates"]["PHP"];
      luLastXRateFetchTime = UTC;

      Serial.printf("Current PHP Conversion rate %.2f\r\n", fPHP_Rate);
#if defined CONFIG4MIKE
      sprintf(caReadingTime, "As of %02i:%02i %s, ", iCurrHour, iCurrMinute, cBotDST);
#else
      sprintf(caReadingTime, "As of %02i:%02i %s, ", iCurrHour, iCurrMinute, cTopDST);
#endif
    } else {
      bPktValidity = false;
      Serial.printf("Bad HTTP return code %i. Exchange rate packet ignored.\r\n", iHttpResponseCode );
      //      fPHP_Rate = 0.;  // This is now reset on a timer.
    }
  } else {
    Serial.println("WiFi not available now. No api fetch possible");
  }
  return bPktValidity;
}
/****************************************************************************/
void initOTA()
/****************************************************************************/
{
  ArduinoOTA.setHostname(OTAhostname.c_str()); //define OTA port hostname
  ArduinoOTA.begin();
}
/****************************************************************************/
void initTime()
/****************************************************************************/
{
  sntp_set_sync_interval(86400000);  // 1 day in ms.
  sntp_set_time_sync_notification_cb(timeSyncCallback);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", cZulu, 1); tzset();
  displayW_Header("Waiting for right time");
  //  Serial.println("Waiting for correct time...");

  strftime(cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
  iYear = atoi(cCharWork);
  int iLooper = 0;
  while (iYear < 2023) {
    time(&UTC);
    delay(1000);
    strftime (cCharWork, 100, "%Y", localtime(&UTC));
    iYear = atoi(cCharWork);
    Serial.println(localtime(&UTC), "UTC %a %m-%d-%Y %T");
    if (iLooper++ > 30) {
      Serial.println("Cannot get time set. Rebooting.");
      ESP.restart();
    }
  }
  displayW_Header("Get zone offsets");
  //  Serial.println("\r\nDetermining zone offsets.");
  deduceOffsets();
  time(&UTC);
#if defined CONFIG4MIKE
  workTime = UTC + iBotOffset;  // Assuming, for now, that the bottom clock is local time.
#else
  workTime = UTC + iTopOffset;  // Assuming, for now, that the bottom clock is local time.
#endif
}
/****************************************************************************/
void allocateSprites()
/****************************************************************************/
{
  int *a = 0;

  iRadius = min(tft.width(), tft.height()) / 2 - 1;
  Serial.printf("Analog clock radius is %i\r\n", iRadius);
  // This must be allocated in PSRAM since ESP folks screwed up allocation.  There is a 32 bit
  //  and an 8 bit allocation but no 16 bit so you get double what you need and it blows the stack!

  a = (int*)clockSprite.createSprite(tft.width(), tft.height()); // Leave room for 1st sprite
  if (a == 0) {
    Serial.println("clockSprite creation failed.  Cannot continue.");
    spriteAllocError("clockSprite");
  }
  Serial.printf("createclockSprite dispWidth x dispHeight returned: %p\r\n", a);

  a = (int*)scrollSprite.createSprite(iScrollSpriteW, iScrollSpriteH); // 1st sprite for scrolling info
  if (a == 0) {
    Serial.println("scrollSprite creation failed.  Cannot continue.");
    spriteAllocError("scrollSprite");
    while (1) ArduinoOTA.handle();
  }
  scrollSprite.fillSprite(DarkBlue);
  scrollSprite.setTextColor(TFT_WHITE, DarkBlue);
}
/***************************************************************************/
void spriteAllocError(String which)
/***************************************************************************/
{
  int txtLen;

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  txtLen = tft.textWidth("Sprite Allocation");
  tft.drawString("Sprite Allocation", iXCenter - txtLen / 2, iDisplayLine1);

  txtLen = tft.textWidth(which + " failed");
  tft.drawString(which + " failed", iXCenter - txtLen / 2, iDisplayLine3);

  txtLen = tft.textWidth("Got PSRAM?");
  tft.drawString("Got PSRAM?", iXCenter - txtLen / 2, iDisplayLine5);

  txtLen = tft.textWidth("OTA is Available");
  tft.drawString("OTA is Available", iXCenter - txtLen / 2, iDisplayLine5 + 40);

  while (1) ArduinoOTA.handle();
}

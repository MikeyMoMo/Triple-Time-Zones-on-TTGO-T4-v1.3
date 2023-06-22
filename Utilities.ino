/***************************************************************************/
void decodeAC_Bits()
/***************************************************************************/
{
  if (acCustom & 0x1 << showShortTimeBit) {  // See if short time requested
    showShortTime = true;
  } else {
    showShortTime = false;
  }
  if (acCustom & 0x1 << showLongTimeBit) {
    //    Serial.println("Turning on showLongTime, off showShortTime");
    showLongTime = true;
    showShortTime = false;  // Override short time bit
  } else {
    showLongTime = false;
  }
  if (acCustom & 0x1 << showSecondHandBit)
    showSecondHand = true;
  else
    showSecondHand = false;
}
/***************************************************************************/
void initDisplay()
/***************************************************************************/
{
  tft.init(); // Initialize the screen.
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
  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT); // TFT_BL, 0 - 15
  ledcWrite(pwmLedChannelTFT, 200);
#endif
  tft.invertDisplay(false); // Where it is true or false.  False is "normal" on this display.
  ofr.setFontColor(TFT_WHITE, DarkerRed);  // Foreground color, Background color

  tft.setRotation(3);       // Power on top.  1 for power at bottom

  scrollSprite.loadFont(TimesNewRoman32Bold);  // For use with tft print statements only.
  clockSprite.loadFont(TimesNewRoman32Bold);  // For use with tft print statements only.
}
/***************************************************************************/
void displayW_Header(int y, String what)
/***************************************************************************/
{
  clockSprite.fillSprite(TFT_BLACK); clockSprite.setTextColor(TFT_WHITE);
  ofr.setCursor(xCenter, displayLine1);      ofr.cprintf("Dual NTP Time");
  ofr.setCursor(xCenter, displayLine2 + 32); ofr.cprintf(myVersion);
  clockSprite.pushSprite(0, 0);
  ofr.setCursor(xCenter, y); ofr.cprintf(what.c_str());
  clockSprite.pushSprite(0, 0);
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

  startMillis = millis();

  Serial.print(TopCityname);
  setenv("TZ", TopTZ, 1); tzset();  // Anybody but me see how silly that 1 is?
  time(&workTime);
  strftime (charWork, sizeof(charWork), "%Y", localtime(&workTime)); iYear = atoi(charWork);
  Serial.println(localtime(&workTime), " initial set %a, %d-%m-%Y %T %Z %z");
  while (iYear < 2023) {
    delay(1000);
    time(&workTime);
    strftime (charWork, sizeof(charWork), "%Y", localtime(&workTime));
    iYear = atoi(charWork);
    Serial.print(TopCityname);
    Serial.println(localtime(&workTime), " waiting %a, %d-%m-%Y %T %Z %z");
  }
  Serial.println(localtime(&workTime), " after waiting %a, %d-%m-%Y %T %Z %z");

  strftime (TopDST, 10, "%Z", localtime(&workTime));
  strftime (charWork, sizeof(charWork), "%z", localtime(&workTime));
  tempOffset = atoi(charWork);
  TopOffset = (tempOffset / 100) * 3600 + tempOffset % 100 * 60;
  Serial.print(TopCityname);
  Serial.printf(" offset = %+i\r\n", TopOffset);

  setenv("TZ", BotTZ, 1); tzset();
  Serial.print(BotCityname);
  time(&workTime);
  strftime (charWork, sizeof(charWork), "%Y", localtime(&workTime));
  iYear = atoi(charWork);
  Serial.println(localtime(&workTime), " initial set %a, %d-%m-%Y %T %Z %z");
  while (iYear < 2023) {
    delay(1000);
    time(&workTime);
    strftime (charWork, sizeof(charWork), "%Y", localtime(&workTime));
    iYear = atoi(charWork);
    Serial.print(BotCityname);
    Serial.println(localtime(&workTime), " waiting %a, %d-%m-%Y %T %Z %z");
  }
  Serial.println(localtime(&workTime), " after waiting %a, %d-%m-%Y %T %Z %z - ");
  strftime (BotDST, 10, "%Z", localtime(&workTime));  // Characters of time zone and DST indicator.
  strftime (charWork, sizeof(charWork), "%z", localtime(&workTime));  // Seconds offset from UTC.
  tempOffset = atoi(charWork);
  BotOffset = (tempOffset / 100) * 3600 + tempOffset % 100 * 60;
  Serial.print(BotCityname);
  Serial.printf(" offset = %+i\r\n", BotOffset);

  // This must be done and must be last.  The local time(s) are based off of this.  They are
  //  not kept separately, only created when needed by adding the offset to UTC.
  setenv("TZ", Zulu, 1); tzset();
  strftime (charWork, sizeof(charWork), "%Y", localtime(&workTime));
  time(&UTC);
  iYear = atoi(charWork);
  Serial.println(localtime(&UTC), "Zulu initial set %a, %d-%m-%Y %T %Z %z");
  while (iYear < 2023) {
    delay(1000);
    time(&UTC);  // was now.  Not sure if it matters.  Maybe only on Dec 31/Jan 1st.
    strftime (charWork, sizeof(charWork), "%Y", localtime(&UTC));
    iYear = atoi(charWork);
    Serial.println(localtime(&UTC), "Zulu waiting %a, %d-%m-%Y %T %Z %z");
  }
  Serial.println(localtime(&UTC), " after waiting %a, %d-%m-%Y %T %Z %z");

  // It has been taking about 32-33ms to run this routine.
  //  Serial.printf("Total time for computing offsets %lu ms.\r\n", millis() - startMillis);
}
//***************************************************************************/
void startWiFiManager()
//***startWiFiManager********************************************************/
{

  //  wifiManager.resetSettings(); // wipe settings -- emergency use only.

  wifiManager.setTimeout(60);  // 30 second connect timeout then reboot.
  wifiManager.disconnect();
  //  wifiManager.mode(WIFI_STA); // switch off AP
  if (!wifiManager.autoConnect(myPortalName, "BigLittleClock")) {
    Serial.println(F("failed to connect and hit timeout"));
    //Reboot and try again.
#ifdef ESP32
    ESP.restart();
#else
    ESP.reset();
#endif
  }
  // Set callback that gets called when connecting to previous WiFi fails,
  //  and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //Fetches ssid and pass and tries to connect
  //If it does not connect it starts an access point with the specified name
  //here  "ESPWiFiMgr"
  //and goes into a blocking loop awaiting configuration
  wifiManager.setDebugOutput(false);  // Quieter on the Serial Monitor
  if (!wifiManager.autoConnect(myPortalName)) {
    Serial.println(F("Failed to connect and hit timeout."));
    //Reset and try again, or maybe put it to deep sleep
#ifdef ESP32
    ESP.restart();
#else
    ESP.reset();
#endif
  }
  // If you get here you have connected to the WiFi
  Serial.println(F("Now connected. To work, we go..."));
  delay(2000);
}
/***************************************************************************/
void configModeCallback (WiFiManager * myWiFiManager)
/***************************************************************************/
{
  Serial.println(F("Entered config mode..."));
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  int yPos = 10;
  tft.fillScreen(DarkerRed);
  tft.setTextColor(TFT_WHITE, DarkerRed);  // Foreground color, Background color
  tft.drawString("Need SSID", xCenter, yPos);
  tft.drawString("Connect to", xCenter, yPos + 43);
  tft.drawString(myPortalName, xCenter, yPos + 85);
}
/***************************************************************************/
void  startWifi()
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
void timeSyncCallback(struct timeval *tv)
/***************************************************************************/
{
  //  struct timeval {
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
  //  return;  // Uncomment to deactivate XRate fetching.
  bool fetchOK;
  unsigned long entryEpoch = UTC;
  static bool firstXPass = true;
  static unsigned long lastXRateFetchEpoch = 0;  // 0 forces it to initialize.
  static unsigned long XRateFetchInterval = 10800;  // 3 hours in seconds
  static int lastEntryMin = -1;
  const long unsigned int resetXRateTime = 43200;  // 1/2 day.  After that, data is stale, don't show

  if (lastEntryMin == currMinute) return;
  lastEntryMin = currMinute;

  time(&UTC);
  if (lastXRateFetchTime + resetXRateTime < UTC) {
    Serial.println("Stale XRate cleared.");
    PHP_Rate = 0.;  // Data too old. Clear it.
  }

  int currFontSize = ofr.getFontSize();
  ofr.setFontSize(38);

  Serial.printf("\r\n%02i:%02i:%02i Entering X Rate fetch routine for rate update.\r\n",
                currHour, currMinute, currSecond);

  if (firstXPass) {
    firstXPass = false;
    Serial.println("This is the initial pass, not fetching X Rate this time.");
  } else {
    Serial.printf("UTC epoch now = %lu, ", entryEpoch);  // UTC epoch now
    Serial.printf("elapsed seconds: %lu/%lu.\r\n",
                  entryEpoch - lastXRateFetchEpoch, XRateFetchInterval);
    //    if (currMinute % 10 == 0) {  // Really not necessary. Calls are only on % 10 minutes.
    // 1 minute of slack added, just to be sure.
    if ((entryEpoch - lastXRateFetchEpoch + 60) > XRateFetchInterval) {
      Serial.println("\r\nI will try an X Rate fetch now.");
      fetchOK = xRateWorker(1);  // Try to get the data from apilayer server.
      if (!fetchOK) {
        Serial.println("\r\nFirst X Rate fetch failed, waiting 5 seconds for a retry. (try 2)");
        delay(5000);  // Wait 5 seconds and try a second time.
        fetchOK = xRateWorker(2);  // Try to get the data from apilayer server again.
      }
      if (!fetchOK) {
        Serial.println("\r\nSecond X Rate fetch failed, waiting 5 seconds for final retry. (try 3)");
        delay(5000);  // Wait 5 seconds and try a second time.
        fetchOK = xRateWorker(3);  // Try to get the data from apilayer server for the last time.
      }
      if (fetchOK) {
        lastXRateFetchEpoch = entryEpoch;  // Got a live one, update timer.
        Serial.println("\r\nXRate fetch successful.");
        int yPos = 35;
        clockSprite.fillSprite(RGB565(0, 80, 0));
        ofr.setFontColor(TFT_WHITE, RGB565(0, 80, 0));  // Foreground White, Background Dark Green
        //                                             ("Maximum length msg.");
        ofr.setCursor(xCenter, yPos);       ofr.cprintf("Exchange Rate");
        ofr.setCursor(xCenter, yPos + 70);  ofr.cprintf("fetch succeeded!");
        ofr.setCursor(xCenter, yPos + 140); ofr.cprintf("Sweet!!");
        clockSprite.pushSprite(0, 0);
        newRate = true;                     // Resize sooner than next minute.
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
        ofr.setCursor(xCenter, yPos);       ofr.cprintf("XRate fetch");
        ofr.setCursor(xCenter, yPos + 70);  ofr.cprintf("failed. Retry");
        ofr.setCursor(xCenter, yPos + 140); ofr.cprintf("in 10 minutes.");
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
  tft.fillRect(0, 0, tft.width(), scrollSpriteH, DarkBlue);
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
  static String serverPath;
  bool  pktValidity = false;  // True if good exchange rate fetched
  int httpResponseCode = 429;  // To get it into the loop. This is the code for API key exhausted.

  StaticJsonDocument<200> doc;  // Allocate 200 bytes on the stack.
  //Get just PHP compared to USD.  Returns about 136 bytes.
  //https://api.apilayer.com/exchangerates_data/latest?symbols=PHP
  // &base=USD&apikey=k5MJFkvlen6ebpAvKRpUlbbBd7uPAzAC
  //Get every rate they have in one go
  //https://api.apilayer.com/exchangerates_data/latest
  // &apikey=k5MJFkvlen6ebpAvKRpUlbbBd7uPAzAC
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    while (useAPIkey < maxAPIkeys && httpResponseCode == 429) {
      serverPath = "https://api.apilayer.com/exchangerates_data/latest?"
                   "symbols=PHP&base=USD&apikey=";
      serverPath = serverPath + apiArray[useAPIkey];
      Serial.printf("Fetching exchange rate with API key %i with the following URI:\r\n%s\r\n",
                    useAPIkey + 1, serverPath.c_str());

      sprintf(charWork, "Try %i, Key %i", iTry, useAPIkey + 1);
      Serial.println(charWork);

      int yPos = 35;
      clockSprite.fillSprite(RGB565(0, 80, 0));
      ofr.setFontColor(TFT_WHITE, RGB565(0, 80, 0));  // Foreground color, Background color
      ofr.setCursor(xCenter, yPos);       ofr.cprintf("Attempting to");
      ofr.setCursor(xCenter, yPos + 70);  ofr.cprintf("fetch X rate.");
      ofr.setCursor(xCenter, yPos + 140); ofr.cprintf(charWork);
      clockSprite.pushSprite(0, 0);

      http.setTimeout(30000);         // Yeah, maybe...
      http.setConnectTimeout(30000);  // Yeah, maybe...
      Serial.printf("%lu - Sending message to X Rate server.\r\n", millis());
      http.begin(serverPath.c_str());
      httpResponseCode = http.GET();
      Serial.printf("%lu - End sending message to X Rate server.\r\n", millis());
      Serial.printf("HTTP.GET response code: %i\r\n", httpResponseCode);
      if (httpResponseCode == 429) useAPIkey++;
    }

    if (httpResponseCode == HTTPC_ERROR_CONNECTION_REFUSED) {
      Serial.println("http.GET request connection refused.");
      return false;
    } else if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println("Returned data:"); Serial.print(payload);
      http.end();  // Free resources
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return false;
      }
      pktValidity = doc["success"];
      // It is called ternary!
      //  Compare ? True return action : False return action
      Serial.print(pktValidity ? "V" : "Inv");
      Serial.println("alid packet received.");
      if (!pktValidity) {
        Serial.println("Rate packet not valid. Not decoded further.");
        //        PHP_Rate = 0.;
        return pktValidity;
      }
      unsigned long packetTime = doc["timestamp"];
      long int os = BotOffset;
      // Really!  Get rid of this.  Convert to what's used elsewhere in this pgm.
      //  Then, remove the #include for timeLib.h.
      tmElements_t tm;
      packetTime += os;
      breakTime(packetTime, tm);
      Serial.printf("Currency Conversion packet time: "
                    "%02i/%02i/%02i %02i:%02i:%02i\r\n",
                    tm.Month, tm.Day, tm.Year + 1970,
                    tm.Hour, tm.Minute, tm.Second);
      PHP_Rate = doc["rates"]["PHP"];
      lastXRateFetchTime = UTC;


      Serial.printf("Current PHP Conversion rate %.2f\r\n", PHP_Rate);
      sprintf(readingTime, "As of %02i:%02i %s, ", currHour, currMinute, BotDST);
    } else {
      pktValidity = false;
      Serial.printf("Bad HTTP return code %i. Exchange rate packet ignored.\r\n", httpResponseCode );
      //      PHP_Rate = 0;
    }
  } else {
    Serial.println("WiFi not available now. No api fetch possible");
  }
  return pktValidity;
}
void initOTA()
{
  ArduinoOTA.setHostname(OTAhostname.c_str()); //define OTA port hostname
  ArduinoOTA.begin();
}

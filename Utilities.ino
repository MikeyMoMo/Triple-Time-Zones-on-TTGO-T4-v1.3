/***************************************************************************/
void countXRate(bool printList, bool printTot)
/***************************************************************************/
{
  totXRatesaved = 0;
  if (printList)
    Serial.println("Current non-zero XRate history table entries.");
  for (i = 0; i < XRateHistLen; i++) {
    if (XRateHist[i] > 0.) {
      if (printList) {
        // Sets XRateMon, XRateDay & XRateYr.
        JulianToGregorian(XRateJulian[i]);
        Serial.printf("XRate for element %i value %.3f, Julian %u, "
                      "%02i/%02i/%02i\r\n",
                      i, XRateHist[i], XRateJulian[i], XRateMon, XRateDay,
                      XRateYr);
      }
      totXRatesaved++;
    }
  }
  if (printTot)
    Serial.printf("There are %i saved daily rates\r\n", totXRatesaved);
}
/***************************************************************************/
int DaysInMonth(int iYear, int iMonth)
/***************************************************************************/
{
  return iMonth * 275 % 9 > 3 ? 31
         : iMonth != 2     ? 30
         : iYear % 4     ? 28
         : iYear % 100  ? 29
         : iYear % 400 ? 28
         : 29;
}
/***************************************************************************/
// SUBROUTINE GDATE (JD, YEAR,MONTH,DAY)
/***************************************************************************/
//C
//C---COMPUTES THE GREGORIAN CALENDAR DATE (YEAR,MONTH,DAY)
//C   GIVEN THE JULIAN DATE (JD).
//C
//      INTEGER JD,YEAR,MONTH,DAY,I,J,K
//C
//      L= JD+68569
//      N= 4*L/146097
//      L= L-(146097*N+3)/4
//      I= 4000*(L+1)/1461001
//      L= L-1461*I/4+31
//      J= 80*L/2447
//      K= L-2447*J/80
//      L= J/11
//      J= J+2-12*L
//      I= 100*(N-49)+I+L
//C
//      YEAR= I
//      MONTH= J
//      DAY= K
//C
//      RETURN
//      END
/***************************************************************************/
unsigned int leap(unsigned int year)
/***************************************************************************/
{
  if (year % 400 == 0)
    return 1;
  else if (year % 100 != 0 && year % 4 == 0)  //
    return 1;
  else
    return 0;
}
/***************************************************************************/
unsigned int days_in_month(unsigned int month, unsigned int year)
/***************************************************************************/
{
  unsigned int days = 0;

  if (month == 2) {
    days = 28 + leap(year);  // function leap returns 1 for leap years
  }
  else if (month == 4 || month == 6 || month == 9 || month == 11) {
    days = 30;
  }
  else {
    days = 31;
  }
  return days;
}
/***************************************************************************/
bool valiDATE(unsigned int month, unsigned int day, unsigned int year)
/***************************************************************************/
{
  unsigned int flag = 0;
  unsigned int days = 0;

  // Validate the month first
  if (month < 1 || month > 12) {
    Serial.printf ("\r\nValue for month %i is out of range.\r\n", month);
    return false;
  }

  // Validate the day of month
  days = days_in_month(month, year);

  if (day < 1 || day > days) {
    Serial.printf ("\r\nValue for day %i is out of range.\r\n", day);
    return false;
  }

  // Validate the year
  if (year < 1990 || year > 2030) {
    Serial.printf ("\r\nValue for year %i is out of range.\r\n", year);
    return false;
  }
  return true;  // If we got here, it is good!
}
/***************************************************************************/
unsigned int calcJulian(unsigned int myyear, unsigned int mymonth,
                        unsigned int myday)
/***************************************************************************/
{
  // Returns the number of days since 1 Jan. 4713 BCE.
  if (!valiDATE(mymonth, myday, myyear)) return 0;
  Serial.printf("calcJulian using %02u/%02u/%u\r\n",
                mymonth, myday, myyear);
  myyear += 8000;
  if (mymonth < 3) {
    myyear--;
    mymonth += 12;
  }
  return (myyear * 365) + (myyear / 4)
         - (myyear / 100)
         + (myyear / 400) - 1200820
         + (mymonth * 153 + 3) / 5 - 92
         + myday - 1;
}
/***************************************************************************/
void JulianToGregorian(unsigned int JD)
/***************************************************************************/
{
  int i, j, k, l, n;

  l = JD + 68569;
  n = 4 * l / 146097;
  l = l - (146097 * n + 3) / 4;
  i = 4000 * (l + 1) / 1461001;
  l = l - 1461 * i / 4 + 31;
  j = 80 * l / 2447;
  k = l - 2447 * j / 80;
  l = j / 11;
  j = j + 2 - 12 * l;
  i = 100 * (n - 49) + i + l;
  // And, now, the results of today's race...
  XRateYr = i; XRateMon = j; XRateDay = k;
}
/***************************************************************************/
void printVers()
/***************************************************************************/
{
  int      lastDot, lastV;

  //  Serial.println(__FILENAME__);  // Same as __FILE__
  sTemp = String(__FILE__);
  // Get rid of the trailing .ino tab name. In this case, "\Utilities.ino"
  sTemp = sTemp.substring(0, sTemp.lastIndexOf("\\"));
  Serial.print("Running from: "); Serial.println(sTemp);

  sTemp = String(__FILE__);  // Start again for the version number.
  lastDot = sTemp.lastIndexOf(".");
  if (lastDot > -1) {  // Found a dot.  Thank goodness!
    lastV = sTemp.lastIndexOf("v");  // Find start of version number
    if (lastV > -1) {  // Oh, good, found version number, too
      sVer = sTemp.substring(lastV + 1, lastDot); // Pick up version number
      lastV = sVer.lastIndexOf("\\");
      if (lastV > -1) sVer = sVer.substring(0, lastV);
    } else {
      sVer = "0.00";  // Unknown version.
    }
  } else {
    sVer = "n/a";  // Something badly wrong here!
  }
  Serial.print("Version " + sVer + " ");
  Serial.println("Compiled on: " + String(__DATE__) + " at " +
                 String(__TIME__));
}
/***************************************************************************/
void decodeAC_Bits()
/***************************************************************************/
{
  // This unnecessarily complex.  I just felt like doing it this way to
  //  sharpen my skill at decoding bit coded stuff.  It should have just
  //  been a switch case.  Others have done worse!
  // See if short time requested
  if (uiAC_Custom & 0x1 << bShowShortTimeBit) {
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
  // Note: Currently, font size 28 is used for everything except for
  //  notificaiton screens.  Size 38 is used for notifications screens
  //  and is reset after use, at routine end.
  tft.init(); // Initialize the screen.
  tft.setRotation(3);       // Power on top.  1 for power at bottom
  iXCenter = tft.width() / 2;
  iYCenter = tft.height() / 2;
  clockSprite.setTextColor(TFT_WHITE);
  clockSprite.setTextWrap(false, false);
  scrollSprite.setTextWrap(false, false);
  ofr.setDrawer(clockSprite);
  if (ofr.loadFont(BritanicBoldTTF, sizeof(BritanicBoldTTF))) {
    Serial.println("Render loadFont error for BritanicBoldTTF. "
                   "InitDisplay 1");
    while (1);
  }
  ofr.setFontSize(28);
  //  Serial.printf("Text height for size 28 is %i\r\n",
  //                ofr.getTextHeight("AB8Myp"));

  tft.fillScreen(TFT_BLACK);
#if defined TFT_BL
  // This is for boards definitions 2.x.x
  //  ledcSetup(iPWM_LedChannelTFT, iPWM_Freq, iPWM_Resolution);
  //  ledcAttachPin(TFT_BL, iPWM_LedChannelTFT); // TFT_BL, 0 - 15
  //  ledcWrite(iPWM_LedChannelTFT, 200);
  // This is for boards definitions 3.x.x
  ledcAttach(TFT_BL, iPWM_Freq, iPWM_Resolution);
  ledcWrite(TFT_BL, 200);  // Get the display on for init messages.
#endif
  // Where it is true or false.  False is "normal" on this display.
  tft.invertDisplay(false);
  // Foreground color, Background color
  ofr.setFontColor(TFT_WHITE, DarkerRed);

  tft.loadFont(TimesNewRoman32Bold);

  scrollSprite.loadFont(TimesNewRoman32);
  clockSprite.loadFont(BritanicBold28);
}
/**************************************************************************/
void displayW_Header(String what)
/**************************************************************************/
{
  Serial.println(what);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  txtLen = tft.textWidth("Dual NTP Time");
  //  Serial.printf("Center screen: %i, txtLen: %i\r\n", iXCenter, txtLen);
  tft.drawString("Triple NTP Time",
                 iXCenter - txtLen / 2, iDisplayLine1 + 5);

  txtLen = tft.textWidth(sVer);
  tft.drawString(sVer, iXCenter - txtLen / 2, iDisplayLine3);

  txtLen = tft.textWidth(what);
  tft.drawString(what, iXCenter - txtLen / 2, iDisplayLine5);
  //  Serial.println(what);
}
/***************************************************************************/
void deduceOffsets()
/***************************************************************************/
{
  // Only update the TZ hourly to get the current offset and DST setting.
  // That's really more than necessary but not such a big deal.
  // I was doing it every second.  However, it only takes 32 ms., so...
  // No biggy.
  // It totally saves me from having to figure out DST for myself.
  // I did it once and it was NOT pretty and only applied to the U.S.!
  // I put in the time env string and activate it then see what conditions
  //  it creates. It gives me complete time, the time type (standard or DST)
  //  and the seconds offset. SO much easier than trying to do all of this
  //  myself.  It takes care of changing the time from DST to standard time
  //  and I just use that number.  Sweet!
  //  I just have to be sure that the time is valid and I do that by being
  //  sure that the computed year is less than or equal to my
  //  birth year (2024).  (The program, not me!)

  //  Serial.printf("Today is Julian day %i\r\n",
  //                calcJulian(iYear, iMonth, iDay));

  iStartMillis = millis();

  Serial.print(cTopCityname);
  // Anybody but me see how silly that 1 is?
  setenv("TZ", cTopTZ, 1); tzset();
  time(&workTime);
  strftime (cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
  iYear = atoi(cCharWork);
  strftime (cCharWork, sizeof(cCharWork), "%m", localtime(&workTime));
  iMonth = atoi(cCharWork);
  strftime (cCharWork, sizeof(cCharWork), "%d", localtime(&workTime));
  iDay = atoi(cCharWork);
  //  Serial.println(localtime(&workTime),
  //                 " initial set %a, %d-%m-%Y %T %Z %z");
  while (iYear < 2024) {
    time(&workTime);
    strftime (cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
    iYear = atoi(cCharWork);
    Serial.print(cTopCityname);
    Serial.println(localtime(&workTime), " waiting %a, %d-%m-%Y %T %Z %z");
    delay(1000);
  }
  //  Serial.println(localtime(&workTime),
  //                 " after waiting %a, %d-%m-%Y %T %Z %z");
  Serial.print(localtime(&workTime), " %a, %d-%m-%Y %T %Z %z");

  strftime (cTopDST, 10, "%Z", localtime(&workTime));
  strftime (cCharWork, sizeof(cCharWork), "%z", localtime(&workTime));
  iTempOffset = atoi(cCharWork);
  iTopOffset = (iTempOffset / 100) * 3600 + iTempOffset % 100 * 60;
  Serial.printf(" offset = %+i\r\n", iTopOffset);

  Serial.print(cBotCityname);
  setenv("TZ", cBotTZ, 1); tzset();
  time(&workTime);
  strftime (cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
  iYear = atoi(cCharWork);
  //  Serial.println(localtime(&workTime),
  //                 " initial set %a, %d-%m-%Y %T %Z %z");
  while (iYear < 2024) {
    time(&workTime);
    strftime (cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
    iYear = atoi(cCharWork);
    Serial.print(cBotCityname);
    Serial.println(localtime(&workTime), " waiting %a, %d-%m-%Y %T %Z %z");
    delay(1000);
  }
  //  Serial.println(localtime(&workTime),
  //                 " after waiting %a, %d-%m-%Y %T %Z %z");
  Serial.print(localtime(&workTime), " %a, %d-%m-%Y %T %Z %z");
  // Characters of time zone and DST indicator.
  strftime (cBotDST, 10, "%Z", localtime(&workTime));
  // Seconds offset from UTC.
  strftime (cCharWork, sizeof(cCharWork), "%z", localtime(&workTime));
  iTempOffset = atoi(cCharWork);
  iBotOffset = (iTempOffset / 100) * 3600 + iTempOffset % 100 * 60;
  //  Serial.print(cBotCityname);
  Serial.printf(" offset = %+i\r\n", iBotOffset);

  // This must be done and must be last.  The local time(s) are
  //  based off of this. They are not kept separately, only created
  //  when needed by adding the offset to UTC.
  Serial.print("UTC");
  setenv("TZ", cZulu, 1); tzset();
  strftime (cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
  time(&UTC);
  iYear = atoi(cCharWork);
  //  Serial.println(localtime(&UTC),
  //                 "Zulu initial set %a, %d-%m-%Y %T %Z %z");
  while (iYear < 2024) {
    time(&UTC);
    strftime (cCharWork, sizeof(cCharWork), "%Y", localtime(&UTC));
    iYear = atoi(cCharWork);
    Serial.println(localtime(&UTC), "cZulu waiting %a, %d-%m-%Y %T %Z %z");
    delay(1000);
  }
  //  Serial.println(localtime(&UTC),
  //                 " after waiting %a, %d-%m-%Y %T %Z %z");
  Serial.println(localtime(&UTC), " %a, %d-%m-%Y %T %Z %z");

  // It has been taking about 32-33ms to run this routine.
  //  Serial.printf("Total time for computing offsets %lu ms.\r\n",
  //                millis() - iStartMillis);
}
//**************************************************************************/
void startWiFiManager()
//***startWiFiManager*******************************************************/
{

  //  wifiManager.resetSettings(); // wipe settings -- emergency use only.

  //  wifiManager.setTimeout(60);  // 30 second connect timeout then reboot.
  //  wifiManager.disconnect();
  //  //  wifiManager.mode(WIFI_STA); // switch off AP
  //  if (!wifiManager.autoConnect(cMyPortalName, "BigLittleClock")) {
  //    Serial.println("failed to connect and hit timeout");
  //    //Reboot and try again.
  //#ifdef ESP32
  //    ESP.restart();
  //#else
  //    ESP.reset();
  //#endif
  //  }
  //  Set callback that gets called when connecting to previous WiFi fails,
  //   and enters Access Point mode
  //   wifiManager.setAPCallback(configModeCallback);
  //
  //  Fetches ssid and pass and tries to connect
  //  If it does not connect it starts an access point with the specified
  //  name here  "ESPWiFiMgr"
  //  and goes into a blocking loop awaiting configuration
  //  wifiManager.setDebugOutput(false);  // Quieter on the Serial Monitor
  //  if (!wifiManager.autoConnect(cMyPortalName)) {
  //    Serial.println("Failed to connect and hit timeout.");
  //    //Reset and try again, or maybe put it to deep sleep
  //#ifdef ESP32
  //    ESP.restart();
  //#else
  //    ESP.reset();
  //#endif
  //  }
  //  // If you get here you have connected to the WiFi
  //  Serial.println("Now connected. To work, we go...");
  //  delay(2000);
}
/***************************************************************************/
//void configModeCallback (WiFiManager * myWiFiManager)
/***************************************************************************/
//{
//  Serial.println("Entered config mode...");
//  Serial.println(WiFi.softAPIP());
//  //if you used auto generated SSID, print it
//  Serial.println(myWiFiManager->getConfigPortalSSID());
//  int yPos = 10;
//  tft.fillScreen(DarkerRed);
// Foreground color, Background color
//  tft.setTextColor(TFT_WHITE, DarkerRed);
//  tft.drawString("Need SSID", iXCenter, yPos);
//  tft.drawString("Connect to", iXCenter, yPos + 43);
//  tft.drawString(cMyPortalName, iXCenter, yPos + 85);
//}
/***************************************************************************/
void  startWiFi()
/***************************************************************************/
{
  //  WiFi.begin(ssid, wifipw);
  //  Serial.println("Connecting to WiFi");
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
  //  String subS = myMACAddress.substring(   0,  2) +
  //                myMACAddress.substring(3,  5) +
  //                myMACAddress.substring( 6,  8) +
  //                myMACAddress.substring(9, 11) +
  //                myMACAddress.substring(12, 14) +
  //                myMACAddress.substring(15);
  //  Serial.print("Scrubbed MAC:\t");
  // String of MAC address without the ":" characters.
  //  Serial.println(subS);
}
/***************************************************************************/
void startWiFiMulti()
/***************************************************************************/
{
  wifiMulti.addAP("Converge2G", "Lallave@Family7");
  wifiMulti.addAP("MikeysWAP",  "Noogly99");
  //  Serial.print("Connecting to WifiMulti...");
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.print("Trying again...");
    wifiMulti.run();
    delay(15000);
  }
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.print("Host:\t\t"); Serial.println(WiFi.SSID());
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
    // String of MAC address without the ":" characters.
    Serial.println(subS);
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
  // Number of whole seconds of elapsed time
  //   time_t      tv_sec;
  // Number of microseconds of rest of elapsed time minus tv_sec.
  //                             Always less than one million
  //   long int    tv_usec;
  //};
  Serial.println("\n----- Time Sync Received -----");
  Serial.printf("Time sync at %u ms. UTC Epoch: ", millis());
  Serial.print(tv->tv_sec); Serial.print(" - ");
  Serial.println(ctime(&tv->tv_sec));
  delay(100);
}
/****************************************************************************/
void getXchangeRate()
/****************************************************************************/
{
  // It is silly to fetch all night long while sleeping.  Just pause it
  //  for a while.  It will stop with the hour shown at night and start
  //  with the hour shown in the morning.  Right now, you can't stop after
  //  midnight.  I have the code for that somewhere.  Will find it soon.

  //   return;  // Uncomment to completely deactivate XRate fetching.

  bool bFetchOK;
  unsigned long ulEntryUTC = UTC;
  // If bfirstXRatePass is set to true, here, then the first fetch will be
  //  skipped and the first attempt to fetch an XRate will be the second
  //  entry to this routine.  If set to false, it will try to fetch on
  //  the first entry here.
  static bool bfirstXRatePass = false;  // False = fetch on the first entry
  // True  = delay one 10 minute interval
  // Now variable weekday/weekend
  static unsigned long ulXRateFetchInterval;
  static int iLastEntryMin = -1;
  static unsigned int XR_Month, XR_DOM, XR_Year, XR_JDay;

  //  Serial.printf("iLastEntryMin %i, iCurrMinute %i\r\n",
  //                iLastEntryMin, iCurrMinute);
  if (iLastEntryMin == iCurrMinute) return;  // Once per 10 minute gate.
  iLastEntryMin = iCurrMinute;

  int currFontSize = ofr.getFontSize();
  ofr.setFontSize(38);

  //  Serial.printf("%02i:%02i:%02i Entering X Rate fetch. ",
  //                iCurrHour, iCurrMinute, iCurrSecond);

  if (bfirstXRatePass) {
    bfirstXRatePass = false;  // Define this as false in Definitions.h to
    //                            not skip the first cycle.
    // The reason for delaying one cycle is to reduce the fetch count while
    //  I am working on the code.  If it is fetched every upload, it will
    //  rip through the 250 free fetches really quickly.  So, I wait until
    //  the next fetch window.  By then, I have either recompiled or
    //  decided to let it run and want to do the fetch.
    //  It could be removed if you want.
    Serial.println("\r\nThis is the initial pass, not fetching X Rate "
                   "this time.\r\n");
  } else {

    //#define fetchStartHour 10  // Will start to fetch at 10:00
    //#define fetchStopHour  23  // Will not fetch starting at 23:00

    static bool doFetch;
    if (fetchStartHour > fetchStopHour)
      doFetch = (iCurrHour >= fetchStartHour) || (iCurrHour < fetchStopHour);
    else
      doFetch = (iCurrHour >= fetchStartHour) && (iCurrHour < fetchStopHour);

    // Testing - Force a fetch even during "paused" hours.
    //    doFetch = true;

    if (!doFetch) {
      Serial.printf("XRate fetching paused till %i:00.\r\n", fetchStartHour);
      return;
    }
    if (iCurrDOW == 0 || iCurrDOW == 6)
      ulXRateFetchInterval = 21600;  // Slow down to 6 hours on weekends.
    else
      ulXRateFetchInterval = 10800;  // 3 hour interval during the week.
    // Let's get the seconds until the next fetch into XR_UTC.
    // Serial.printf("ulXRateFetchInterval %lu, ulEntryUTC %lu,
    //               ulLastXRateFetchEpoch %lu\r\n",
    //               ulXRateFetchInterval, ulEntryUTC, ulLastXRateFetchEpoch);
    XR_UTC = ulXRateFetchInterval - (ulEntryUTC - ulLastXRateFetchEpoch);
    // Then add in UTC plus local time offset but the seconds till fetch.

    //    Serial.printf("XR_UTC %lu, UTC %lu, iBotOffset %lu\r\n",
    //                  XR_UTC, UTC, iBotOffset);

    XR_UTC = UTC + iBotOffset + XR_UTC;

    // Serial.printf("XR_UTC %lu, UTC %lu, iBotOffset %lu\r\n",
    //               XR_UTC, UTC, iBotOffset);

    if (XR_UTC > 100000)  // Don't show if not initialized (still 0).
      Serial.print(localtime(&XR_UTC), "Next fetch: %c\r\n");
      
    time(&UTC);
    // Been too long? Data old?  This also runs on program startup.
    if (UTC > ulLastXRateFetchEpoch + ulResetXRateTime) {
      Serial.print(localtime(&UTC),
                   "\r\n%a %m-%d-%Y %T %Z - Stale XRate cleared.\r\n");
      // Data too old. Clear it. Zero it (will make it disappear)
      fPHP_Rate = 0.;
    }
    // 1 minute of slack added, just to be sure.
    // This is the once per 3/6 hour (currently) gate.
    // The following might be backwards, the 60 might need to be after the >.
    // Or, it might should be:
    if ((ulEntryUTC - ulLastXRateFetchEpoch) >= ulXRateFetchInterval) {
      //      if ((ulEntryUTC - ulLastXRateFetchEpoch + 60) >
      //          ulXRateFetchInterval) {
      Serial.println("I will try an X Rate fetch now.");
      // Try to get the data from apilayer server.
      bFetchOK = xRateWorker(1);
      if (!bFetchOK) {
        Serial.println("First X Rate fetch failed, waiting 5 seconds "
                       "for a retry. (try 2)\r\n");
        delay(5000);  // Wait 5 seconds and try a second time.
        // Try to get the data from apilayer server again.
        bFetchOK = xRateWorker(2);
      }
      if (!bFetchOK) {
        Serial.println("Second X Rate fetch failed, waiting 5 seconds "
                       "for final retry. (try 3)\r\n");
        delay(5000);  // Wait 5 seconds and try a second time.
        // Try to get the data from apilayer server for the last time.
        bFetchOK = xRateWorker(3);
      }
      if (bFetchOK) {
        // Got a live one, update timer.
        ulLastXRateFetchEpoch = ulEntryUTC;
        Serial.println("XRate fetch successful.");
        int yPos = 35;
        clockSprite.fillSprite(RGB565(0, 80, 0));
        // Foreground White, Background Dark Green
        ofr.setFontColor(TFT_WHITE, RGB565(0, 80, 0));
        ofr.setCursor(iXCenter, yPos);      ofr.cprintf("Exchange Rate");
        ofr.setCursor(iXCenter, yPos + 70); ofr.cprintf("fetch succeeded!");
        sTemp = sFetchesLeft + "/250 left";  // i.e.,  100/250 left.
        ofr.setCursor(iXCenter, yPos + 140);
        ofr.cprintf(sTemp.c_str());  // ofr.cprintf("Sweet!!");
        clockSprite.pushSprite(0, 0);
        bNewRate = true;             // Resize sooner than next minute.
        // Screen stolen. If graph screen shown, refresh it.
        refreshGraph = true;
        for (i = 0; i < 2; i++) {
          tft.invertDisplay(true); delay(200);
          tft.invertDisplay(false); delay(200);
          tft.invertDisplay(true); delay(200);
          tft.invertDisplay(false); delay(200);
        }
        delay(3000);

        // The fetch was successful.  The new value is in fPHP_Rate.
        /* Now, calculate today's day, month and year.  Then get the
            Julian day number from that and record it if it is different
            than the last element of the array after pushing down
            everything in the array already.  That way, we will only
            record one value per day.
        */
        time(&UTC);
        timeinfo = localtime(&workTime);
        XR_Month = timeinfo->tm_mon + 1;
        XR_DOM = timeinfo->tm_mday;
        XR_Year = timeinfo->tm_year + 1900;
        XR_JDay = calcJulian(XR_Year, XR_Month, XR_DOM);
        // Note to self.  Use the returned Julian date to feed the
        //                Gregorian conversion and compare with the
        //                XR_ values to be sure all is in sync.
        JulianToGregorian(XR_JDay);
        Serial.printf("Today's Julian Day is %u (%02i/%02i/%i)\r\n",
                      XR_JDay, XRateMon, XRateDay, XRateYr);

        if (XR_JDay != XRateJulian[XRateHistLen - 1]) {
          Serial.println("New Julian day, update XRate history.");
          // New Day. Make room for the new entry.
          // Scoot all of the date entries left by 1.
          for (i = 0; i < XRateHistLen - 1; i++)
            XRateJulian[i] = XRateJulian[i + 1];
          // Put in the new Julian day number
          XRateJulian[XRateHistLen - 1] = XR_JDay;

          // New Day. Make room for the new XRate value.
          for (i = 0; i < XRateHistLen - 1; i++)
            XRateHist[i] = XRateHist[i + 1];
          // Put in the XRate for today.
          XRateHist[XRateHistLen - 1] = fPHP_Rate;

          countXRate(false, true);  // Count and print the total non-zero.

          Serial.println("Saving XRate history in NVS flash entries. (New)");
          preferences.begin("TripleTime", RW_MODE);
          preferences.putBytes("XRateHist", XRateHist, sizeof(XRateHist));
          preferences.putBytes("XRateJulian", XRateJulian,
                               sizeof(XRateJulian));
          Serial.printf("There are %i entries left in preferences "
                        "storage.\r\n",
                        preferences.freeEntries());
          preferences.end();
        } else {
          // Serial.println("Already have an XRate value for today.");
          // Serial.printf("Old Close %.2f, Older Close %.2f, New %.2f\r\n",
          //               XRateHist[XRateHistLen - 1],
          //               XRateHist[XRateHistLen - 2],
          //               fPHP_Rate);
          // Put in the latest XRate for today.
          if (fabs(XRateHist[XRateHistLen - 1] - fPHP_Rate) > .15) {
            Serial.println("Saving XRate history in NVS flash entries. "
                           "(Change)");
            preferences.begin("TripleTime", RW_MODE);
            preferences.putBytes("XRateHist", XRateHist, sizeof(XRateHist));
            preferences.putBytes("XRateJulian", XRateJulian,
                                 sizeof(XRateJulian));
            Serial.printf("There are %i entries left in preferences "
                          "storage.\r\n",
                          preferences.freeEntries());
            preferences.end();
          }
          Serial.println("Updating to latest XRate value for today.");
          // Update value in RAM array.
          XRateHist[XRateHistLen - 1] = fPHP_Rate;
          // Count but don't show all non-zero XRate history entries.
          countXRate(false, true);  // Count and print the total non-zero.
        }
        Serial.printf("Old Close %.2f, Older Close %.2f, New %.2f\r\n",
                      XRateHist[XRateHistLen - 1],
                      XRateHist[XRateHistLen - 2],
                      fPHP_Rate);

      } else {
        Serial.println("\r\nThird X Rate fetch failed. Retry in "
                       "10 minutes.\r\n");
        int yPos = 35;
        clockSprite.fillSprite(DarkerRed);
        // Foreground color, Background color
        ofr.setFontColor(TFT_YELLOW, DarkerRed);
        ofr.setCursor(iXCenter, yPos);       ofr.cprintf("XRate fetch");
        ofr.setCursor(iXCenter, yPos + 70);  ofr.cprintf("failed. Retry");
        ofr.setCursor(iXCenter, yPos + 140); ofr.cprintf("in 10 minutes.");
        clockSprite.pushSprite(0, 0);
        // Screen stolen from paused graph screen. Enable updating.
        refreshGraph = true;
        for (i = 0; i < 2; i++) {
          tft.invertDisplay(true); delay(200);
          tft.invertDisplay(false); delay(200);
        }
        delay(2000);
      }
    }
  }

  ofr.setFontSize(currFontSize);
  if (iWhichClock == iDigitalClock)  // Restart the scroll with new length
    tft.fillRect(0, 0, tft.width(), iScrollSpriteH, DarkBlue);
  //  Serial.println();
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
  // To get it into the loop. This is the code for API key exhausted.
  int iHttpResponseCode = 429;

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
      Serial.printf("Fetching exchange rate with API key %i with the "
                    "following URI:\r\n%s\r\n",
                    iUseAPIkey + 1, sServerPath.c_str());

      sprintf(cCharWork, "Try %i, Key %i", iTry, iUseAPIkey + 1);
      Serial.printf("%02i:%02i:%02i ",
                    iCurrHour, iCurrMinute, iCurrSecond);
      Serial.println(cCharWork);

      int yPos = 35;
      clockSprite.fillSprite(RGB565(0, 80, 0));
      // Foreground color, Background color
      ofr.setFontColor(TFT_WHITE, RGB565(0, 80, 0));
      ofr.setCursor(iXCenter, yPos);       ofr.cprintf("Attempting to");
      ofr.setCursor(iXCenter, yPos + 70);  ofr.cprintf("fetch X rate.");
      ofr.setCursor(iXCenter, yPos + 140); ofr.cprintf(cCharWork);
      clockSprite.pushSprite(0, 0);
      // Screen stolen from paused graph screen. Enable updating.
      refreshGraph = true;

      http.setTimeout(30000);         // Yeah, maybe...
      http.setConnectTimeout(30000);  // Yeah, maybe...
      Serial.printf("%lu - Sending message to X Rate server.\r\n", millis());
      http.begin(sServerPath.c_str());
      Serial.printf("%lu - Message sent, issuing http.GET.\r\n", millis());

      /* The possible headers for rates are (as of 2/22/25):
          X-RateLimit-Limit-Day: nnnn
          X-RateLimit-Limit-Month: nnnn
          X-RateLimit-Remaining-Day: nnnn
          X-RateLimit-Remaining-Month: nnnn
      */
      const char *headerKeys[] = {"x-ratelimit-remaining-month"};
      const size_t headerKeysCount = sizeof(headerKeys) /
                                     sizeof(headerKeys[0]);
      http.collectHeaders(headerKeys, headerKeysCount);

      iHttpResponseCode = http.GET();
      // Serial.printf("%lu - Message sent to X Rate server.\r\n", millis());
      Serial.printf("%lu - http.GET response code: %i\r\n",
                    millis(), iHttpResponseCode);
      // 429 means that you have run out of uses of the key for this month.
      // It is time to use a different key.  Oh, you did get two, right?
      // If you get a -11 (timeout) code back, it may use up keys quickly so
      //  it is good to have a backup plan -- another key.  iUseAPIkey points
      //  to the currently active one.  Step it up on a 429 to start a
      //  fresh key.
      if (iHttpResponseCode == 429) iUseAPIkey++;
    }

    if (iHttpResponseCode == HTTPC_ERROR_CONNECTION_REFUSED)
    {
      Serial.println("http.GET request connection refused.");
      return false;

    }
    else if (iHttpResponseCode == 200) {  // 200 is goodness!

      // Print all headers
      int headerCount = http.headers();
      //      Serial.printf("\r\n%i HTTP header(s) sought:\r\n",
      //                    headerCount);
      for (int i = 0; i < headerCount; i++) {
        //        Serial.println(http.header((size_t) 0));
        sFetchesLeft = http.header((size_t) 0);
        sFetchesLeft = sFetchesLeft.substring(0,
                                              sFetchesLeft.lastIndexOf(":"));
        //        Serial.printf("%s: %s gave %s\r\n",
        //                      http.headerName(i).c_str(),
        //                      http.header(i).c_str(), sFetchesLeft);
        if (sFetchesLeft == "")
        {
          Serial.println("No header data received for calls left value.");
          sFetchesLeft = "n/a";
        }
        else
          Serial.printf("There are %s XRate calls left for this key"
                        " this month.\r\n", sFetchesLeft/*, sTemp.toInt()*/);
      }
      // Now, get the payload (the actual data I asked for).
      String payload = http.getString();
      http.end();  // Free resources
      Serial.printf("%02i:%02i:%02i Returned data:",
                    iCurrHour, iCurrMinute, iCurrSecond);
      Serial.print(payload);
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
        return bPktValidity;
      }
      unsigned long packetTime = doc["timestamp"];
      //      long int os = iBotOffset;
#if defined CONFIG4MIKE
      // Assuming, for now, that the bottom clock is local time.
      long int os = iBotOffset;
#else
      // Assuming, for now, that the bottom clock is local time.
      long int os = iTopOffset;
#endif
      // Really!  Get rid of this.  Convert to what's used elsewhere
      //  in this pgm. Then, remove the #include for timeLib.h.
      //  Not done yet as of 6/24/23.
      tmElements_t tm;
      packetTime += os;
      breakTime(packetTime, tm);
      Serial.printf("Currency Conversion packet time: "
                    "%02i/%02i/%02i %02i:%02i:%02i\r\n",
                    tm.Month, tm.Day, tm.Year + 1970,
                    tm.Hour, tm.Minute, tm.Second);
      fPHP_Rate = doc["rates"]["PHP"];
      time(&UTC);
      ulLastXRateFetchEpoch = UTC;

      Serial.printf("Current PHP Conversion rate %.2f\r\n", fPHP_Rate);
#if defined CONFIG4MIKE
      sprintf(caReadingTime, "As of %02i:%02i %s, ",
              iCurrHour, iCurrMinute, cBotDST);
#else
      sprintf(caReadingTime, "As of %02i:%02i %s, ",
              iCurrHour, iCurrMinute, cTopDST);
#endif
    } else {
      bPktValidity = false;
      if (iHttpResponseCode == -11) {
        Serial.printf("%lu - Bad HTTP return code %i. "
                      "Exchange rate packet ignored.\r\n",
                      millis(), iHttpResponseCode );
        Serial.println("Probably timeout condition.");
      }
      if (iHttpResponseCode == -1)
        Serial.println("Looks like we could not connect to the server.");
    }
  } else {
    Serial.println("WiFi not available now. No data fetch possible");
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
  configTime(0, 0, "oceania.pool.ntp.org", "time.nist.gov");
  setenv("TZ", cZulu, 1); tzset();
  displayW_Header("Waiting for right time");
  //  Serial.println("Waiting for correct time...");

  //  Serial.printf("Incoming epoch: %lu\r\n", workTime);
  strftime(cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
  iYear = atoi(cCharWork);
  //  Serial.printf("0 iYear %i\r\n", iYear);
  int iLooper = 0;
  while (iYear < 2024) {
    time(&UTC);
    strftime (cCharWork, 100, "%Y", localtime(&UTC));
    iYear = atoi(cCharWork);
    //    Serial.printf("1 iYear %i\r\n", iYear);
    Serial.println(localtime(&UTC), "UTC %a %m-%d-%Y %T");
    if (iLooper++ > 30) {
      Serial.println("Cannot get time set. Rebooting.");
      ESP.restart();
    }
    delay(2000);
  }
  displayW_Header("Get zone offsets");
  //  Serial.println("\r\nDetermining zone offsets.");
  deduceOffsets();
  time(&UTC);
#if defined CONFIG4MIKE
  // Assuming, for now, that the bottom clock is local time.
  workTime = UTC + iBotOffset;
#else
  // Assuming, for now, that the bottom clock is local time.
  workTime = UTC + iTopOffset;
#endif
}
/****************************************************************************/
void allocateSprites()
/****************************************************************************/
{
  int *a = 0;

  iRadius = min(tft.width(), tft.height()) / 2 - 1;
  // This must be allocated in PSRAM since ESP folks screwed up allocation.
  //  There is a 32 bit and an 8 bit allocation but no 16 bit so you get
  //  double what you need and it blows the stack!

  // Leave room for 1st sprite
  a = (int*)clockSprite.createSprite(tft.width(), tft.height());
  if (a == 0) {
    Serial.println("clockSprite creation failed.  Cannot continue.");
    spriteAllocError("clockSprite");
    while (1) ArduinoOTA.handle();
  }
  //  Serial.printf("createclockSprite returned: %p\r\n", a);

  // 1st sprite for scrolling info
  a = (int*)scrollSprite.createSprite(iScrollSpriteW, iScrollSpriteH);
  if (a == 0) {
    Serial.println("scrollSprite creation failed.  Cannot continue.");
    spriteAllocError("scrollSprite");
    while (1) ArduinoOTA.handle();
  }
  //  Serial.printf("createscrollSprite returned: %p\r\n", a);

  a = (int*)digitalSprite.createSprite(tft.width(), tft.height());
  if (a == 0) {
    Serial.println("digitalSprite creation failed.  Cannot continue.");
    spriteAllocError("digitalSprite");
    while (1) ArduinoOTA.handle();
  }
  //  Serial.printf("createDigitalSprite returned: %p\r\n", a);

  scrollSprite.fillSprite(DarkBlue);
  scrollSprite.setTextColor(TFT_WHITE, DarkBlue);

  a = (int*)XRateSprite.createSprite(tft.width(), tft.height());
  if (a == 0) {
    Serial.println("XRateSprite creation failed.  Cannot continue.");
    spriteAllocError("XRateSprite");
    while (1) ArduinoOTA.handle();
  }
  //  Serial.printf("createXRateGraphSprite returned: %p\r\n", a);

  Serial.printf("Analog clock radius is %i\r\n", iRadius);
}
/***************************************************************************/
void spriteAllocError(String which)
/***************************************************************************/
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  txtLen = tft.textWidth("Sprite Allocation");
  tft.drawString("Sprite Allocation", iXCenter - txtLen / 2, iDisplayLine1);

  txtLen = tft.textWidth(which + " failed");
  tft.drawString(which + " failed", iXCenter - txtLen / 2, iDisplayLine3);

  txtLen = tft.textWidth("Got PSRAM?");
  tft.drawString("Got PSRAM?", iXCenter - txtLen / 2, iDisplayLine5);

  txtLen = tft.textWidth("OTA is Available");
  tft.drawString("OTA is Available",
                 iXCenter - txtLen / 2, iDisplayLine5 + 40);

  while (1) ArduinoOTA.handle();
}
/***************************************************************************/
void drawGradientLine(TFT_eSprite *targetSprite,
                      int x1, int y1, int x2, int y2,
                      int blendLvls, uint16_t colorStart, uint16_t colorEnd)
/***************************************************************************/
{
  static int x, y, steps;

  // The 6th value (steps) changes the look of the gradient line.
  //  If it is 0, the gradient is completed along the length of the
  //  line no matter how long or short.  If there is a value here, its
  //  value is used for all lines. This gives a more regular look
  //  to the gradient lines.

  // steps is the length of the line in pixels.
  steps = abs(x2 - x1) > abs(y2 - y1) ? abs(x2 - x1) : abs(y2 - y1);
  // if blandLvls came in as 0, use the line pixel length for step count
  if (blendLvls == 0) blendLvls = steps;
  if (blendLvls < steps) {
    Serial.printf("BlendLvls (%i px) is less than line length "
                  "(%i px). Please correct and recompile.\r\n",
                  blendLvls, steps);
  }
  // Serial.printf("x1 % i, y1 % i, x2 % i, y2 % i, "
  //               "steps % i, blendLvls % i\r\n",
  //               x1, y2, x2, y1, steps, blendLvls);
  x = x1; y = y1;
  for (int i = 0; i <= steps; i++) {
    pctBlend = (float)i / (float)blendLvls;
    blendedColor = alphaBlend((uint8_t)(pctBlend * 255),
                              colorStart, colorEnd);
    targetSprite->drawPixel(x, y, blendedColor);

    if ((x1 - x2) == 0) y2 > y1 ? y++ : y--;  // Am I drawing horizontal or
    if ((y1 - y2) == 0) x2 > x1 ? x++ : x--;  // vertical. This decides.

  }
}
/***************************************************************************/
uint16_t alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc)
/***************************************************************************/
{
  uint32_t rxb = bgc & 0xF81F;
  rxb += ((fgc & 0xF81F) - rxb) * (alpha >> 2) >> 6;
  uint32_t xgx = bgc & 0x07E0;
  xgx += ((fgc & 0x07E0) - xgx) * alpha >> 8;
  return (rxb & 0xF81F) | (xgx & 0x07E0);
}
/***************************************************************************/
void drawArrowLine(TFT_eSPI* targetPallete,  // where line and arrow goes.
                   // Line start and end x & y
                   int x0, int y0, int x1, int y1,
                   // Line color.
                   uint16_t lineColor,
                   // Length of the little winglets to make the arrow.
                   int arrowLength,
                   // Angle away from the line for the two winglets.
                   float arrowAngle,  // 30º (PI / 6) looks good.
                   // Color of the 2 or 3 arrow builder lines.
                   uint16_t arrowColor,
                   // Do or do not draw back (3rd) line on arrow.
                   bool drawBackLine,
                   // Do or do not fill in the triangle with arrow color.
                   bool fillTheTriangle)
/***************************************************************************/
{
  static float angle;
  static int x2, y2, x3, y3;

  // Draw main line
  targetPallete->drawLine(x0, y0, x1, y1, lineColor);
  // Determine the angle of the original line.
  angle = atan2(y1 - y0, x1 - x0);

  // Calculate arrowhead lines length and angles
  // The arrowhead lines angles are offset by the original line angle.
  x2 = x1 - arrowLength * cos(angle - arrowAngle);
  y2 = y1 - arrowLength * sin(angle - arrowAngle);
  x3 = x1 - arrowLength * cos(angle + arrowAngle);
  y3 = y1 - arrowLength * sin(angle + arrowAngle);

  if (fillTheTriangle) {
    targetPallete->fillTriangle(x1, y1, x2, y2, x3, y3, arrowColor);
  } else {
    // Draw arrowhead
    targetPallete->drawLine(x1, y1, x2, y2, arrowColor);
    targetPallete->drawLine(x1, y1, x3, y3, arrowColor);
    // Do you want the back line (3rd line) to complete the arrow?
    if (drawBackLine) targetPallete->drawLine(x2, y2, x3, y3, arrowColor);
    // Do you want the triangle filled with the triangle line color?
  }
}

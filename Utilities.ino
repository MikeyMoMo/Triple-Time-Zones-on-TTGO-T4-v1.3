/***************************************************************************/
void decodeAC_Bits()
/***************************************************************************/
{
//  for (int i = 0; i < 5; i++)
//    Serial.printf("i %x, pick %x, result %i\r\n", i, 0x1 << i, acCustom & 0x1 << i);

  if (acCustom & 0x1 << showShortTimeBit) {  // See if short time requested
//    Serial.println("Turning on showShortTime");
    showShortTime = true;
  } else {
//    Serial.println("Turning off showShortTime");
    showShortTime = false;
  }
//  Serial.println(acCustom & 0x1 << showLongTimeBit);  // if showlong bit it on, remember.
  if (acCustom & 0x1 << showLongTimeBit) {
//    Serial.println("Turning on showLongTime, off showShortTime");
    showLongTime = true;
    showShortTime = false;  // Override short time bit
  } else {
//    Serial.println("Turning off showLongTime");
    showLongTime = false;
  }

  // This version never worked.  Don't know why.
  //unsigned int showShortTimeBit = 0x1;
  //unsigned int showLongTimeBit = 0x2;
  //unsigned int showSecondHandBit = 0x4;

  // This is the number of bits to shift before compare.  It works.
  //unsigned int showShortTimeBit = 0;
  //unsigned int showLongTimeBit = 1;
  //unsigned int showSecondHandBit = 2;

//  Serial.println(acCustom & 0x1 << showSecondHandBit);
  if (acCustom & 0x1 << showSecondHandBit)
    showSecondHand = true;
  else
    showSecondHand = false;
//  Serial.printf("acCustom %i, showShortTime %s, showLongTime %s, showSecondHand %s\r\n",
//                acCustom,
//                showShortTime ? "true" : "false",
//                showLongTime ? "true" : "false",
//                showSecondHand ? "true" : "false");
 }
/***************************************************************************/
void initDisplay()
/***************************************************************************/
{
  tft.init(); // Initialize the screen.
  clockSprite.setTextColor(TFT_WHITE);
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

}
/***************************************************************************/
void displayW_Header(int y, String what)
/***************************************************************************/
{
  showDisplayHeader();
  ofr.setCursor(xCenter, y); ofr.cprintf(what.c_str());
  //  ofr.display();
  clockSprite.pushSprite(0, 0);
}
/***************************************************************************/
void showDisplayHeader()
/***************************************************************************/
{
  clockSprite.fillSprite(TFT_BLACK);
  clockSprite.setTextColor(TFT_WHITE);
  ofr.setCursor(xCenter, displayLine1);
  ofr.cprintf("Dual NTP Time");
  ofr.setCursor(xCenter, displayLine2 + 32);
  ofr.cprintf(myVersion);
  clockSprite.pushSprite(0, 0);
}
/***************************************************************************/
void fetchOffsets()
/***************************************************************************/
{
  // Only update the TZ hourly to get the current offset and DST setting.
  // That's really more than necessary but not such a big deal.
  // I was doing it every second.  However, it only takes 32 ms.
  // It totally saves me from having to figure out DST for myself.
  // I did it once and it was NOT pretty!

  static bool firstTime = true;
  startMillis = millis();

  setenv("TZ", TopTZ, 1); tzset();
  Serial.print(TopCityname);
  time(&now);
  strftime (charWork, 100, "%Y", localtime(&now));
  iYear = atoi(charWork);
  Serial.println(localtime(&now), " initial set %a, %d-%m-%Y %T %Z %z");
  while (iYear < 2023) {
    delay(1000);
    time(&now);
    strftime (charWork, 100, "%Y", localtime(&now));
    iYear = atoi(charWork);
    Serial.print(TopCityname);
    Serial.println(localtime(&now), " waiting %a, %d-%m-%Y %T %Z %z");
  }
  Serial.println(localtime(&now), " after waiting %a, %d-%m-%Y %T %Z %z");

  strftime (TopDST, 10, "%Z", localtime(&now));
  strftime (charWork, 100, "%z", localtime(&now));
  tempOffset = atoi(charWork);
  TopOffset = (tempOffset / 100) * 3600 + tempOffset % 100 * 60;
  Serial.print(TopCityname);
  Serial.printf(" Offset = %+i\r\n", TopOffset);

  setenv("TZ", BotTZ, 1); tzset();
  Serial.print(BotCityname);
  time(&now);
  strftime (charWork, 100, "%Y", localtime(&now));
  iYear = atoi(charWork);
  Serial.println(localtime(&now), " initial set %a, %d-%m-%Y %T %Z %z");
  while (iYear < 2023) {
    delay(1000);
    time(&now);
    strftime (charWork, 100, "%Y", localtime(&now));
    iYear = atoi(charWork);
    Serial.print(BotCityname);
    Serial.println(localtime(&now), " waiting %a, %d-%m-%Y %T %Z %z");
  }
  Serial.println(localtime(&now), " after waiting %a, %d-%m-%Y %T %Z %z");
  strftime (BotDST, 10, "%Z", localtime(&now));  // Characters of time zone and DST indicator.
  strftime (charWork, 100, "%z", localtime(&now));  // Seconds offset from UTC.
  tempOffset = atoi(charWork);
  BotOffset = (tempOffset / 100) * 3600 + tempOffset % 100 * 60;
  Serial.print(BotCityname);
  Serial.printf(" Offset = %+i\r\n", BotOffset);

  // This must be done and must be last.  The local time(s) are based off of this.  They are
  //  not kept separately, only created when needed by adding the offset to UTC.
  setenv("TZ", Zulu, 1); tzset();
  strftime (charWork, 100, "%Y", localtime(&now));
  time(&UTC);
  iYear = atoi(charWork);
  Serial.println(localtime(&UTC), "Zulu initial set %a, %d-%m-%Y %T %Z %z");
  while (iYear < 2023) {
    delay(1000);
    time(&UTC);  // was now.  Not sure if it matters.  Maybe only on Dec 31/Jan 1st.
    strftime (charWork, 100, "%Y", localtime(&UTC));
    iYear = atoi(charWork);
    Serial.println(localtime(&UTC), "Zulu waiting %a, %d-%m-%Y %T %Z %z");
  }
  Serial.println(localtime(&UTC), " after waiting %a, %d-%m-%Y %T %Z %z");

  // It has been taking about 32-33ms to run this routine.
  //  Serial.printf("Total time for computing offsets %lu ms.\r\n", millis() - startMillis);
}
/***************************************************************************/
void  startWifi()
/***************************************************************************/
{
  WiFi.begin(ssid, wifipw);
  Serial.println("Connecting Wifi");
  int waitCt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if (waitCt++ > 20) ESP.restart();
  }
  Serial.println("Connected!");
  Serial.print("Host:\t");       Serial.println(WiFi.SSID());
  Serial.print("IP Address:\t"); Serial.println(WiFi.localIP());
  Serial.print("Wifi RSSI =\t"); Serial.println(WiFi.RSSI());
  String myMACAddress = WiFi.macAddress();
  Serial.print("MAC Address =\t"); Serial.println(myMACAddress);
  //           1111111
  // 01234567890123456
  // 84:CC:A8:47:53:98
  String subS = myMACAddress.substring(   0,  2) + myMACAddress.substring(3,  5)
                + myMACAddress.substring( 6,  8) + myMACAddress.substring(9, 11)
                + myMACAddress.substring(12, 14) + myMACAddress.substring(15);
  Serial.print("Scrubbed MAC:\t");
  Serial.println(subS);  // String of MAC address without the ":" characters.
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
}
/***************************************************************************/
void Hello()  // Testing only.  Signon message.
//               Just used to be sure the sprite code was working.
/***************************************************************************/
{
  //  clockSprite.fillSprite(TFT_BLACK);
  //  clockSprite.setTextColor(TFT_WHITE, TFT_BLACK); // Do not plot the background colour
  //  String asdf = "Triple Time v" + String(myVersion);
  //  ofr.setCursor(xCenter, displayLine3 + 42);
  //  ofr.cprintf(asdf.c_str());
  //  Serial.print("Hello from version "); Serial.println(myVersion);
  //  clockSprite.pushSprite(0, 0);
}

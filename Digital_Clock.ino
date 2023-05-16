/***************************************************************************/
void updateDigitalDisplay()
/***************************************************************************/
{
  startMillis = millis();
  time(&UTC);

  scrollIt(OKscrollUpdate);
  //  ofr.setDrawer(clockSprite);  // This is set in scrollIt
  clockSprite.fillSprite(TFT_BLACK);
  scrollIt(NOscrollUpdate);
  clockSprite.fillRect(0,  0, tft.width(), 90, RGB565(80, 0, 0)); // Dark Red
  scrollIt(NOscrollUpdate);
  clockSprite.fillRect(0, 90, tft.width(), 90, RGB565(0, 80, 0));  // Dark Green
  scrollIt(NOscrollUpdate);
  ofr.setFontSize(28);
  scrollIt(NOscrollUpdate);
  now = UTC + TopOffset;
  scrollIt(NOscrollUpdate);
  ofr.setCursor(  5, displayLine2); ofr.printf(TopCityname);
  scrollIt(NOscrollUpdate);
  ofr.setCursor(155, displayLine2);
  scrollIt(NOscrollUpdate);
  strftime(charWork, 100, "%A", localtime(&now)); ofr.printf(charWork);
  scrollIt(NOscrollUpdate);
  ofr.setCursor(  5, displayLine3);
  scrollIt(NOscrollUpdate);
  strftime(charWork, 100, "%x %X ", localtime(&now)); ofr.printf(charWork);
  scrollIt(NOscrollUpdate);
  ofr.printf(TopDST);
  scrollIt(NOscrollUpdate);

  ofr.setDrawer(clockSprite);
  now = UTC + BotOffset;
  scrollIt(NOscrollUpdate);
  ofr.setCursor(  5, displayLine4); ofr.printf(BotCityname);
  scrollIt(NOscrollUpdate);
  ofr.setCursor(155, displayLine4);
  scrollIt(NOscrollUpdate);
  strftime(charWork, 100, "%A", localtime(&now)); ofr.printf(charWork);
  scrollIt(NOscrollUpdate);
  ofr.setCursor(  5, displayLine5);
  scrollIt(NOscrollUpdate);
  strftime(charWork, 100, "%x %X ", localtime(&now)); ofr.printf(charWork);
  scrollIt(NOscrollUpdate);
  ofr.printf(BotDST);
  scrollIt(NOscrollUpdate);
  clockSprite.pushSprite(0, 42);
  scrollIt(OKscrollUpdate);
}
/***************************************************************************/
void scrollIt(bool OKscrollUpdate)
/***************************************************************************/
{
  static unsigned int lastScrollms = 0;            // in ms. It's the counter for when to scroll again.
  static const unsigned int scrollTime = 20;       // in ms. It's the time for when to scroll again.
  static const int scrollAmt = 1;                  // in screen pixels. How many pixels to scroll.
  static const int scrollMax = tft.width() * 2.25;  // in screen pixels. How far along to repeat the text.
  static const int insertPixel = 370;              // Where on the line to put the new information.
  static int scrollCt = scrollMax + 1;  // Force a text fill on the first pass.
  static int lastMinute = -1;
  static int trackTimeDate = insertPixel;  // Where to drop in the live update for time/date.

  if ((millis() - lastScrollms) > scrollTime) {  // If it is time to scroll...
    lastScrollms = millis();                     // Reset time for next scroll
    scrollSprite.scroll(-scrollAmt, 0);          // scroll scrollSprite 1 pixel left, 0 up/down
    trackTimeDate -= scrollAmt;                  // Track head of data position
    //    Serial.printf("Scrolled. trackTimeDate now %i\r\n", trackTimeDate);
    // If the minute has changed, update the scroll.
    // For now, force it in.
    strftime(charWork, 100, "%M", localtime(&UTC));
    if (lastMinute != atoi(charWork)) {
      lastMinute = atoi(charWork);
      // Clear only from where we will write to the end of the sprite.
      scrollSprite.fillRect(trackTimeDate, 0, scrollSpriteW, scrollSpriteH, TFT_BLACK);
      ofr.setDrawer(scrollSprite);
      strftime(charWork, 100, "%A %x %R %Z  ", localtime(&UTC));
      ofr.setCursor(trackTimeDate, 0); ofr.printf(charWork);
      strftime(charWork, 100, "%M", localtime(&UTC));
      // Add on my ID.
      ofr.printf("Triple NTP Time "); ofr.printf(myVersion);
    }
    if (scrollCt++ > scrollMax && OKscrollUpdate) {
      ofr.setFontColor(TFT_WHITE, TFT_BLACK);
      ofr.setDrawer(scrollSprite);
      ofr.setCursor(insertPixel, 0);
      trackTimeDate = insertPixel;
      strftime(charWork, 100, "%A %x %R %Z  ", localtime(&UTC));
      ofr.printf(charWork);
      strftime(charWork, 100, "%M", localtime(&UTC));
      // Add on my ID.
      ofr.printf("Triple NTP Time "); ofr.printf(myVersion);
      scrollCt = 0;
    }
    scrollSprite.pushSprite(0, 0);
  }
  ofr.setDrawer(clockSprite);  // Set it back since we might be doing this mid-clockSprite update.
}

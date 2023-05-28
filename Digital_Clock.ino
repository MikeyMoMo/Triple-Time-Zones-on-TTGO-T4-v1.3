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
  //  clockSprite.fillRectHGradient(0,  0, tft.width(), 90, RGB565(180, 0, 0), RGB565(0, 0, 180));
  scrollIt(NOscrollUpdate);
  clockSprite.fillRect(0, 90, tft.width(), 90, RGB565(0, 80, 0));  // Dark Green
  //  clockSprite.fillRectHGradient(0, 90, tft.width(), 90, RGB565(0, 0, 180), RGB565(0, 180, 0));
  scrollIt(NOscrollUpdate);
  //  ofr.setFontSize(28);
  scrollIt(NOscrollUpdate);
  now = UTC + TopOffset;
  scrollIt(NOscrollUpdate);
  ofr.setCursor(  5, displayLine2); ofr.printf(TopCityname);
  scrollIt(NOscrollUpdate);
  ofr.setCursor(155, displayLine2);
  scrollIt(NOscrollUpdate);
  strftime(charWork, sizeof(charWork), "%A", localtime(&now)); ofr.printf(charWork);
  scrollIt(NOscrollUpdate);
  ofr.setCursor(  5, displayLine3);
  scrollIt(NOscrollUpdate);
  strftime(charWork, sizeof(charWork), "%x %X ", localtime(&now)); ofr.printf(charWork);
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
  strftime(charWork, sizeof(charWork), "%A", localtime(&now)); ofr.printf(charWork);
  scrollIt(NOscrollUpdate);
  ofr.setCursor(  5, displayLine5);
  scrollIt(NOscrollUpdate);
  strftime(charWork, sizeof(charWork), "%x %X ", localtime(&now)); ofr.printf(charWork);
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
#define ScrollMultiplier 2
  static unsigned int lastScrollms = 0;                         // in ms. It's the count for when to scroll again.
  static const unsigned int scrollTime = 20 * ScrollMultiplier; // in ms. It's the deadline time for when to scroll again.
  static const int scrollAmt = ScrollMultiplier;                // in screen pixels. How many pixels to scroll left.
  static const int scrollMax = 752 / ScrollMultiplier;          // in screen pixels. How far along to repeat the text.
  static const int insertPixel = 370;            // Where on the line to put the new information, after a gap.
  static int scrollCt = scrollMax + 1;           // Force a text fill on the first pass then it gets reset.
  static int lastMinute = -1;                    // Force a time update on the first pass then it gets reset.
  static int trackTimeDate = insertPixel;        // Where to drop in the live update for time/date.

  if ((millis() - lastScrollms) > scrollTime) {  // If it is time to scroll...
    lastScrollms = millis();                     // Reset time for next scroll
    scrollSprite.scroll(-scrollAmt, 0);          // scroll scrollSprite 1 pixel left, 0 up/down
    trackTimeDate -= scrollAmt;                  // Track head of data position
    //    Serial.printf("Scrolled. trackTimeDate now %i\r\n", trackTimeDate);
    // If the minute has changed, update the scroll.
    // For now, force it in.
    strftime(charWork, sizeof(charWork), "%M", localtime(&UTC));
    if (lastMinute != atoi(charWork)) {
      lastMinute = atoi(charWork);
      // Clear only from where we will write to the end of the sprite.
      scrollSprite.fillRect(trackTimeDate, 0, scrollSpriteW, scrollSpriteH, DarkBlue);
      ofr.setDrawer(scrollSprite);
      ofr.setFontColor(TFT_WHITE, DarkBlue);
      ofr.setBackgroundFillMethod(BgFillMethod::Block);
      strftime(charWork, sizeof(charWork), "%A %x %R %Z  ", localtime(&UTC));
      ofr.setCursor(trackTimeDate, 6); ofr.printf(charWork);
      strftime(charWork, sizeof(charWork), "%M", localtime(&UTC));
      // Add on my ID.
      ofr.printf("Triple NTP Time "); ofr.printf(myVersion);
    }
    if (scrollCt++ > scrollMax && OKscrollUpdate) {
      ofr.setFontColor(TFT_WHITE, TFT_BLACK);
      ofr.setDrawer(scrollSprite);
      ofr.setCursor(insertPixel, 6);
      ofr.setFontColor(TFT_WHITE, DarkBlue);
      trackTimeDate = insertPixel;
      strftime(charWork, sizeof(charWork), "%A %x %R %Z  ", localtime(&UTC));
      ofr.printf(charWork);
      strftime(charWork, sizeof(charWork), "%M", localtime(&UTC));
      // Add on my ID.
      ofr.printf("Triple NTP Time "); ofr.printf(myVersion);
      scrollCt = 0;
    }
    scrollSprite.pushSprite(0, 0);
  }
  // BgFillMethod is a struct in the OFR library.  Options are None, Minimum and Block.
  ofr.setBackgroundFillMethod(BgFillMethod::None);
  ofr.setDrawer(clockSprite);  // Set it back since we might be doing this mid-clockSprite update.
}

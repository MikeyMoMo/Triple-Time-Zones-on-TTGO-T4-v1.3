/***************************************************************************/
void updateDigitalDisplay()
/***************************************************************************/
{
  //  startMillis = millis();
  scrollIt(); time(&UTC);
  scrollIt(); clockSprite.fillSprite(TFT_BLACK);
  scrollIt(); clockSprite.fillRect(0,  0, tft.width(), 90, RGB565(80, 0, 0)); // Dark Red
  scrollIt(); clockSprite.fillRect(0, 90, tft.width(), 90, RGB565(0, 80, 0)); // Dark Green
  scrollIt(); workTime = UTC + TopOffset;
  scrollIt(); clockSprite.setCursor( 5, displayLine2 + 3);
  scrollIt(); clockSprite.print(TopCityname);
  scrollIt(); clockSprite.setCursor(155, displayLine2 + 3);
  scrollIt(); strftime(charWork, sizeof(charWork), "%A", localtime(&workTime));
  scrollIt(); clockSprite.print(charWork);
  scrollIt(); clockSprite.setCursor(  5, displayLine3 + 7);
  scrollIt(); strftime(charWork, sizeof(charWork), "%x %X ", localtime(&workTime));
  scrollIt(); clockSprite.print(charWork);
  scrollIt(); clockSprite.print(TopDST);

  scrollIt(); workTime = UTC + BotOffset;
  scrollIt(); clockSprite.setCursor(  5, displayLine4 + 3);
  scrollIt(); clockSprite.print(BotCityname);
  scrollIt(); clockSprite.setCursor(155, displayLine4 + 3);
  scrollIt(); strftime(charWork, sizeof(charWork), "%A", localtime(&workTime));
  scrollIt(); clockSprite.print(charWork);
  scrollIt(); clockSprite.setCursor(  5, displayLine5 + 7);
  scrollIt(); strftime(charWork, sizeof(charWork), "%x %X ", localtime(&workTime));
  scrollIt(); clockSprite.print(charWork);
  scrollIt(); clockSprite.print(BotDST);
  scrollIt(); clockSprite.pushSprite(0, scrollSpriteH);
  //  Serial.printf("Time totally through Digital_Clock %u\r\n", millis() - startMillis);
}
/***************************************************************************/
void scrollIt()
/***************************************************************************/
{
  static const int scrollAmt = 2;             // in screen pixels. How many pixels to scroll left.
  static const int spacer = 50;               // in screen pixels. Blank space.
  static unsigned int lastScrollms = 0;       // in ms. The count for when last scroll was done.
  static const unsigned int scrollTime = 30;  // in ms. The deadline interval for when to scroll again.
  static int msg1Head = tft.width(), txtWidth;
  static int prevMinute = -1, prevTxtWidth = -1, currTxtWidth;

  // I check every action done, above, but only scroll 50 times per second.  The other 49 calls,
  //  the routine just returns immediately.
  if ((millis() - lastScrollms) < scrollTime) return;  // If it is not time to scroll, go back.
  lastScrollms = millis();                     // Reset time for next scroll
  // Since I am not showing seconds on the UTC scroll, it does not need to be
  //  updated more often than once per minute at the time the minute changes.
  if ((prevMinute != currMinute) || newRate) {  // Text only needs to be changed each minute.
    prevMinute = currMinute;  // Remember last minute of scrolling.
    newRate = false;
    Serial.println(UTC);
    strftime(charWork, sizeof(charWork), "%A  %x  %R %Z  ", localtime(&UTC));
    if (PHP_Rate > 0.) {
      strcat(charWork, readingTime);
      sprintf(charWork2, " PHP%.2f/$  ", PHP_Rate);
      strcat (charWork, charWork2);
    }
    strcat(charWork, myName); strcat(charWork, myVersion);  // Now the scroll text is ready to show.
    // The idea here is to change the length of the sprite to match the new data.
    //  Most of it does not change but the day name surely does change.  By doing
    //  this, the same spacing is maintained between the end of sprite copy 1 and
    //  sprite copy 2 as the day changes and that is done by changing the length
    //  of the sprite.  Remember, there is only 1, you frequently see the same one
    //  in two places on the screen.
    //      Serial.print(charWork);
    //      Serial.printf(" is %u bytes long.\r\n", (unsigned)strlen(charWork));
    currTxtWidth = scrollSprite.textWidth(charWork) + spacer;
    if (prevTxtWidth != currTxtWidth) {
      Serial.printf("Resizing scrollSprite to %i\r\n", currTxtWidth);
      prevTxtWidth = currTxtWidth;
      scrollSprite.deleteSprite();
      scrollSpriteW = currTxtWidth;
      int *a = (int*)scrollSprite.createSprite(scrollSpriteW, scrollSpriteH); // Sprite for scrolling info
      if (a == 0) {
        Serial.println("scrollSprite creation failed in scrollIt.  Cannot continue.");
        while (1);
      }
      tft.fillRect(0, 0, tft.width(), scrollSpriteH, DarkBlue);
      msg1Head = tft.width();  // Restart from the left to avoid an irritating jump in the sprite.
    }
    scrollSprite.fillSprite(DarkBlue); // Initialize (clear) it to Dark Blue.
    scrollSprite.setCursor(0, 10); scrollSprite.print(charWork);  // Add all text.
  }
  msg1Head -= scrollAmt;  // Move everything to the left.
  // This may look a little odd but it is simply resetting the head of the
  //  leftmost sprite.  Then the followup sprite is added to the end of the
  //  leading sprite.
  if (msg1Head + scrollSpriteW < 0) msg1Head = msg1Head + scrollSpriteW;
  // Copy 1 (at msg1Head) -- leftmost.  Copy 2 -- to the right. (not always visible)
  scrollSprite.pushSprite(msg1Head, 0);
  // Only throw it at the screen if it can be seen.
  if ((msg1Head + scrollSpriteW) > tft.width()) return;
  scrollSprite.pushSprite(msg1Head + scrollSpriteW, 0);
}

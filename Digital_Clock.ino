/***************************************************************************/
void updateDigitalDisplay()
/***************************************************************************/
{
#define timeDivHeight 99  // Height of the two color time divisions.

  // Look a little odd to have "scrollIt();" in front of every line to you?
  //  That's the only way I could get smooth scrolling and accurate second
  //  changing on the lower two sections.
  //  iStartMillis = millis();

//  scrollIt(); clockSprite.fillSprite(TFT_BLACK);
  scrollIt(); clockSprite.fillRect(0,             0, tft.width(), timeDivHeight,
                                   RGB565(80, 0, 0)); // Dark Red
  scrollIt(); clockSprite.fillRect(0, timeDivHeight, tft.width(), timeDivHeight,
                                   RGB565(0, 80, 0)); // Dark Green
  scrollIt(); clockSprite.setTextColor(TFT_WHITE);
  //  scrollIt(); clockSprite.drawLine(0, timeDivHeight, tft.width(), timeDivHeight, TFT_WHITE);
  scrollIt(); time(&UTC);
  scrollIt(); workTime = UTC + iTopOffset;
  scrollIt(); clockSprite.setCursor( 5,  16);
  scrollIt(); clockSprite.print(cTopCityname);
  scrollIt(); clockSprite.setCursor(155, 16);
  scrollIt(); strftime(cCharWork, sizeof(cCharWork), "%A", localtime(&workTime));
  scrollIt(); clockSprite.print(cCharWork);
  scrollIt(); clockSprite.setCursor(  5, 64);
  scrollIt(); strftime(cCharWork, sizeof(cCharWork), "%x %X ", localtime(&workTime));
  scrollIt(); clockSprite.print(cCharWork);
  scrollIt(); clockSprite.print(cTopDST);

  scrollIt(); workTime = UTC + iBotOffset;
  scrollIt(); clockSprite.setCursor(  5, timeDivHeight + 16);
  scrollIt(); clockSprite.print(cBotCityname);
  scrollIt(); clockSprite.setCursor(155, timeDivHeight + 16);
  scrollIt(); strftime(cCharWork, sizeof(cCharWork), "%A", localtime(&workTime));
  scrollIt(); clockSprite.print(cCharWork);
  scrollIt(); clockSprite.setCursor(  5, timeDivHeight + 64);
  scrollIt(); strftime(cCharWork, sizeof(cCharWork), "%x %X ", localtime(&workTime));
  scrollIt(); clockSprite.print(cCharWork);
  scrollIt(); clockSprite.print(cBotDST);
  scrollIt(); clockSprite.pushSprite(0, iScrollSpriteH);
  //  Serial.printf("Time totally through Digital_Clock %u\r\n", millis() - iStartMillis);
}
/***************************************************************************/
void scrollIt()
/***************************************************************************/
{
  static const int iScrollAmt = 2;             // in screen pixels. How many pixels to scroll left.
  static const int iSpacer = 50;               // in screen pixels. Blank space.
  static unsigned int uiLastScrollms = 0;       // in ms. The count for when last scroll was done.
  static const unsigned int uiScrollTime = 30;  // in ms. The deadline interval for when to scroll again.
  static int msg1Head = tft.width(), iTxtWidth;
  static int iPrevMinute = -1, iPreviTxtWidth = -1, iCurrTxtWidth;

  // I check every action done, above, but only scroll 50 times per second.  The other 49 calls,
  //  the routine just returns immediately.
  if ((millis() - uiLastScrollms) < uiScrollTime) return;  // If it is not time to scroll, go back.
  uiLastScrollms = millis();                     // Reset time for next scroll
  // Since I am not showing seconds on the UTC scroll, it does not need to be
  //  updated more often than once per minute at the time the minute changes.
  if ((iPrevMinute != iCurrMinute) || bNewRate) {  // Text only needs to be changed each minute.
    iPrevMinute = iCurrMinute;  // Remember last minute of scrolling.
    bNewRate = false;
    //    Serial.println(UTC);
    strftime(cCharWork, sizeof(cCharWork), "%A  %x  %R %Z  ", localtime(&UTC));
    if (fPHP_Rate > 0.) {
      strcat(cCharWork, caReadingTime);
      sprintf(cCharWork2, " PHP%.2f/$  ", fPHP_Rate);
      strcat (cCharWork, cCharWork2);
    }
    strcat(cCharWork, myName); strcat(cCharWork, myVersion);  // Now the scroll text is ready to show.
    // The idea here is to change the length of the sprite to match the new data.
    //  Most of it does not change but the day name surely does change.  By doing
    //  this, the same spacing is maintained between the end of sprite copy 1 and
    //  sprite copy 2 as the day changes and that is done by changing the length
    //  of the sprite.  Remember, there is only 1, you frequently see the same one
    //  in two places on the screen.
    //      Serial.print(cCharWork);
    //      Serial.printf(" is %u bytes long.\r\n", (unsigned)strlen(cCharWork));
    iCurrTxtWidth = scrollSprite.textWidth(cCharWork) + iSpacer;
    if (iPreviTxtWidth != iCurrTxtWidth) {
      Serial.printf("Resizing scrollSprite to %i\r\n", iCurrTxtWidth);
      iPreviTxtWidth = iCurrTxtWidth;
      scrollSprite.deleteSprite();
      iScrollSpriteW = iCurrTxtWidth;
      int *a = (int*)scrollSprite.createSprite(iScrollSpriteW, iScrollSpriteH); // Sprite for scrolling info
      if (a == 0) {
        Serial.println("scrollSprite creation failed in scrollIt.  Cannot continue.");
        while (1);
      }
      tft.fillRect(0, 0, tft.width(), iScrollSpriteH, DarkBlue);
      msg1Head = tft.width();  // Restart from the left to avoid an irritating jump in the sprite.
    }
    scrollSprite.fillSprite(DarkBlue); // Initialize (clear) it to Dark Blue.
    scrollSprite.setCursor(0, 10); scrollSprite.print(cCharWork);  // Add all text.
  }
  msg1Head -= iScrollAmt;  // Move everything to the left.
  // This may look a little odd but it is simply resetting the head of the
  //  leftmost sprite.  Then the followup sprite is added to the end of the
  //  leading sprite.
  if (msg1Head + iScrollSpriteW < 0) msg1Head = msg1Head + iScrollSpriteW;
  // Copy 1 (at msg1Head) -- leftmost.  Copy 2 -- to the right. (not always visible)
  scrollSprite.pushSprite(msg1Head, 0);
  // Copy 2 - Only throw it at the screen if it can be seen.
  if ((msg1Head + iScrollSpriteW) > tft.width()) return;
  scrollSprite.pushSprite(msg1Head + iScrollSpriteW, 0);  // Samg song, second verse.
}

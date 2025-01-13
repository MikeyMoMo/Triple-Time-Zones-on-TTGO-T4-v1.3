/***************************************************************************/
void updateDigitalDisplay()
/***************************************************************************/
{
#define timeDivHeight 99  // Height of the two color time divisions.

  // Look a little odd to have "scrollIt();" in front of every line to you?
  //  That's the only way I could get smooth scrolling and accurate second
  //  changing on the lower two sections.

  //  iStartMillis = millis();

  scrollIt(); digitalSprite.fillRect(0,             0,
                                     tft.width(), timeDivHeight,
                                     RGB565(80, 0, 0)); // Dark Red
  scrollIt(); digitalSprite.fillRect(0, timeDivHeight,
                                     tft.width(), timeDivHeight,
                                     RGB565(0, 60, 0)); // Darker Green
  scrollIt(); digitalSprite.drawFastHLine(0,             0,
                                          tft.width(), TFT_WHITE);
  scrollIt(); digitalSprite.drawFastHLine(0, timeDivHeight,
                                          tft.width(), TFT_WHITE);
  scrollIt(); digitalSprite.drawFastHLine(0, timeDivHeight * 2 - 1,
                                          tft.width(), TFT_WHITE);
  scrollIt(); digitalSprite.setTextColor(TFT_WHITE);
  scrollIt(); time(&UTC);

  // Top: City Name
  scrollIt(); workTime = UTC + iTopOffset;
  scrollIt(); digitalSprite.setTextDatum(TL_DATUM);
  scrollIt(); digitalSprite.drawString(cTopCityname, 5, 8, 4);

  // Top: Tue, Mar 03
  scrollIt(); digitalSprite.setTextDatum(TR_DATUM);
  scrollIt(); strftime(cCharWork, sizeof(cCharWork), "%a, %h %d",
                       localtime(&workTime));
  scrollIt(); digitalSprite.drawString(cCharWork, tft.width() - 5, 8, 4);

  // Top: 12:24:30 PST (with the PST in a smaller size)
  scrollIt(); digitalSprite.setTextDatum(BL_DATUM);
  scrollIt(); strftime(cCharWork, sizeof(cCharWork), "%X",
                       localtime(&workTime));
  scrollIt(); digitalSprite.setFreeFont(FS24);
  scrollIt(); digitalSprite.drawString(cCharWork, 35, timeDivHeight - 10);
  scrollIt(); digitalSprite.setFreeFont(FS18);
  scrollIt(); digitalSprite.drawString(cTopDST, 218, timeDivHeight - 10);
  scrollIt(); digitalSprite.setFreeFont(NULL);

  // Bottom: City Name
  scrollIt(); workTime = UTC + iBotOffset;
  scrollIt(); digitalSprite.setTextDatum(TL_DATUM);
  scrollIt(); digitalSprite.drawString(cBotCityname, 5,
                                       timeDivHeight + 8, 4);

  // Bottom: Tue, Mar 03
  scrollIt(); digitalSprite.setTextDatum(TR_DATUM);
  scrollIt(); strftime(cCharWork, sizeof(cCharWork), "%a, %h %d",
                       localtime(&workTime));
  scrollIt(); digitalSprite.drawString(cCharWork, tft.width() - 5,
                                       timeDivHeight + 8, 4);

  // Bottom: 12:24:30 PST (with the PST in a smaller size)
  scrollIt(); digitalSprite.setTextDatum(BL_DATUM);
  scrollIt(); strftime(cCharWork, sizeof(cCharWork), "%X",
                       localtime(&workTime));
  scrollIt(); digitalSprite.setFreeFont(FS24);
  scrollIt(); digitalSprite.drawString(cCharWork, 35,
                                       timeDivHeight * 2 - 10);
  scrollIt(); digitalSprite.setFreeFont(FS18);
  scrollIt(); digitalSprite.drawString(cBotDST, 218,
                                       timeDivHeight * 2 - 10);
  scrollIt(); digitalSprite.setFreeFont(NULL);

  scrollIt(); digitalSprite.pushSprite(0, iScrollSpriteH);
  //  Serial.printf("Time totally through Digital_Clock %u\r\n",
  //                millis() - iStartMillis);
}
/***************************************************************************/
void scrollIt()
/***************************************************************************/
{
  // in screen pixels. How many pixels to scroll left.
  static const int iScrollAmt = 2;
  // in screen pixels. Blank space.
  static const int iSpacer = 50;
  // in ms. The count for when last scroll was done.
  static unsigned int uiLastScrollms = 0;
  // in ms. The deadline interval for when to scroll again.
  static const unsigned int uiScrollTime = 30;
  static int msg1Head = tft.width(), iTxtWidth;
  static int iPrevMinute = -1, iPreviTxtWidth = -1, iCurrTxtWidth;

  // I check every action done, above, but only scroll 50 times per second.
  //  The other 49 calls, the routine just returns immediately.

  // If it is not time to scroll, go back.
  if ((millis() - uiLastScrollms) < uiScrollTime) return;

  // Reset time for next scroll
  uiLastScrollms = millis();
  // Since I am not showing seconds on the UTC scroll, it does not
  //  need to be updated more often than once per minute at the time
  //  the minute changes.
  // Text only needs to be changed each minute.
  if ((iPrevMinute != iCurrMinute) || bNewRate) {
    iPrevMinute = iCurrMinute;  // Remember last minute of scrolling.
    bNewRate = false;
    //    Serial.println(UTC);
    strftime(cCharWork, sizeof(cCharWork), "%A  %x  %R %Z  ",
             localtime(&UTC));
    if (fPHP_Rate > 0.) {
      strcat(cCharWork, caReadingTime);
      sprintf(cCharWork2, " PHP%.2f/$  ", fPHP_Rate);
      strcat (cCharWork, cCharWork2);
    }
    // Now the scroll text is ready to show.
    strcat(cCharWork, myName); strcat(cCharWork, sVer.c_str());
    // The idea here is to change the length of the sprite to match
    //  the new data.  Most of it does not change but the day name
    //  surely does change.  By doing this, the same spacing is
    //  maintained between the end of sprite copy 1 and sprite
    //  copy 2 as the day changes and that is done by changing the length
    //  of the sprite.  Remember, there is only 1, you frequently
    //  see the same one in two places on the screen.
    //      Serial.print(cCharWork);
    //    Serial.printf(" is %u bytes long.\r\n",
    //                  (unsigned)strlen(cCharWork));
    iCurrTxtWidth = scrollSprite.textWidth(cCharWork) + iSpacer;
    if (iPreviTxtWidth != iCurrTxtWidth) {
      Serial.printf("Resizing scrollSprite to %i\r\n", iCurrTxtWidth);
      iPreviTxtWidth = iCurrTxtWidth;
      scrollSprite.deleteSprite();
      iScrollSpriteW = iCurrTxtWidth;
      int *a = (int*)scrollSprite.createSprite(iScrollSpriteW,
               iScrollSpriteH); // Sprite for scrolling info
      if (a == 0) {
        Serial.println("scrollSprite creation failed in scrollIt.  "
                       "Cannot continue.");
        while (1);
      }
      tft.fillRect(0, 0, tft.width(), iScrollSpriteH, DarkBlue);
      // Restart from the left to avoid an irritating jump in the sprite.
      msg1Head = tft.width();
    }
    // Initialize (clear) it to Dark Blue.
    scrollSprite.fillSprite(DarkBlue);
    // Add all text.
    scrollSprite.setCursor(0, 6); scrollSprite.print(cCharWork);
  }
  msg1Head -= iScrollAmt;  // Move everything to the left.
  // This may look a little odd but it is simply resetting the head of the
  //  leftmost sprite.  Then the followup sprite is added to the end of the
  //  leading sprite.
  if (msg1Head + iScrollSpriteW < 0) msg1Head = msg1Head + iScrollSpriteW;
  // Copy 1 (at msg1Head) -- leftmost.  Copy 2 -- to the right.
  //  (not always visible)
  scrollSprite.pushSprite(msg1Head, 0);
  // Copy 2 - Only throw it at the screen if it can be seen.
  if ((msg1Head + iScrollSpriteW) > tft.width()) return;
  // Same song, second verse.
  scrollSprite.pushSprite(msg1Head + iScrollSpriteW, 0);
}

/***************************************************************************/
void updateAnalogClock(time_t workTime)
/***************************************************************************/
{
  static int iPrevHour = -1;
  static int iForeColor = TFT_GOLD, iBackColor, iTotBrightness, R, G, B;

  //  iStartMillis = millis();
  // Change the background color on the hour.  But not too bright!
  if ((iPrevHour != iCurrHour) || bChangeACBG) {
    iPrevHour = iCurrHour; bChangeACBG = false;
    iTotBrightness = 241;
    while (iTotBrightness > 240) {
      R = esp_random() % 80 + 40;  // Don't let it get too high, initially
      G = esp_random() % 60 + 40;  //  but add 40 to brighten it up a bit.
      B = esp_random() % 80 + 40;  //  If the % is too high, then too bright.
      iTotBrightness = R + G + B;
      Serial.printf("Changing AC BG color to R %03i, G %03i, "
                    "B %03i for total of %i\r\n", R, G, B, iTotBrightness);
    }
    iBackColor = RGB565(R, G, B);
  }
  clockSprite.fillSprite(iBackColor);
  // Outside boundary circle of clock
  clockSprite.drawCircle(iXCenter, iYCenter, iRadius, TFT_YELLOW);
  clockSprite.setTextColor(iForeColor);
  // Draw minute's ticks (60 lines) in white or gold.
  for (int ordinal = 0; ordinal < 60; ordinal++) {
    dAngle = ordinal * 6 * 0.0174533;
    iMyX1 = iXCenter + (sin(dAngle) * iRadius);
    iMyY1 = iYCenter + (cos(dAngle) * iRadius);
    if (ordinal % 5 == 0) {
      // ordinal tic marks are 7 pixels long
      iMyX2 = iXCenter + (sin(dAngle) * (iRadius - 7));
      iMyY2 = iYCenter + (cos(dAngle) * (iRadius - 7));
      clockSprite.drawLine(iMyX1, iMyY1, iMyX2, iMyY2, TFT_WHITE);
    } else {
      // other tic marks are 3 pixels long
      iMyX2 = iXCenter + (sin(dAngle) * (iRadius - 3));
      iMyY2 = iYCenter + (cos(dAngle) * (iRadius - 3));
      clockSprite.drawLine(iMyX1, iMyY1, iMyX2, iMyY2, iForeColor);
    }
  }
  clockSprite.setTextDatum(TC_DATUM);
  for (int ordinal = 0; ordinal < 12; ordinal++) {
    // Calculate in degrees then change to radians.
    dAngle = ordinal * 30 * 0.0174533;

    iMyX2 = iXCenter + (sin(dAngle) * (iRadius - 20));
    iMyY2 = iYCenter + (cos(dAngle) * (iRadius - 20));
    iMyX2 -= 0; iMyY2 -= 8;

    switch (ordinal) {  // Fine tune the ordinal digit placements
      case 0: iMyX2 += 1; iMyY2 -= 3; break;   // 0 is   6
      case 1: iMyX2 -= 1; iMyY2 -= 3; break;   // 1 is   5
      case 2: iMyX2 -= 1; iMyY2 -= 1; break;   // 2 is   4
      case 3: iMyX2 += 3; iMyY2 -= 2; break;   // 3 is   3
      case 4: iMyX2 += 0; iMyY2 -= 3; break;   // 4 is   2
      case 5: iMyX2 += 0; iMyY2 -= 1; break;   // 5 is   1
      case 6: iMyX2 -= 2; iMyY2 -= 2; break;   // 6 is  12
      case 7: iMyX2 += 2; iMyY2 -= 2; break;   // 7 is  11
      case 8: iMyX2 += 3; iMyY2 -= 3; break;   // 8 is  10
      case 9: iMyX2 += 2; iMyY2 -= 2; break;   // 9 is   9
      case 10: iMyX2 += 3; iMyY2 -= 1; break;  // 10 is  8
      case 11: iMyX2 += 2; iMyY2 -= 4; break;  // 11 is  7
    }
    clockSprite.drawString(numbers[ordinal], iMyX2, iMyY2);
  }

  //Now calculate the minute hand.
  //Every minute gives 6° to the minute hand. Every second gives .1°
  // to the minute hand.
  // All in degrees.
  dAngle = (iCurrMinute * 6 + iCurrSecond * .1) +
           (millis() - secStartMillis) / 1000. * .1;
  //  Serial.printf("%02i:%02i - Minute hand dAngle %3.5fº or ",
  //                iCurrMinute, iCurrSecond, dAngle);
  dAngle *= 0.0174533; // Radians now
  //  Serial.printf("%3.5f Radians.\r\n", dAngle);
  iMyX2 = iXCenter + (sin(dAngle) * iRadius * .6);
  iMyY2 = iYCenter - (cos(dAngle) * iRadius * .6);
  // Now get the back end of the hand
#if defined WEDGE_HANDS
  // The widths coorespond to the x,y pairs.
  // iMyX2/Y2 goes with 2, iXCenter/iYCenter goes with 10.
  clockSprite.drawWedgeLine(iMyX2, iMyY2, iXCenter, iYCenter,
                            2, 8, TFT_WHITE);
#else  // Wire frame hands, coloron one side, alternating across center
  // Original minute hand with bulges
  dAngle = (iCurrMinute * 6 + 180);
  dAngle *= 0.0174533;
  iBackX = iXCenter + (sin(dAngle) * iRadius * .2);
  iBackY = iYCenter - (cos(dAngle) * iRadius * .2);
  // Now, make a wide minute hand
  dAngle = (iCurrMinute * 6 + 90);
  dAngle *= 0.0174533; // Radians now
  iPlus90BulgeX = iXCenter + (sin(dAngle) * iRadius * .075);
  iPlus90BulgeY = iYCenter - (cos(dAngle) * iRadius * .075);
  dAngle = (iCurrMinute * 6 - 90);
  dAngle *= 0.0174533;
  iMinus90BulgeX = iXCenter + (sin(dAngle) * iRadius * .075);
  iMinus90BulgeY = iYCenter - (cos(dAngle) * iRadius * .075);
  clockSprite.drawTriangle(iXCenter, iYCenter, iPlus90BulgeX,
                           iPlus90BulgeY, iBackX, iBackY, TFT_WHITE);
  clockSprite.fillTriangle(iXCenter, iYCenter, iMinus90BulgeX,
                           iMinus90BulgeY, iBackX, iBackY, TFT_WHITE);
  clockSprite.fillTriangle(iXCenter, iYCenter, iPlus90BulgeX,
                           iPlus90BulgeY, iMyX2, iMyY2, TFT_WHITE);
  clockSprite.drawTriangle(iXCenter, iYCenter, iMinus90BulgeX,
                           iMinus90BulgeY, iMyX2, iMyY2, TFT_WHITE);
#endif

  // Place hour hand
  // Each hour gives 30° to the hour hand.
  // Every minute gives 1/2° to the hour hand.
  dAngle = (iCurrHour * 30 + iCurrMinute * .5);
  // Isn't this modulo?  Seems like % didn't work right.
  while (dAngle >= 360) dAngle -= 360;
  dAngle *= 0.0174533;  // Radians now
  iMyX2 = iXCenter + (sin(dAngle) * iRadius * .5);
  iMyY2 = iYCenter - (cos(dAngle) * iRadius * .5);
  // The widths coorespond to the x,y pairs.
  // iMyX2/Y2 goes with 2, iXCenter/iYCenter goes with 10.
#if defined WEDGE_HANDS
  clockSprite.drawWedgeLine(iMyX2, iMyY2, iXCenter, iYCenter, 2, 8, TFT_RED);
#else  // Wire frame hands, coloron one side, alternating across center
  // Locate backside of hand (across the center).
  dAngle = ((iCurrHour * 30 + ((iCurrMinute / 12) * 6)) + 180);
  while (dAngle >= 360.) dAngle -= 360.;
  dAngle *= 0.0174533;  // Radians now
  iBackX = iXCenter + (sin(dAngle) * iRadius * .15);
  iBackY = iYCenter - (cos(dAngle) * iRadius * .15);
  // Now, make a wide hour hand
  dAngle = ((iCurrHour * 30 + ((iCurrMinute / 12) * 6)) + 90);
  while (dAngle >= 360) dAngle -= 360;
  dAngle *= 0.0174533;  // Radians now
  iPlus90BulgeX = iXCenter + (sin(dAngle) * iRadius * .075);
  iPlus90BulgeY = iYCenter - (cos(dAngle) * iRadius * .075);
  dAngle = ((iCurrHour * 30 + ((iCurrMinute / 12) * 6)) - 90);
  while (dAngle >= 360) dAngle -= 360;
  dAngle *= 0.0174533;  // Radians now
  iMinus90BulgeX = iXCenter + (sin(dAngle) * iRadius * .075);
  iMinus90BulgeY = iYCenter - (cos(dAngle) * iRadius * .075);
  clockSprite.drawTriangle(iXCenter, iYCenter, iPlus90BulgeX,
                           iPlus90BulgeY, iBackX, iBackY, TFT_RED);
  clockSprite.fillTriangle(iXCenter, iYCenter, iMinus90BulgeX,
                           iMinus90BulgeY, iBackX, iBackY, TFT_RED);
  clockSprite.fillTriangle(iXCenter, iYCenter, iPlus90BulgeX,
                           iPlus90BulgeY, iMyX2, iMyY2, TFT_RED);
  clockSprite.drawTriangle(iXCenter, iYCenter, iMinus90BulgeX,
                           iMinus90BulgeY, iMyX2, iMyY2, TFT_RED);
#endif

  if (bShowSecondHand) {  // Show second hand on 2 and 4
    // Place second hand
    dAngle = iCurrSecond * 6.;
#ifdef DO_SWEEP_HANDS
    dAngle += (millis() - secStartMillis) / 1000. * 6.;  // All in degrees.
#endif
    dAngle *= 0.0174533;        // Radians now
    iMyX2 = iXCenter + (sin(dAngle) * (iRadius * .7));
    iMyY2 = iYCenter - (cos(dAngle) * (iRadius * .7));
    // The widths coorespond to the x,y pairs.
    // iMyX2/Y2 goes with 2, iXCenter/iYCenter goes with 10.
#if defined WEDGE_HANDS
    clockSprite.drawWedgeLine(iMyX2, iMyY2, iXCenter, iYCenter, 1, 6,
                              TFT_BLUE);
#else  // Wire frame hands, coloron one side, alternating across center
    // Locate backside of hand (across the center).
    dAngle = (iCurrSecond * 6 + 180) * 0.0174533;  // Radians now
    iBackX = iXCenter + (sin(dAngle) * iRadius * .25);
    iBackY = iYCenter - (cos(dAngle) * iRadius * .25);
    // Now, make a wide second hand
    dAngle = iCurrSecond * 6 + 90;
    dAngle = dAngle * 0.0174533;  // Radians now
    iPlus90BulgeX = iXCenter + (sin(dAngle) * iRadius * .075);
    iPlus90BulgeY = iYCenter - (cos(dAngle) * iRadius * .075);
    dAngle = iCurrSecond * 6 - 90;
    dAngle = dAngle * 0.0174533;  // Radians now
    iMinus90BulgeX = iXCenter + (sin(dAngle) * iRadius * .075);
    iMinus90BulgeY = iYCenter - (cos(dAngle) * iRadius * .075);
    clockSprite.drawTriangle(iXCenter, iYCenter, iPlus90BulgeX,
                             iPlus90BulgeY, iBackX, iBackY, TFT_BLUE);
    clockSprite.fillTriangle(iXCenter, iYCenter, iMinus90BulgeX,
                             iMinus90BulgeY, iBackX, iBackY, TFT_BLUE);
    clockSprite.fillTriangle(iXCenter, iYCenter, iPlus90BulgeX,
                             iPlus90BulgeY, iMyX2, iMyY2, TFT_BLUE);
    clockSprite.drawTriangle(iXCenter, iYCenter, iMinus90BulgeX,
                             iMinus90BulgeY, iMyX2, iMyY2, TFT_BLUE);
#endif
  }

  clockSprite.fillSmoothCircle(iXCenter, iYCenter, 4, iForeColor);

  // Fill in the day, month, date and year in the four corners.
  showCorners(workTime);

  // This is not done with else since there is a third option,
  //  no digital time showing.
  clockSprite.setTextDatum(TC_DATUM);
  if (bShowShortTime) {
    // Hour and minute only.
    strftime(cCharWork, sizeof(cCharWork), "%H:%M", localtime(&workTime));
    txtLen = tft.textWidth(cCharWork);
    if (txtLen > maxTxtLen1) maxTxtLen1 = txtLen;
    iSavDatum = clockSprite.getTextDatum();
    clockSprite.setTextDatum(TL_DATUM);
    clockSprite.drawString(cCharWork,  iXCenter - maxTxtLen1 / 2,
                           tft.height()*.64);
    clockSprite.setTextDatum(iSavDatum);
  }
  if (bShowLongTime) {
    // Hour, minute & second.
    strftime(cCharWork, sizeof(cCharWork), "%H:%M:%S",
             localtime(&workTime));
    txtLen = tft.textWidth(cCharWork);
    if (txtLen > maxTxtLen2) maxTxtLen2 = txtLen;
    iSavDatum = clockSprite.getTextDatum();
    clockSprite.setTextDatum(TL_DATUM);
    clockSprite.drawString(cCharWork,  iXCenter - maxTxtLen2 / 2,
                           tft.height()*.64);
    clockSprite.setTextDatum(iSavDatum);
  }

  // Show the XRate on if digital time is shown (long or short).
  // It will be on top of hands.
  // Yeah, the extra parens is needed!
  if ((bShowShortTime || bShowLongTime) && fPHP_Rate > 0.) {
    dtostrf(fPHP_Rate, 2, 2, cCharWork);
    txtLen = tft.textWidth(cCharWork);
    if (txtLen > maxTxtLen3) maxTxtLen3 = txtLen;
    iSavDatum = clockSprite.getTextDatum();
    clockSprite.setTextDatum(TL_DATUM);
    clockSprite.drawString(cCharWork,  iXCenter - maxTxtLen3 / 2,
                           tft.height()*.28);
    clockSprite.setTextDatum(iSavDatum);
  }
  clockSprite.pushSprite(0, 0);
  // It has been taking around 65ms to run this routine including pushing
  //  the sprite to the display.
  //  if ((millis() - iStartMillis) > 900)
  //  Serial.printf("Total time for analog face update %lu ms.\r\n",
  //                millis() - iStartMillis);
}
/***************************************************************************/
void showCorners(time_t workTime)
/***************************************************************************/
{
  strftime(cCharWork, sizeof(cCharWork), "%a", localtime(&workTime));
  clockSprite.setTextDatum(TL_DATUM);
  clockSprite.drawString(cCharWork, 0, 0);
  // Assuming, for now, that the bottom clock is local time.
#if defined CONFIG4MIKE
  workTime = UTC + iBotOffset;
#else
  workTime = UTC + iTopOffset;
#endif
  // Load up the timeinfo struct for immediate use.
  timeinfo = localtime(&workTime);
  // Use full month name in June and July since there is room.
  if (timeinfo->tm_mon == 5 || timeinfo->tm_mon == 6)
    // Short month name
    strftime(cCharWork, sizeof(cCharWork), "%B", localtime(&workTime));
  else
    // Short month name
    strftime(cCharWork, sizeof(cCharWork), "%b", localtime(&workTime));
  clockSprite.setTextDatum(TR_DATUM);
  clockSprite.drawString(cCharWork, tft.width() - 2, 0);

  // Numeric day of month
  strftime(cCharWork, sizeof(cCharWork), "%d", localtime(&workTime));
  clockSprite.setTextDatum(BL_DATUM);
  clockSprite.drawString(cCharWork, 0, tft.height());

  // Full (4 digits) year
  strftime(cCharWork, sizeof(cCharWork), "%Y", localtime(&workTime));
  clockSprite.setTextDatum(BR_DATUM);
  clockSprite.drawString(cCharWork, tft.width(), tft.height());
}

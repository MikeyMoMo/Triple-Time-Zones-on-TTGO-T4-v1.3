#define WEDGE_HANDS
/***************************************************************************/
void updateAnalogClock(int hours, int minutes, int seconds, time_t now)
/***************************************************************************/
{
  startMillis = millis();
  //  ofr.setFontSize(28);
  clockSprite.fillSprite(RGB565(0, 60, 0));
  clockSprite.drawCircle(xCenter, yCenter, radius, TFT_YELLOW);
  ofr.setFontColor(TFT_GOLD, DarkerRed);

  // Draw minute's ticks (60 lines)
  for (int ordinal = 0; ordinal < 60; ordinal++) {
    angle = ordinal * 6 * 0.0174533;
    myX1 = xCenter + (sin(angle) * radius);
    myY1 = yCenter + (cos(angle) * radius);
    if (ordinal % 5 == 0) {
      myX2 = xCenter + (sin(angle) * (radius - 7));  // tic marks are 4 pixels long
      myY2 = yCenter + (cos(angle) * (radius - 7));  // tic marks are 4 pixels long
      clockSprite.drawLine(myX1, myY1, myX2, myY2, TFT_WHITE);
    } else {
      myX2 = xCenter + (sin(angle) * (radius - 3));
      myY2 = yCenter + (cos(angle) * (radius - 3));
      clockSprite.drawLine(myX1, myY1, myX2, myY2, TFT_GOLD);
    }
  }
  for (int ordinal = 0; ordinal < 12; ordinal++) {
    angle = ordinal * 30 * 0.0174533;

    myX2 = xCenter + (sin(angle) * (radius - 20));  // was - 8
    myY2 = yCenter + (cos(angle) * (radius - 20));  // was - 8
    myX2 -= 10; myY2 -= 10;

    switch (ordinal) {  // Fine tune the ordinal placements
      case 0: myX2 += 1; myY2 -= 3; break;   // 0 is   6
      case 1: myX2 -= 1; myY2 -= 3; break;   // 1 is   5
      case 2: myX2 -= 1; myY2 -= 1; break;   // 2 is   4
      case 3: myX2 += 3; myY2 -= 2; break;   // 3 is   3
      case 4: myX2 += 0; myY2 -= 3; break;   // 4 is   2
      case 5: myX2 += 1; myY2 -= 2; break;   // 5 is   1
      case 6: myX2 -= 4; myY2 -= 2; break;   // 6 is  12
      case 7: myX2 += 2; myY2 -= 2; break;   // 7 is  11
      case 8: myX2 += 1; myY2 -= 3; break;   // 8 is  10
      case 9: myX2 += 2; myY2 -= 2; break;   // 9 is   9
      case 10: myX2 += 3; myY2 -= 1; break;  // 10 is  8
      case 11: myX2 += 2; myY2 -= 4; break;  // 11 is  7
    }
    ofr.setCursor(myX2, myY2); ofr.printf(numbers[ordinal]);
  }
  if (showShortTime) {
    strftime(charWork, sizeof(charWork), "%H:%M", localtime(&now));  // Hour and minute only.
    ofr.setCursor(tft.width() / 2 - 39, tft.height()*.64); ofr.printf(charWork);
  }
  if (showLongTime) {
    strftime(charWork, sizeof(charWork), "%H:%M:%S", localtime(&now));  // Hour, minute & second.
    ofr.setCursor(tft.width() / 2 - 58, tft.height()*.64); ofr.printf(charWork);
  }
  //Now calculate the minute hand.
  //Every minute gives 6° to the minute hand. Every second gives .1°
  // to the minute hand.
  angle = (minutes * 6 + seconds * .1);
  angle *= 0.0174533; // Radians now
  myX2 = xCenter + (sin(angle) * radius * .6);
  myY2 = yCenter - (cos(angle) * radius * .6);
  // Now get the back end of the hand
#if defined WEDGE_HANDS
  // The widths coorespond to the x,y pairs.  myX2/Y2 goes with 2, xCenter/yCenter goes with 10.
  clockSprite.drawWedgeLine(myX2, myY2, xCenter, yCenter, 2, 10, TFT_WHITE);
#else  // Wire frame hands, coloron one side, alternating across center
  // Original minute hand with bulges
  angle = (minutes * 6 + 180);
  angle *= 0.0174533;
  backX = xCenter + (sin(angle) * radius * .2);
  backY = yCenter - (cos(angle) * radius * .2);
  // Now, make a wide minute hand
  angle = (minutes * 6 + 90);
  angle *= 0.0174533; // Radians now
  plus90BulgeX = xCenter + (sin(angle) * radius * .075);
  plus90BulgeY = yCenter - (cos(angle) * radius * .075);
  angle = (minutes * 6 - 90);
  angle *= 0.0174533;
  minus90BulgeX = xCenter + (sin(angle) * radius * .075);
  minus90BulgeY = yCenter - (cos(angle) * radius * .075);
  clockSprite.drawTriangle(xCenter, yCenter, plus90BulgeX, plus90BulgeY, backX, backY, TFT_WHITE);
  clockSprite.fillTriangle(xCenter, yCenter, minus90BulgeX, minus90BulgeY, backX, backY, TFT_WHITE);
  clockSprite.fillTriangle(xCenter, yCenter, plus90BulgeX, plus90BulgeY, myX2, myY2, TFT_WHITE);
  clockSprite.drawTriangle(xCenter, yCenter, minus90BulgeX, minus90BulgeY, myX2, myY2, TFT_WHITE);
#endif

  // Place hour hand
  // Each hour gives 30° to the hour hand.
  // Every minute gives 1/2° to the hour hand.
  angle = (hours * 30 + minutes * .5); while (angle >= 360) angle -= 360;
  angle *= 0.0174533;  // Radians now
  myX2 = xCenter + (sin(angle) * radius * .5);
  myY2 = yCenter - (cos(angle) * radius * .5);
  // The widths coorespond to the x,y pairs.  myX2/Y2 goes with 2, xCenter/yCenter goes with 10.
#if defined WEDGE_HANDS
  clockSprite.drawWedgeLine(myX2, myY2, xCenter, yCenter, 2, 10, TFT_RED);
#else  // Wire frame hands, coloron one side, alternating across center
  // Locate backside of hand (across the center).
  angle = ((hours * 30 + ((minutes / 12) * 6)) + 180); while (angle >= 360) angle -= 360;
  angle *= 0.0174533;  // Radians now
  backX = xCenter + (sin(angle) * radius * .15);
  backY = yCenter - (cos(angle) * radius * .15);
  // Now, make a wide hour hand
  angle = ((hours * 30 + ((minutes / 12) * 6)) + 90); while (angle >= 360) angle -= 360;
  angle *= 0.0174533;  // Radians now
  plus90BulgeX = xCenter + (sin(angle) * radius * .075);
  plus90BulgeY = yCenter - (cos(angle) * radius * .075);
  angle = ((hours * 30 + ((minutes / 12) * 6)) - 90); while (angle >= 360) angle -= 360;
  angle *= 0.0174533;  // Radians now
  minus90BulgeX = xCenter + (sin(angle) * radius * .075);
  minus90BulgeY = yCenter - (cos(angle) * radius * .075);
  clockSprite.drawTriangle(xCenter, yCenter, plus90BulgeX, plus90BulgeY, backX, backY, TFT_RED);
  clockSprite.fillTriangle(xCenter, yCenter, minus90BulgeX, minus90BulgeY, backX, backY, TFT_RED);
  clockSprite.fillTriangle(xCenter, yCenter, plus90BulgeX, plus90BulgeY, myX2, myY2, TFT_RED);
  clockSprite.drawTriangle(xCenter, yCenter, minus90BulgeX, minus90BulgeY, myX2, myY2, TFT_RED);
#endif

  if (showSecondHand) {  // Show second hand on 2 and 4
    // Place second hand
    angle = seconds * 6 * 0.0174533;        // Radians now
    myX2 = xCenter + (sin(angle) * (radius * .7));
    myY2 = yCenter - (cos(angle) * (radius * .7));
    // The widths coorespond to the x,y pairs.  myX2/Y2 goes with 2, xCenter/yCenter goes with 10.
#if defined WEDGE_HANDS
    clockSprite.drawWedgeLine(myX2, myY2, xCenter, yCenter, 2, 10, TFT_BLUE);
#else  // Wire frame hands, coloron one side, alternating across center
    // Locate backside of hand (across the center).
    angle = (seconds * 6 + 180) * 0.0174533;  // Radians now
    backX = xCenter + (sin(angle) * radius * .25);
    backY = yCenter - (cos(angle) * radius * .25);
    // Now, make a wide second hand
    angle = seconds * 6 + 90; angle = angle * 0.0174533;   // Radians now
    plus90BulgeX = xCenter + (sin(angle) * radius * .075);
    plus90BulgeY = yCenter - (cos(angle) * radius * .075);
    angle = seconds * 6 - 90; angle = angle * 0.0174533;   // Radians now
    minus90BulgeX = xCenter + (sin(angle) * radius * .075);
    minus90BulgeY = yCenter - (cos(angle) * radius * .075);
    clockSprite.drawTriangle(xCenter, yCenter, plus90BulgeX, plus90BulgeY, backX, backY, TFT_BLUE);
    clockSprite.fillTriangle(xCenter, yCenter, minus90BulgeX, minus90BulgeY, backX, backY, TFT_BLUE);
    clockSprite.fillTriangle(xCenter, yCenter, plus90BulgeX, plus90BulgeY, myX2, myY2, TFT_BLUE);
    clockSprite.drawTriangle(xCenter, yCenter, minus90BulgeX, minus90BulgeY, myX2, myY2, TFT_BLUE);
#endif
  }

  // These two seem to be the same.  They look the same, anyway... At least at size 4.
  clockSprite.fillSmoothCircle(xCenter, yCenter, 4, TFT_GOLD);
  //  clockSprite.drawSpot(xCenter, yCenter, 4, TFT_GOLD);  // Guess!

  showCorners(now);  // Fill in the day, month, date and year in the four corners.

  clockSprite.pushSprite(0, 0);
  // It has been taking around 100ms to run this routine including pushing the sprite to the display.
  if ((millis() - startMillis) > 900)
    Serial.printf("Total time for analog face update %lu ms.\r\n", millis() - startMillis);
}

/***************************************************************************/
void showCorners(time_t now)
/***************************************************************************/
{
  //  int asdf = ofr.getTextHeight("456789");  // Returns the wrong answer.
  //  Serial.printf("Text height %i\r\n", asdf);
  strftime(charWork, sizeof(charWork), "%a", localtime(&now));  // name Day
  ofr.setCursor(0, 0); ofr.printf(charWork);
  strftime(charWork, sizeof(charWork), "%b", localtime(&now));  // Short month name
  ofr.setCursor(tft.width() - 2, 0); ofr.rprintf(charWork);
  strftime(charWork, sizeof(charWork), "%d", localtime(&now));  // Numeric day of month
  ofr.setCursor(0, tft.height() - 26); ofr.printf(charWork);
  strftime(charWork, sizeof(charWork), "%Y", localtime(&now));  // Full (4 digits) year
  ofr.setCursor(tft.width(), tft.height() - 26); ofr.rprintf(charWork);
}

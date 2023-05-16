/***************************************************************************/
void updateAnalogClock(int hours, int minutes, int seconds, time_t now)
/***************************************************************************/
{
  //  hours = 12; minutes = 0; seconds = 0;  // For testing hands look
  startMillis = millis();
  ofr.setFontSize(28);
  clockSprite.fillSprite(DarkPurple);
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
    } else {
      myX2 = xCenter + (sin(angle) * (radius - 3));
      myY2 = yCenter + (cos(angle) * (radius - 3));
    }
    clockSprite.drawLine(myX1, myY1, myX2, myY2, TFT_WHITE);
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

  //Now calculate the minute hand.
  //Every minute gives 6° to the minute hand. Every second gives .1°
  // to the minute hand.
  angle = (minutes * 6 + seconds * .1);
  angle *= 0.0174533; // Radians now
  myX2 = xCenter + (sin(angle) * radius * .6);
  myY2 = yCenter - (cos(angle) * radius * .6);
  // Now get the back end of the hand
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

  // Place hour hand
  // Each hour gives 30° to the hour hand.
  // Every minute gives 1/2° to the hour hand.
  angle = (hours * 30 + minutes * .5); while (angle >= 360) angle -= 360;
  angle *= 0.0174533;  // Radians now
  myX2 = xCenter + (sin(angle) * radius * .5);
  myY2 = yCenter - (cos(angle) * radius * .5);
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

  // Place second hand
  angle = seconds * 6 * 0.0174533;        // Radians now
  myX2 = xCenter + (sin(angle) * (radius * .7));
  myY2 = yCenter - (cos(angle) * (radius * .7));
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

  clockSprite.fillCircle(xCenter, yCenter, 4, TFT_GOLD);  // Guess!
  showCorners(now);  // 1st time for analog show or hourly.

  clockSprite.pushSprite(0, 0);
  if ((millis() - startMillis) > 999)
    Serial.printf("Total time for analog face update %lu ms.\r\n", millis() - startMillis);
}

/***************************************************************************/
void showCorners(time_t now)
/***************************************************************************/
{
  //  int asdf = ofr.getTextHeight("456789");  // Returns the wrong answer.
  //  Serial.printf("Text height %i\r\n", asdf);
  strftime(charWork, 100, "%a", localtime(&now));  // name Day
  ofr.setCursor(0, 0); ofr.printf(charWork);
  strftime(charWork, 100, "%b", localtime(&now));  // Short month name
  ofr.setCursor(tft.width() - 2, 0); ofr.rprintf(charWork);
  strftime(charWork, 100, "%d", localtime(&now));  // Numeric day of month
  ofr.setCursor(0, tft.height() - 26); ofr.printf(charWork);
  strftime(charWork, 100, "%Y", localtime(&now));  // Full (4 digits) year
  ofr.setCursor(tft.width(), tft.height() - 26); ofr.rprintf(charWork);
}

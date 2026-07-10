/***************************************************************************/
void showXGraph()
/***************************************************************************/
{
  static uint32_t dropColor = TFT_GREEN;  // DarkerGreen;
  static uint32_t dateDropColor = TFT_RED;
  static uint32_t diffLineColor, diffBackColor;

  // The display is 240 x 320.  We will allow some margin at the left and
  //  bottom so less than 240 or 320 will be utilized.
  // I multiply the highest XRate by 100 and then divide into the
  //  number of pixels that will be used.  All in float.
  // Well... more or less...  Sorry, it turned out to be messy code.

  /* Note about the stupid thing I did with the array:
      I made the array 365 values big.  I did not think about the left
      margin that I wanted to leave so this means that I will ignore the
      first 85 values and dates in the array.  Since I had already
      started saving data and I did not want to write a program to fix
      the problem in the saved Preferences field, I left it alone and made
      adjustments in the code.  It makes the code look messy but so be it.
      I have seen worse mistakes and remedy code.  I feel bad about it but
      not bad enough to fix it!  If you want to, YOU fix it!  It will
      take a lot of screwing around to fix all the references.  Enjoy...

      365 array elements - 320 screen columns leaves 45 out.  Then add the
      forty for graphLeftMargin and you get 85.  365-85 = 280 that will be
      plotted (maximum).  After day 280, some will be ignored. Live with it!
  */
  /* Note about the dates on the bottom:
      The dates are centered under the column they represent.  The column
      that is associated with the date is in a highlight color (see below)
      to show which rate is associated with which date.  They are placed
      every "DROP_DATE_EVERY" columns so, if you think they are too close,
      just make the define a little bigger (or smaller) to your
      liking.
  */

  //  if (refreshGraph) {
  // iStartMillis = millis();  // Time this sucker!
  //    refreshGraph = false;

  int dropDate = -1;  // Force setting this on first non-zero XRate.

  static int thisX, thisY;
  static float yHigh, yLow, yRange;

  yHigh = 0;
  yLow = 10000;  // Values to force a valid update.
  int prevDay = -1, intMyDay, intBackup, dayX;

  // String legend = "Daily Rate History";
  // int halfLegend = XRateSprite.textWidth(legend, 2) / 2;

  countXRate(false, false);  // Count and print the total non-zero.

  // Find the highest and lowest values in the array.
  for (int i = XRateHistLen - tft.width() + graphLeftMargin;
       i < XRateHistLen; i++) {
    if (XRateHist[i] > 0.)  // Exclude 0's from being the yLow winner.
    {
      if (dropDate < 0) dropDate = i;
      if (XRateHist[i] < yLow) yLow = XRateHist[i];
      if (XRateHist[i] > yHigh) yHigh = XRateHist[i];
    }
  }
  // Serial.printf("Actual high & low %.2f & %.2f, ",
  //               yHigh, yLow);
  // Serial.flush();
  float TenMonthHi = yHigh;

  // Using ceil and floor can leave too much border.
  // Leave a small border hi & lo.
  // yHigh = ceil(yHigh); yLow = floor(yLow);
  // Going with .5 as a high & low border.  Smaller borderland.
  yHigh += .5;
  yLow -= .5;             // Leave a little border hi & lo.
  yRange = yHigh - yLow;  // The range I will plot including
  //                         internal borders.
  // Serial.printf("adjusted %.2f & %.2f, yRange %.2f.\r\n",
  //               yHigh, yLow, yRange); Serial.flush();
  XRateSprite.fillSprite(TFT_BLACK);
  XRateSprite.setTextColor(TFT_YELLOW, TFT_BLACK);

  XRateSprite.setTextDatum(TC_DATUM);  // XRate History legend
  XRateSprite.drawString("Daily Rate History",
                         (tft.width() / 2) + 10, 0, 2);

  // Put the time in the upper left corner of the graph area.
  time(&UTC);  // Probably not needed but fast and cheap.
  //#if defined CONFIG4MIKE
  //    workTime = UTC + iBotOffset;
  //#else
  //    workTime = UTC + iTopOffset;  // Use top as local time for Joe.
  //#endif
  //    strftime(cCharWork, sizeof(cCharWork), "%H:%M", localtime(&workTime));
  //    XRateSprite.setTextDatum(TL_DATUM);  // XRate History legend
  //    XRateSprite.drawString(cCharWork, graphLeftMargin + 2, 0, 4);

  // Draw Y axis (on the left)
  XRateSprite.drawFastVLine(graphLeftMargin, 0,
                            tft.height() - graphFloorMargin, TFT_YELLOW);
  // Draw X axis across the bottom
  XRateSprite.drawFastHLine(graphLeftMargin,
                            tft.height() - graphFloorMargin,
                            tft.width() - graphLeftMargin, TFT_YELLOW);

  if (yRange == 0)  // Avoid that pesky, possibly upcoming division by 0!
    pixelsPerHundredthV = 0;
  else
    pixelsPerHundredthV = (tft.height() - graphFloorMargin) / (yRange * 100.);

#if defined FILL_GRAPH
  //--------------> Now, draw the drop lines on the graph (if requested)
  //                 and the dates.
  // A double pass is done so the lines will not overdraw the dots.

  // For this graph, I am plotting dots and dropping a line to the X axis
  //  to implement a bar graph.  I am not using line segments since they
  //  are not needed and, therefore, I don't need the lastX and lastY
  //  elements.

  thisX = graphLeftMargin;

  // I plot one dot per pixel column.  That should make a bar graph.
  i = XRateHistLen - tft.width() + graphLeftMargin;
  while (i < XRateHistLen) {
    if (XRateHist[i] > 0)  // If there is a value in the bucket...
    {
      thisY = (yHigh - XRateHist[i]) * pixelsPerHundredthV * 100.;
      // Less code that putting this in an }else{ clause.
      if (i == dropDate) {  // Time for a date on the X axis?
        iSavDatum = XRateSprite.getTextDatum();
        XRateSprite.setTextDatum(TC_DATUM);
        JulianToGregorian(XRateJulian[i]);
        // Serial.printf("Dropping date on column %i\r\n", thisX);
        XRateSprite.drawString(String(XRateMon) + "/" + String(XRateDay),
                               thisX, tft.height() - graphFloorMargin + 3,
                               2);  // Use font 2 (quite small).
        XRateSprite.drawLine(thisX, tft.height() - graphFloorMargin - 1,
                             thisX, thisY, dateDropColor);

        XRateSprite.setTextDatum(iSavDatum);
        dropDate += DROP_DATE_EVERY;
      } else {
#if defined COLOR_FILL_GRAPH
        // The 6th value (tft.height() - graphFloorMargin) changes the
        //  look of the gradient line.  If it is 0, the gradient is
        //  completed along the length of the line no matter how long
        //  or short. If there is a value here, its value is used for
        //  all lines. This gives a more regular look to the gradient
        //  lines.
        //
        //  thisX and thisY are the coordinates of the current value of
        //   interest.  It is the top of the line value where I will put
        //   a little white dot.  The bottom of the line is the value of
        //   tft.height() - graphFloorMargin - 1.  The "- 1" is so the
        //   graph fill line will not obliterate the X axis line
        //   (at the bottom).

        // XRateSprite.drawLine(thisX, tft.height() - graphFloorMargin - 1,
        //                      thisX, thisY, DarkerGreen);

        drawGradientLine(&XRateSprite,
                         thisX,                                // botX
                         tft.height() - graphFloorMargin - 1,  // botY
                         thisX, thisY,                         // top
                         tft.height() - graphFloorMargin,
                         //top,     bottom.  Yeah, I know!
                         dropColor, RGB565(255, 80, 80));
      }
#endif
    }
    thisX++;
    i++;  // Move to next pixel and XRate array entries.
  }
#endif

  //--------------> Now, draw the dots on the graph.
  // A double pass is done so the lines will not overdraw the dots.
  // For this graph, I am only the dots so they will appear on top
  // of the lines.

  thisX = graphLeftMargin;

  //    XRateSprite.setTextDatum(BR_DATUM);
  //    drawYAxisScaleLines(.25, yLow, yRange);
  //    drawYAxisScaleLines(.5, yLow, yRange);
  //    drawYAxisScaleLines(.75, yLow, yRange);
  //    XRateSprite.setTextColor(TFT_YELLOW, TFT_BLACK);
  //    XRateSprite.drawString(String(yHigh, 2),
  //                           graphLeftMargin - 2, 10, 2);
  //    XRateSprite.drawString(String(yLow, 2), graphLeftMargin - 2,
  //                           tft.height() - graphFloorMargin - 5, 2);

  // If current XRate is > ending yesterday,
  //  draw background and dot in green, else red.
  diffLineColor = RGB565(255, 120, 80);
  diffBackColor = TFT_BLACK;
  if (fPHP_Rate > XRateHist[XRateHistLen - 2]) {
    diffLineColor = TFT_GREEN;  // diffBackColor = TFT_BLACK;
  }
  if (fPHP_Rate == XRateHist[XRateHistLen - 2]) {
    diffLineColor = TFT_WHITE;  // diffBackColor = TFT_BLACK;
  }
  // Draw in the current rate in the upper right of the graph.
  if (fPHP_Rate > 0.) {
    dtostrf(fPHP_Rate, 2, 2, cCharWork);
    strcat(cCharWork, "/$");
    //      iSavDatum = XRateSprite.getTextDatum();
    XRateSprite.setTextColor(diffLineColor, diffBackColor);
    XRateSprite.setTextDatum(TR_DATUM);
    XRateSprite.drawString(cCharWork, XRateSprite.width(), 0, 4);
    dtostrf(TenMonthHi, 2, 2, cCharWork);
    // Serial.printf("10 Month High %.2f\r\n", TenMonthHi);
    XRateSprite.setTextColor(TFT_YELLOW, diffBackColor);
    XRateSprite.setTextDatum(TL_DATUM);
    XRateSprite.drawString(cCharWork, graphLeftMargin + 3, 0, 4);
    //      XRateSprite.setTextDatum(iSavDatum);
  }

  // I plot one dot at the top of each pixel column in white.
  i = XRateHistLen - tft.width() + graphLeftMargin;
  while (i < XRateHistLen) {
    if (XRateHist[i] > 0) {
      thisY = (yHigh - XRateHist[i]) * pixelsPerHundredthV * 100.;
      XRateSprite.fillCircle(thisX, thisY, 1, TFT_WHITE);  // was 2
    }
    thisX++;
    i++;  // Move to next pixel and XRate array entries.
  }

  // Right side color dot on current reading.
  // XRateSprite.fillCircle(thisX, thisY, 5, TFT_BLUE);
  // Going to try a multicolor line to see if that will show up better.
  // Real challenge!

  // XRateSprite.drawLine(thisX, thisY, graphLeftMargin, thisY, TFT_RED);
  // xRateSprite.drawLine(&XRateSprite,
  //                      thisX - 2, thisY, graphLeftMargin, thisY,
  //                      TFT_RED, 12, PI / 6, TFT_RED, true););
  // Draw a line for the current sample.  Put an arrow on the right end.
  // The arrow's diagonals will be 15 pixels long, at an angle of 30º
  //  and the triangle will be filled in with red.
  //    drawArrowLine(&XRateSprite,
  //                  graphLeftMargin, thisY, thisX - 2, thisY,
  //                  diffLineColor, 15, PI / 6, TFT_BLUE, false, true);
  //    delay(3000);
  // Left side color dot at current reading height.
  //  XRateSprite.fillCircle(graphLeftMargin, thisY, 5, TFT_BLUE);

  XRateSprite.setTextDatum(BR_DATUM);
  XRateSprite.setTextColor(TFT_YELLOW, TFT_BLACK);
  drawYAxisScaleLines(.25, yLow, yRange);
  drawYAxisScaleLines(.5, yLow, yRange);
  drawYAxisScaleLines(.75, yLow, yRange);
  XRateSprite.drawString(String(yHigh, 2),
                         graphLeftMargin - 2, 10, 2);
  XRateSprite.drawString(String(yLow, 2), graphLeftMargin - 2,
                         tft.height() - graphFloorMargin - 5, 2);

  // Serial.printf("Drawing multiBlend line from %i, %i to %i, %i\r\n",
  //               graphLeftMargin, thisY, thisX, thisY);

  // The one line was not really thick enough to see.
  // Trying 3 lines this time
  doDebugPrints = false;
  //#define MB_Honor_Line_Length = true
  //#define MB_Proportional_Line_Length = false
  //#define MB_Rotate_Colors = true
  //#define MB_No_Rotate = false
  //#define MB_Rotate_Right true
  //#define MB_Rotate_Left  false
  drawMultiBlendLine(&XRateSprite,
                     graphLeftMargin, thisY,
                     thisX - 2, thisY,
                     blendSequence, blendCt,
                     MB_Proportional_Line_Length,  // was false,
                     MB_Rotate_Colors,             // was true,
                     MB_Rotate_Right,              // Rotate to right
                     100,
                     &line1State);

  XRateSprite.fillTriangle(thisX, thisY,
                           thisX - 6, thisY - 6,
                           thisX - 6, thisY + 5,
                           RGB565(100, 100, 255));

  XRateSprite.pushSprite(0, 0);  // As the teachers say... SHOW YOUR WORK!
  // Serial.printf("Graph routine took %lu ms.\r\n",
  //               millis() - iStartMillis); Serial.flush();
  //  }
  delay(100);
}
/***************************************************************************/
void drawYAxisScaleLines(float myPctHeight, float myLowEnd, float myRange)
/***************************************************************************/
{
  // These are the 1 or more lines across the graph with a label on the left.

  XRateSprite.drawFastHLine(graphLeftMargin,
                            (tft.height() - graphFloorMargin) * myPctHeight,
                            tft.width() - graphLeftMargin, TFT_YELLOW);

  float a = (1. - myPctHeight) * myRange;
  float c = a + myLowEnd;

  XRateSprite.setTextDatum(MR_DATUM);
  XRateSprite.drawString(String(c, 2), graphLeftMargin - 2,
                         (tft.height() - graphFloorMargin) * myPctHeight, 2);
}

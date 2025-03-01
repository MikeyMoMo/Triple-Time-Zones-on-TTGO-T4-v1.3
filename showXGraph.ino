/***************************************************************************/
void showXGraph()
/***************************************************************************/
{
  static uint32_t dropColor;
  // The display is 240 x 320.  We will allow some margin at the bottom so
  //  less than 240 will be utilized.
  // I multiply the highest XRate by 100 and then divide into the
  //  number of pixels that will be used.  All in float.
  // Well... more or less...

  /* Note about the stupid thing I did with the array:
      I made the array 365 values big.  I did not think about the left margin
      that I wanted to leave so this means that I will ignore the first 85
      values and dates in the array.  Since I had already started saving data
      and I did not want to write a program to fix the problem in the
      saved Preferences field, I left it alone and made adjustments in the
      code.  It makes the code look messy but so be it.  I have seen worse
      mistakes and remedy code.  I feel bad about it but not bad enough to
      fix it!  If you want to, YOU fix it!  It will take a lot of screwing
      around to fix all the references.  Enjoy...
      365 array elements - 320 screen columns leaves 45 out.  Then add the
      forty for graphLeftMargin and you get 85.  365-85 = 280 that will be
      plotted (maximum).  After day 280, some will be ignored.  Live with it!
  */
  /* Note about the dates on the bottom:
      The dates are centered under the column they represent.  The column
      that is associated with the date is in a highlight color (see below)
      to show which rate is associated with which date.  They are placed
      every "DROP_DATE_EVERY" columns so, if you think they are too close,
      just make the following define a little bigger (or smaller) to your
      liking.
  */
  // How many columns to skip before adding another date.
#define DROP_DATE_EVERY 50

  if (refreshGraph) {
    iStartMillis = millis();  // Time this sucker!
    int dropDate = -1;  // Force setting this on first non-zero XRate.

    static int   thisX, thisY;
    static float yHigh, yLow, yRange;

    yHigh = 0; yLow = 10000;  // Values to force a valid update.
    int prevDay = -1, intMyDay, intBackup, dayX;

    String legend = "Daily Rate History";
    int halfLegend = XRateSprite.textWidth(legend, 2) / 2;

    countXRate(false, false);  // Count and print the total non-zero.
    // Find the highest and lowest values in the array.
    for (int i = XRateHistLen - tft.width() + graphLeftMargin;
         i < XRateHistLen; i++)
    {
      if (XRateHist[i] > 0)  // Exclude 0's from being the yLow winner.
      {
        if (dropDate < 0) dropDate = i ;
        if (XRateHist[i] < yLow) yLow = XRateHist[i];
        if (XRateHist[i] > yHigh) yHigh = XRateHist[i];
      }
    }
    Serial.printf("Actual high & low %.2f & %.2f, ",
                  yHigh, yLow); Serial.flush();
    // Using ceil and floor can leave too much border.
    // Leave a small border hi & lo.
    // yHigh = ceil(yHigh); yLow = floor(yLow);
    // Going with .5 as a high & low border.  Smaller borderland.
    yHigh += .5; yLow -= .5;  // Leave a little border hi & lo.
    yRange = yHigh - yLow;  // The range I will plot including internal
    //  borders.
    Serial.printf("adjusted %.2f & %.2f, yRange %.2f.\r\n",
                  yHigh, yLow, yRange); Serial.flush();
    XRateSprite.fillSprite(TFT_BLACK);
    XRateSprite.setTextColor(TFT_YELLOW, TFT_BLACK);

    // Decorations part 1
    XRateSprite.setTextDatum(TC_DATUM);  // XRate History legend
    XRateSprite.drawString(legend,
                           (tft.width() + graphLeftMargin) / 2, 0, 2);
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
      pixelsPerHundredthV = (tft.height() - graphFloorMargin) /
                            (yRange * 100.);

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
      if (XRateHist[i] > 0)
      {
        thisY = (yHigh - XRateHist[i]) * pixelsPerHundredthV * 100.;
        // Less code that putting this in an }else{ clause.
        dropColor = DarkerGreen;  
        if (i == dropDate) {
          dropColor = TFT_YELLOW;
          iSavDatum = XRateSprite.getTextDatum();
          XRateSprite.setTextDatum(TC_DATUM);
          JulianToGregorian(XRateJulian[i]);
          Serial.printf("Dropping date on column %i\r\n", thisX);
          XRateSprite.drawString(String(XRateMon) + "/" + String(XRateDay),
                                 thisX, tft.height() - graphFloorMargin + 3,
                                 2);  // Use font 2 (quite small).
          XRateSprite.setTextDatum(iSavDatum);
          dropDate += DROP_DATE_EVERY;
        }
//        Serial.printf("Line i %i, thisX %i, thisY %i\r\n",
//                      i, thisX, thisY);
        XRateSprite.drawLine(thisX, tft.height() - graphFloorMargin - 1,
                             thisX, thisY, dropColor);
      }
      thisX++; i++;  // Move to next pixel and XRate array entries.
    }
#endif

    //--------------> Now, draw the dots on the graph.
    // A double pass is done so the lines will not overdraw the dots.
    // For this graph, I am only the dots so they will appear on top 
    // of the lines.

    thisX = graphLeftMargin;

    // I plot one dot per pixel column.  That makes a bar graph.
    i = XRateHistLen - tft.width() + graphLeftMargin;
    while (i < XRateHistLen) {
      if (XRateHist[i] > 0)
      {
        thisY = (yHigh - XRateHist[i]) * pixelsPerHundredthV * 100.;
//        Serial.printf("Circle i %i, thisX %i, thisY %i\r\n",
//                      i, thisX, thisY);
//        XRateSprite.drawSpot(thisX, thisY, 2, TFT_WHITE, TFT_BLACK);
        XRateSprite.fillCircle(thisX, thisY, 2, TFT_WHITE);
      }
      thisX++; i++;  // Move to next pixel and XRate array entries.
    }

    // Decorations part 2
    XRateSprite.setTextDatum(BR_DATUM);
    drawYAxisScaleLines(.5, yLow, yRange);
    XRateSprite.setTextColor(TFT_YELLOW, TFT_BLACK);
    XRateSprite.drawString(String(yHigh, 2),
                           graphLeftMargin - 2, 10, 2);
    XRateSprite.drawString(String(yLow, 2), graphLeftMargin - 2,
                           tft.height() - graphFloorMargin - 5, 2);

    // Draw in the current rate in the upper left of the graph.
    if (fPHP_Rate > 0.) {
      dtostrf(fPHP_Rate, 2, 2, cCharWork);
      strcat(cCharWork, "/$");
      //      iSavDatum = XRateSprite.getTextDatum();
      XRateSprite.setTextColor(TFT_YELLOW, TFT_BLACK);
      XRateSprite.setTextDatum(TL_DATUM);
      XRateSprite.drawString(cCharWork, graphLeftMargin + 10, 20, 4);
      //      XRateSprite.setTextDatum(iSavDatum);
    }
    XRateSprite.pushSprite(0, 0);  // As the teacher says... SHOW YOUR WORK!
    Serial.printf("Graph routine took %lu ms.\r\n",
                  millis() - iStartMillis); Serial.flush();
    refreshGraph = false;
  }
  delay(100);
}
/***************************************************************************/
void drawYAxisScaleLines(float myPctHeight, float myLowEnd, float myRange)
/***************************************************************************/
{ // These are the 1 or more lines across the graph with a label on the left.

  XRateSprite.drawFastHLine(graphLeftMargin,
                            (tft.height() - graphFloorMargin) * myPctHeight,
                            tft.width() - graphLeftMargin, TFT_YELLOW);

  float a = (1. - myPctHeight) * myRange;
  float c = a + myLowEnd;

  XRateSprite.setTextDatum(MR_DATUM);
  XRateSprite.drawString(String(c, 2), graphLeftMargin - 2,
                         (tft.height() - graphFloorMargin) * myPctHeight, 2);
}

/***************************************************************************/
void drawGradientLineTester(int y, uint16_t c0, uint16_t c1,
                            uint16_t (*blendFunc)(uint8_t, uint16_t, uint16_t))
/***************************************************************************/
{
  int width = 240; // adjust to your TFT width
  for (int x = 0; x < width; x++) {
    uint8_t alpha = (uint8_t)((x * 255) / (width - 1));
    uint16_t col = blendFunc(alpha, c0, c1);
    tft.drawPixel(x, y, col);
  }
}
/***************************************************************************/
void drawMultiColorSegments(int y,
                            uint16_t (*blendFunc)(uint8_t, uint16_t, uint16_t))
/***************************************************************************/
{
  // Example: 4 segments across the screen
  int width = 240;
  int segs = 4;
  int segWidth = width / segs;
  uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, RGB565(255, 80, 80)};
  for (int s = 0; s < segs - 1; s++) {
    for (int x = 0; x < segWidth; x++) {
      uint8_t alpha = (uint8_t)((x * 255) / (segWidth - 1));
      uint16_t col = blendFunc(alpha, colors[s], colors[s + 1]);
      tft.drawPixel(s * segWidth + x, y, col);
    }
  }
}
/***************************************************************************/
void testAllBlends() 
/***************************************************************************/
{
  // Long lines
  drawGradientLineTester(20, TFT_GREEN, RGB565(255, 80, 80), alphaBlendLinear);
  drawGradientLineTester(40, TFT_GREEN, RGB565(255, 80, 80), alphaBlendGamma);
  drawGradientLineTester(60, TFT_GREEN, RGB565(255, 80, 80), alphaBlendHSV);
  drawGradientLineTester(80, TFT_GREEN, RGB565(255, 80, 80), alphaBlendLAB);

  // Short multicolor segments
  drawMultiColorSegments(120, alphaBlendLinear);
  drawMultiColorSegments(140, alphaBlendGamma);
  drawMultiColorSegments(160, alphaBlendHSV);
  drawMultiColorSegments(180, alphaBlendLAB);
}

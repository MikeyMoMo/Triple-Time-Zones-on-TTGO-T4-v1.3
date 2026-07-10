/*

Add parameters to the function:

void drawMultiBlendLine(TFT_eSPI *targetPallete,
                         int x0, int y0, int x1, int y1,
                         ColorStage *stages, int numStages,
                         bool honorLineLength,
                         bool rotateColors, unsigned long rotateInterval)

Track rotation timing with a static variable:

static unsigned long nextRotateTime = 0;

Perform rotation when the interval has passed:

if (rotateColors && millis() >= nextRotateTime) {
  nextRotateTime = millis() + rotateInterval;

  // Rotate the sequence to the right
  ColorStage last = stages[numStages - 1];
  for (int i = numStages - 1; i > 0; i--) {
    stages[i] = stages[i - 1];
  }
  stages[0] = last;
}

Place this rotation check at the start of your function, before drawing begins.

Example Integration:

void drawMultiBlendLine(TFT_eSPI *targetPallete,
                         int x0, int y0, int x1, int y1,
                         ColorStage *stages, int numStages,
                         bool honorLineLength,
                         bool rotateColors, unsigned long rotateInterval)
{
  static unsigned long nextRotateTime = 0;

  if (rotateColors && millis() >= nextRotateTime) {
    nextRotateTime = millis() + rotateInterval;

    ColorStage last = stages[numStages - 1];
    for (int i = numStages - 1; i > 0; i--) {
      stages[i] = stages[i - 1];
    }
    stages[0] = last;
  }

  // ... rest of your existing drawing code ...
}

*/

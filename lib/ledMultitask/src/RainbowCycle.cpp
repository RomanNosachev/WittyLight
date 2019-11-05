#include "RainbowCycle.h"

RainbowCycle::RainbowCycle(Adafruit_NeoPixel &strip, short totalSteps) : Effect(strip, totalSteps)
{

}

int RainbowCycle::wheel(byte wheelPos)
{
  wheelPos = 255 - wheelPos;

  if (wheelPos < 85)
    return strip.Color(255 - wheelPos * 3, 0, wheelPos * 3);

  if (wheelPos < 170)
  {
    wheelPos -= 85;

    return strip.Color(0, wheelPos * 3, 255 - wheelPos * 3);
  }

  wheelPos -= 170;

  return strip.Color(wheelPos * 3, 255 - wheelPos * 3, 0);
}

void RainbowCycle::update()
{
  for (int i = 0; i < strip.numPixels(); i++)
    strip.setPixelColor(i, wheel(((i * 256 / strip.numPixels()) + index) & 255));

  strip.show();
  increment();
}

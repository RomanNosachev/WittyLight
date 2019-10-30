#include "RainbowCycle.h"

RainbowCycle::RainbowCycle(Adafruit_NeoPixel &strip, short totalSteps) : Effect(strip, totalSteps)
{

}

void RainbowCycle::update()
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, i * random(255 * 255));
  }

  strip.show();
  increment();
}

#include "Police.h"

Police::Police(Adafruit_NeoPixel &strip, short totalSteps) : Effect(strip, totalSteps)
{

}

void Police::update()
{
  strip.fill(strip.Color(0, 0, 0), 0, strip.numPixels());

  switch (index % 4)
  {
    case 1:
    {
      for (int i = 0; i < strip.numPixels() / 2; i += 2)
        strip.setPixelColor(i, strip.Color(255, 0, 0));

      break;
    }

    case 2:
    {
      for (int i = strip.numPixels() / 2; i < strip.numPixels() - 1; i += 2)
        strip.setPixelColor(i, strip.Color(0, 0, 255));

      break;
    }

    case 3:
    {
      for (int i = 1; i < strip.numPixels() / 2; i += 2)
        strip.setPixelColor(i, strip.Color(255, 0, 0));

      break;
    }

    case 0:
    {
      for (int i = strip.numPixels() / 2 + 1; i < strip.numPixels(); i += 2)
        strip.setPixelColor(i, strip.Color(0, 0, 255));

      break;
    }

    default:
      break;
  }

  strip.show();

  increment();
}

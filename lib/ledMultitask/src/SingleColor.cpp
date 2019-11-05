#include "SingleColor.h"

SingleColor::SingleColor(Adafruit_NeoPixel& strip, Color color) : Effect(strip, color)
{

}

void SingleColor::update()
{
  strip.fill(color.getColor(), 0, strip.numPixels());
  strip.show();
}

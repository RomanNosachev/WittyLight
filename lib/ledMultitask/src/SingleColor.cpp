#include "SingleColor.h"

SingleColor::SingleColor(Adafruit_NeoPixel& strip, Color color) : Effect(strip, color)
{

}

void SingleColor::update()
{
  strip.fill(color.getColor(), 0, strip.numPixels());

  //for (int i = 0; i < strip.numPixels(); i++)
  //  strip.setPixelColor(i, color.getRed(), color.getGreen(), color.getBlue());

  strip.show();
}

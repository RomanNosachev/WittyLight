#include "Pulse.h"

Pulse::Pulse(Adafruit_NeoPixel &strip, Color color) : Effect(strip, color)
{

}

void Pulse::update()
{
  strip.fill(color.getColor(), 0, strip.numPixels());
  strip.setBrightness(index);

  Serial.println(index);

  if (index == 254)
    direction = REVERSE;
  else if (index <= 1)
    direction = FORWARD;

  strip.show();

  increment();
}

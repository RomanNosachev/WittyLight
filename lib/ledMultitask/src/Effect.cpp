#include "Effect.h"

Effect::Effect()
{

}

Effect::Effect(Adafruit_NeoPixel &strip, short totalSteps, Color color, void (*callback)(), Direction direction)
{
  this->strip = strip;
  this->color = color;
  this->totalSteps = totalSteps;
  this->onComplete = callback;
  this->direction = direction;
}

Effect::Effect(Adafruit_NeoPixel &strip, short totalSteps, Color color)
{
  this->strip = strip;
  this->totalSteps = totalSteps;
  this->color = color;
}

Effect::Effect(Adafruit_NeoPixel &strip, Color color)
{
  this->strip = strip;
  this->color = color;
}

Effect::Effect(Adafruit_NeoPixel &strip, short totalSteps)
{
  this->strip = strip;
  this->totalSteps = totalSteps;
}

void Effect::increment()
{
  if (direction == FORWARD)
  {
    index++;

    if (index >= totalSteps)
    {
      index = 0;

      if (onComplete != NULL)
        onComplete();
    }
  }
  else
  {
    --index;

    if (index <= 0)
    {
      index = totalSteps - 1;

      if (onComplete != NULL)
        onComplete();
    }
  }
}

void Effect::setColor(Color color)
{
  this->color = color;
}

Color Effect::getColor()
{
  return color;
}

void Effect::setTotalSteps(short totalSteps)
{
  this->totalSteps = totalSteps;
}

short Effect::getTotalSteps()
{
  return totalSteps;
}

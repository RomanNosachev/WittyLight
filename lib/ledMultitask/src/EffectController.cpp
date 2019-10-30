#include "EffectController.h"

EffectController::EffectController()
{

}

void EffectController::setActivePattern(Effect &effect)
{
  this->activePattern = &effect;
}

void EffectController::setInterval(long interval)
{
  this->interval = interval;
}

void EffectController::update()
{
  if (activePattern != NULL)
  {
    if ((millis() - lastUpdate) > interval)
    {
      lastUpdate = millis();
      activePattern->update();
    }
  }
}

#pragma once

#include "Effect.h"
#include "SingleColor.h"
#include "RainbowCycle.h"

class EffectController
{
  private:
    Effect *activePattern;

    long lastUpdate;
    long interval;

  public:
    EffectController();

    void setActivePattern(Effect &effect);
    void setInterval(long interval);

    void update();
};

#pragma once
#include "Effect.h"

class RainbowCycle : public Effect
{
  public:
    RainbowCycle(Adafruit_NeoPixel &strip, short totalSteps);

    void update() override;
};

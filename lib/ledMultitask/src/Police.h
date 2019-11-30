#pragma once
#include "Effect.h"

class Police : public Effect
{
  public:
    Police(Adafruit_NeoPixel &strip, short totalSteps);

    void update() override;
};

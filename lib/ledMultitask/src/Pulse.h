#pragma once
#include "Effect.h"

class Pulse : public Effect
{
  public:
    Pulse(Adafruit_NeoPixel &strip, Color color);

    void update() override;
};

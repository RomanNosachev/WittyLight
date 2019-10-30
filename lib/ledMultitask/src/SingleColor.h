#pragma once
#include "Effect.h"

class SingleColor : public Effect
{
  public:
    SingleColor(Adafruit_NeoPixel& strip, Color color);

    void update() override;
};

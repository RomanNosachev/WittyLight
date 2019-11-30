#pragma once
#include "Color.h"

enum Direction { FORWARD, REVERSE };

class Effect
{
  protected:
    Adafruit_NeoPixel strip;

    Color color;

    Direction direction = FORWARD;

    short index = 0;
    short totalSteps;

    void increment();

  public:
    Effect();
    Effect(Adafruit_NeoPixel &strip, short totalSteps, Color color, void (*callback)(), Direction direction = FORWARD);
    Effect(Adafruit_NeoPixel &strip, short totalSteps, Color color);
    Effect(Adafruit_NeoPixel &strip, Color color);
    Effect(Adafruit_NeoPixel &strip, short totalSteps);
    void (*onComplete)();
    virtual void update() = 0;

    void setColor(Color color);
    Color getColor();

    void setTotalSteps(short totalSteps);
    short getTotalSteps();
};

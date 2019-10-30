#pragma once
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

class Color
{
  private:
    byte red;
    byte green;
    byte blue;

  public:
    Color();
    Color(byte red, byte green, byte blue);
    Color(String colorString);

    void setRed(byte red);
    void setGreen(byte green);
    void setBlue(byte blue);

    bool setColorString(String colorString);

    byte getRed();
    byte getGreen();
    byte getBlue();

    int getColor();
};

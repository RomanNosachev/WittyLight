#include "Color.h"

Color::Color()
{
  red = 0;
  green = 0;
  blue = 0;
}

Color::Color(byte red, byte green, byte blue)
{
  this->red = red;
  this->green = green;
  this->blue = blue;
}

Color::Color(String colorString)
{
  if (!setColorString(colorString))
    Color();
}

void Color::setRed(byte red)
{
  this->red = red;
}

void Color::setGreen(byte green)
{
  this->green = green;
}

void Color::setBlue(byte blue)
{
  this->blue = blue;
}

bool Color::setColorString(String colorString)
{
  if (colorString.length() == 7)
  {
    red = strtol(colorString.substring(1, 3).c_str(), NULL, 16);
    green = strtol(colorString.substring(3, 5).c_str(), NULL, 16);
    blue = strtol(colorString.substring(5, 7).c_str(), NULL, 16);

    return true;
  }

  return false;
}

byte Color::getRed()
{
  return red;
}

byte Color::getGreen()
{
  return green;
}

byte Color::getBlue()
{
  return blue;
}

int Color::getColor()
{
  return Adafruit_NeoPixel::Color(red, green, blue);
}

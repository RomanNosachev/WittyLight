#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

String ssid;
String password;

AsyncWebServer server(80);

const String CONFIG_FILE = "/config.json";

bool fileSystemMounted = false;

const int led_pin = 5;
const int led_count = 60;

struct Color {
  byte RED;
  byte GREEN;
  byte BLUE;
} ledValues = {0, 0, 0};

String currentColorString = "#000000";

const int RGB_PATTERN_LENGTH = 6;

const String EFFECT_REQUEST_PATTERN = "effect";
const String COLOR_REQUEST_PATTERN = "color";

bool saveConfig();
bool loadConfig();

void displayStaticColor();

bool handleMessage();
bool processRequest(String request);

Color parseRGB(String tempColorString);
bool validateColorString(String tempColorString);

Adafruit_NeoPixel strip(led_count, led_pin, NEO_GRB + NEO_KHZ800);

bool operator==(const Color& lColor, const Color& rColor);
bool operator!=(const Color& lColor, const Color& rColor);

void setup()
{
  strip.begin();
  strip.show();
  strip.setBrightness(255);

  Serial.begin(115200);

  if (loadConfig())
    displayStaticColor();

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.persistent(false);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String requestString;

    if (request->hasParam(COLOR_REQUEST_PATTERN))
    {
      requestString = request->getParam(COLOR_REQUEST_PATTERN)->value();

      Serial.println(requestString);

      processColorRequest(requestString);
    }

    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.begin();

  Serial.println("[Server started]");
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop()
{

}

String processor(const String& var)
{
  if (var == "COLOR_STRING")
  {
    Serial.println("Cathed");
    Serial.println(currentColorString);

    return currentColorString;
  }
}

Color parseRGB(String colorString)
{
  Color color;

  Serial.println("PARSE");
  Serial.println(colorString);

  if (colorString.length() == 7)
  {
    color.RED = strtol(colorString.substring(1, 3).c_str(), NULL, 16);
    color.GREEN = strtol(colorString.substring(3, 5).c_str(), NULL, 16);
    color.BLUE = strtol(colorString.substring(5, 7).c_str(), NULL, 16);

    currentColorString = colorString;

    Serial.println(currentColorString);

    return color;
  }
  else
    return ledValues;
}

bool processColorRequest(String request)
{
  bool result = false;

  Serial.println("Request:");
  Serial.println(request);

  Color color = parseRGB(request);

  if (ledValues != color)
  {
    ledValues = color;

    displayStaticColor();
    saveConfig();

    Serial.println("Color changed");

    result = true;
  }

  return result;
}

bool saveConfig()
{
  bool result = false;

  if (fileSystemMounted)
  {
    File jsonFile = SPIFFS.open(CONFIG_FILE, "w");

    if (jsonFile)
    {
      StaticJsonDocument<512> doc;

      doc["SSID"] = ssid;
      doc["password"] = password;
      doc["LED"] = currentColorString;

      if (serializeJson(doc, jsonFile) == 0)
        Serial.println("! Failed to write to file !");
      else
        result = true;
    }
    else
      Serial.println("! Unable to create file to save !");

    jsonFile.close();
  }
  else
    Serial.println("! File system not mounted, cannot be saved !");

  return result;
}

bool loadConfig()
{
  bool result = true;

  if (SPIFFS.begin())
  {
    Serial.println();
    Serial.println("Mounted file system");

    fileSystemMounted = true;

    File jsonFile;

    if (SPIFFS.exists(CONFIG_FILE))
      jsonFile = SPIFFS.open(CONFIG_FILE, "r");

    if (jsonFile)
    {
      StaticJsonDocument<512> doc;

      DeserializationError error = deserializeJson(doc, jsonFile);

      if (!error)
      {
        ssid = doc["SSID"] | "1";
        password = doc["password"] | "11111111";
        currentColorString = doc["LED"] | "#000000";

        Serial.println(ssid);
        Serial.println(password);
        Serial.println(currentColorString);

        if (validateColorString(currentColorString))
          ledValues = parseRGB(currentColorString);
        else
        {
          Serial.println("! Invalid color value, using default !");

          result = false;
        }
      }
      else
      {
        Serial.println("! Failed load configuration, using default !");
        Serial.print("Deserialization error: ");
        Serial.println(error.c_str());

        result = false;
      }
    }
    else
    {
      Serial.println("! Configuration file does not exist !");

      result = false;
    }

    jsonFile.close();
  }
  else
  {
    Serial.println("! File system not mounted !");

    result = false;
  }

  return result;
}

void displayStaticColor()
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, ledValues.RED,
                          ledValues.GREEN,
                          ledValues.BLUE);
  }

  strip.show();
}

bool validateColorString(String colorString)
{
  //fixit
  return true;
}

bool operator==(const Color& lColor, const Color& rColor)
{
  return (lColor.RED == rColor.RED &&
          lColor.GREEN == rColor.GREEN &&
          lColor.BLUE == rColor.BLUE);
}

bool operator!=(const Color& lColor, const Color& rColor)
{
  return (lColor.RED != rColor.RED ||
          lColor.GREEN != rColor.GREEN ||
          lColor.BLUE != rColor.BLUE);
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait)
{
  for (int i = 0; i < strip.numPixels(); i++) // For each pixel in strip...
  {
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait)
{
  for (int a = 0; a < 10; a++) // Repeat 10 times...
  {
    for (int b = 0; b < 3; b++) //  'b' counts from 0 to 2...
    {
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for (int c = b; c < strip.numPixels(); c += 3)
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'

      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait)
{
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256)
  {
    for (int i = 0; i < strip.numPixels(); i++) // For each pixel in strip...
    {
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:

      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));

      if (handleMessage())
        return;
    }

    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait)
{
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for (int a = 0; a < 30; a++) // Repeat 30 times...
  {
    for (int b = 0; b < 3; b++) //  'b' counts from 0 to 2...
    {
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for (int c = b; c < strip.numPixels(); c += 3)
      {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }

      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

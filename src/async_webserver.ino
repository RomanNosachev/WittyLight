#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#include "Effect.h"
#include "RainbowCycle.h"
#include "SingleColor.h"

#include "EffectController.h"

String ssid;
String password;

AsyncWebServer server(80);

IPAddress ap_local_IP(192, 168, 0, 1);
IPAddress ap_gateway(192, 168, 0, 1);
IPAddress ap_subnet(255, 255, 255, 0);

const int led_pin = 5;
const int led_count = 60;

Adafruit_NeoPixel strip(led_count, led_pin, NEO_GRB + NEO_KHZ800);

long lastUpdate;
long interval = 100;

EffectController controller;

RainbowCycle effect(strip, 255);
SingleColor singleColor(strip, Color(0, 0, 0));

const String CONFIG_FILE = "/config.json";

bool fileSystemMounted = false;

struct SColor {
  byte RED;
  byte GREEN;
  byte BLUE;
} ledValues = {0, 0, 0};

String currentColorString = "#000000";

const int RGB_PATTERN_LENGTH = 6;

const String EFFECT_REQUEST_PATTERN = "effect";
const String COLOR_REQUEST_PATTERN = "color";

void onWiFiConnected();

bool saveConfig();
bool loadConfig();

void displayStaticColor();

bool processColorRequest(String request);

SColor parseRGB(String tempColorString);
bool validateColorString(String tempColorString);

void setup()
{
  strip.begin();
  strip.show();
  strip.setBrightness(255);

  Serial.begin(115200);

  loadConfig();
  //if (loadConfig())
  //  displayStaticColor();

  controller.setInterval(200);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attemptsCount = 30;

  while (WiFi.status() != WL_CONNECTED && attemptsCount--)
  {
    delay(500);
    Serial.print(".");
  }

  if (attemptsCount <= 0)
  {
    Serial.println();
    Serial.println("! Could not connected to " + ssid + " !");
    Serial.println("Switch to access point mode");

    WiFi.disconnect();

    WiFi.mode(WIFI_AP);

    Serial.print("Setting soft-AP configuration ... ");
    Serial.println(WiFi.softAPConfig(ap_local_IP, ap_gateway, ap_subnet) ? "Ready" : "Failed!");

    bool apReady = WiFi.softAP("WittyLight_" + WiFi.softAPmacAddress());

    Serial.print("Starting soft-AP ... ");
    Serial.println(apReady ? "Ready" : "Failed!");

    if (apReady)
    {
      Serial.println("Soft-AP MAC: ");
      Serial.println(WiFi.softAPmacAddress());

      Serial.println("Soft-AP local IP: ");
      Serial.println(WiFi.softAPIP());

      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/config.html", String(), false, processor);
      });

      server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
      });

      server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("ssid", true) && request->hasParam("password", true))
        {
          ssid = request->getParam("ssid", true)->value();
          password = request->getParam("password", true)->value();

          Serial.println("SSID and password received, connection attempt...");

          request->send(SPIFFS, "/config.html", String(), false, processor);

          saveConfig();

          SPIFFS.end();
          ESP.restart();
        }

      });

      server.begin();
    }
  }
  else
    onWiFiConnected();
}

void onWiFiConnected()
{
  Serial.println();
  Serial.println("WiFi connected");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String requestString;

    if (request->hasParam(COLOR_REQUEST_PATTERN))
    {
      requestString = request->getParam(COLOR_REQUEST_PATTERN)->value();

      processColorRequest(requestString);
    }

    if (request->hasParam(EFFECT_REQUEST_PATTERN))
    {
      requestString = request->getParam(EFFECT_REQUEST_PATTERN)->value();

      processEffectRequest(requestString);
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
  controller.update();
}

String processor(const String& var)
{
  if (var == "COLOR_STRING")
  {
    return currentColorString;
  }
}

SColor parseRGB(String colorString)
{
  SColor color;

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

bool processEffectRequest(String request)
{
  Serial.println("Effect request:");
  Serial.println(request);

  if (request == "rainbowCycle")
  {
    controller.setActivePattern(effect);

    return true;
  }

  return false;
}

bool processColorRequest(String request)
{
  Serial.println("Color request:");
  Serial.println(request);

  Color color(request);
  singleColor.setColor(color);

  controller.setActivePattern(singleColor);

  return true;
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

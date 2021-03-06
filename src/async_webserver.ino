#include <FS.h>                 //for ESP8266
//#include <SPIFFS.h>           //for ESP32
#include <ESP8266WiFi.h>        //for ESP8266
//#include <Wifi.h>             //for ESP32
#include <ESPAsyncWebServer.h>
#include <ESPAsyncUDP.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#include "Effect.h"
#include "RainbowCycle.h"
#include "SingleColor.h"
#include "Police.h"
#include "Pulse.h"

#include "EffectController.h"

#include <PubSubClient.h>

#define BUFFER_SIZE 100

String ssid;
String password;

AsyncWebServer server(80);
AsyncUDP udp;

const uint16_t updPort = 1234;

String mqttServer = "192.168.0.100";             //host
const int mqttPort = 1883;                       //port
const char *mqttUser = "";                       //login
const char *mqttPass = "";                       //pass

const char *lightTopic = "light";                //topic name
const char *effectTopic = "light/effect";
const char *colorTopic = "light/color";

WiFiClient wclient;
PubSubClient client(wclient);

String clientId;

IPAddress apLocal_IP(192, 168, 0, 1);
IPAddress apGateway(192, 168, 0, 1);
IPAddress apSubnet(255, 255, 255, 0);

const int ledPin = 5;
const int ledCount = 60;

Adafruit_NeoPixel strip(ledCount, ledPin, NEO_GRB + NEO_KHZ800);

long lastUpdate;
long interval = 100;

EffectController controller;

RainbowCycle effect(strip, 255);
Police policeEffect(strip, 255);
Pulse pulseEffect(strip, Color(0, 255, 0));
SingleColor singleColor(strip, Color(0, 0, 0));

const String CONFIG_FILE = "/config.json";

bool fileSystemMounted = false;

String currentColorString = "#000000";

const int RGB_PATTERN_LENGTH = 6;

const String EFFECT_REQUEST_PATTERN = "effect";
const String COLOR_REQUEST_PATTERN = "color";

void onWiFiConnected();

bool saveConfig();
bool loadConfig();

void displayStaticColor();

bool processColorRequest(String request);

bool validateColorString(String tempColorString);

void reconnect()
{
  Serial.println("Connection to MQTT-broker attempt...");

  client.connect(clientId.c_str(), mqttUser, mqttPass);

  if (client.connected())
  {
    Serial.println("Connected!");

    client.subscribe(lightTopic);
    client.subscribe(effectTopic);
    client.subscribe(colorTopic);
  }
  else
  {
    Serial.println("Connection failed!");
    Serial.println(client.state());
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.println();
  Serial.print(topic);
  Serial.print(": ");

  payload[length] = '\n';
  char* encryptedPayload = (char*) payload;

  String message = String(encryptedPayload);
  message = message.substring(0, message.indexOf('\n'));

  if (strcmp(topic, effectTopic) == 0)
    processEffectRequest(message);

  if (strcmp(topic, colorTopic) == 0)
    processColorRequest(message);
}

void onUdpPacket(AsyncUDPPacket packet)
{
  Serial.print("Datagram received from: ");
  Serial.print(packet.remoteIP());
  Serial.print(" with content: ");
  Serial.write(packet.data(), packet.length());
  Serial.println();

  String broadcastMessage = (char*) packet.data();

  if (broadcastMessage == "WittyServer" && !client.connected())
  {
    mqttServer = packet.remoteIP().toString().c_str();

    Serial.println("Server IP changed");
  }
}

void setup()
{
  strip.begin();
  strip.show();
  strip.setBrightness(255);

  Serial.begin(115200);

  loadConfig();

  controller.setInterval(200);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid.c_str(), password.c_str());

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
    Serial.println(WiFi.softAPConfig(apLocal_IP, apGateway, apSubnet) ? "Ready" : "Failed!");

    bool apReady = WiFi.softAP(("WittyLight_" + WiFi.softAPmacAddress()).c_str());

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
  {
    onWiFiConnected();

    if (udp.listen(updPort))
    {
      Serial.print("UDP Listening on: ");
      Serial.print(WiFi.localIP());
      Serial.print(":");
      Serial.println(updPort);

      udp.onPacket([](AsyncUDPPacket packet) {
        onUdpPacket(packet);
      });
    }

    client.setServer(mqttServer.c_str(), mqttPort);
    client.setCallback(callback);

    clientId = "WittyLight_" + WiFi.softAPmacAddress();
  }
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
  if (millis() - lastUpdate >= 5000)
  {
    lastUpdate = millis();

    if (!client.connected())
      reconnect();
  }

  client.loop();

  controller.update();
}

String processor(const String& var)
{
  if (var == "COLOR_STRING")
  {
    return currentColorString;
  }
}

bool processEffectRequest(String request)
{
  Serial.println();
  Serial.println("Effect request:");
  Serial.println(request);

  if (request == "rainbowCycle")
  {
    controller.setActivePattern(effect);
    controller.setInterval(50);
  }

  if (request == "police")
  {
    controller.setActivePattern(policeEffect);
    controller.setInterval(200);
  }

  if (request == "pulse")
  {
    controller.setActivePattern(pulseEffect);
    controller.setInterval(20);
  }

  return true;
}

bool processColorRequest(String request)
{
  Serial.println();
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
        //  fixit
          Serial.println("Parse");
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

bool validateColorString(String colorString)
{
  //fixit
  return true;
}

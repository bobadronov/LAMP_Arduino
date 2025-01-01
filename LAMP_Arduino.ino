/*
    https://github.com/CDFER/Captive-Portal-ESP32/tree/main
    ESP tries to connect to WiFi network and if it fails,
    switch to access point mode to provide access to the device.
    There you can connect to the access point and use the app to configure WiFi,
    or at: http://device_name.local/wifi .
    Management: http://device_name.local/ .
    Firmware update: http://device_name.local/update .
    (http://main_led.local/update)
    TODO:
      Encoder management
      GyverNTP
*/

// ======================= Constants and Libraries =======================
#include <ArduinoJson.h>
#include <FastLED.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include <Preferences.h>
#include <EEPROM.h>  // Для работы с энергонезависимой памятью
#include <ESPmDNS.h>
#include <DHT.h>
#include <esp_wifi.h>
#include <GyverDS3231.h>
#include <GyverNTP.h>
#include <AutoOTA.h>

#define VERSION "1.0.5"
#define AP_SSID "ESP32-AP-LED"
#define AP_PASSWORD "123456789"
#define DHT_PIN 15
#define DHT_TYPE DHT11
#define DATA_PIN 12
#define SAVE_INTERVAL 120000  // Интервал в миллисекундах (1 минута = 60000)

// ======================= Variables =======================
// WiFi Settings
String SSID = "VapeCity";
String PASSWORD = "ivus31415926";
String HOSTNAME = "ESP32-LED";
//
bool espInAPMode = false;

// LED Settings
#define NUM_LEDS 600
uint16_t REAL_NUM_LEDS = 5;
CRGB leds[NUM_LEDS];
CRGB color = CRGB::White;
CRGB customColorsArray[NUM_LEDS];
uint8_t commonBrightness = 255;
bool ledState = false;
bool flagIsStatic = true;
uint8_t flagSpeed = 1;
float rainbowSpeed = 2;
bool rainbowIsStatic = false;

// Temperature and Humidity
float temperature = 0.0;
float humidity = 0.0;
DHT dht(DHT_PIN, DHT_TYPE);

// Modes
const char *modeList[] = {
  "STATIC_COLOR",
  "RAINBOW",
  "BREATHING",
  "STROBE",
  "SPARKLE",
  "RUNNING LIGHTS",
  "FIRE",
  "COLOR WIPE",
  "METEOR",
  "FLAG OF UKRAINE",
  "CUSTOM LIGHTING"
};
const uint8_t NUM_MODES = sizeof(modeList) / sizeof(modeList[0]);
uint8_t currentMode = 6;

// Timer
GyverDS3231 ds;
uint8_t timerHour = 0, timerMinute = 0, timerDay = 1, timerMonth = 1;
uint16_t timerYear = 2024;
bool timerIsActive = false;

// Preferences
Preferences preferences;

// WebServer and WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AutoOTA ota(VERSION, "bobadronov/LAMP_Arduino");

// DNS Server
DNSServer dnsServer;
bool dnsServerStarted = false;

// Auto Save Timer
unsigned long lastSaveTime = 0;

// ======================= Function Declarations =======================
void updateDHT();
void checkUpdate();
void setupNetwork();
void saveToMemory();
void updateLEDState();
void handleAutoSave();
void loadSettingsFromMemory();
void espConectionStatusIndicator();
void checkTimer(AsyncWebSocket *server);
void sendCurrentStatus(AsyncWebSocket *server);
void onWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

// ======================= Setup Function =======================
void setup() {
  setStampZone(2);
  // Serial.setTxBufferSize(1024);
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial)
    ;  // Wait for Serial to be ready

  setupNetwork();
  checkUpdate();
  if (MDNS.begin(HOSTNAME)) {
    Serial.println("mDNS responder started");
    Serial.print("You can access the server at http://");
    Serial.print(HOSTNAME);
    Serial.println(".local/");
    MDNS.addService("http", "tcp", 80);
  } else Serial.println("Error setting up mDNS responder!");
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.begin();
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  loadSettingsFromMemory();
  Wire.begin();
  if (!ds.begin()) ds.setBuildTime();
  Serial.println("\nSetup completed!\n\n");
}

// ======================= Main Loop =======================
void loop() {
  espConectionStatusIndicator();
  ws.cleanupClients();
  updateLEDState();
  updateDHT();
  if (dnsServerStarted) dnsServer.processNextRequest();
  checkTimer(&ws);
  if (ota.tick()) {
    Serial.println((int)ota.getError());
  }
  handleAutoSave();
}

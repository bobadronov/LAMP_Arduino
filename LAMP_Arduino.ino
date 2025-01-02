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
uint8_t gmt = 2;
const String ntpUrl = "0.ua.pool.ntp.org";
// #define DS3231_MODULE_ENABLE
// #define DTH_ENABLE // comment out if not enabled
// #define OTA_ENABLE // comment out if not enabled
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
#define NUM_LEDS 1800
uint16_t REAL_NUM_LEDS = 5;
CRGB leds[NUM_LEDS];
CRGB color = CRGB::White;
CRGB customColorsArray[NUM_LEDS];
CRGB customGradient[2];
uint8_t commonBrightness = 255;
bool ledState = false;
bool flagIsStatic = true;
uint8_t flagSpeed = 1;
float rainbowSpeed = 2;
bool rainbowIsStatic = false;
float breathingSpeed = 100;
// Temperature and Humidity
float temperature = 0.0;
float humidity = 0.0;
DHT dht(DHT_PIN, DHT_TYPE);
// Modes
const char *modeList[] = {
  "STATIC COLOR",
  "RAINBOW",
  "BREATHING",
  "GRADIENT FILL",
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
uint8_t timerHour = 0, timerMinute = 0, timerDay = 0, timerMonth = 0;
uint16_t timerYear = 0;
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
void updateLEDState();
void handleAutoSave();
void loadSettingsFromMemory();
void espConectionStatusIndicator();
void checkTimer(AsyncWebSocket *server);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

// ======================= Setup Function =======================
void setup() {
  // setStampZone(2);
  // Serial.setTxBufferSize(1024);
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial)
    ;  // Wait for Serial to be ready

  setupNetwork();
#ifdef OTA_ENABLE
  checkUpdate();
#endif
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
#ifdef DS3231_MODULE_ENABLE
  if (!ds.begin()) ds.setBuildTime();
#endif
  NTP.begin(gmt);  // запустить
  NTP.setPeriod(5000);
  NTP.setHost(ntpUrl);
  Serial.println("\nSetup completed!\n\n");
}

// ======================= Main Loop =======================
void loop() {
  espConectionStatusIndicator();
  ws.cleanupClients();
  updateLEDState();
#ifdef DTH_ENABLE
  updateDHT();
#endif
  if (dnsServerStarted) dnsServer.processNextRequest();
  checkTimer(&ws);
#ifdef OTA_ENABLE
  if (ota.tick()) {
    Serial.println((int)ota.getError());
  }
#endif
  NTP.tick();  // вызывать тикер в loop
  handleAutoSave();
}

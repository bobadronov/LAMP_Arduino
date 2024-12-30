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

const char *VERSION = "0.0.4";

#include <ArduinoJson.h>
#include <FastLED.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <DHT.h>
#include <esp_wifi.h>
#include <GyverDS3231.h>
#include <GyverNTP.h>
#include <AutoOTA.h>

#define AP_SSID "ESP32-AP-LED"
#define AP_PASSWORD "123456789"
// String SSID = "krishome";
// String PASSWORD = "sladkaya";
String SSID = "VapeCity";
String PASSWORD = "ivus31415926";
String HOSTNAME = "ESP32-LED";  // Имя хоста для mDNS
// Настройки DHT11
#define DHT_PIN 15
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);
// Настройки WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AutoOTA ota(VERSION, "bobadronov/LAMP_Arduino");
const char *modeList[] = {
  "STATIC_COLOR",    // Однотонный статический цвет
  "RAINBOW",         // Радуга
  "BREATHING",       // Эффект дыхания
  "STROBE",          // Стробоскоп
  "SPARKLE",         // Искры
  "RUNNING_LIGHTS",  // Бегущие огни
  "FIRE",            // Эффект огня
  "COLOR_WIPE",      // Заливка цвета
  "METEOR",          // Эффект метеора
  "FLAG OF UKRAINE",
  "CUSTOM LIGHTING"
};

const uint8_t NUM_MODES = sizeof(modeList) / sizeof(modeList[0]);  // Total number of modes

#define DATA_PIN 12
#define MAX_BRIGHTNESS 220
const uint16_t NUM_LEDS = 10;
CRGB leds[NUM_LEDS];
CRGB color = CRGB::White;
CRGB customColorsArray[NUM_LEDS];  // Массив для хранения всех цветов светодиодов

// Переменная состояния ленты
bool ledState = false;
// Flag
bool flagIsStatic = true;
uint8_t flagSpeed = 1;
// Rasinbow
float rainbowSpeed = 2;
bool rainbowIsStatic= false;
// Переменные температуры и влажности
float temperature = 0.0;
float humidity = 0.0;
// Переменная текущего режима работы ленты
uint8_t currentMode = 1;
// timer
GyverDS3231 ds;
uint8_t timerHour = 0;
uint8_t timerMinute = 0;
uint8_t timerDay = 1;
uint8_t timerMonth = 1;
uint16_t timerYear = 2024;
bool timerIsActive = false;

Preferences preferences;

DNSServer dnsServer;
bool dnsServerStarted = false;
// Функция для управления режима работы ленты
void updateLEDState();
// Обработка Датчика влажности и температуры
void updateDHT();
// Обработка сообщений WebSocket
void onWebSocketMessage(void *arg, uint8_t *data, size_t len);
// Функция обработки подключения клиента
void sendCurrentStatus(AsyncWebSocket *server);
// Обработка событий WebSocket
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
// Настройка Сети и служб
void setupNetwork();
// Timer
void checkTimer(AsyncWebSocket *server);
void checkUpdate();
void setup() {
  setStampZone(2);
  Serial.setTxBufferSize(1024);
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  // Wait for the Serial object to become available.
  while (!Serial)
    ;

  // preferences.begin("led_settings", false);

  // Проверка, есть ли сохраненные данные
  // if (preferences.isKey("NUM_LEDS")) {
  //   NUM_LEDS = preferences.getInt("NUM_LEDS", DEFAULT_NUM_LEDS);
  //   Serial.print("Loaded numLEDs from memory: ");
  //   Serial.println(NUM_LEDS);
  // }
  // Настройка светодиодной ленты
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(MAX_BRIGHTNESS);
  setupNetwork();

  // Start mDNS
  if (MDNS.begin(HOSTNAME)) {
    Serial.println("mDNS responder started");
    Serial.println("You can access the server at http://esp32.local/");
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("Error setting up mDNS responder!");
  }
  // Инициализация WebSocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  // Запуск HTTP-сервера
  server.begin();

  Wire.begin();

  if (!ds.begin()) ds.setBuildTime();
  // checkUpdate();
  Serial.println("\nSetup completed!");
  Serial.println();
}

void loop() {
  // Обработка WebSocket событий
  ws.cleanupClients();
  updateLEDState();
  updateDHT();
  if (dnsServerStarted) dnsServer.processNextRequest();
  checkTimer(&ws);
  if (ota.tick()) {
    Serial.println((int)ota.getError());
  }
}

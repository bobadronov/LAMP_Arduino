/*
  https://github.com/CDFER/Captive-Portal-ESP32/tree/main
  Есп пытаеться подключиться к WiFi сети и в случае неудачи
  переключиться в режим точки доступа для предоставления доступа к устройству.
  Там вы можете подключиться к точке доступа и с помощью приложения настроить WiFi,
  или по адресу: http://device_name.local/wifi .
  Управление: http://device_name.local/ .
  Обновление прошивки: http://device_name.local/update .
  (http://main_led.local/update)
  TODO:
  Управление енкодером
*/

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
#define AP_SSID "ESP32-AP-LED"
#define AP_PASSWORD "123456789"
// String SSID = "krishome";
// String PASSWORD = "sladkaya";
String SSID = "VapeCity";
String PASSWORD = "ivus31415926";
String hostname = "ESP32-LED";  // Имя хоста для mDNS
// Настройки DHT11
#define DHT_PIN 15
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);
// Настройки WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

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
  "FLAG OF UKRAINE"
};

const uint8_t NUM_MODES = sizeof(modeList) / sizeof(modeList[0]);  // Total number of modes

#define DATA_PIN 12
#define NUM_LEDS 5
#define MAX_BRIGHTNESS 220
CRGB leds[NUM_LEDS];
CRGB color = CRGB::White;
// Переменная состояния ленты
bool ledState = false;
// Flag
bool flagIsStatic = true;
uint8_t flagSpeed = 1;
// Rasinbow
float rainbowSpeed = 2;
// Переменные температуры и влажности
float temperature = 0.0;
float humidity = 0.0;
// Переменная текущего режима работы ленты
uint8_t currentMode = 0;
// timer
GyverDS3231 ds;
bool timerIsActive = false;
uint8_t timerHour = 0;
uint8_t timerMinute = 0;

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
void checkTimer();

void setup() {
 
  Serial.setTxBufferSize(1024);
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  // Wait for the Serial object to become available.
  while (!Serial)
    ;
  // Настройка светодиодной ленты
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(MAX_BRIGHTNESS);
  setupNetwork();

  // Start mDNS
  if (MDNS.begin(hostname)) {
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
  setStampZone(-2);
  if (!ds.begin()) ds.setBuildTime();

  Serial.println("\nSetup completed!");
}

void loop() {
  // Обработка WebSocket событий
  ws.cleanupClients();
  updateLEDState();
  updateDHT();
  if (dnsServerStarted) dnsServer.processNextRequest();
  if (ds.tick() && timerIsActive) {
    Serial.println(ds.toString());  // вывод даты и времени строкой
  }
  checkTimer();
}

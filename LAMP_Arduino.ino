/*
Button Usage Manual:

Button Functionality:

1. Single Click:
   - Toggles the LED state (on/off).

2. Double Click:
   - Switches between predefined offline modes.

3. Triple Click:
   - Changes the color from a predefined color palette.

4. Hold and Release:
   - Adjusts parameters with different hold clicks:
     
     a) 1 Hold Click:
        - Adjusts brightness.
        - Brightness increases or decreases in steps of 5 until it reaches the maximum (255) or minimum (10).
        - Releases toggle the direction (increasing/decreasing).

     b) 2 Hold Clicks:
        - Adjusts animation speed.
        - Speed increases or decreases in steps of 10 until it reaches the maximum (150.0f) or minimum (10.0f).
        - Releases toggle the direction (increasing/decreasing).

---

Network Information:

- ESP tries to connect to a WiFi network. If it fails:
  1. Switch to access point mode to provide access to the device.
  2. Connect to the access point and use the app to configure WiFi.
  3. Alternatively, use the following addresses:
     - WiFi Configuration: http://device_name.local/wifi
     - Management: http://device_name.local/
     - Firmware Update: http://device_name.local/update
       (Example: http://main_led.local/update)

---

TODO:
- Additional functionality improvements

---

Source: https://github.com/CDFER/Captive-Portal-ESP32/tree/main
*/

#define LED_PIN 12
#define DHT_PIN 2
#define DHT_TYPE DHT11
#define BUTTON_PIN 15
const String ntpUrl = "ua.pool.ntp.org";
// #define DS3231_MODULE_ENABLE
// #define DTH_ENABLE // comment out if not enabled
// #define OTA_ENABLE // comment out if not enabled

// ======================= Constants and Libraries =======================
#define DEBUG_ENABLE
#include "config.h"

#define AP_SSID "ESP32-AP-LED"
#define AP_PASSWORD "123456789"
#define SAVE_INTERVAL 120000  // Интервал в миллисекундах (1 минута = 60000)

// ======================= Variables =======================
// WiFi Settings
String SSID = "VapeCity";
String PASSWORD = "ivus31415926";
String HOSTNAME = "ESP32-LED";
//
bool espInAPMode = false;

// LED Settings

uint16_t REAL_NUM_LEDS = 5;
CRGB leds[NUM_LEDS];
CRGB color = CRGB::White;
CRGB colors[] = {
  CRGB::Red,
  CRGB::Green,
  CRGB::Blue,
  CRGB::Yellow,
  CRGB::Cyan,
  CRGB::Magenta,
  CRGB::White
};
uint8_t currentColorIndex = 0;  // Индекс текущего цвета
const uint8_t numColors = sizeof(colors) / sizeof(colors[0]);
CRGB customColorsArray[10];
CRGB customGradient[2];
CRGB fadeColor1 = CRGB::Yellow;
CRGB fadeColor2 = CRGB::Blue;
uint8_t commonBrightness = 255;
bool ledState = true;
bool flagIsStatic = true;
bool rainbowIsStatic = false;
float animationSpeed = 100;  //10-150

// Modes
const char *modeList[] = {
  "STATIC COLOR",  //offline
  "RAINBOW",       //offline
  "GRADIENT FILL",
  "SPARKLE",         //offline
  "RUNNING LIGHTS",  //offline
  "FIRE",            //offline
  "FADE1",
  "FADE2",
  "METEOR",           //offline
  "FLAG OF UKRAINE",  //offline
  "CUSTOM LIGHTING"
};
const int offlineModes[] = { 0, 1, 3, 4, 5, 8, 9 };                    // Indices of offline modes
int numOfflineModes = sizeof(offlineModes) / sizeof(offlineModes[0]);  // Number of offline modes

const uint8_t NUM_MODES = sizeof(modeList) / sizeof(modeList[0]);
uint8_t currentMode = 0;
// Temperature and Humidity
float temperature = 0.0;
float humidity = 0.0;
DHT dht(DHT_PIN, DHT_TYPE);
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

// Кнопка
// HIGH_PULL - кнопка подключена к GND, пин подтянут к VCC (BTN_PIN --- КНОПКА --- GND)
// LOW_PULL  - кнопка подключена к VCC, пин подтянут к GND
// по умолчанию стоит HIGH_PULL
// NORM_OPEN - нормально-разомкнутая кнопка
// NORM_CLOSE - нормально-замкнутая кнопка
// по умолчанию стоит NORM_OPEN
GButton btn(BUTTON_PIN, HIGH_PULL, NORM_CLOSE);

// ======================= Function Declarations =======================
void updateDHT();
void checkUpdate();
void setupNetwork();
void handleButton();
void updateLEDState();
void handleAutoSave();
void loadSettingsFromMemory();
void espConectionStatusIndicator();
void checkTimer(AsyncWebSocket *server);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

// ======================= Setup Function =======================
void setup() {
  setStampZone(2);
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial)
    ;  // Wait for Serial to be ready
  Wire.begin();
  setupNetwork();
#ifdef OTA_ENABLE
  checkUpdate();
#endif
  if (MDNS.begin(HOSTNAME)) {
    DEBUG_PRINTLN("mDNS responder started");
    DEBUG_PRINT("You can access the server at http://");
    DEBUG_PRINT(HOSTNAME);
    DEBUG_PRINTLN(".local/");
    MDNS.addService("http", "tcp", 80);
  } else DEBUG_PRINTLN("Error setting up mDNS responder!");
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.begin();
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  loadSettingsFromMemory();
  NTP.begin();  // запустить
  // NTP.begin(gmt);  // запустить
  NTP.setPeriod(10000);
  NTP.setHost(ntpUrl);

#ifdef DS3231_MODULE_ENABLE
  if (!ds.begin()) ds.setBuildTime();
#endif

  if (NTP.online() && NTP.tick()) {
    Datime dt(NTP);
    ds.setTime(dt);
  }
  btn.setTickMode(AUTO);
  DEBUG_PRINTLN("\n\nSetup completed!\n\n");
}

// ======================= Main Loop =======================
void loop() {
  espConectionStatusIndicator();
  // ws.cleanupClients();
  handleButton();
  updateLEDState();
#ifdef DTH_ENABLE
  updateDHT();
#endif
  if (dnsServerStarted) dnsServer.processNextRequest();
  checkTimer(&ws);
#ifdef OTA_ENABLE
  if (ota.tick()) {
    DEBUG_PRINTLN((int)ota.getError());
  }
#endif
  NTP.tick();  // вызывать тикер в loop
  // handleAutoSave();
}

#ifndef CONFIG_H
#define CONFIG_H


#define VERSION "1.0.6"
#define NUM_LEDS 1800

#include <DHT.h>
#include <EEPROM.h>  // Для работы с энергонезависимой памятью
#include <ESPmDNS.h>
#include <AutoOTA.h>
#include <FastLED.h>
#include <AsyncTCP.h>
#include <esp_wifi.h>
#include <GyverNTP.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <GyverDS3231.h>
#include <GyverButton.h>
#include <ESPAsyncWebServer.h>
// Якщо DEBUG_ENABLE не визначено, можна задати його тут (необов'язково)

#define DEBUG_ENABLE  // Увімкнути за замовчуванням, якщо не визначено

// Макроси для виводу у Serial
#ifdef DEBUG_ENABLE
#define DEBUG_PRINT(x) Serial.print(x)                            // Вивід без переносу рядка
#define DEBUG_PRINTLN(x) Serial.println(x)                        // Вивід з переносом рядка
#define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)  // Форматований вивід
#else
#define DEBUG_PRINT(x)          // Нічого не робити, якщо DEBUG_DISABLE
#define DEBUG_PRINTLN(x)        // Нічого не робити
#define DEBUG_PRINTF(fmt, ...)  // Нічого не робити
#endif

#endif  // CONFIG_H
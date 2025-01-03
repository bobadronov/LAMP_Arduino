#include <ArduinoJson.h>
// Функция для перезапуска ESP
void espRestart();
void saveRealLedCount(uint16_t ledCount);
// Функция для сохранения новых учетных данных WiFi
void saveNewCreds(const String &ssid, const String &password, const String &deviceName);
void sendError(AsyncWebSocket *server, String state) {
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["message"] = state;
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  // Отправляем JSON клиенту
  server->textAll("ERROR:" + jsonString);
}
void sendAllStatus(AsyncWebSocket *server) {
  // Создаем JSON объект
  StaticJsonDocument<200> jsonDoc;
  // Добавляем данные о состоянии ленты и цвете
  jsonDoc["version"] = VERSION;
  jsonDoc["ledState"] = ledState;
  jsonDoc["REAL_NUM_LEDS"] = REAL_NUM_LEDS;
  jsonDoc["commonBrightness"] = commonBrightness;
  // Преобразуем цвет в HEX
  char hexColor[8];
  sprintf(hexColor, "#%02X%02X%02X", color.r, color.g, color.b);
  jsonDoc["color"] = hexColor;
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["currentMode"] = currentMode;
  jsonDoc["flagIsStatic"] = flagIsStatic;
  jsonDoc["flagSpeed"] = flagSpeed;
  jsonDoc["rainbowSpeed"] = rainbowSpeed;
  jsonDoc["rainbowIsStatic"] = rainbowIsStatic;
  jsonDoc["timerIsActive"] = timerIsActive;
  jsonDoc["breathingSpeed"] = breathingSpeed;
  JsonObject timerObj = jsonDoc.createNestedObject("timer");
  timerObj["hour"] = timerHour;
  timerObj["minute"] = timerMinute;
  // Add modes as a JSON array
  JsonArray modesArray = jsonDoc.createNestedArray("modes");
  for (uint8_t i = 0; i < NUM_MODES; i++) {
    modesArray.add(modeList[i]);
  }
  JsonArray colorArray = jsonDoc.createNestedArray("customColorsArray");
  for (int i = 0; i < NUM_LEDS; ++i) {
    char hexColor[12];
    sprintf(hexColor, "#%02X%02X%02X", customColorsArray[i].r, customColorsArray[i].g, customColorsArray[i].b);
    colorArray.add(hexColor);
  }
  // Сериализуем JSON в строку
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  // Отправляем JSON клиенту
  server->textAll("STATUS:" + jsonString);
}
// Обработка обновления времени
void handleTimeUpdate(String timeData, AsyncWebSocket *server) {
  DynamicJsonDocument doc(1024);  // Создаем документ для парсинга JSON
  // Попробуем разобрать строку как JSON
  DeserializationError error = deserializeJson(doc, timeData);
  if (error) {
    Serial.println("Ошибка парсинга JSON: " + String(error.c_str()));
    return;
  }
  // Извлечение данных из JSON с безопасными значениями по умолчанию
  uint8_t hour = doc["hour"] | 0;      // Значение по умолчанию: 0 (часы)
  uint8_t minute = doc["minute"] | 0;  // Значение по умолчанию: 0 (минуты)
  uint8_t day = doc["day"] | 1;        // Значение по умолчанию: 1 (день)
  uint8_t month = doc["month"] | 1;    // Значение по умолчанию: 1 (месяц)
  uint16_t year = doc["year"] | 2025;  // Значение по умолчанию: 2024 (год)
  // Проверка корректности извлеченных данных
  if (hour < 24 && minute < 60) {
    // Если данные времени корректны
    timerHour = hour;
    timerMinute = minute;
    timerDay = day;
    timerMonth = month;
    timerYear = year;
    timerIsActive = true;  // Устанавливаем флаг активности таймера
    // Печать полученных данных
    // Serial.println("Получено время: " + String(hour) + ":" + String(minute));
    // Serial.println("Дата: " + String(day) + "/" + String(month) + "/" + String(year));
  } else {
    Serial.println("Некорректные данные времени");
  }
  sendAllStatus(server);
}
// Обработка WiFi-сообщений
void handleWiFiMessage(const String &jsonPart) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonPart);
  if (!error) {
    if (doc.containsKey("ssid") && doc.containsKey("password") && doc.containsKey("deviceName")) {
      String ssid = doc["ssid"].as<String>();
      String password = doc["password"].as<String>();
      String deviceName = doc["deviceName"].as<String>();
      Serial.println("Received Setup:");
      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("Password: ");
      Serial.println(password);
      Serial.print("Device Name: ");
      Serial.println(deviceName);
      saveNewCreds(ssid, password, deviceName);  // Функция для сохранения данных Wi-Fi
      espRestart();
    }
  } else {
    Serial.println("Failed to parse JSON");
  }
}
// Обработка setup-сообщений
void handleSetupMessage(const String &jsonPart, AsyncWebSocket *server) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonPart);
  if (!error) {
    if (doc.containsKey("ledState")) {
      ledState = doc["ledState"].as<bool>();
      // Serial.print("ledState: ");
      // Serial.println(ledState);
    }
    if (doc.containsKey("color")) {
      String colorHex = doc["color"].as<String>();
      // Serial.print("Color: ");
      // Serial.println(colorHex);
      // Преобразование цвета из HEX в RGB
      long newColor = strtol(colorHex.c_str() + 1, NULL, 16);  // Пропускаем '#'
      color = CRGB((newColor >> 16) & 0xFF, (newColor >> 8) & 0xFF, newColor & 0xFF);
    }
    if (doc.containsKey("currentMode")) {
      uint8_t mode = doc["currentMode"].as<uint8_t>();
      // Serial.print("CurrentMode: ");
      // Serial.println(mode);
      currentMode = mode;
    }
    if (doc.containsKey("commonBrightness")) {
      commonBrightness = doc["commonBrightness"].as<uint8_t>();
    }
    if (doc.containsKey("flagIsStatic")) {
      flagIsStatic = doc["flagIsStatic"].as<bool>();
    }
    if (doc.containsKey("flagSpeed")) {
      flagSpeed = doc["flagSpeed"].as<uint8_t>();
    }
    if (doc.containsKey("rainbowSpeed")) {
      rainbowSpeed = doc["rainbowSpeed"].as<float>();
      // Serial.print("Flag speed set to: ");
      // Serial.println(flagSpeed);
    }
    if (doc.containsKey("rainbowIsStatic")) {
      rainbowIsStatic = doc["rainbowIsStatic"].as<bool>();
    }
    if (doc.containsKey("customColors")) {
      JsonArrayConst customColors = doc["customColors"].as<JsonArrayConst>();
      size_t numColors = customColors.size();  // Количество цветов в массиве
      if (numColors > 0) {
        // Заполняем массив savedColors
        for (size_t i = 0; i < numColors && i < REAL_NUM_LEDS; ++i) {
          String colorHex = customColors[i].as<String>();                                                 // Преобразуем каждый элемент в строку
          long newColor = strtol(colorHex.c_str() + 1, NULL, 16);                                         // Пропускаем '#', преобразуем в число
          customColorsArray[i] = CRGB((newColor >> 16) & 0xFF, (newColor >> 8) & 0xFF, newColor & 0xFF);  // Преобразуем в CRGB
        }
        // Если цветов меньше, чем светодиодов, обнуляем оставшиеся
        for (size_t i = numColors; i < REAL_NUM_LEDS; ++i) {
          customColorsArray[i] = CRGB::Black;  // Обнуляем цвет
        }
      }
    }
    if (doc.containsKey("REAL_NUM_LEDS")) {
      REAL_NUM_LEDS = doc["REAL_NUM_LEDS"].as<uint16_t>();
      // Serial.print("Led count: ");
      // Serial.println(REAL_NUM_LEDS);
      FastLED.clear();
      saveRealLedCount(REAL_NUM_LEDS);
      // espRestart();
    }
    if (doc.containsKey("customGradient")) {
      JsonArrayConst customColors = doc["customGradient"].as<JsonArrayConst>();
      size_t numColors = customColors.size();  // Количество цветов в массиве
      if (numColors == 2) {
        for (size_t i = 0; i < numColors; ++i) {
          String colorHex = customColors[i].as<String>();                                              // Преобразуем каждый элемент в строку
          long newColor = strtol(colorHex.c_str() + 1, NULL, 16);                                      // Пропускаем '#', преобразуем в число
          customGradient[i] = CRGB((newColor >> 16) & 0xFF, (newColor >> 8) & 0xFF, newColor & 0xFF);  // Преобразуем в CRGB
        }
      }
    }
    if (doc.containsKey("breathingSpeed")) {
      breathingSpeed = doc["breathingSpeed"].as<float>();
    }
  } else {
    Serial.println("Failed to parse setup JSON");
  }
  sendAllStatus(server);
}

void onWebSocketMessage(AsyncWebSocket *server, void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    String message = String((char *)data, len);  // Преобразуем данные в строку
    DynamicJsonDocument doc(1024);
    // Проверяем префиксы "wifi" и "setup"
    if (message.startsWith("WIFI:")) {
      handleWiFiMessage(message.substring(5));
    } else if (message.startsWith("SETUP:")) {
      handleSetupMessage(message.substring(6), server);
    } else if (message.startsWith("TIME:")) {
      handleTimeUpdate(message.substring(5), server);  // Передаем данные о времени
    } else if (message.startsWith("CANCEL_TIMER")) {
      timerHour = 0;
      timerMinute = 0;
      timerIsActive = false;  // Устанавливаем флаг активности таймера
      sendError(server, "Timer turn off.");
    } else if (message.startsWith("REBOOT")) {
      espRestart();
    } else {
      sendError(server, "Unknown message format");
      Serial.println("Unknown message format");
      Serial.println(message);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("Client connected: %u\n", client->id());
    sendAllStatus(server);
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("Client disconnected: %u\n", client->id());
  } else if (type == WS_EVT_DATA) {
    onWebSocketMessage(server, arg, data, len);
  }
}

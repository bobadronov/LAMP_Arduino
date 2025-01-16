String formatColorToString(CRGB color);
// Функция для перезапуска ESP
void espRestart();
// Сохранение количества светодиодов
void saveRealLedCount(uint16_t ledCount);
// Функция для сохранения новых учетных данных WiFi
void saveNewCreds(const String &ssid, const String &password, const String &deviceName);
// Отправка сообщения об ошибке всем клиентам WebSocket
void sendError(AsyncWebSocket *server, String state) {
  DynamicJsonDocument jsonDoc(512);
  jsonDoc["message"] = state;  // Добавляем сообщение об ошибке
  String jsonString;
  serializeJson(jsonDoc, jsonString);      // Сериализуем JSON в строку
  server->textAll("ERROR:" + jsonString);  // Отправляем всем клиентам
}
// Отправка полного состояния устройства всем клиентам WebSocket
void sendAllStatus(AsyncWebSocket *server) {
  DynamicJsonDocument jsonDoc(2048);  // Увеличить размер при необходимости
  jsonDoc["version"] = VERSION;
  jsonDoc["ledState"] = ledState;
  jsonDoc["REAL_NUM_LEDS"] = REAL_NUM_LEDS;
  jsonDoc["currentMode"] = currentMode;
  jsonDoc["commonBrightness"] = commonBrightness;
  jsonDoc["animationSpeed"] = animationSpeed;
  // Добавление списка режимов в массив JSON
  JsonArray modesArray = jsonDoc.createNestedArray("modes");
  for (uint8_t i = 0; i < NUM_MODES; i++) {
    modesArray.add(modeList[i]);
  }
  switch (currentMode) {
    case 0:  // STATIC COLOR
      jsonDoc["color"] = formatColorToString(color);
      break;
    case 1:  // RAINBOW
      jsonDoc["rainbowIsStatic"] = rainbowIsStatic;
      break;
    case 2:  // GRADIENT FILL
      jsonDoc["gradientStart"] = formatColorToString(customGradient[0]);
      jsonDoc["gradientEnd"] = formatColorToString(customGradient[1]);
      break;
    case 3:  // SPARKLE
      jsonDoc["color"] = formatColorToString(color);
      break;
    case 4:  // RUNNING LIGHTS
      jsonDoc["color"] = formatColorToString(color);
      // jsonDoc["speed"] = 0.5;  // Example: Add speed if it's configurable
      break;
    case 5:  // FIRE
      jsonDoc["commonBrightness"] = commonBrightness;
      break;
    case 6:  // FADE1
      jsonDoc["color"] = formatColorToString(color);
      break;
    case 7:  // FADE2
      jsonDoc["customFadeColor1"] = formatColorToString(customFadeColor1);
      jsonDoc["customFadeColor2"] = formatColorToString(customFadeColor2);
      break;
    case 8:  // METEOR
      jsonDoc["color"] = formatColorToString(color);
      break;
    case 9:  // FLAG OF UKRAINE
      jsonDoc["flagIsStatic"] = flagIsStatic;
      break;
    case 10:  // CUSTOM LIGHTING
      JsonArray colorArray = jsonDoc.createNestedArray("customColorsArray");
      for (int i = 0; i < 10; ++i) {
        colorArray.add(formatColorToString(customColorsArray[i]));
      }
      break;
  }
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["timerIsActive"] = timerIsActive;
  // Вложенный объект для таймера
  if (timerIsActive) {
    JsonObject timerObj = jsonDoc.createNestedObject("timer");
    timerObj["hour"] = timerHour;
    timerObj["minute"] = timerMinute;
  }
  // Отправка JSON
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  server->textAll("STATUS:" + jsonString);
}
// Обработка обновления времени через WebSocket
void handleTimerUpdate(String timeData, AsyncWebSocket *server) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, timeData);
  if (error) {
    DEBUG_PRINTLN("Ошибка парсинга JSON: " + String(error.c_str()));
    return;
  }
  // Извлечение времени и даты из JSON
  uint8_t hour = doc["hour"] | 0;
  uint8_t minute = doc["minute"] | 0;
  uint8_t day = doc["day"] | 1;
  uint8_t month = doc["month"] | 1;
  uint16_t year = doc["year"] | 2025;
  // Вывод входящих данных времени
  DEBUG_PRINT("Получено время: ");
  DEBUG_PRINT(year);
  DEBUG_PRINT("-");
  DEBUG_PRINT(month);
  DEBUG_PRINT("-");
  DEBUG_PRINT(day);
  DEBUG_PRINT(" ");
  DEBUG_PRINT(hour);
  DEBUG_PRINT(":");
  DEBUG_PRINTLN(minute);
  // Проверка корректности времени
  if (hour < 24 && minute < 60) {
    timerHour = hour;
    timerMinute = minute;
    timerDay = day;
    timerMonth = month;
    timerYear = year;
    timerIsActive = true;  // Активируем таймер
  } else {
    DEBUG_PRINTLN("Некорректные данные времени");
  }
  sendAllStatus(server);
}
// Обработка WiFi сообщений
void handleWiFiMessage(const String &jsonPart) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonPart);
  if (!error) {
    if (doc.containsKey("ssid") && doc.containsKey("password") && doc.containsKey("deviceName")) {
      String ssid = doc["ssid"].as<String>();
      String password = doc["password"].as<String>();
      String deviceName = doc["deviceName"].as<String>();

      // Сохранение новых учетных данных и перезагрузка
      saveNewCreds(ssid, password, deviceName);
      espRestart();
    }
  } else {
    DEBUG_PRINTLN("Failed to parse JSON");
  }
}
// Обработка сообщений настройки устройства
void handleSetupMessage(const String &jsonPart, AsyncWebSocket *server) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonPart);
  if (!error) {
    // Проверка и применение параметров, переданных через JSON
    if (doc.containsKey("ledState")) {
      ledState = doc["ledState"].as<bool>();  // Включение/выключение светодиодов
      DEBUG_PRINTF("LED State: %s\n", ledState ? "ON" : "OFF");
    }
    if (doc.containsKey("color")) {
      String colorHex = doc["color"].as<String>();
      long newColor = strtol(colorHex.c_str() + 1, NULL, 16);                          // Пропускаем символ '#'
      color = CRGB((newColor >> 16) & 0xFF, (newColor >> 8) & 0xFF, newColor & 0xFF);  // Устанавливаем цвет
    }
    if (doc.containsKey("customFadeColor1")) {
      String colorHex = doc["customFadeColor1"].as<String>();
      long newColor = strtol(colorHex.c_str() + 1, NULL, 16);                               // Пропускаем символ '#'
      customFadeColor1 = CRGB((newColor >> 16) & 0xFF, (newColor >> 8) & 0xFF, newColor & 0xFF);  // Устанавливаем цвет
    }
    if (doc.containsKey("customFadeColor2")) {
      String colorHex = doc["customFadeColor2"].as<String>();
      long newColor = strtol(colorHex.c_str() + 1, NULL, 16);                               // Пропускаем символ '#'
      customFadeColor2 = CRGB((newColor >> 16) & 0xFF, (newColor >> 8) & 0xFF, newColor & 0xFF);  // Устанавливаем цвет
    }
    if (doc.containsKey("currentMode")) {
      currentMode = doc["currentMode"].as<uint8_t>();  // Устанавливаем текущий режим
      // DEBUG_PRINTF("Mode: %s\n", currentMode);
      FastLED.clear();
      FastLED.show();
    }
    if (doc.containsKey("commonBrightness")) {
      commonBrightness = doc["commonBrightness"].as<uint8_t>();  // Устанавливаем общую яркость
      // DEBUG_PRINTF("Brightness: %s\n", commonBrightness);
    }
    if (doc.containsKey("flagIsStatic")) {
      flagIsStatic = doc["flagIsStatic"].as<bool>();  // Флаг статического режима
    }
    if (doc.containsKey("animationSpeed")) {
      animationSpeed = doc["animationSpeed"].as<float>();  // Скорость радуги
      // DEBUG_PRINTF("AnimationSpeed: %s\n", animationSpeed);
    }
    if (doc.containsKey("rainbowIsStatic")) {
      rainbowIsStatic = doc["rainbowIsStatic"].as<bool>();  // Флаг статичной радуги
    }
    if (doc.containsKey("customColors")) {
      // Обработка массива пользовательских цветов
      JsonArrayConst customColors = doc["customColors"].as<JsonArrayConst>();
      size_t numColors = customColors.size();

      if (numColors > 0) {
        // Заполнение массива пользовательских цветов
        for (size_t i = 0; i < numColors && i < REAL_NUM_LEDS; ++i) {
          String colorHex = customColors[i].as<String>();
          long newColor = strtol(colorHex.c_str() + 1, NULL, 16);
          customColorsArray[i] = CRGB((newColor >> 16) & 0xFF, (newColor >> 8) & 0xFF, newColor & 0xFF);
        }
        // Очистка оставшихся светодиодов, если их меньше, чем в массиве
        for (size_t i = numColors; i < REAL_NUM_LEDS; ++i) {
          customColorsArray[i] = CRGB::Black;
        }
      }
    }
    if (doc.containsKey("REAL_NUM_LEDS")) {
      uint16_t numLeds = doc["REAL_NUM_LEDS"].as<uint16_t>();  // Обновляем количество светодиодов
      if (numLeds <= NUM_LEDS) REAL_NUM_LEDS = numLeds;
      // DEBUG_PRINTF("NUM_LEDS: %s\n", NUM_LEDS);
    }
    if (doc.containsKey("customGradient")) {
      // Обработка градиента
      JsonArrayConst customColors = doc["customGradient"].as<JsonArrayConst>();
      size_t numColors = customColors.size();
      if (numColors == 2) {
        for (size_t i = 0; i < numColors; ++i) {
          String colorHex = customColors[i].as<String>();
          long newColor = strtol(colorHex.c_str() + 1, NULL, 16);
          customGradient[i] = CRGB((newColor >> 16) & 0xFF, (newColor >> 8) & 0xFF, newColor & 0xFF);
        }
      }
    }
  } else {
    DEBUG_PRINTLN("Failed to parse setup JSON");
  }
}
// Обработка сообщений WebSocket
void onWebSocketMessage(AsyncWebSocket *server, void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    String message = String((char *)data, len);  // Преобразуем данные в строку
    if (message.startsWith("WIFI:")) {
      handleWiFiMessage(message.substring(5));  // Обработка WiFi-сообщения
    } else if (message.startsWith("SETUP:")) {
      handleSetupMessage(message.substring(6), server);  // Обработка сообщения настройки
    } else if (message.startsWith("TIME:")) {
      handleTimerUpdate(message.substring(5), server);  // Обработка обновления времени
    } else if (message.startsWith("CANCEL_TIMER")) {
      // Отмена таймера
      timerHour = 0;
      timerMinute = 0;
      timerIsActive = false;
      DEBUG_PRINTLN("Timer turn off.");
      sendError(server, "Timer turn off.");
    } else if (message.startsWith("REBOOT")) {
      espRestart();  // Перезапуск устройства
    } else if (message.startsWith("____")) {
      sendAllStatus(server);
    } else {
      sendError(server, "Unknown message format");
      DEBUG_PRINTLN("Unknown message format: " + message);
    }
  }
}
// Обработка событий WebSocket
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    DEBUG_PRINTF("Client connected: %u\n", client->id());
    sendAllStatus(server);  // Отправка состояния нового клиенту
  } else if (type == WS_EVT_DISCONNECT) {
    DEBUG_PRINTF("Client disconnected: %u\n", client->id());
    server->cleanupClients();
  } else if (type == WS_EVT_DATA) {
    onWebSocketMessage(server, arg, data, len);  // Обработка данных
    sendAllStatus(server);
  }
}

void sendCurrentStatus(AsyncWebSocket *server) {
  // Создаем JSON объект
  StaticJsonDocument<200> jsonDoc;
  // Добавляем данные о состоянии ленты и цвете
  jsonDoc["ledState"] = ledState;
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
  jsonDoc["timerIsActive"] = timerIsActive;

  JsonObject timerObj = jsonDoc.createNestedObject("timer");
  timerObj["hour"] = timerHour;
  timerObj["minute"] = timerMinute;
  // Add modes as a JSON array
  JsonArray modesArray = jsonDoc.createNestedArray("modes");
  for (uint8_t i = 0; i < NUM_MODES; i++) {
    modesArray.add(modeList[i]);
  }
  // Сериализуем JSON в строку
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  // Отправляем JSON клиенту
  server->textAll(jsonString);
}

// Обработка обновления времени
void handleTimeUpdate(String timeData) {
  DynamicJsonDocument doc(1024);  // Создаем документ для парсинга JSON

  // Попробуем разобрать строку как JSON
  DeserializationError error = deserializeJson(doc, timeData);
  if (error) {
    Serial.println("Ошибка парсинга JSON: " + String(error.c_str()));
    return;
  }
  // Извлечение данных из JSON
  uint8_t hour = doc["hour"] | -1;  // Используем значение по умолчанию -1
  uint8_t minute = doc["minute"] | -1;
  if (hour != -1 && minute != -1) {
    Serial.println("Получено время: " + String(hour) + ":" + String(minute));
    timerHour = hour;
    timerMinute = minute;
    timerIsActive = true;  // Устанавливаем флаг активности таймера
  } else {
    Serial.println("Некорректные данные времени");
  }
}

void onWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    String message = String((char *)data, len);  // Преобразуем данные в строку
    DynamicJsonDocument doc(1024);
    // Проверяем префиксы "wifi" и "setup"
    if (message.startsWith("WIFI:")) {
      handleWiFiMessage(message.substring(4));
    } else if (message.startsWith("SETUP:")) {
      handleSetupMessage(message.substring(6));
    } else if (message.startsWith("TIME:")) {
      handleTimeUpdate(message.substring(5));  // Передаем данные о времени
    } else if (message.startsWith("CANCEL_TIMER")) {
      timerHour = 0;
      timerMinute = 0;
      timerIsActive = false;  // Устанавливаем флаг активности таймера
    } else {
      Serial.println("Unknown message format");
      Serial.println(message);
    }
  }
}

// Функция для перезапуска ESP
void espRestart();

// Функция для сохранения новых учетных данных WiFi
void saveNewCreds(const String &ssid, const String &password, const String &deviceName);

// Обработка WiFi-сообщений
void handleWiFiMessage(const String &jsonPart) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonPart);
  if (!error) {
    if (doc.containsKey("ssid") && doc.containsKey("password") && doc.containsKey("deviceName")) {
      String ssid = doc["ssid"].as<String>();
      String password = doc["password"].as<String>();
      String deviceName = doc["deviceName"].as<String>();
      Serial.println("Received WiFi setup:");
      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("Password: ");
      Serial.println(password);
      saveNewCreds(ssid, password, deviceName);
    }
  } else {
    Serial.println("Failed to parse WiFi JSON");
  }
}

// Обработка setup-сообщений
void handleSetupMessage(const String &jsonPart) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonPart);
  if (!error) {
    parseCommonFields(doc);
  } else {
    Serial.println("Failed to parse setup JSON");
  }
}

// Универсальная функция для обработки общих полей
void parseCommonFields(const DynamicJsonDocument &doc) {
  if (doc.containsKey("ledState")) {
    ledState = doc["ledState"].as<bool>();
    Serial.print("ledState: ");
    Serial.println(ledState);
  }
  if (doc.containsKey("color")) {
    String colorHex = doc["color"].as<String>();
    Serial.print("Color: ");
    Serial.println(colorHex);
    // Преобразование цвета из HEX в RGB
    long newColor = strtol(colorHex.c_str() + 1, NULL, 16);  // Пропускаем '#'
    color = CRGB((newColor >> 16) & 0xFF, (newColor >> 8) & 0xFF, newColor & 0xFF);
  }
  if (doc.containsKey("currentMode")) {
    uint8_t mode = doc["currentMode"].as<uint8_t>();
    Serial.print("CurrentMode: ");
    Serial.println(mode);
    currentMode = mode;
  }
  if (doc.containsKey("flagIsStatic")) {
    flagIsStatic = doc["flagIsStatic"].as<bool>();
  }
  if (doc.containsKey("flagSpeed")) {
    flagSpeed = doc["flagSpeed"].as<uint8_t>();
    Serial.print("Flag speed set to: ");
    Serial.println(flagSpeed);
  }
  if (doc.containsKey("rainbowSpeed")) {
    rainbowSpeed = doc["rainbowSpeed"].as<float>();
    Serial.print("Flag speed set to: ");
    Serial.println(flagSpeed);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("Client connected: %u\n", client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("Client disconnected: %u\n", client->id());
  } else if (type == WS_EVT_DATA) {
    onWebSocketMessage(arg, data, len);
  }
  sendCurrentStatus(server);
}

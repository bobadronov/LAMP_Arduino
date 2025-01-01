void sendCurrentStatus(AsyncWebSocket *server);

void espRestart() {
  Serial.println("Restarting ESP...");
  delay(1000);
  ESP.restart();
}

void checkTimer(AsyncWebSocket *server) {
  if (timerIsActive) {
    // Serial.print("getStampZone: ");
    // Serial.println(getStampZone());
    if (ds.tick()) {
      // Получаем текущую дату и время с DS3231
      uint8_t currentHour = ds.hour() - 2;
      uint8_t currentMinute = ds.minute() + 2;
      uint8_t currentDay = ds.day();
      uint8_t currentMonth = ds.month();
      uint16_t currentYear = ds.year();
      Serial.println("Set Date and Time: " + String(timerDay) + "." + String(timerMonth) + "." + String(timerYear) + " " + String(timerHour) + ":" + String(timerMinute));
      Serial.println("Current Date and Time: " + String(currentDay) + "." + String(currentMonth) + "." + String(currentYear) + " " + String(currentHour) + ":" + String(currentMinute));
      // Проверяем, совпадает ли текущее время с установленным таймером
      if (timerHour == currentHour && timerMinute == currentMinute && timerDay == currentDay && timerMonth == currentMonth && timerYear == currentYear) {
        // Отключаем таймер и сбрасываем состояние
        timerIsActive = false;
        timerHour = 0;
        timerMinute = 0;
        timerDay = 0;  // Сбрасываем дату
        timerMonth = 0;
        timerYear = 0;
        ledState = false;
        sendCurrentStatus(server);
        // Serial.println("Timer expired, turning off LED.");
      }
    }
  }
}

void checkUpdate() {
  String ver, notes;
  if (ota.checkUpdate(&ver, &notes)) {
    Serial.println(ver);
    Serial.println(notes);
    ota.update();
  }
  Serial.print("Version ");
  Serial.println(ota.version());
}

void espConectionStatusIndicator() {
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 2000;  // Інтервал оновлення
  static bool state = false;  // Стан світлодіода
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    if (espInAPMode) {
      // Блимання
      state = !state;  // Перемикаємо стан
      digitalWrite(LED_BUILTIN, state ? LOW : HIGH);  // Світлодіод активується низьким рівнем
    } else {
      // Постійне ввімкнення
      digitalWrite(LED_BUILTIN, HIGH);  // Світлодіод ввімкнений
    }
  }
}
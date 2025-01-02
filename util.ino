
void espRestart() {
  Serial.println("Restarting ESP...");
  delay(1000);
  ESP.restart();
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
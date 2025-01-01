void saveNewCreds(const String &ssid, const String &password, const String &deviceName) {
  preferences.begin("config", false);           // Открываем пространство "wifi" для записи
  preferences.putString("ssid", ssid);          // Сохраняем SSID
  preferences.putString("password", password);  // Сохраняем пароль
  preferences.putString("deviceName", deviceName);
  Serial.println("New WiFi credentials saved:");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.print("Device name: ");
  Serial.println(deviceName);
  preferences.end();  // Закрываем хранилище
}
// Функция для получения текущих учетных данных
void getWifiCreds() {
  preferences.begin("config", true);  // Открываем пространство "wifi" только для чтения
  String ssid = preferences.getString("ssid", SSID);
  String pass = preferences.getString("password", PASSWORD);
  String deviceName = preferences.getString("deviceName", HOSTNAME);
  HOSTNAME = deviceName;
  SSID = ssid;
  PASSWORD = pass;
  Serial.println("New WiFi credentials load:");
  Serial.print("SSID: ");
  Serial.println(SSID);
  Serial.print("Password: ");
  Serial.println(PASSWORD);
  Serial.print("Device name: ");
  Serial.println(HOSTNAME);
  preferences.end();  // Закрываем хранилище
}
// Функция сохранения данных
void saveToMemory() {
  preferences.begin("config", false);  // Открываем Preferences в режиме записи
  preferences.putBool("ledState", ledState);
  preferences.putUInt("color", (uint32_t)color);  // CRGB преобразуем в uint32_t
  preferences.putUChar("currentMode", currentMode);
  preferences.putUChar("commonBrightness", commonBrightness);
  preferences.putBool("flagIsStatic", flagIsStatic);
  preferences.putUChar("flagSpeed", flagSpeed);
  preferences.putFloat("rainbowSpeed", rainbowSpeed);
  preferences.putBool("rainbowIsStatic", rainbowIsStatic);
  // Сохраняем массив цветов
  for (uint8_t i = 0; i < REAL_NUM_LEDS; i++) {
    preferences.putUInt(String("customColor" + String(i)).c_str(), (uint32_t)customColorsArray[i]);
  }
  preferences.end();  // Закрываем Preferences
  Serial.println("Data saved to memory.");
}
// Функция загрузки данных
void loadSettingsFromMemory() {
  preferences.begin("config", true);  // Открываем Preferences в режиме чтения
  REAL_NUM_LEDS = preferences.getUInt("REAL_NUM_LEDS", 10);
  ledState = preferences.getBool("ledState", false);
  color = (CRGB)preferences.getUInt("color", 0xFFFFFF);  // uint32_t преобразуем обратно в CRGB
  currentMode = preferences.getUChar("currentMode", 0);
  commonBrightness = preferences.getUChar("commonBrightness", 255);
  flagIsStatic = preferences.getBool("flagIsStatic", true);
  flagSpeed = preferences.getUChar("flagSpeed", 1);
  rainbowSpeed = preferences.getFloat("rainbowSpeed", 2.0);
  rainbowIsStatic = preferences.getBool("rainbowIsStatic", false);
  // Загружаем массив цветов
  for (uint8_t i = 0; i < REAL_NUM_LEDS; i++) {
    customColorsArray[i] = (CRGB)preferences.getUInt(String("customColor" + String(i)).c_str(), 0x000000);
  }
  preferences.end();  // Закрываем Preferences
  Serial.println("Data loaded from memory.");
}
// Вызов сохранения каждые 60 секунд
void handleAutoSave() {
  static unsigned long lastSaveTime = 0;
  if (millis() - lastSaveTime >= SAVE_INTERVAL) {
    lastSaveTime = millis();
    saveToMemory();
  }
}
// Функция для сохранения реального количества светодиодов 
void saveRealLedCount(uint16_t ledCount) {
  preferences.begin("config", false);              // Відкриваємо розділ "config" для запису
  preferences.putUInt("REAL_NUM_LEDS", ledCount);  // Зберігаємо кількість світлодіодів
  preferences.end();                               // Завершуємо роботу з Preferences
}
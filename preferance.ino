void saveNewCreds(const String &ssid, const String &password, const String &deviceName) {
  preferences.begin("config", false);           // Открываем пространство "wifi" для записи
  preferences.putString("ssid", ssid);          // Сохраняем SSID
  preferences.putString("password", password);  // Сохраняем пароль
  preferences.putString("deviceName", deviceName);
  DEBUG_PRINTLN("New WiFi credentials saved:");
  DEBUG_PRINT("SSID: ");
  DEBUG_PRINTLN(ssid);
  DEBUG_PRINT("Password: ");
  DEBUG_PRINTLN(password);
  DEBUG_PRINT("Device name: ");
  DEBUG_PRINTLN(deviceName);
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
  DEBUG_PRINTLN("New WiFi credentials load:");
  DEBUG_PRINT("SSID: ");
  DEBUG_PRINTLN(SSID);
  DEBUG_PRINT("Password: ");
  DEBUG_PRINTLN(PASSWORD);
  DEBUG_PRINT("Device name: ");
  DEBUG_PRINTLN(HOSTNAME);
  preferences.end();  // Закрываем хранилище
}
// Функция сохранения данных
void saveToMemory() {
  preferences.begin("config", false);                   // Открываем Preferences в режиме записи
  preferences.putUInt("REAL_NUM_LEDS", REAL_NUM_LEDS);  // Зберігаємо кількість світлодіодів
  preferences.putBool("ledState", ledState);
  preferences.putUInt("color", (uint32_t)color);  // CRGB преобразуем в uint32_t
  preferences.putUChar("currentMode", currentMode);
  preferences.putUChar("commonBrightness", commonBrightness);
  preferences.putBool("flagIsStatic", flagIsStatic);
  preferences.putFloat("animationSpeed", animationSpeed);
  preferences.putBool("rainbowIsStatic", rainbowIsStatic);
  // Сохраняем массив цветов
  for (uint8_t i = 0; i < 10; i++) {
    preferences.putUInt(String("customColor" + String(i)).c_str(), (uint32_t)customColorsArray[i]);
  }
  preferences.end();  // Закрываем Preferences
  DEBUG_PRINTLN("Data saved to memory.");
}
// Функция загрузки данных
void loadSettingsFromMemory() {
  preferences.begin("config", true);  // Открываем Preferences в режиме чтения
  REAL_NUM_LEDS = preferences.getUInt("REAL_NUM_LEDS", 10);
  ledState = preferences.getBool("ledState", true);
  color = (CRGB)preferences.getUInt("color", 0xFFFFFF);  // uint32_t преобразуем обратно в CRGB
  currentMode = preferences.getUChar("currentMode", 0);
  commonBrightness = preferences.getUChar("commonBrightness", 255);
  flagIsStatic = preferences.getBool("flagIsStatic", true);
  animationSpeed = preferences.getFloat("animationSpeed", 2.0);
  rainbowIsStatic = preferences.getBool("rainbowIsStatic", false);
  // Загружаем массив цветов
  for (uint8_t i = 0; i < 10; i++) {
    customColorsArray[i] = (CRGB)preferences.getUInt(String("customColor" + String(i)).c_str(), 0x000000);
  }
  preferences.end();  // Закрываем Preferences
  DEBUG_PRINTLN("Data loaded from memory.");
}
// Вызов сохранения каждые 60 секунд
void handleAutoSave() {
  static unsigned long lastSaveTime = 0;
  if (millis() - lastSaveTime >= SAVE_INTERVAL) {
    lastSaveTime = millis();
    saveToMemory();
  }
}

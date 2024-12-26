
void saveNewCreds(const String &ssid, const String &password, const String &deviceName) {
  preferences.begin("wifi", false);  // Открываем пространство "wifi" для записи
  preferences.putString("ssid", ssid);  // Сохраняем SSID
  preferences.putString("password", password);  // Сохраняем пароль
  preferences.putString("deviceName", deviceName);
  Serial.println("New WiFi credentials saved:");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  preferences.end();  // Закрываем хранилище
}

// Функция для получения текущих учетных данных
void getWifiCreds() {
  preferences.begin("wifi", true);  // Открываем пространство "wifi" только для чтения
  String ssid = preferences.getString("ssid", SSID);
  String pass = preferences.getString("password", PASSWORD);
  String deviceName = preferences.getString("deviceName", hostname);
  hostname = deviceName;
  SSID = ssid;
  PASSWORD = pass;
  preferences.end();  // Закрываем хранилище
}
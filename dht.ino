#include "config.h"

void updateDHT() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 60000) {  // 60 секунд
    lastUpdate = millis();
    // Чтение температуры и влажности
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();
    // Обновление температуры, если чтение прошло успешно
    temperature = isnan(newTemp) ? 0.0 : newTemp;
    // Serial.printf("Temperature updated: %.2f\n", temperature);
    // Обновление влажности, если чтение прошло успешно
    humidity = isnan(newHum) ? 0.0 : newHum;
    // Serial.printf("Humidity updated: %.2f\n", humidity);
    // Логирование ошибок
    if (isnan(newTemp) || isnan(newHum)) {
      DEBUG_PRINTLN("Failed to read temperature.");
      DEBUG_PRINTLN("Failed to read humidity.");
    }
  }
}

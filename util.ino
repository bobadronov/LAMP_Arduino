#include "config.h"
// Функция для перезапуска ESP
void espRestart() {
  DEBUG_PRINTLN("Restarting ESP...");  // Сообщение о перезапуске
  delay(1000);                          // Небольшая задержка перед перезапуском
  ESP.restart();                        // Перезапуск устройства
}
// Функция для проверки и применения обновлений прошивки
void checkUpdate() {
  String ver, notes;                    // Переменные для хранения версии и примечаний обновления
  if (ota.checkUpdate(&ver, &notes)) {  // Проверка наличия обновления
    DEBUG_PRINTLN(ver);                // Вывод версии обновления
    DEBUG_PRINTLN(notes);              // Вывод примечаний к обновлению
    ota.update();                       // Применение обновления
  }
  DEBUG_PRINT("Version ");  // Вывод текущей версии прошивки
  DEBUG_PRINTLN(ota.version());
}
// Функция-индикатор состояния подключения ESP
void espConectionStatusIndicator() {
  static unsigned long lastUpdate = 0;  // Время последнего обновления
  const unsigned long interval = 5000;  // Интервал обновления в миллисекундах
  static bool state = false;            // Переменная состояния светодиода (вкл/выкл)

  if (millis() - lastUpdate >= interval) {  // Проверка интервала времени
    lastUpdate = millis();                  // Обновление времени последнего выполнения

    // espInAPMode = WiFi.status() == WL_CONNECTED;

    if (espInAPMode) {
      // Если устройство находится в режиме точки доступа (AP)
      state = !state;                                 // Переключение состояния светодиода
      digitalWrite(LED_BUILTIN, state ? LOW : HIGH);  // Управление светодиодом
    } else {
      // Если устройство подключено в обычном режиме
      digitalWrite(LED_BUILTIN, HIGH);  // Постоянно включаем светодиод
    }
  }
}

String formatColorToString(CRGB color) {
  char hexColor[8];
  sprintf(hexColor, "#%02X%02X%02X", color.r, color.g, color.b);
  return String(hexColor);
}
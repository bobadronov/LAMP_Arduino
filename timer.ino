void sendAllStatus(AsyncWebSocket *server);
void sendError(AsyncWebSocket *server, String state);

void resetTimer() {
  timerIsActive = false;  // Деактивация таймера
  timerHour = 0;          // Сброс часов
  timerMinute = 0;        // Сброс минут
  timerDay = 0;           // Сброс дня
  timerMonth = 0;         // Сброс месяца
  timerYear = 0;          // Сброс года
  ledState = false;       // Выключение светодиодов
}
bool checkTimerMatch(uint8_t hour, uint8_t minute, uint8_t day, uint8_t month, uint16_t year) {
  return timerHour == hour && timerMinute == minute && timerDay == day && timerMonth == month && timerYear == year;
}
void checkTimer(AsyncWebSocket *server) {
  if (!timerIsActive) return;  // Выход, если таймер неактивен
  DEBUG_PRINTLN("Timer off at: " + String(timerDay) + "." + String(timerMonth) + "." + String(timerYear) + " " + String(timerHour) + ":" + String(timerMinute));
  #ifdef DS3231_MODULE_ENABLE
  if (ds.tick()) {  // Проверка обновления данных модуля DS3231
    // Получаем текущие дату и время
    uint8_t currentHour = ds.hour() - 2;  // Скорректируйте часовой пояс
    uint8_t currentMinute = ds.minute() + 2;
    uint8_t currentDay = ds.day() + 1;
    uint8_t currentMonth = ds.month();
    uint16_t currentYear = ds.year();

    // Логирование текущих значений и времени таймера
    DEBUG_PRINTLN("DS3231      : " + String(currentDay) + "." + String(currentMonth) + "." + String(currentYear) + " " + String(currentHour) + ":" + String(currentMinute));

    // Сравнение с таймером
    if (checkTimerMatch(currentHour, currentMinute, currentDay, currentMonth, currentYear)) {
      DEBUG_PRINTLN("Таймер совпал с текущим временем. Сбрасываю таймер.");
      resetTimer();           // Сброс таймера
      sendAllStatus(server);  // Обновление состояния для клиентов
      return;
    }
  }
  #endif

  if (NTP.online() && NTP.tick()) {  // Если интернет доступен, используем NTP
    Datime dt(NTP);                  // Получение времени
    DEBUG_PRINTLN("NTP         : " + String(dt.day) + "." + String(dt.month) + "." + String(dt.year) + " " + String(dt.hour) + ":" + String(dt.minute));

    if (checkTimerMatch(dt.hour, dt.minute, dt.day, dt.month, dt.year)) {
      DEBUG_PRINTLN("Таймер совпал с временем NTP. Сбрасываю таймер.");
      resetTimer();
      sendAllStatus(server);
      return;
    }
  }
  // else {
  // Если модуль времени не работает и интернет недоступен
  // DEBUG_PRINTLN("Ошибка: Модуль времени DS3231 не работает и интернет недоступен.");
  // sendError(server, "The timer does not work or there is no internet access. Check the internet status or connect the real time module.");
  // }
}

void sendAllStatus(AsyncWebSocket *server);
void sendError(AsyncWebSocket *server, String state);

void resetTimer() {
  timerIsActive = false;
  timerHour = 0;
  timerMinute = 0;
  timerDay = 0;
  timerMonth = 0;
  timerYear = 0;
  ledState = false;
}

bool checkTimerMatch(uint8_t hour, uint8_t minute, uint8_t day, uint8_t month, uint16_t year) {
  return timerHour == hour && timerMinute == minute && timerDay == day && timerMonth == month && timerYear == year;
}

void checkTimer(AsyncWebSocket *server) {
  if (!timerIsActive) return;

  if (ds.tick() && ds.isOK()) {
    // Получаем время из DS3231
    uint8_t currentHour = ds.hour() - 2;  // Скорректируйте часовой пояс
    uint8_t currentMinute = ds.minute() + 2;
    uint8_t currentDay = ds.day();
    uint8_t currentMonth = ds.month();
    uint16_t currentYear = ds.year();

    Serial.println("Set Date and Time: " + String(timerDay) + "." + String(timerMonth) + "." + String(timerYear) + " " + String(timerHour) + ":" + String(timerMinute));
    Serial.println("Current Date and Time: " + String(currentDay) + "." + String(currentMonth) + "." + String(currentYear) + " " + String(currentHour) + ":" + String(currentMinute));

    if (checkTimerMatch(currentHour, currentMinute, currentDay, currentMonth, currentYear)) {
      resetTimer();
      sendAllStatus(server);
      return;
    }
  } else if (NTP.online() && NTP.tick()) {
    Datime dt(NTP);  // Получение времени через NTP
    if (checkTimerMatch(dt.hour, dt.minute, dt.day, dt.month, dt.year)) {
      resetTimer();
      sendAllStatus(server);
      return;
    }
    Serial.println(NTP.toString());
  } else {
    sendError(server, "The timer does not work or there is no internet access. Check the internet status or connect the real time module.");
  }
}

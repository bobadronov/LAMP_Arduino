void espRestart() {
  Serial.println("Restarting ESP...");
  delay(1000);
  ESP.restart();
}

void checkTimer() {
  if (timerIsActive) {
    if (ds.tick()) {
      if (timerHour == ds.hour() && timerMinute == ds.minute()) {
        timerIsActive = false;
        timerHour = 0;
        timerMinute = 0;
        ledState = false;
      }
    }
  }
}
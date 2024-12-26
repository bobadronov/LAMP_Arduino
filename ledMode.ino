void normalMode() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 500) {
    lastUpdate = millis();
    fill_solid(leds, NUM_LEDS, color);  // Включить выбранный цвет
    FastLED.show();
  }
}

void rainbowMode() {
  static uint8_t hue = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 2 * rainbowSpeed) {
    lastUpdate = millis();
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hue + (i * 10), 255, 255);
    }
    hue++;
    FastLED.show();
  }
}

void breathingMode() {
  static uint8_t brightness = 0;
  static int direction = 1;  // 1 для увеличения, -1 для уменьшения
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 30;  // Интервал обновления яркости

  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    brightness += direction * 5;  // Шаг изменения яркости

    if (brightness == 0 || brightness == 255) {
      direction *= -1;  // Смена направления при достижении предела
    }

    // Преобразование цвета из RGB в HSV
    CHSV hsvColor = rgb2hsv_approximate(color);

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hsvColor.h, hsvColor.s, brightness);
    }
    FastLED.show();
  }
}

void strobeMode() {
  static bool on = true;
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 200;  // Интервал переключения состояния

  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    if (on) {
      fill_solid(leds, NUM_LEDS, color);
    } else {
      fill_solid(leds, NUM_LEDS, CRGB::Black);  // Выключить светодиоды
    }
    FastLED.show();
    on = !on;  // Переключить состояние
  }
}

void meteorMode() {
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 200;  // Интервал обновления
  static int pos = 0;

  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    fadeToBlackBy(leds, NUM_LEDS, 64);  // Постепенное затухание следов

    leds[pos] = color;  // Основное тело метеора
    pos++;
    if (pos >= NUM_LEDS) pos = 0;  // Возврат к началу
    FastLED.show();
  }
}

void colorWipeMode() {
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 100;  // Интервал обновления
  static int index = 0;

  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    leds[index] = color;  // Заливаем текущий светодиод
    index++;
    if (index >= NUM_LEDS) {
      fill_solid(leds, NUM_LEDS, CRGB::Black);  // Выключить светодиоды
      index = 0;                                // Возврат в начало
    }
    FastLED.show();
  }
}

void fireMode() {
  static byte heat[NUM_LEDS];           // Массив для хранения тепла для каждого светодиода
  static unsigned long lastUpdate = 0;  // Время последнего обновления
  const unsigned long interval = 30;    // Интервал обновления (в миллисекундах)

  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();  // Обновляем время последнего обновления

    // "Охлаждение" светодиодов
    for (int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8(heat[i], random(10, 30));  // Более агрессивное охлаждение
    }

    // "Нагрев" центра и случайных позиций
    int centerHeat = random(150, 255);  // Высокое тепло в центре
    heat[NUM_LEDS / 2] = qadd8(heat[NUM_LEDS / 2], centerHeat);

    for (int i = 0; i < NUM_LEDS / 8; i++) {
      int pos = random(NUM_LEDS);
      heat[pos] = qadd8(heat[pos], random(120, 200));  // Локальный нагрев
    }

    // Распространение тепла с уменьшением интенсивности
    for (int i = 1; i < NUM_LEDS - 1; i++) {
      heat[i] = (heat[i - 1] + heat[i] + heat[i + 1]) / 3;  // Учитываем соседей
    }

    // Преобразование тепла в цвета с эффектом затемнения
    for (int i = 0; i < NUM_LEDS; i++) {
      CRGB color = HeatColor(heat[i]);    // Преобразуем температуру в цвет
      leds[i] = color.fadeToBlackBy(50);  // Плавное затемнение
    }

    // Дополнительный эффект мерцания
    for (int i = 0; i < NUM_LEDS / 15; i++) {
      int flickerPos = random(NUM_LEDS);
      leds[flickerPos] = CRGB::White;  // Вспышки белого цвета
    }

    FastLED.show();  // Отображаем результат на светодиодах
  }
}

void runningLightsMode() {
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 50;  // Інтервал оновлення (мс)
  static float position = 0;          // Поточна позиція хвилі
  const float speed = 0.5;            // Швидкість руху хвилі
  const uint8_t brightness = 255;     // Максимальна яскравість хвилі

  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    position += speed;  // Рух хвилі

    for (int i = 0; i < NUM_LEDS; i++) {
      // Формула хвилі: синусоїда для плавного ефекту
      float wave = sin((i + position) * 0.2) * 0.5 + 0.5;  // Значення від 0 до 1
      uint8_t intensity = wave * brightness;               // Інтенсивність кольору

      leds[i] = color;                         // Базовий колір
      leds[i].fadeToBlackBy(255 - intensity);  // Застосовуємо яскравість хвилі
    }

    FastLED.show();  // Оновлення стрічки
  }
}

void sparkleMode() {
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 100;  // Интервал обновления
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    fadeToBlackBy(leds, NUM_LEDS, 60);  // Постепенно затухают остальные светодиоды
    int pos = random(NUM_LEDS);         // Случайная позиция
    leds[pos] = color;                  // Яркая белая вспышка
    FastLED.show();
  }
}

void flagMode() {
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 50;  // Интервал обновления
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();  // Обновляем время последнего обновления

    if (flagIsStatic) {
      // Верхняя половина синий, нижняя желтая
      for (int i = 0; i < NUM_LEDS / 2; i++) {
        leds[i] = CRGB::Blue;
      }
      for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Yellow;
      }
      FastLED.show();
    } else {
      // Динамический флаг с эффектом волны
      static uint8_t offset = 0;  // Смещение волны
      for (int i = 0; i < NUM_LEDS; i++) {
        // Рассчитываем волновую амплитуду
        uint8_t wave = sin8(i * 10 + offset);  // sin8 создает синусоидальную волну
        if (i < NUM_LEDS / 2) {
          leds[i] = CHSV(160, 255, wave);  // Верхняя половина - синий
        } else {
          leds[i] = CHSV(40, 255, wave);  // Нижняя половина - желтый
        }
      }
      offset += flagSpeed;  // Смещение волны для создания движения
      FastLED.show();
    }
  }
}

const unsigned long shortInterval = 1000;  // Интервал для мигания при отсутствии подключения (1 секунда)
const unsigned long longInterval = 5000;   // Интервал для мигания при подключении (5 секунд)

void blinkLED(unsigned long interval) {
  unsigned long previousMillis = 0;
  static bool onBoardLedState = false;
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();  // Обновляем время последнего переключения
    onBoardLedState = !onBoardLedState;            // Инвертируем состояние светодиода
    digitalWrite(LED_BUILTIN, !onBoardLedState);
  }
}

void updateLEDState() {
  // Вызываем текущий режим на основе текущего времени
  if (ledState) {
    switch (currentMode) {
      case 0:
        normalMode();  // Статический цвет
        break;
      case 1:
        rainbowMode();  // Режим радуги
        break;
      case 2:
        breathingMode();  // Эффект дыхания
        break;
      case 3:
        strobeMode();  // Стробоскоп
        break;
      case 4:
        sparkleMode();  // Эффект искр
        break;
      case 5:
        runningLightsMode();  // Бегущие огни
        break;
      case 6:
        fireMode();  // Эффект огня
        break;
      case 7:
        colorWipeMode();  // Заливка цвета
        break;
      case 8:
        meteorMode();  // Эффект метеора
        break;
      case 9:
        flagMode();
        break;
      default:
        normalMode();  // Режим по умолчанию
        break;
    }
  } else {
    fadeToBlackBy(leds, NUM_LEDS, 2);  // Постепенно выключить ленту
    FastLED.show();
  }

  blinkLED(WiFi.status() == WL_CONNECTED ? longInterval : shortInterval);
}
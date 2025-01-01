void normalMode() {
  FastLED.setBrightness(255);
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 500) {
    lastUpdate = millis();
    fill_solid(leds, REAL_NUM_LEDS, color);  // Включить выбранный цвет
    FastLED.show();
  }
}

void rainbowMode() {
  static uint8_t hue = 0;
  static unsigned long lastUpdate = 0;
  FastLED.setBrightness(commonBrightness);
  if (millis() - lastUpdate >= 5 * rainbowSpeed) {
    lastUpdate = millis();
    if (rainbowIsStatic) {
      fill_solid(leds, REAL_NUM_LEDS, CHSV(hue, 255, 255));
    } else {
      for (int i = 0; i < REAL_NUM_LEDS; i++) {
        leds[i] = CHSV(hue + (i * 10), 255, 255);
      }
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
  FastLED.setBrightness(255);
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    brightness += direction * 5;  // Шаг изменения яркости

    if (brightness == 0 || brightness == 255) {
      direction *= -1;  // Смена направления при достижении предела
    }

    // Преобразование цвета из RGB в HSV
    CHSV hsvColor = rgb2hsv_approximate(color);

    for (int i = 0; i < REAL_NUM_LEDS; i++) {
      leds[i] = CHSV(hsvColor.h, hsvColor.s, brightness);
    }
    FastLED.show();
  }
}

void strobeMode() {
  static bool on = true;
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 200;  // Интервал переключения состояния
  FastLED.setBrightness(255);
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    if (on) {
      fill_solid(leds, REAL_NUM_LEDS, color);
    } else {
      fill_solid(leds, REAL_NUM_LEDS, CRGB::Black);  // Выключить светодиоды
    }
    FastLED.show();
    on = !on;  // Переключить состояние
  }
}

void meteorMode() {
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 200;  // Интервал обновления
  static int pos = 0;
  FastLED.setBrightness(255);
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    fadeToBlackBy(leds, REAL_NUM_LEDS, 64);  // Постепенное затухание следов
    leds[pos] = color;  // Основное тело метеора
    pos++;
    if (pos >= REAL_NUM_LEDS) pos = 0;  // Возврат к началу
    FastLED.show();
  }
}

void colorWipeMode() {
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 100;  // Интервал обновления
  static int index = 0;
  static int direction = 1;  // Направление (1 - вправо, -1 - влево)
  FastLED.setBrightness(255);
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    // Очистить предыдущий светодиод
    leds[index] = CRGB::Black;
    // Обновить индекс в зависимости от направления
    index += direction;
    // Если дошли до конца или в начало, меняем направление
    if (index >= REAL_NUM_LEDS || index < 0) {
      direction = -direction;  // Меняем направление
      index += direction;      // Корректируем индекс, чтобы не выйти за пределы
    }
    // Заливаем новый светодиод
    leds[index] = color;
    FastLED.show();
  }
}

void fireMode() {
  static byte* heat = nullptr;              // Указатель на массив тепла
  static unsigned long lastUpdate = 0;      // Время последнего обновления
  const unsigned long interval = 30;        // Интервал обновления (в миллисекундах)
  // Проверяем, нужно ли переинициализировать массив
  static uint16_t previousLedCount = 0;
  if (REAL_NUM_LEDS != previousLedCount) {
    if (heat != nullptr) {
      delete[] heat;  // Освобождаем старую память
    }
    heat = new byte[REAL_NUM_LEDS];  // Выделяем новый массив
    memset(heat, 0, REAL_NUM_LEDS);  // Инициализируем нулями
    previousLedCount = REAL_NUM_LEDS;
  }
  FastLED.setBrightness(commonBrightness);
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();  // Обновляем время последнего обновления
    // "Охлаждение" светодиодов
    for (int i = 0; i < REAL_NUM_LEDS; i++) {
      heat[i] = qsub8(heat[i], random(10, 30));  // Более агрессивное охлаждение
    }
    // "Нагрев" центра и случайных позиций
    int centerHeat = random(150, 255);  // Высокое тепло в центре
    heat[REAL_NUM_LEDS / 2] = qadd8(heat[REAL_NUM_LEDS / 2], centerHeat);
    for (int i = 0; i < REAL_NUM_LEDS / 8; i++) {
      int pos = random(REAL_NUM_LEDS);
      heat[pos] = qadd8(heat[pos], random(120, 200));  // Локальный нагрев
    }
    // Распространение тепла с уменьшением интенсивности
    for (int i = 1; i < REAL_NUM_LEDS - 1; i++) {
      heat[i] = (heat[i - 1] + heat[i] + heat[i + 1]) / 3;  // Учитываем соседей
    }
    // Преобразование тепла в цвета с эффектом затемнения
    for (int i = 0; i < REAL_NUM_LEDS; i++) {
      CRGB color = HeatColor(heat[i]);    // Преобразуем температуру в цвет
      leds[i] = color.fadeToBlackBy(50);  // Плавное затемнение
    }
    FastLED.show();  // Отображаем результат на светодиодах
  }
}

void runningLightsMode() {
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 50;  // Інтервал оновлення (мс)
  static float position = 0;          // Поточна позиція хвилі
  const float speed = 0.5;            // Швидкість руху хвилі
  FastLED.setBrightness(255);
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    position += speed;  // Рух хвилі
    for (int i = 0; i < REAL_NUM_LEDS; i++) {
      // Формула хвилі: синусоїда для плавного ефекту
      float wave = sin((i + position) * 0.2) * 0.5 + 0.5;  // Значення від 0 до 1
      uint8_t intensity = wave * 255;                      // Інтенсивність кольору

      leds[i] = color;                         // Базовий колір
      leds[i].fadeToBlackBy(255 - intensity);  // Застосовуємо яскравість хвилі
    }

    FastLED.show();  // Оновлення стрічки
  }
}

void sparkleMode() {
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 100;  // Интервал обновления
  FastLED.setBrightness(255);
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    fadeToBlackBy(leds, REAL_NUM_LEDS, 60);  // Постепенно затухают остальные светодиоды
    int pos = random(REAL_NUM_LEDS);         // Случайная позиция
    leds[pos] = color;                  // Яркая белая вспышка
    FastLED.show();
  }
}

void flagMode() {
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 50;  // Интервал обновления
  FastLED.setBrightness(commonBrightness);
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();  // Обновляем время последнего обновления

    if (flagIsStatic) {
      // Верхняя половина синий, нижняя желтая
      for (int i = 0; i < REAL_NUM_LEDS / 2; i++) {
        leds[i] = CRGB::Blue;
      }
      for (int i = REAL_NUM_LEDS / 2; i < REAL_NUM_LEDS; i++) {
        leds[i] = CRGB::Yellow;
      }
      FastLED.show();
    } else {
      // Динамический флаг с эффектом волны
      static uint8_t offset = 0;  // Смещение волны
      for (int i = 0; i < REAL_NUM_LEDS; i++) {
        // Рассчитываем волновую амплитуду
        uint8_t wave = sin8(i * 10 + offset);  // sin8 создает синусоидальную волну
        if (i < REAL_NUM_LEDS / 2) {
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

void customMode() {
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 50;  // Интервал обновления
  FastLED.setBrightness(commonBrightness);
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();  // Обновляем время последнего обновления
    for (size_t i = 0; i < REAL_NUM_LEDS; ++i) {
      leds[i] = customColorsArray[i];
    }
    FastLED.show();
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
      case 10:
        customMode();
        break;
      default:
        normalMode();  // Режим по умолчанию
        break;
    }
  } else {
    fadeToBlackBy(leds, REAL_NUM_LEDS, 2);  // Постепенно выключить ленту
    FastLED.show();
  }
}
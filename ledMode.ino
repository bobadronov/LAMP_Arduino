// 0. STATIC COLOR
void normalMode() {
  FastLED.setBrightness(255);
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 500) {
    lastUpdate = millis();
    fill_solid(leds, REAL_NUM_LEDS, color);  // Включить выбранный цвет
    FastLED.show();
  }
}
// 1. RAINBOW
void rainbowMode() {
  static uint8_t hue = 0;
  static unsigned long lastUpdate = 0;
  FastLED.setBrightness(commonBrightness);
  if (millis() - lastUpdate >= animationSpeed) {
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
// 2. GRADIENT FILL
void gradientFillMode() {
  static unsigned long lastUpdate = 0;
  FastLED.setBrightness(commonBrightness);
  if (millis() - lastUpdate >= 200) {
    lastUpdate = millis();
    // Fill gradient between colors
    fill_gradient_RGB(leds, 0, customGradient[0], REAL_NUM_LEDS - 1, customGradient[1]);
    FastLED.show();
  }
}
// 3. SPARKLE
void sparkleMode() {
  static unsigned long lastUpdate = 0;
  FastLED.setBrightness(255);
  if (millis() - lastUpdate >= animationSpeed) {
    lastUpdate = millis();
    fadeToBlackBy(leds, REAL_NUM_LEDS, 50);  // Постепенно затухают остальные светодиоды
    int pos = random(REAL_NUM_LEDS);         // Случайная позиция
    leds[pos] = color;                       // Яркая белая вспышка
    FastLED.show();
  }
}
// 4. RUNNING LIGHTS
void runningLightsMode() {
  static unsigned long lastUpdate = 0;
  static float position = 0;  // Поточная позиция волны
  FastLED.setBrightness(255);
  if (millis() - lastUpdate >= animationSpeed * 2) {
    lastUpdate = millis();
    position += 0.5;  // Движение волны
    for (int i = 0; i < REAL_NUM_LEDS; i++) {
      // Формула волны: синусоида для плавного эффекта
      float wave = sin((i + position) * 0.25) * 0.5 + 0.5;  // Значения от 0 до 1
      uint8_t intensity = wave * 255;                       // Интенсивность цвета

      leds[i] = color;                         // Базовый цвет
      leds[i].fadeToBlackBy(255 - intensity);  // Применяем яркость волны
    }
    FastLED.show();  // Обновление ленты
  }
}
// 5. FIRE
void fireMode() {
  static byte* heat = nullptr;          // Указатель на массив тепла
  static unsigned long lastUpdate = 0;  // Время последнего обновления
  static uint16_t previousLedCount = 0;
  if (REAL_NUM_LEDS != previousLedCount) {
    if (heat != nullptr) {
      delete[] heat;
    }
    heat = new byte[REAL_NUM_LEDS];
    memset(heat, 0, REAL_NUM_LEDS);
    previousLedCount = REAL_NUM_LEDS;
  }
  FastLED.setBrightness(commonBrightness);
  if (millis() - lastUpdate >= animationSpeed) {
    lastUpdate = millis();
    for (int i = 0; i < REAL_NUM_LEDS; i++) {
      heat[i] = qsub8(heat[i], random(10, 30));
    }
    int centerHeat = random(150, 255);
    heat[REAL_NUM_LEDS / 2] = qadd8(heat[REAL_NUM_LEDS / 2], centerHeat);
    for (int i = 0; i < REAL_NUM_LEDS / 8; i++) {
      int pos = random(REAL_NUM_LEDS);
      heat[pos] = qadd8(heat[pos], random(120, 200));
    }
    for (int i = 1; i < REAL_NUM_LEDS - 1; i++) {
      heat[i] = (heat[i - 1] + heat[i] + heat[i + 1]) / 3;
    }
    for (int i = 0; i < REAL_NUM_LEDS; i++) {
      CRGB color = HeatColor(heat[i]);
      leds[i] = color.fadeToBlackBy(50);
    }
    FastLED.show();
  }
}
// 6. FADE1
void fade1Mode() {
  static uint8_t brightness = 0;
  static int direction = 1;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= animationSpeed / 1.5) {
    lastUpdate = millis();
    brightness += direction * 5;
    if (brightness == 0 || brightness == 255) {
      direction *= -1;
    }
    CHSV hsvColor = rgb2hsv_approximate(color);
    for (int i = 0; i < REAL_NUM_LEDS; i++) {
      leds[i] = CHSV(hsvColor.h, hsvColor.s, brightness);
    }
    FastLED.show();
  }
}
// 7. FADE2
void fade2Mode() {
  static bool evenPhase = true;
  static unsigned long lastUpdate = 0;
  static uint8_t phase = 0; // Phase for cubic wave

  if (millis() - lastUpdate >= animationSpeed) {
    lastUpdate = millis();

    // Calculate the cubic wave value for smooth brightness transitions
    uint8_t brightness = cubicwave8(phase);

    // Increment phase to animate the wave
    phase++;

    // When phase completes a full cycle, reset it
    if (phase >= 256) {
      phase = 0;
      evenPhase = !evenPhase; // Toggle between even and odd phase
    }

    // Update LED colors based on cubic wave brightness
    for (int i = 0; i < REAL_NUM_LEDS; i++) {
      if (evenPhase) {
        // Even phase: Apply color1 to even-indexed LEDs
        leds[i] = (i % 2 == 0) 
            ? CRGB(customFadeColor1.r * brightness / 255,
                   customFadeColor1.g * brightness / 255,
                   customFadeColor1.b * brightness / 255)
            : CRGB::Black;
      } else {
        // Odd phase: Apply color2 to odd-indexed LEDs
        leds[i] = (i % 2 != 0)
            ? CRGB(customFadeColor2.r * brightness / 255,
                   customFadeColor2.g * brightness / 255,
                   customFadeColor2.b * brightness / 255)
            : CRGB::Black;
      }
    }

    FastLED.show();
  }
}

// 8. METEOR
void meteorMode() {
  static unsigned long lastUpdate = 0;
  static int pos = 0;
  FastLED.setBrightness(255);
  if (millis() - lastUpdate >= animationSpeed * 4) {
    lastUpdate = millis();
    fadeToBlackBy(leds, REAL_NUM_LEDS, 70);
    leds[pos] = color;
    pos++;
    if (pos >= REAL_NUM_LEDS) pos = 0;
    FastLED.show();
  }
}
// 9. FLAG OF UKRAINE
void flagMode() {
  static unsigned long lastUpdate = 0;
  FastLED.setBrightness(commonBrightness);
  if (millis() - lastUpdate >= animationSpeed * 2) {
    lastUpdate = millis();
    if (flagIsStatic) {
      for (int i = 0; i < REAL_NUM_LEDS / 2; i++) {
        leds[i] = CRGB::Blue;
      }
      for (int i = REAL_NUM_LEDS / 2; i < REAL_NUM_LEDS; i++) {
        leds[i] = CRGB::Yellow;
      }
      FastLED.show();
    } else {
      static uint8_t offset = 0;
      for (int i = 0; i < REAL_NUM_LEDS; i++) {
        uint8_t wave = sin8(i * 10 + offset);
        if (i < REAL_NUM_LEDS / 2) {
          leds[i] = CHSV(160, 255, wave);
        } else {
          leds[i] = CHSV(40, 255, wave);
        }
      }
      offset += 10;
      FastLED.show();
    }
  }
}
// 10. CUSTOM LIGHTING
void customMode() {
  static unsigned long lastUpdate = 0;
  FastLED.setBrightness(commonBrightness);
  if (millis() - lastUpdate >= 50) {
    lastUpdate = millis();
    // Определяем количество диодов на каждый цвет
    size_t ledsPerColor = (REAL_NUM_LEDS + 9) / 10;  // округляем вверх
    for (size_t i = 0; i < REAL_NUM_LEDS; ++i) {
      // Определяем текущий цвет из массива customColorsArray
      size_t colorIndex = i / ledsPerColor;  // индекс цвета
      if (colorIndex >= 10) colorIndex = 9;  // защита от выхода за пределы массива
      leds[i] = customColorsArray[colorIndex];
    }
    FastLED.show();
  }
}

void updateLEDState() {
  // Вызываем текущий режим на основе текущего времени
  if (ledState) {
    switch (currentMode) {
      case 0:
        normalMode();  // STATIC COLOR
        break;
      case 1:
        rainbowMode();  // RAINBOW
        break;
      case 2:
        gradientFillMode();  // GRADIENT FILL
        break;
      case 3:
        sparkleMode();  // SPARKLE
        break;
      case 4:
        runningLightsMode();  // RUNNING LIGHTS
        break;
      case 5:
        fireMode();  // FIRE
        break;
      case 6:
        fade1Mode();  // FADE1
        break;
      case 7:
        fade2Mode();  // FADE2
        break;
      case 8:
        meteorMode();  // METEOR
        break;
      case 9:
        flagMode();  // FLAG OF UKRAINE
        break;
      case 10:
        customMode();  // CUSTOM LIGHTING
        break;
      default:
        normalMode();  // Режим по умолчанию (STATIC COLOR)
        break;
    }
  } else {
    fadeToBlackBy(leds, REAL_NUM_LEDS, 30);  // Постепенное выключение ленты
    FastLED.show();
  }
}

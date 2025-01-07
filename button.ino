#include "config.h"

// Function to handle button presses and actions
void handleButton() {
  static bool incrementBrightness = true;         // Brightness direction: true for increasing, false for decreasing
  static bool incrementSpeed = true;              // Speed direction: true for increasing, false for decreasing
  static unsigned long lastBrightnessChange = 0;  // Last time brightness was changed
  static unsigned long lastSpeedChange = 0;       // Last time animation speed was changed
  const unsigned long brightnessInterval = 100;   // Interval for brightness change (milliseconds)
  const unsigned long speedInterval = 100;        // Interval for animation speed change (milliseconds)

  // Single click: Toggle LED state
  if (btn.isSingle()) {
    DEBUG_PRINTLN("Toggle ledState");
    ledState = !ledState;  // Turn LED on or off
  }

  if (ledState) {
    // Double click: Switch between offline modes
    if (btn.isDouble()) {
      static int offlineModeIndex = 0;  // Start from the first offline mode
      offlineModeIndex = (offlineModeIndex + 1) % numOfflineModes;  // Move to the next offline mode (wrap around)
      currentMode = offlineModes[offlineModeIndex];                 // Set the current mode
      DEBUG_PRINTLN("Current Mode: " + String(modeList[currentMode]));
    }

    // Triple click: Change color
    if (btn.isTriple()) {
      currentColorIndex = (currentColorIndex + 1) % numColors;  // Move to the next color
      color = colors[currentColorIndex];                        // Set the new color
      DEBUG_PRINTLN("Color changed to: " + String(currentColorIndex));
    }

    // Hold actions
    if (btn.isHold()) {
      // Hold with 1 click: Adjust brightness
      if (btn.getHoldClicks() == 1) {
        if (millis() - lastBrightnessChange >= brightnessInterval) {
          lastBrightnessChange = millis();  // Update last change time
          if (incrementBrightness) {
            commonBrightness += 5;
            if (commonBrightness >= 255) {  // Maximum brightness reached
              commonBrightness = 255;
            }
          } else {
            commonBrightness -= 5;
            if (commonBrightness <= 10) {  // Minimum brightness reached
              commonBrightness = 10;
            }
          }
          DEBUG_PRINTLN("Brightness: " + String(commonBrightness));
        }
      }

      // Hold with 2 clicks: Adjust animation speed
      if (btn.getHoldClicks() == 2) {
        if (millis() - lastSpeedChange >= speedInterval) {
          lastSpeedChange = millis();  // Update last change time
          if (incrementSpeed) {
            animationSpeed += 10.0f;
            if (animationSpeed >= 150.0f) {  // Maximum speed reached
              animationSpeed = 150.0f;
            }
          } else {
            animationSpeed -= 10.0f;
            if (animationSpeed <= 10.0f) {  // Minimum speed reached
              animationSpeed = 10.0f;
            }
          }
          DEBUG_PRINTLN("Animation Speed: " + String(animationSpeed));
        }
      }

      // Release the button: Toggle increment direction
      if (btn.isRelease()) {
        if (btn.getHoldClicks() == 1) {
          incrementBrightness = !incrementBrightness;  // Toggle brightness direction
          DEBUG_PRINTLN("Brightness direction changed: " + String(incrementBrightness ? "Increasing" : "Decreasing"));
        } else if (btn.getHoldClicks() == 2) {
          incrementSpeed = !incrementSpeed;  // Toggle speed direction
          DEBUG_PRINTLN("Speed direction changed: " + String(incrementSpeed ? "Increasing" : "Decreasing"));
        }
      }
    }
  }
}

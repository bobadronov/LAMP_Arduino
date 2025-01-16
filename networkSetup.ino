// Captive Portal configuration
const IPAddress localIP(4, 3, 2, 1);
const IPAddress gatewayIP(4, 3, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);
const String localIPURL = "http://4.3.2.1";

// Функция для получения текущих WiFi-учетных данных
void getWifiCreds();
// Функция для сохранения новых учетных данных WiFi
void saveNewCreds(const String &ssid, const String &password, const String &deviceName);
// Функция для перезапуска ESP (микроконтроллера)
void espRestart();

void startCaptivePortal() {
  // Configure and start the Access Point
  WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
  WiFi.softAP(AP_SSID, NULL, 6, false, 4);

  // Disable AMPDU RX on the ESP32 WiFi to fix a bug on Android
  esp_wifi_stop();
  esp_wifi_deinit();
  wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
  my_config.ampdu_rx_enable = false;
  esp_wifi_init(&my_config);
  esp_wifi_start();
  vTaskDelay(100 / portTICK_PERIOD_MS);  // Add a small delay

  // Start DNS Server
  dnsServer.setTTL(3600);
  dnsServer.start(53, "*", localIP);
  dnsServerStarted = true;
  // Set up web server
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html",
                                                              reinterpret_cast<const uint8_t *>(index_html), index_html_len);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    request->send(response);
  });

  // Required for handling different captive portal requests from various OS
  server.on("/connecttest.txt", [](AsyncWebServerRequest *request) {
    request->redirect("http://logout.net");  // windows 11 captive portal workaround
  });

  server.on("/wpad.dat", [](AsyncWebServerRequest *request) {
    request->send(404);  // Stop Win10 from requesting this file repeatedly
  });

  // Redirect to the attacker's local web server for captive portal redirection
  server.on("/generate_204", [](AsyncWebServerRequest *request) {
    request->redirect("http://connect.rom.miui.com");  // Redirect to attacker's server
  });

  server.on("/redirect", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);  // redirect for Microsoft captive portal
  });

  server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);  // apple captive portal redirection
  });

  server.on("/canonical.html", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);  // Firefox captive portal redirect
  });

  server.on("/success.txt", [](AsyncWebServerRequest *request) {
    request->send(200);  // Firefox captive portal success check
  });

  server.on("/ncsi.txt", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);  // Windows captive portal redirect
  });

  // B Tier (uncommon cases, but helpful for complete coverage)
  server.on("/chrome-variations/seed", [](AsyncWebServerRequest *request) {
    request->send(200);  // Chrome captive portal check
  });

  // Handle requests from other OS and browsers
  server.on("/startpage", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);  // Redirect for devices calling a "start" page
  });

  // Return 404 for favicon requests (we don't need a favicon here)
  server.on("/favicon.ico", [](AsyncWebServerRequest *request) {
    request->send(404);  // favicon icon request
  });

  // Catch-all for any other request that doesn't match
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);  // Redirect unknown paths to the captive portal
  });

  // Log the AP IP address for debugging
  DEBUG_PRINT("AP IP Address: ");
  DEBUG_PRINTLN(WiFi.softAPIP());
}

// Main network setup
void setupNetwork() {
  getWifiCreds();
  WiFi.begin(SSID, PASSWORD);
  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 10000;  // 10 seconds timeout
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    DEBUG_PRINTLN("Connecting to WiFi...");
  }
  if (WiFi.status() != WL_CONNECTED) {
    espInAPMode = true;
    DEBUG_PRINTLN("WiFi connection failed. Starting AP Mode...");
    startCaptivePortal();
  } else {
    espInAPMode = false;
    DEBUG_PRINTLN("Connected to WiFi.");
    DEBUG_PRINT("IP Address: ");
    DEBUG_PRINTLN(WiFi.localIP());
  }
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html",
                                                              reinterpret_cast<const uint8_t *>(index_html), index_html_len);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    request->send(response);
  });

  server.on("/wifi-config", HTTP_POST, [](AsyncWebServerRequest *request) {
    String ssid = "";
    String password = "";
    String deviceName = "ESP_LED";  // Значение по умолчанию

    if (request->hasParam("ssid", true)) {
      ssid = request->getParam("ssid", true)->value();
    }
    if (request->hasParam("password", true)) {
      password = request->getParam("password", true)->value();
    }
    if (request->hasParam("deviceName", true)) {
      deviceName = request->getParam("deviceName", true)->value();
    }

    DEBUG_PRINTLN("Received Wi-Fi credentials:");
    DEBUG_PRINTLN("SSID: " + ssid);
    DEBUG_PRINTLN("Password: " + password);
    DEBUG_PRINTLN("Device Name: " + deviceName);

    saveNewCreds(ssid, password, deviceName);

    // Сохранение данных в EEPROM или другой постоянной памяти (реализация по желанию)
    request->send(200, "text/html", "<h1>Wi-Fi settings saved. Please restart the device.</h1>");
    espRestart();
  });
}

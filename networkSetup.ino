// Captive Portal configuration
const IPAddress localIP(4, 3, 2, 1);
const IPAddress gatewayIP(4, 3, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);
const String localIPURL = "http://4.3.2.1";
// Функция для получения текущих WiFi-учетных данных
void getWifiCreds();

void saveNewCreds(const String &ssid, const String &password, const String &deviceName);
void espRestart();
const char index_html[] PROGMEM = R"=====(
      <!DOCTYPE html>
      <html>
      <head>
          <title>Wi-Fi Setup</title>
          <style>
              body {
                  font-family: Arial, sans-serif;
                  background-color: #f4f4f9;
                  color: #333;
                  display: flex;
                  justify-content: center;
                  align-items: center;
                  height: 100vh;
                  margin: 0;
              }
              .container {
                  text-align: center;
                  background: white;
                  padding: 20px;
                  border-radius: 8px;
                  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
              }
              h1 {
                  color: #06cc13;
              }
              input[type="text"], input[type="password"] {
                  width: 80%;
                  padding: 10px;
                  margin: 10px 0;
                  border: 1px solid #ccc;
                  border-radius: 4px;
                  box-sizing: border-box;
              }
              button {
                  background-color: #06cc13;
                  color: white;
                  border: none;
                  padding: 10px 20px;
                  border-radius: 4px;
                  cursor: pointer;
                  font-size: 16px;
              }
              button:hover {
                  background-color: #05a110;
              }
          </style>
          <meta name="viewport" content="width=device-width, initial-scale=1.0">
      </head>
      <body>
          <div class="container">
              <h1>Wi-Fi Setup</h1>
              <form action="/wifi-config" method="POST">
                  <label for="ssid">Wi-Fi SSID:</label><br>
                  <input type="text" id="ssid" name="ssid" placeholder="Enter SSID" required><br>
                  <label for="password">Wi-Fi Password:</label><br>
                  <input type="password" id="password" name="password" placeholder="Enter Password"><br>
                  <label for="deviceName">Device Name:</label><br>
                  <input type="text" id="deviceName" name="deviceName" placeholder="Enter Device Name" required><br>
                  <button type="submit">Save</button>
              </form>
          </div>
      </body>
      </html>
    )====="
;

// const char control_html[] PROGMEM = R"=====( 
//   <!DOCTYPE html>
//   <html>
//   <head>
//     <title>LED Control</title>
//     <style>
//         body {
//             font-family: Arial, sans-serif;
//             background-color: #1e1e2f;
//             color: #f4f4f9;
//             display: flex;
//             justify-content: center;
//             align-items: center;
//             height: 100vh;
//             margin: 0;
//         }
//         .container {
//             text-align: center;
//             background: #262837;
//             padding: 20px;
//             border-radius: 8px;
//             box-shadow: 0 4px 8px rgba(0, 0, 0, 0.5);
//         }
//         h1 {
//             color: #06cc13;
//         }
//         label {
//             font-size: 16px;
//             color: #ccc;
//             margin-top: 10px;
//         }
//         select, input[type="color"], button {
//             width: 80%;
//             padding: 10px;
//             margin: 10px 0;
//             border: 1px solid #444;
//             border-radius: 4px;
//             box-sizing: border-box;
//             background-color: #333;
//             color: #f4f4f9;
//         }
//         button {
//             background-color: #06cc13;
//             color: white;
//             border: none;
//             cursor: pointer;
//             font-size: 16px;
//         }
//         button:hover {
//             background-color: #05a110;
//         }
//     </style>
//     <meta name="viewport" content="width=device-width, initial-scale=1.0">
//   </head>
//   <body>
//     <div class="container">
//         <h1>LED Control</h1>
//         <form action="/led-control" method="POST">
//             <label for="mode">Select Mode:</label><br>
//             <select id="mode" name="mode">
//               <option value="0">Static Color</option>
//               <option value="1">Rainbow</option>
//               <option value="2">Breathing</option>
//               <option value="3">Strobe</option>
//               <option value="4">Sparkle</option>
//               <option value="5">Running Lights</option>
//               <option value="6">Fire</option>
//               <option value="7">Color Wipe</option>
//               <option value="8">Meteor</option>
//               <option value="9">Flag of Ukraine</option>
//             </select><br>
//             <label for="color">Pick a Color:</label><br>
//             <input type="color" id="color" name="color" value="#06cc13"><br>

//             <button type="submit">Apply</button>
//         </form>
//     </div>
//   </body>
//   </html>
//   )====="
// ;

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
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", index_html);
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

    Serial.println("Received Wi-Fi credentials:");
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
    Serial.println("Device Name: " + deviceName);

    saveNewCreds(ssid, password, deviceName);

    // Сохранение данных в EEPROM или другой постоянной памяти (реализация по желанию)
    request->send(200, "text/html", "<h1>Wi-Fi settings saved. Please restart the device.</h1>");
    espRestart();
  });

  server.on("/led-control", HTTP_POST, [](AsyncWebServerRequest *request) {
    int mode = -1;  // Неверное значение по умолчанию
    String newColor = "#FFFFFF";

    if (request->hasParam("mode", true)) {
      mode = request->getParam("mode", true)->value().toInt();
    }

    if (request->hasParam("color", true)) {
      newColor = request->getParam("color", true)->value();
    }

    // Валидация входных данных
    if (mode < 0 || mode > 9) {
      request->send(400, "text/plain", "Invalid mode value");
      return;
    }

    if (newColor[0] != '#' || newColor.length() != 7) {
      request->send(400, "text/plain", "Invalid color format");
      return;
    }

    // Лог для отладки
    Serial.println("Selected Mode Index: " + String(mode));
    Serial.println("Selected Color: " + newColor);

    // Применение настроек
    currentMode = mode;
    long hexColor = strtol(newColor.c_str() + 1, nullptr, 16);
    color = CRGB((hexColor >> 16) & 0xFF, (hexColor >> 8) & 0xFF, hexColor & 0xFF);

    // Ответ для клиента
    request->send(200, "text/html", "<h1>Settings Applied</h1>");
  });


  // Required
  server.on("/connecttest.txt", [](AsyncWebServerRequest *request) {
    request->redirect("http://logout.net");
  });  // windows 11 captive portal workaround
  server.on("/wpad.dat", [](AsyncWebServerRequest *request) {
    request->send(404);
  });  // Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)
  server.on("/generate_204", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  });  // android captive portal redirect
  server.on("/redirect", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  });  // microsoft redirect
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  });  // apple call home
  server.on("/canonical.html", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  });  // firefox captive portal call home
  server.on("/success.txt", [](AsyncWebServerRequest *request) {
    request->send(200);
  });  // firefox captive portal call home
  server.on("/ncsi.txt", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  });  // windows call home

  // B Tier (uncommon)
  server.on("/chrome-variations/seed", [](AsyncWebServerRequest *request) {
    request->send(200);
  });  //chrome captive portal call home
       //  server.on("/service/update2/json",[](AsyncWebServerRequest *request){request->send(200);}); //firefox?
       //  server.on("/chat",[](AsyncWebServerRequest *request){request->send(404);}); //No stop asking Whatsapp, there is no internet connection
  server.on("/startpage", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  });

  // return 404 to webpage icon
  server.on("/favicon.ico", [](AsyncWebServerRequest *request) {
    request->send(404);
  });  // webpage icon

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  });
  // Serial.println("Captive Portal started.");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());
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
    Serial.println("Connecting to WiFi...");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection failed. Starting AP Mode...");
    startCaptivePortal();
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Connected to WiFi.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

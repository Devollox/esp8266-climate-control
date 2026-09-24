#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SHT4x.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_SHT4x sht40 = Adafruit_SHT4x();

const char* WIFI_SSID = "HH71VM_C66E_2.4G";
const char* WIFI_PASSWORD = "00334519";
const char* API_TOKEN = "replace-with-a-long-random-token";

ESP8266WebServer server(80);

struct SensorReading {
  float temperature = 0.0;
  float humidity = 0.0;
  unsigned long updatedAtMs = 0;
};

SensorReading reading;

unsigned long lastSensorUpdate = 0;
const unsigned long SENSOR_PERIOD = 3000;

const char INDEX_HTML[] PROGMEM = R"=====(
<!doctype html>
<html lang="ru">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="theme-color" content="#09090b">
  <title>~/ room-sensor</title>

  <style>
    :root {
      --background: #09090b;
      --foreground: #fafafa;
      --card: #111113;
      --secondary: #18181b;
      --border: #27272a;
      --muted: #a1a1aa;
      --muted-dim: #71717a;
      --accent: #22c55e;
      --danger: #f87171;
    }

    * {
      box-sizing: border-box;
    }

    html {
      background: var(--background);
    }

    body {
      margin: 0;
      min-height: 100vh;
      background: var(--background);
      color: var(--foreground);
      font-family:
        ui-monospace,
        SFMono-Regular,
        Menlo,
        Monaco,
        Consolas,
        "Liberation Mono",
        "Courier New",
        monospace;
      -webkit-font-smoothing: antialiased;
      text-rendering: optimizeLegibility;
    }

    header {
      border-bottom: 1px solid var(--border);
    }

    .nav {
      width: min(720px, calc(100% - 32px));
      margin: 0 auto;
      min-height: 64px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 16px;
    }

    .brand {
      display: flex;
      align-items: baseline;
      gap: 8px;
      color: var(--foreground);
      text-decoration: none;
      font-weight: 700;
      letter-spacing: -0.03em;
      font-size: 18px;
    }

    .accent {
      color: var(--accent);
    }

    .nav-status {
      color: var(--muted);
      font-size: 12px;
      white-space: nowrap;
    }

    .container {
      width: min(720px, calc(100% - 32px));
      margin: 0 auto;
      padding: 40px 0;
    }

    .terminal {
      overflow: hidden;
      border: 1px solid var(--border);
      background: var(--card);
    }

    .terminal-bar {
      display: flex;
      align-items: center;
      gap: 8px;
      min-height: 40px;
      padding: 0 12px;
      border-bottom: 1px solid var(--border);
      background: var(--secondary);
      color: var(--muted);
      font-size: 12px;
    }

    .dot {
      color: var(--accent);
      font-size: 14px;
      line-height: 1;
    }

    .terminal-content {
      padding: 24px;
    }

    .command {
      margin: 0 0 6px;
      color: var(--foreground);
      font-size: 14px;
    }

    .comment {
      margin: 0 0 28px;
      color: var(--muted);
      font-size: 12px;
      line-height: 1.6;
    }

    .readings {
      border-top: 1px solid var(--border);
    }

    .reading {
      display: grid;
      grid-template-columns: minmax(120px, 1fr) auto;
      align-items: center;
      gap: 16px;
      min-height: 92px;
      border-bottom: 1px solid var(--border);
    }

    .reading-label {
      color: var(--muted);
      font-size: 12px;
    }

    .reading-key {
      display: block;
      margin-top: 5px;
      color: var(--foreground);
      font-size: 15px;
    }

    .reading-value {
      color: var(--accent);
      font-size: clamp(28px, 8vw, 42px);
      font-weight: 700;
      letter-spacing: -0.06em;
      line-height: 1;
      white-space: nowrap;
      font-variant-numeric: tabular-nums;
    }

    .reading-unit {
      margin-left: 4px;
      color: var(--muted);
      font-size: 16px;
      font-weight: 400;
      letter-spacing: normal;
    }

    .meta {
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      gap: 12px;
      padding-top: 18px;
      color: var(--muted-dim);
      font-size: 12px;
    }

    .meta-value {
      color: var(--muted);
      font-variant-numeric: tabular-nums;
    }

    .status {
      display: inline-flex;
      align-items: center;
      gap: 7px;
    }

    .status-dot {
      width: 7px;
      height: 7px;
      border-radius: 999px;
      background: var(--accent);
      box-shadow: 0 0 10px rgba(34, 197, 94, 0.7);
    }

    .status-dot.error {
      background: var(--danger);
      box-shadow: 0 0 10px rgba(248, 113, 113, 0.7);
    }

    #error {
      min-height: 18px;
      margin: 18px 0 0;
      color: var(--danger);
      font-size: 12px;
      line-height: 1.5;
    }

    .footer {
      margin-top: 18px;
      color: var(--muted-dim);
      font-size: 11px;
      line-height: 1.6;
    }

    @media (max-width: 480px) {
      .nav {
        width: min(100% - 24px, 720px);
      }

      .container {
        width: min(100% - 24px, 720px);
        padding: 24px 0;
      }

      .terminal-content {
        padding: 18px;
      }

      .reading {
        min-height: 82px;
      }

      .reading-value {
        font-size: 30px;
      }

      .reading-unit {
        font-size: 14px;
      }
    }
  </style>
</head>

<body>
  <header>
    <nav class="nav">
      <a class="brand" href="/">
        <span class="accent">~/</span>
        <span>room-sensor</span>
      </a>

      <span class="nav-status">[ ESP8266 ]</span>
    </nav>
  </header>

  <main class="container">
    <section class="terminal" aria-label="Данные датчика комнаты">
      <div class="terminal-bar">
        <span class="dot" aria-hidden="true">●</span>
        <span>room-sensor — monitor</span>
      </div>

      <div class="terminal-content">
        <p class="command">
          <span class="accent">$</span> sensor.read()
        </p>

        <p class="comment">
          // temperature and relative humidity<br>
          // refresh interval: 3 seconds
        </p>

        <div class="readings">
          <div class="reading">
            <div class="reading-label">
              <span>sensor.temperature</span>
              <span class="reading-key">Температура</span>
            </div>

            <div class="reading-value">
              <span id="temperature">--.-</span><span class="reading-unit">°C</span>
            </div>
          </div>

          <div class="reading">
            <div class="reading-label">
              <span>sensor.humidity</span>
              <span class="reading-key">Влажность</span>
            </div>

            <div class="reading-value">
              <span id="humidity">--.-</span><span class="reading-unit">%</span>
            </div>
          </div>
        </div>

        <div class="meta">
          <span class="status">
            <span id="status-dot" class="status-dot"></span>
            <span id="status">connecting...</span>
          </span>

          <span>
            updated:
            <span id="updated" class="meta-value">--:--:--</span>
          </span>
        </div>

        <p id="error" role="alert"></p>
      </div>
    </section>

    <footer class="footer">
      <span class="accent">[ info ]</span>
      SHT40 → ESP8266 → local HTTP API
    </footer>
  </main>

  <script>
    const temperatureEl = document.querySelector('#temperature');
    const humidityEl = document.querySelector('#humidity');
    const updatedEl = document.querySelector('#updated');
    const statusEl = document.querySelector('#status');
    const statusDotEl = document.querySelector('#status-dot');
    const errorEl = document.querySelector('#error');

    function setStatus(text, isError) {
      statusEl.textContent = text;
      statusDotEl.classList.toggle('error', isError);
    }

    function formatValue(value) {
      const number = Number(value);

      if (!Number.isFinite(number)) {
        return '--.-';
      }

      return number.toFixed(1);
    }

    async function loadReadings() {
      try {
        setStatus('fetching...', false);

        const response = await fetch('/api/readings', {
          cache: 'no-store'
        });

        if (!response.ok) {
          throw new Error('HTTP ' + response.status);
        }

        const data = await response.json();

        temperatureEl.textContent = formatValue(data.temperature);
        humidityEl.textContent = formatValue(data.humidity);
        updatedEl.textContent = new Date().toLocaleTimeString('ru-RU');

        setStatus('online', false);
        errorEl.textContent = '';
      } catch (error) {
        setStatus('offline', true);
        errorEl.textContent = '[ error ] ' + error.message;
      }
    }

    loadReadings();
    setInterval(loadReadings, 3000);
  </script>
</body>
</html>
)=====";

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("IP: ");
  display.print(WiFi.localIP());

  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print("T: ");
  display.print(reading.temperature, 1);
  display.print(" C");

  display.setCursor(0, 45);
  display.print("H: ");
  display.print(reading.humidity, 1);
  display.print(" %");

  display.display();
}

void readPhysicalSensor() {
  sensors_event_t humidityEvent, tempEvent;

  sht40.getEvent(&humidityEvent, &tempEvent);

  reading.temperature = tempEvent.temperature;
  reading.humidity = humidityEvent.relative_humidity;
  reading.updatedAtMs = millis();

  Serial.printf("SHT40 Read -> T: %.2f C, H: %.2f%%\n", reading.temperature, reading.humidity);

  updateOLED();
}

bool isAuthorized() { return server.hasHeader("X-Api-Token") && server.header("X-Api-Token") == API_TOKEN; }
void sendJson(int statusCode, JsonDocument& json) { String body; serializeJson(json, body); server.sendHeader("Access-Control-Allow-Origin", "*"); server.send(statusCode, "application/json; charset=utf-8", body); }
void handleIndex() { server.send_P(200, "text/html; charset=utf-8", INDEX_HTML); }

void handleStatus() {
  JsonDocument json;
  json["ok"] = true;
  json["device"] = "esp8266-nodemcu-v3-sht40";
  json["ip"] = WiFi.localIP().toString();
  json["uptimeMs"] = millis();
  json["freeHeap"] = ESP.getFreeHeap();
  sendJson(200, json);
}

void handleGetReadings() {
  JsonDocument json;
  json["temperature"] = reading.temperature;
  json["humidity"] = reading.humidity;
  json["updatedAtMs"] = reading.updatedAtMs;
  sendJson(200, json);
}

void handlePostReadings() {
  if (!isAuthorized()) { JsonDocument json; json["ok"] = false; json["error"] = "Unauthorized."; sendJson(401, json); return; }
  JsonDocument input;
  DeserializationError error = deserializeJson(input, server.arg("plain"));
  if (error || !input["temperature"].is<float>() || !input["humidity"].is<float>()) { JsonDocument json; json["ok"] = false; json["error"] = "Invalid payload."; sendJson(400, json); return; }
  reading.temperature = input["temperature"].as<float>();
  reading.humidity = input["humidity"].as<float>();
  reading.updatedAtMs = millis();
  updateOLED();
  JsonDocument json; json["ok"] = true; sendJson(200, json);
}

void handleNotFound() { JsonDocument json; json["ok"] = false; json["error"] = "Route not found."; sendJson(404, json); }

void connectToWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Connecting to Wi-Fi: %s", WIFI_SSID);

  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - startedAt > 30000) { Serial.println("\nWi-Fi timeout. Restarting..."); ESP.restart(); }
  }
  Serial.println("\nWi-Fi connected.");
  Serial.print("Open in browser: http://");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(300);

  Wire.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED на адресе 0x3C не найден. Пробуем адрес 0x3D..."));

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println(F("Ошибка: OLED дисплей вообще не определился на шине I2C!"));
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Connecting Wi-Fi...");
  display.display();

  if (!sht40.begin()) {
    Serial.println("SHT40 не найден! Проверьте контакты.");
  } else {
    Serial.println("SHT40 успешно определился!");
    sht40.setPrecision(SHT4X_HIGH_PRECISION);
    sht40.setHeater(SHT4X_NO_HEATER);
  }

  connectToWifi();

  server.collectHeaders("X-Api-Token");
  server.on("/", HTTP_GET, handleIndex);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/readings", HTTP_GET, handleGetReadings);
  server.on("/api/readings", HTTP_POST, handlePostReadings);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP server started.");

  readPhysicalSensor();
}

void loop() {
  server.handleClient();

  if (millis() - lastSensorUpdate >= SENSOR_PERIOD) {
    lastSensorUpdate = millis();
    readPhysicalSensor();
  }
}

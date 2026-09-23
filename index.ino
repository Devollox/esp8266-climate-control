#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

const char* WEBHOOK_URL = "http://192.168.1.100:3000";

WiFiClient client;
unsigned long requestId = 0;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Wi-Fi");

  unsigned long startedAt = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if (millis() - startedAt > 20000) {
      Serial.println("\nWi-Fi timeout, restarting...");
      ESP.restart();
    }
  }

  Serial.println("\nConnected");
  Serial.print("ESP IP: ");
  Serial.println(WiFi.localIP());
}

void sendWebhook() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi lost");
    connectWiFi();
    return;
  }

  HTTPClient http;
  http.setTimeout(10000);

  if (!http.begin(client, WEBHOOK_URL)) {
    Serial.println("Cannot initialize HTTP request");
    return;
  }

  requestId++;

  String body = "{";
  body += "\"source\":\"esp8266\",";
  body += "\"event\":\"local-test\",";
  body += "\"requestId\":" + String(requestId) + ",";
  body += "\"uptimeMs\":" + String(millis()) + ",";
  body += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  body += "}";

  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(body);

  Serial.print("HTTP status: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    Serial.print("Response: ");
    Serial.println(http.getString());
  } else {
    Serial.print("Request error: ");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  connectWiFi();
}

void loop() {
  sendWebhook();
  delay(5000);
}

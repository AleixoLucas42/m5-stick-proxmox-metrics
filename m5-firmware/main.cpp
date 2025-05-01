#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid     = "";
const char* password = "";
const char* url      = "http://10.11.12.42:8000";

void drawBar(int x, int y, int w, int h, float percent, const char* label, uint32_t color) {
  M5.Display.setCursor(x, y - 10);
  M5.Display.setTextColor(WHITE);
  M5.Display.print(label);
  
  M5.Display.drawRect(x, y, w, h, WHITE);
  int fillWidth = (int)(w * percent / 100.0);
  M5.Display.fillRect(x, y, fillWidth, h, color);
  
  M5.Display.setCursor(x + w + 5, y);
  M5.Display.printf("%.1f%%", percent);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setRotation(3); // Rotacione se necessário
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(0, 0);
  M5.Display.println("Conectando Wi-Fi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Display.print(".");
  }

  M5.Display.println("\nConectado!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == 200) {
      String payload = http.getString();

      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        float cpu = doc["cpu_percent"];
        float mem = doc["memory_percent"];
        float disk = doc["disk_percent"];

        M5.Display.fillScreen(BLACK);

        drawBar(10, 40, 100, 10, cpu, "CPU", GREEN);
        drawBar(10, 80, 100, 10, mem, "Mem", CYAN);
        drawBar(10, 120, 100, 10, disk, "Disk", ORANGE);
      } else {
        M5.Display.setCursor(0, 0);
        M5.Display.println("Erro JSON");
      }

    } else {
      M5.Display.setCursor(0, 0);
      M5.Display.println("Erro HTTP");
    }

    http.end();
  }

  delay(5000);
}


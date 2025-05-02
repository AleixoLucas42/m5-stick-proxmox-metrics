#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid     = "QUARK DOWN";
const char* password = "Aleixo@123";
const char* url      = "http://m5-stick-proxmox-metrics.aleixohome.lan";

// Função para desenhar uma barra com label e percentual
void drawBar(int x, int y, int w, int h, float percent, const char* label, uint32_t color) {
  // Desenha o label mais afastado acima da barra
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(x, y - 20);  // Aumenta a distância vertical
  M5.Display.print(label);

  // Desenha o contorno da barra
  M5.Display.drawRect(x, y, w, h, WHITE);

  // Preenche a barra proporcionalmente ao percentual
  int fillWidth = (int)(w * percent / 100.0);
  M5.Display.fillRect(x, y, fillWidth, h, color);

  // Exibe o valor percentual à direita da barra
  M5.Display.setCursor(x + w + 5, y - 2);  // Leve ajuste para alinhar verticalmente
  M5.Display.printf("%.1f%%", percent);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setRotation(3); // Rotaciona a tela, se necessário
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(0, 0);
  M5.Display.println("Conecting to Wi-Fi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Display.print(".");
  }

  M5.Display.println("\nConected!");
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
        drawBar(10, 30, 100, 10, cpu, "CPU", GREEN);
        drawBar(10, 75, 100, 10, mem, "Mem", CYAN);
        drawBar(10, 120, 100, 10, disk, "Disk", ORANGE);
      } else {
        M5.Display.setCursor(0, 0);
        M5.Display.println("Error JSON");
      }

    } else {
      M5.Display.setCursor(0, 0);
      M5.Display.println("Error HTTP");
    }

    http.end();
  }

  delay(3000);
}


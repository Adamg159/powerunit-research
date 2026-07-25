// Arrival-day board verification for AITRIP ESP-WROOM-32 DevKits (3 boards).
// Proves per board: USB/serial enumeration, flashability, GPIO (onboard LED), and WiFi radio.
// Serial monitor at 115200 baud. Onboard LED (GPIO 2) blinks continuously;
// a WiFi scan runs every 15 s and prints visible network count + strongest SSIDs.

#include <WiFi.h>

const int LED_PIN = 2;
unsigned long lastScan = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("=== PowerUnit arrival test ===");
  Serial.printf("Chip: %s rev %d, %d cores, %d MHz\n",
                ESP.getChipModel(), ESP.getChipRevision(),
                ESP.getChipCores(), ESP.getCpuFreqMHz());
  Serial.printf("Flash: %u KB\n", ESP.getFlashChipSize() / 1024);

  // Factory MAC from eFuse — unique per board, readable without the WiFi driver.
  uint64_t mac = ESP.getEfuseMac();
  Serial.printf("MAC:   %02X:%02X:%02X:%02X:%02X:%02X\n",
                (uint8_t)(mac), (uint8_t)(mac >> 8), (uint8_t)(mac >> 16),
                (uint8_t)(mac >> 24), (uint8_t)(mac >> 32), (uint8_t)(mac >> 40));

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
}

void loop() {
  // Heartbeat blink: 200 ms on / 200 ms off
  digitalWrite(LED_PIN, (millis() / 200) % 2);

  if (millis() - lastScan > 15000 || lastScan == 0) {
    lastScan = millis();
    Serial.println("Scanning WiFi...");
    int n = WiFi.scanNetworks();
    Serial.printf("Networks found: %d\n", n);
    for (int i = 0; i < n && i < 5; i++) {
      Serial.printf("  %2d dBm  %s\n", WiFi.RSSI(i), WiFi.SSID(i).c_str());
    }
    WiFi.scanDelete();
  }
}

// Arrival-day board verification. Target-aware: works on the original AITRIP
// ESP-WROOM-32 DevKits and on the Hosyond ESP32-S3 N16R8 boards.
//
// Proves per board: USB/serial enumeration, flashability, GPIO (onboard LED),
// WiFi radio, and — on the S3 — that the flash and PSRAM are actually the
// parts the listing claimed. Serial monitor at 115200 baud.
//
// WHY THE S3 NEEDS MORE THAN A BLINK: these boards were chosen specifically
// because the listing claimed a genuine Espressif ESP32-S3-WROOM-1 N16R8
// module rather than a clone. 16 MB flash and 8 MB octal PSRAM is exactly the
// claim a relabelled module fails, and it fails silently — everything still
// blinks. So the test asserts the memory sizes and prints PASS/FAIL.
//
// Build (S3, UART Type-C port — NOT the native-USB one, which costs GPIO 19/20):
//   arduino-cli compile --fqbn esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M firmware/arrival-test
// Build (original WROOM-32):
//   arduino-cli compile --fqbn esp32:esp32:esp32 firmware/arrival-test
//
// IF PSRAM REPORTS 0 ON AN S3: suspect the build flags before suspecting the
// board. Octal PSRAM needs PSRAM=opi in the FQBN; without it a perfectly good
// N16R8 reports no PSRAM. Re-flash with the flag before starting a return.

#include <WiFi.h>

#if CONFIG_IDF_TARGET_ESP32S3
const char* kBoardName = "ESP32-S3 (expect N16R8: 16 MB flash, 8 MB PSRAM)";
const uint32_t kExpectFlashKB = 16 * 1024;
const uint32_t kExpectPsramMB = 8;
#else
const char* kBoardName = "ESP32-WROOM-32";
const uint32_t kExpectFlashKB = 0;   // 0 = don't assert
const uint32_t kExpectPsramMB = 0;
#endif

// The S3 dev boards carry an addressable RGB LED, not a plain one on GPIO 2.
// A digitalWrite heartbeat there would appear to "fail" on a good board.
#ifndef RGB_BUILTIN
const int LED_PIN = 2;
#endif

unsigned long lastScan = 0;
bool ledState = false;

static void heartbeat(bool on) {
#ifdef RGB_BUILTIN
  rgbLedWrite(RGB_BUILTIN, 0, on ? 12 : 0, on ? 6 : 0);   // dim; full brightness is blinding
#else
  digitalWrite(LED_PIN, on);
#endif
}

void setup() {
#ifndef RGB_BUILTIN
  pinMode(LED_PIN, OUTPUT);
#endif
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("=== PowerUnit arrival test ===");
  Serial.printf("Expecting: %s\n", kBoardName);
  Serial.printf("Chip: %s rev %d, %d cores, %d MHz\n",
                ESP.getChipModel(), ESP.getChipRevision(),
                ESP.getChipCores(), ESP.getCpuFreqMHz());

  uint32_t flashKB = ESP.getFlashChipSize() / 1024;
  Serial.printf("Flash: %u KB\n", flashKB);

  size_t psram = ESP.getPsramSize();
  Serial.printf("PSRAM: %u bytes (%.1f MB)%s\n", (unsigned)psram, psram / 1048576.0,
                psram ? "" : "  <-- if this is an S3, check PSRAM=opi in the FQBN first");

  // Factory MAC from eFuse — unique per board, readable without the WiFi driver.
  // RECORD THIS: it is how the three boards are told apart for the rest of the
  // project (vehicle / bench / spare).
  uint64_t mac = ESP.getEfuseMac();
  Serial.printf("MAC:   %02X:%02X:%02X:%02X:%02X:%02X\n",
                (uint8_t)(mac), (uint8_t)(mac >> 8), (uint8_t)(mac >> 16),
                (uint8_t)(mac >> 24), (uint8_t)(mac >> 32), (uint8_t)(mac >> 40));

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // --- verdict -------------------------------------------------------------
  bool ok = true;
  if (mac == 0) { Serial.println("FAIL: eFuse MAC reads zero"); ok = false; }
  if (kExpectFlashKB && flashKB < kExpectFlashKB) {
    Serial.printf("FAIL: flash is %u KB, expected %u KB\n", flashKB, kExpectFlashKB);
    ok = false;
  }
  if (kExpectPsramMB && psram < (size_t)kExpectPsramMB * 1048576UL) {
    Serial.printf("FAIL: PSRAM is %.1f MB, expected %u MB\n",
                  psram / 1048576.0, kExpectPsramMB);
    ok = false;
  }
  Serial.printf("MEMORY CHECK: %s\n", ok ? "PASS" : "FAIL");
  Serial.println("LED heartbeat + WiFi scan running; watch for a nonzero network count.");
}

void loop() {
  // Heartbeat: 200 ms on / 200 ms off. Only write on change — the RGB path is
  // a bit-banged protocol, not a pin write, so hammering it every loop is waste.
  bool want = (millis() / 200) % 2;
  if (want != ledState) { ledState = want; heartbeat(ledState); }

  if (millis() - lastScan > 15000 || lastScan == 0) {
    lastScan = millis();
    Serial.println("Scanning WiFi...");
    int n = WiFi.scanNetworks();
    Serial.printf("Networks found: %d  -> RADIO %s\n", n, n >= 0 ? "PASS" : "FAIL");
    for (int i = 0; i < n && i < 5; i++) {
      Serial.printf("  %2d dBm  %s\n", WiFi.RSSI(i), WiFi.SSID(i).c_str());
    }
    WiFi.scanDelete();
  }
}

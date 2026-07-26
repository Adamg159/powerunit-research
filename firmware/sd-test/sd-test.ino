// MicroSD SPI module arrival test — verify each of the 3 modules + the Lexar card.
// Uses the SD library bundled with the ESP32 core; no extra installs.
//
// Wiring (ESP32 DevKit):
//   VCC -> VIN (5 V from USB)   <-- the 5 V rail, NEVER 3V3: the module has its own
//                                   AMS1117 regulator + level buffer that need 5 V in
//   GND -> GND
//   SCK  -> GPIO 18
//   MISO -> GPIO 19
//   MOSI -> GPIO 23
//   CS   -> GPIO 5
//
// PASS looks like: card mounts, correct size reported (~29.1 GiB usable on the Lexar 32),
// file writes, reads back with matching content, and survives an append.
// If mounting fails on jumper wires, the sketch automatically retries at lower SPI clocks
// (the build-companion rule: drop the clock first before blaming hardware).

#include <SD.h>
#include <SPI.h>

const int CS_PIN = 5;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== microSD module arrival test ===");
  SPI.begin(18, 19, 23, CS_PIN);

  const uint32_t clocks[] = {25000000, 10000000, 4000000, 1000000, 400000};
  bool mounted = false;
  for (uint32_t clk : clocks) {
    Serial.printf("Trying %lu Hz... ", clk);
    if (SD.begin(CS_PIN, SPI, clk)) { Serial.println("mounted"); mounted = true; break; }
    Serial.println("no");
  }
  if (!mounted) { Serial.println("FAIL: card never mounted - check wiring, 5 V VCC, card seated"); return; }

  Serial.printf("Card type: %d  size: %.2f GB\n", SD.cardType(), SD.cardSize() / 1073741824.0);

  // Write, read back, verify
  const char* path = "/arrival-test.txt";
  SD.remove(path);
  File f = SD.open(path, FILE_WRITE);
  if (!f) { Serial.println("FAIL: could not open file for write"); return; }
  f.println("PowerUnit SD arrival test line 1");
  f.close();

  f = SD.open(path, FILE_APPEND);
  f.println("line 2 (append survived)");
  f.close();

  f = SD.open(path, FILE_READ);
  Serial.println("Read-back:");
  while (f.available()) Serial.write(f.read());
  f.close();
  Serial.println("=== PASS if both lines printed above ===");
}

void loop() {}

// MAX31855 arrival test — verify the thermocouple amplifier board after soldering headers.
// No libraries needed: raw SPI read of the 32-bit frame.
//
// Wiring (ESP32 DevKit):
//   VIN -> 3V3    GND -> GND
//   SCK -> GPIO 18
//   DO  -> GPIO 19  (MISO; the MAX31855 is read-only, no MOSI)
//   CS  -> GPIO 5
//
// IMPORTANT — what PASS looks like *without a thermocouple attached* (ours are still
// deferred until the engine arrives to size the washer rings):
//   - Internal (cold-junction) temp reads sane room temperature
//   - FAULT + OC (open circuit) bits are SET — that is the fault detection working,
//     exactly what we bought this chip for. An open input SHOULD scream open-circuit.
// If you short the T+ and T- input terminals together, OC clears and the thermocouple
// reading shows ~room temp (a shorted input acts like a junction at the connector).

#include <SPI.h>

const int CS_PIN = 5;

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin(18, 19, 23, CS_PIN);  // SCK, MISO, MOSI(unused), SS
  Serial.println("\n=== MAX31855 arrival test ===");
}

void loop() {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_PIN, LOW);
  delayMicroseconds(1);
  uint32_t v = 0;
  for (int i = 0; i < 4; i++) v = (v << 8) | SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  if (v == 0x00000000 || v == 0xFFFFFFFF) {
    Serial.println("No response on SPI (all 0s or all 1s) - check wiring/power");
  } else {
    // Frame: D31-18 TC temp (signed 14-bit, 0.25 C/LSB) | D16 fault
    //        D15-4 internal temp (signed 12-bit, 0.0625 C/LSB) | D2 SCV | D1 SCG | D0 OC
    int16_t tc = (int16_t)(v >> 16) >> 2;        // arithmetic shift keeps the sign
    int16_t in = ((int16_t)(v & 0xFFFF)) >> 4;
    bool fault = v & 0x10000, scv = v & 4, scg = v & 2, oc = v & 1;

    Serial.printf("internal %.2f C  |  thermocouple %.2f C  |  fault=%d SCV=%d SCG=%d OC=%d",
                  in * 0.0625, tc * 0.25, fault, scv, scg, oc);
    if (fault && oc && !scv && !scg) Serial.print("   <- expected with no probe attached: PASS");
    if (!fault)                      Serial.print("   <- probe/short detected, reading valid");
    Serial.println();
  }
  delay(1000);
}

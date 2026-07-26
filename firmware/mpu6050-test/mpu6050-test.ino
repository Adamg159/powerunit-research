// MPU-6050 (GY-521) arrival test — verify each of the 3 boards after soldering headers.
// No libraries needed: raw I2C via Wire.
//
// Wiring (ESP32 DevKit):
//   VCC -> 3V3   (the GY-521 has an onboard regulator, but 3.3 V direct is fine and safest)
//   GND -> GND
//   SDA -> GPIO 21
//   SCL -> GPIO 22
//   (AD0 unconnected/low -> I2C address 0x68)
//
// PASS looks like: WHO_AM_I = 0x68, temp near room, accel magnitude ~1.00 g at rest.
// Tilt the board and watch the axes swap; flick it and watch gyro spikes.

#include <Wire.h>

const uint8_t MPU = 0x68;

void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU); Wire.write(reg); Wire.write(val); Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Wire.begin(21, 22);

  Serial.println("\n=== MPU-6050 arrival test ===");

  // I2C bus scan — should find exactly one device at 0x68
  Serial.print("I2C scan:");
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) Serial.printf(" 0x%02X", a);
  }
  Serial.println();

  // WHO_AM_I check (reg 0x75) — genuine MPU-6050 answers 0x68
  Wire.beginTransmission(MPU); Wire.write(0x75); Wire.endTransmission(false);
  Wire.requestFrom(MPU, (uint8_t)1);
  uint8_t who = Wire.read();
  Serial.printf("WHO_AM_I: 0x%02X  (%s)\n", who, who == 0x68 ? "PASS" : "FAIL - unexpected ID");

  writeReg(0x6B, 0x00);  // wake from sleep, internal clock
  delay(100);
}

void loop() {
  // Burst-read accel (6), temp (2), gyro (6) starting at 0x3B
  Wire.beginTransmission(MPU); Wire.write(0x3B); Wire.endTransmission(false);
  Wire.requestFrom(MPU, (uint8_t)14);
  int16_t ax = (Wire.read() << 8) | Wire.read();
  int16_t ay = (Wire.read() << 8) | Wire.read();
  int16_t az = (Wire.read() << 8) | Wire.read();
  int16_t tr = (Wire.read() << 8) | Wire.read();
  int16_t gx = (Wire.read() << 8) | Wire.read();
  int16_t gy = (Wire.read() << 8) | Wire.read();
  int16_t gz = (Wire.read() << 8) | Wire.read();

  float axg = ax / 16384.0, ayg = ay / 16384.0, azg = az / 16384.0;  // +/-2 g range
  float mag = sqrt(axg * axg + ayg * ayg + azg * azg);               // ~1.00 at rest
  float tempC = tr / 340.0 + 36.53;

  Serial.printf("accel %+.2f %+.2f %+.2f g (|%.2f|)  gyro %+7.1f %+7.1f %+7.1f dps  %.1f C\n",
                axg, ayg, azg, mag, gx / 131.0, gy / 131.0, gz / 131.0, tempC);
  delay(500);
}

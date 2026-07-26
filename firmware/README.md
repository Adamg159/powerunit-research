# Firmware

## Arrival / verification tests

Each sketch is self-contained (no library installs beyond the ESP32 core) and prints a
clear PASS indication over serial at **115200 baud**. Wiring is documented at the top of
each file. Flash with:

```
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/<name>
arduino-cli upload -p COMx --fqbn esp32:esp32:esp32 firmware/<name>
```

In the Arduino IDE the equivalent board is **ESP32 Dev Module** (not "ESP32-WROOM-DA
Module" — that's the dual-antenna variant). The CP210x driver assigns a COM number per
USB port, so the port changes with which port the board is in — check Tools → Port rather
than assuming (COM4 and COM8 have both been seen).

| Sketch | Verifies | Needs soldering first? |
|---|---|---|
| [arrival-test](arrival-test/arrival-test.ino) | ESP32 board: flash, LED, WiFi, eFuse MAC | no |
| [mpu6050-test](mpu6050-test/mpu6050-test.ino) | MPU-6050 IMU: WHO_AM_I, accel ~1 g at rest, gyro, temp — **all 3 PASS 07-26 at 0x68** | yes — headers |
| [max31855-test](max31855-test/max31855-test.ino) | MAX31855: cold-junction temp + fault bits (OC expected with no probe) | yes — headers |
| [sd-test](sd-test/sd-test.ino) | SD module + card: mount, write, append, read-back (VCC = 5 V!) — **PASS 07-26, mounts at 10 MHz** | no (jumpers ok) |
| [hall-test](hall-test/hall-test.ino) | A3144 Hall: detect + pulse count; also finds each magnet's working face | no (breadboard) |

Board identities (eFuse MAC): #1 `58:2A:BD:7D:A7:D8` · #2 `58:2A:BD:7C:AA:E8` · #3 `58:2A:BD:7E:33:EC`

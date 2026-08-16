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

### ESP32-S3 (Hosyond N16R8) — the vehicle MCU

**Working FQBN** (established 2026-08-16):

```
esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB
```

- `PSRAM=opi` is mandatory. The octal PSRAM is invisible without it, and a good
  N16R8 then reports 0 bytes — **check this flag before concluding a board is bad.**
- `FlashSize=16M` alone is not enough: the default partition scheme is a 4 MB layout,
  which strands 12 MB. Set the partition scheme too.
- Toolchain: `arduino-cli` 1.5.1 + esp32 core 3.3.11.

**Telling the two USB-C ports apart without silkscreen** — check the USB VID:

| Port | Enumerates as | VID:PID |
|---|---|---|
| **UART** (use this one) | `USB-Enhanced-SERIAL CH343` | `1A86:55D3` (WCH) |
| Native USB | `USB JTAG/serial debug unit` | `303A:1001` (Espressif) |

Flash and debug via the **UART** port — native USB costs GPIO 19/20, which the pin
budget has already spent. Serial output over native USB additionally needs
`USBMode=hwcdc,CDCOnBoot=cdc`.

**Board identities (eFuse MAC), tested 2026-08-16:**

| Board | MAC | Result |
|---|---|---|
| S3 #1 | `AC:27:6E:AA:C3:88` | **PASS** — 16 MB flash, 8 MB PSRAM, radio, LED |
| S3 #2 | `AC:27:6E:AA:C1:C4` | **FAIL — defective, RMA.** Bootloader reads `0xffff` at the partition table offset on every build config, mode and speed tried; chip, eFuses and flash contents all verify good over the programmer. See BUILD-LOG 2026-08-16. |
| S3 #3 | `AC:27:6E:AA:C8:B8` | **PASS** — 16 MB flash, 8 MB PSRAM, radio, LED |

| Sketch | Verifies | Needs soldering first? |
|---|---|---|
| [arrival-test](arrival-test/arrival-test.ino) | ESP32 board: flash, LED, WiFi, eFuse MAC | no |
| [mpu6050-test](mpu6050-test/mpu6050-test.ino) | MPU-6050 IMU: WHO_AM_I, accel ~1 g at rest, gyro, temp — **all 3 PASS 07-26 at 0x68** | yes — headers |
| [max31855-test](max31855-test/max31855-test.ino) | MAX31855: cold-junction temp + fault bits (OC expected with no probe) — **PASS 07-26** | yes — headers |
| [sd-test](sd-test/sd-test.ino) | SD module + card: mount, write, append, read-back (VCC = 5 V!) — **PASS 07-26, mounts at 10 MHz** | no (jumpers ok) |
| [hall-test](hall-test/hall-test.ino) | A3144 Hall: detect + pulse count; also finds each magnet's working face | no (breadboard) |
| [slip-cut-test](slip-cut-test/slip-cut-test.ino) | Regen slip-cut logic: boot self-test (**no hardware needed**) then live two-wheel tach + verdict readout | no (breadboard) |

## Shared code

[`libraries/Driveline/Driveline.h`](libraries/Driveline/Driveline.h) — header-only, Arduino-free. Holds the
**single** ERPM → mechanical-RPM conversion (the VESC reports ERPM; pole pairs = 2), the Hall `PulseTach`,
and the regen `SlipMonitor`. Sketches that use it need the library path passed explicitly:

```bash
arduino-cli compile --fqbn esp32:esp32:esp32s3 --library firmware/libraries/Driveline firmware/slip-cut-test
```

Because it takes time as an argument rather than calling `millis()`, the whole module runs as a desktop test
with no board attached — **30 checks, all passing as of 2026-08-16**:

```bash
g++ -std=c++17 -Wall -Wextra -o test_driveline firmware/libraries/Driveline/test/test_driveline.cpp && ./test_driveline
```

Board identities (eFuse MAC): #1 `58:2A:BD:7D:A7:D8` · #2 `58:2A:BD:7C:AA:E8` · #3 `58:2A:BD:7E:33:EC`

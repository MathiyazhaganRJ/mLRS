# mLRS Porting Guide: ExpressLRS DIY 2400 TX (ESP32)

> **Associated Branch:** `RP`
> **Target Name:** `tx-elrs-diy-2400-esp32`
> **HAL File Destination:** `mLRS/Common/hal/esp/tx-hal-elrs-diy-2400-esp32.h`

---

## 1. Overview

This document serves as a reference for the **ExpressLRS DIY 2400 TX** hardware port added to the mLRS `RP` branch. This highly popular open-source hardware integrates an ESP32 microcontroller with an E28 (SX1280 2.4GHz) LoRa module.

This target uses the **PlatformIO** build system (`platformio.ini`), bypassing the traditional STM32 build scripts.

---

## 2. Hardware Mapping (`tx-hal-elrs-diy-2400-esp32.h`)

| Component | ESP32 Pin | Notes |
|---|---|---|
| **SX1280 CS** | GPIO 5 | |
| **SX1280 SCK** | GPIO 18 | Standard ESP32 VSPI |
| **SX1280 MOSI** | GPIO 23 | Standard ESP32 VSPI |
| **SX1280 MISO** | GPIO 19 | Standard ESP32 VSPI |
| **SX1280 RESET** | GPIO 14 | |
| **SX1280 BUSY** | GPIO 21 | |
| **SX1280 DIO1** | GPIO 4 | Triggers RF interrupts (`attachInterrupt`) |
| **TX_EN** | GPIO 26 | E28 PA (Power Amplifier) Enable |
| **RX_EN** | GPIO 27 | E28 LNA Enable |
| **LED (Red)** | GPIO 2 | Onboard LED |
| **JR Pin 5 (CRSF)** | GPIO 13 | Single-wire UART for Handset Telemetry |
| **BOOT Button** | GPIO 0 | Used for CLI/Serial switching at power-on |

---

## 3. Special Feature: Programmatic CLI Switch

ESP32-based TX modules are often securely enclosed in 3D-printed cases, making it physically difficult to hold the BOOT button (GPIO 0) during power-up to enter CLI mode. 

To solve this, a programmatic escape sequence was implemented for this target:

1. Connect the ESP32 to your computer via USB.
2. Open your serial monitor at **115200 baud** (you will see MAVLink data flowing).
3. Type `###` (three hash characters) and press Enter.
4. The module intercepts this sequence and automatically switches from MAVLink/Serial mode directly into the interactive CLI.

---

## 4. PlatformIO Build Configuration

The target is defined inside `platformio.ini` at the root of the repository:

```ini
[env:tx-elrs-diy-2400-esp32]
platform = espressif32
board = esp32dev
build_flags = 
	${env.build_flags}
	-D TX_ELRS_DIY_2400_ESP32
```

### Flashing Instructions

For the initial flash, it is recommended to use a merged binary at offset `0x0` to ensure the ESP32 bootloader and partition tables are correctly configured.

To build and flash via PlatformIO CLI:
```bash
pio run -e tx-elrs-diy-2400-esp32 -t upload
```

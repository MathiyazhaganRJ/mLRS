# mLRS Custom Board Porting Guide (RP & ESP32 Branch)
### Waveshare RP2040 433MHz & ExpressLRS DIY 2400 (ESP32)

> **Status:** Integrated in the `RP` branch
> **Targets:** `tx-waveshare-rp2040-433` / `tx-elrs-diy-2400-esp32`

---

## 1. Overview

This document serves as a reference for the new targets added to the `RP` branch of mLRS:
1. **Waveshare RP2040-LoRa 433MHz** (RP2040 + SX1262)
2. **ExpressLRS DIY 2400 TX** (ESP32 + E28/SX1280)

It explains the hardware mapping, specific constraints (like the RP2040 SPI0 issue and Crystal Oscillator), and how to port similar ESP32 or RP2040 hardware in the future.

---

## 2. Target 1: Waveshare RP2040 433MHz (TX)

This board integrates a Raspberry Pi RP2040 microcontroller with an SX1262 433MHz LoRa chip.

### Hardware Mapping (`tx-hal-waveshare-rp2040-433.h`)

| Component | RP2040 Pin | Notes |
|---|---|---|
| **SX1262 CS** | GP13 | |
| **SX1262 SCK** | GP14 | SPI1 must be used |
| **SX1262 MOSI** | GP15 | SPI1 must be used |
| **SX1262 MISO** | GP24 | SPI1 must be used |
| **SX1262 RESET** | GP23 | |
| **SX1262 BUSY** | GP18 | |
| **SX1262 DIO1** | GP16 | Triggers RF interrupts |
| **SX1262 ANT_SW** | GP17 | Controls RF switch (LOW = TX, HIGH = RX) |
| **LED (Red)** | GP25 | Onboard LED |
| **JR Pin 5 (CRSF)** | GP9 | Half-duplex PIO UART |

### Critical Hardware Rules for Waveshare RP2040:

1. **SPI0 is Unstable:** You **MUST** use `SPI1`. The specific pin routing on the Waveshare board causes crashes if SPI0 is used for the LoRa module.
2. **Crystal Oscillator:** This board does **not** have a TCXO (Temperature Compensated Crystal Oscillator). You must define `#define SX_USE_CRYSTALOSCILLATOR` to prevent the firmware from sending TCXO configuration commands to the SX1262, which would otherwise cause the RF chip to fail to initialize.
3. **No Physical Boot/Bind Button:** The board lacks a secondary button. The firmware relies on USB serial configuration and boots into Serial/MAVLink mode by default.

---

## 3. Target 2: ExpressLRS DIY 2400 TX (ESP32)

This target brings full mLRS support to the highly popular ExpressLRS DIY 2400 transmitter hardware (ESP32 + E28 SX1280 module).

### Hardware Mapping (`tx-hal-elrs-diy-2400-esp32.h`)

| Component | ESP32 Pin | Notes |
|---|---|---|
| **SX1280 CS** | GPIO 5 | |
| **SX1280 SCK** | GPIO 18 | Standard ESP32 VSPI |
| **SX1280 MOSI** | GPIO 23 | Standard ESP32 VSPI |
| **SX1280 MISO** | GPIO 19 | Standard ESP32 VSPI |
| **SX1280 RESET** | GPIO 14 | |
| **SX1280 BUSY** | GPIO 21 | |
| **SX1280 DIO1** | GPIO 4 | |
| **TX_EN** | GPIO 26 | E28 PA Enable |
| **RX_EN** | GPIO 27 | E28 LNA Enable |
| **LED (Red)** | GPIO 2 | |
| **JR Pin 5 (CRSF)** | GPIO 13 | Single-wire UART |
| **BOOT Button** | GPIO 0 | Used for CLI/Serial switching |

### Special Features for ESP32:

1. **Hardware CLI Switch:** Holding the ESP32 `BOOT` button (GPIO 0) during power-up will boot the device into CLI mode instead of the default MAVLink telemetry mode.
2. **Programmatic CLI Switch:** Because ESP32 modules are often enclosed in 3D printed cases without easy button access, a programmatic switch was added:
   - Open your serial monitor at **115200 baud**.
   - Type `###` (three hash characters) and press Enter.
   - The module intercepts this and automatically drops out of MAVLink mode and reboots/switches into the interactive CLI.

---

## 4. Step-by-Step Porting Guide for New RP2040/ESP32 Boards

If you want to add another custom ESP32 or RP2040 board, follow this workflow in the `RP` branch:

### Step 1: Create the HAL File
Copy an existing template to create your board's hardware abstraction layer file.
- **For ESP32:** Copy `tx-hal-elrs-diy-2400-esp32.h` to `tx-hal-myboard-esp32.h`
- **For RP2040:** Copy `tx-hal-waveshare-rp2040-433.h` to `tx-hal-myboard-rp2040.h`

Update the GPIO mappings (SPI, SX pins, UART, LEDs) inside this file to match your schematic.

### Step 2: Register in `device_conf.h`
Open `mLRS/Common/hal/esp/esp-device_conf.h` (or `rp-device_conf.h`) and add your feature flags:

```c
#ifdef TX_MYBOARD_ESP32
  #define DEVICE_NAME "My Custom ESP32 TX"
  #define DEVICE_IS_TRANSMITTER
  #define DEVICE_HAS_SX128x      // or SX126x
  #define FREQUENCY_BAND_2P4_GHZ // or FREQUENCY_BAND_433MHZ
#endif
```

### Step 3: Register in `hal.h`
Open `mLRS/Common/hal/esp/esp-hal.h` (or `rp-hal.h`) and link your HAL file:

```c
#ifdef TX_MYBOARD_ESP32
#include "tx-hal-myboard-esp32.h"
#endif
```

### Step 4: Add to `platformio.ini`
Unlike STM32 targets which use the `run_make_firmwares.py` script, ESP32 and RP2040 targets are built using PlatformIO.

Open `platformio.ini` at the root of the repository and add your environment block:

**For ESP32:**
```ini
[env:tx-myboard-esp32]
platform = espressif32
board = esp32dev
build_flags = 
	${env.build_flags}
	-D TX_MYBOARD_ESP32
```

**For RP2040:**
```ini
[env:tx-myboard-rp2040]
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = pico
build_flags = 
	${env.build_flags}
	-D TX_MYBOARD_RP2040
```

### Step 5: Build and Flash
Use the PlatformIO CLI or the VS Code PlatformIO extension to build and upload:
```bash
pio run -e tx-myboard-esp32 -t upload
```

---

*Generated: 2026-05-19 | mLRS RP Branch*

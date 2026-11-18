# mLRS Porting Guide: Waveshare RP2040-LoRa 433MHz

> **Associated Branch:** `RP`
> **Target Names:** `tx-waveshare-rp2040-433`, `rx-waveshare-rp2040-433`
> **HAL File Destinations:** 
> - `mLRS/Common/hal/rp/tx-hal-waveshare-rp2040-433.h`
> - `mLRS/Common/hal/rp/rx-hal-waveshare-rp2040-433.h`

---

## 1. Overview

This document details the hardware mapping and specific configuration needed to run mLRS on the **Waveshare RP2040-LoRa 433MHz** board, which integrates a Raspberry Pi RP2040 microcontroller with an SX1262 433MHz LoRa chip. 

This target bypasses the traditional `run_make_firmwares.py` script and uses **PlatformIO** for compilation.

---

## 2. Hardware Mapping (`tx-hal-waveshare-rp2040-433.h`)

| Component | RP2040 Pin | Notes |
|---|---|---|
| **SX1262 CS** | GP13 | |
| **SX1262 SCK** | GP14 | `SPI_USE_SPI1` must be used |
| **SX1262 MOSI** | GP15 | `SPI_USE_SPI1` must be used |
| **SX1262 MISO** | GP24 | `SPI_USE_SPI1` must be used |
| **SX1262 RESET** | GP23 | |
| **SX1262 BUSY** | GP18 | |
| **SX1262 DIO1** | GP16 | Triggers RF interrupts (`attachInterrupt`) |
| **SX1262 ANT_SW** | GP17 | Controls RF switch (LOW = TX, HIGH = RX) |
| **LED (Red)** | GP25 | Onboard LED |
| **JR Pin 5 (CRSF)** | GP9 | Half-duplex PIO UART (`UART_USE_PIO_HALF_DUPLEX`) |

---

## 3. Critical Hardware Constraints & Workarounds

When developing or modifying for this specific board, there are severe hardware-level quirks you must account for:

### 1. SPI0 is Unstable (Hardware Bug)
You **MUST** use `SPI1` for the SX1262 module. The specific trace routing on the Waveshare board causes unpredictable crashes if the default SPI0 peripheral is used.
```c
#define SPI_USE_SPI1
#define SPI_MISO IO_P24
#define SPI_MOSI IO_P15
#define SPI_SCK  IO_P14
```

### 2. Crystal Oscillator (No TCXO)
This board does **not** have a TCXO (Temperature Compensated Crystal Oscillator). You must explicitly inform the SX1262 not to expect one. If omitted, the RF chip will fail to initialize.
```c
#define SX_USE_CRYSTALOSCILLATOR
```

### 3. No Physical Boot/Bind Button
The board lacks a secondary button. The firmware relies strictly on USB serial configuration and boots into Serial/MAVLink mode by default.
```c
#define BUTTON IO_P22 // Stub
bool button_pressed(void) { return false; }
```

---

## 4. PlatformIO Build Configuration

Because RP2040 compilation requires the Pico SDK, the build is managed entirely via `platformio.ini`:

```ini
[env:tx-waveshare-rp2040-433]
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = pico
build_flags = 
	${env.build_flags}
	-D TX_WAVESHARE_RP2040_433
```

To build and flash:
```bash
pio run -e tx-waveshare-rp2040-433 -t upload
```

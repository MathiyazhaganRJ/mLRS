# mLRS Custom Board Porting Guide
### STM32F103 Blue Pill (DIY E28 Board01) — Complete Reference

> **Status:** Working ✅ — MAVLink telemetry confirmed, Bind confirmed, CLI confirmed
> **Firmware version:** v1.4.01
> **Target name:** `tx-diy-e28-board01-rj-f103cb`

---

## 1. Overview

This document describes every file changed to add a **custom STM32F103 Blue Pill + E28 (SX1280) 2.4GHz transmitter board** to the mLRS repository. It also provides a complete, repeatable, step-by-step guide for porting any future custom board using OlliW's reference patterns.

---

## 2. Architecture: How mLRS Identifies a Board

When you run:
```bash
python tools/run_make_firmwares.py -t tx-diy-e28-board01-rj-f103cb
```
The following chain fires:

```
run_make_firmwares.py
  └─ Defines: TX_DIY_E28_BOARD01_RJ_F103CB
        └─ device_conf.h: Sets DEVICE_NAME, DEVICE_IS_TRANSMITTER, DEVICE_HAS_SX128x, FREQUENCY_BAND_2P4_GHZ
              └─ hal.h: Includes → stm32/tx-hal-diy-e28-board01-rj-f103cb.h
                    └─ HAL file: Defines all GPIO pins, UARTs, SPI, LEDs, Button, Power PA, JR Pin5
```

Understanding this chain is the key to porting any board correctly.

---

## 3. Files Changed — Complete Reference

### File 1: `tools/run_make_firmwares.py`
**Role:** Registers the custom target in the build system.

**What was changed:**

```diff
# 1. Made GCC detection non-fatal (allows using system arm-none-eabi-gcc in PATH):
-    print('ERROR: gnu-tools not found!')
-    exit(1)
+    print('WARNING: gnu-tools from STM32CubeIDE not found! Assuming arm-none-eabi-gcc is in PATH.')
+    GCC_DIR = ''

# 2. Fixed a compile error — commented out conflicting source file:
-    os.path.join('Common','sx-drivers','lr20xx.cpp'),
+    #os.path.join('Common','sx-drivers','lr20xx.cpp'),

# 3. Registered new RX target in TLIST:
+    },{\
+        'target' : 'rx-diy-e28-board01-rj-f103cb', 'target_D' : 'RX_DIY_E28_BOARD01_RJ_F103CB',
+        'extra_D_list' : [], 'appendix' : ''

# 4. Registered new TX target in TLIST:
+    },{\
+        'target' : 'tx-diy-e28-board01-rj-f103cb', 'target_D' : 'TX_DIY_E28_BOARD01_RJ_F103CB',
+        'extra_D_list' : [], 'appendix' : ''
```

**OlliW's pattern to follow:** Find the block in `TLIST` near other F103 targets and add your entry in the same format. The naming convention is:
- `target` = lowercase with hyphens: `tx-diy-<rfchip>-<boardname>-<mcu>`
- `target_D` = uppercase with underscores: `TX_DIY_<RFCHIP>_<BOARDNAME>_<MCU>`

---

### File 2: `mLRS/Common/hal/glue.h`
**Role:** GCC version compatibility guard.

**What was changed:**
```diff
-#if __GNUC__ > 11
-  #error Must be gnu gcc 11 or lower!
+#if __GNUC__ > 13
+  #error Must be gnu gcc 13 or lower!
```

**Why:** The system toolchain (`arm-none-eabi-gcc 13.2`) is newer than the original guard allowed. This one-line change allows compilation without STM32CubeIDE.

> [!NOTE]
> Only change this once per repository clone. All future boards on the same repo will inherit this fix automatically.

---

### File 3: `mLRS/Common/hal/device_conf.h`
**Role:** Declares the high-level device capabilities (RF chip, frequency band, TX or RX role).

**What was added:**
```c
// For RX variant:
#ifdef RX_DIY_E28_BOARD01_RJ_F103CB
  #define DEVICE_NAME "DIY E28 F103CB"
  #define DEVICE_IS_RECEIVER
  #define DEVICE_HAS_SX128x      // E28 = SX1280 = SX128x family
  #define FREQUENCY_BAND_2P4_GHZ
#endif

// For TX variant:
#ifdef TX_DIY_E28_BOARD01_RJ_F103CB
  #define DEVICE_NAME "DIY E28 F103CB"
  #define DEVICE_IS_TRANSMITTER
  #define DEVICE_HAS_SX128x      // E28 = SX1280 = SX128x family
  #define FREQUENCY_BAND_2P4_GHZ
#endif
```

**OlliW's pattern to follow:**

| RF Module | Correct Define |
|---|---|
| E28 (SX1280, 2.4GHz) | `DEVICE_HAS_SX128x` + `FREQUENCY_BAND_2P4_GHZ` |
| E22 (SX1262, 915MHz FCC) | `DEVICE_HAS_SX126x` + `FREQUENCY_BAND_915MHZ_FCC` |
| E22 (SX1262, 868MHz EU) | `DEVICE_HAS_SX126x` + `FREQUENCY_BAND_868MHZ` |
| E22 (SX1262, 433MHz) | `DEVICE_HAS_SX126x` + `FREQUENCY_BAND_433MHZ` |

Always place your new `#ifdef` block **near other boards with the same RF chip** to keep the file organised.

---

### File 4: `mLRS/Common/hal/hal.h`
**Role:** Routes each board define to its specific HAL header file.

**What was added:**
```c
// For RX:
#ifdef RX_DIY_E28_BOARD01_RJ_F103CB
#include "stm32/rx-hal-diy-e28-board01-rj-f103cb.h"
#endif

// For TX:
#ifdef TX_DIY_E28_BOARD01_RJ_F103CB
#include "stm32/tx-hal-diy-e28-board01-rj-f103cb.h"
#endif
```

**OlliW's pattern to follow:** Always place your include block **next to other boards using the same MCU family** (F103 near F103, G431 near G431). This file is just a routing table — no logic, just includes.

---

### File 5: `mLRS/Common/hal/stm32/tx-hal-diy-e28-board01-rj-f103cb.h` *(New File)*
**Role:** The master pin definition file for your specific board. This is where ALL your hardware decisions are encoded.

**Key sections and their purpose:**

#### 5a. Feature Flags
```c
#define DEVICE_HAS_JRPIN5         // Board has JR Bay Pin 5 (CRSF/MBridge telemetry)
#define DEVICE_HAS_IN_ON_JRPIN5_TX // Input (RC channels) also comes through JR Pin 5
```

#### 5b. UARTs
```c
// UARTB: PA2/PA3 — Serial (MAVLink) and CLI share this port
#define UARTB_USE_UART2_PA2PA3
#define UARTB_BAUD        TX_SERIAL_BAUDRATE   // 115200

// UART (JR Pin5): PB6/PB7 — CRSF/MBridge
#define UART_USE_UART1_PB6PB7
#define UART_BAUD         400000               // CRSF baud rate

// XOR gate control pins for JR Pin5 signal inversion:
// (Required on F103 — hardware cannot do software inversion)
#define JRPIN5_TX_XOR             IO_PB3       // Controls U103 (TX direction)
#define JRPIN5_TX_SET_NORMAL      gpio_low(JRPIN5_TX_XOR)
#define JRPIN5_TX_SET_INVERTED    gpio_high(JRPIN5_TX_XOR)
#define JRPIN5_RX_XOR             IO_PA15      // Controls U105 (RX direction)
#define JRPIN5_RX_SET_NORMAL      gpio_low(JRPIN5_RX_XOR)
#define JRPIN5_RX_SET_INVERTED    gpio_high(JRPIN5_RX_XOR)

// UARTF: PB10 — Debug TX only (PB11 conflicts with SX_DIO1, so RX disabled)
#define UARTF_USE_UART3_PB10PB11
#define UARTF_BAUD        115200
```

#### 5c. SPI (connects to E28 LoRa module)
```c
#define SPI_USE_SPI1               // PA5=SCK, PA6=MISO, PA7=MOSI
#define SPI_CS_IO       IO_PA4     // Chip Select
#define SPI_USE_CLK_LOW_1EDGE      // SX1280: CPOL=0, CPHA=0
#define SPI_USE_CLOCKSPEED_9MHZ
```

#### 5d. SX1280 Control Pins
```c
#define SX_RESET        IO_PB12    // Active LOW reset
#define SX_DIO1         IO_PB11    // IRQ from SX1280 → triggers Rx/Tx done ISR
#define SX_BUSY         IO_PB13    // HIGH when SX1280 is busy
#define SX_TX_EN        IO_PC14    // Enable PA (power amplifier) for TX
#define SX_RX_EN        IO_PC15    // Enable LNA for RX
```

#### 5e. LEDs and Button
```c
#define LED_GREEN       IO_PB5     // Green LED (active LOW)
#define LED_RED         IO_PB4     // Red LED (active LOW)
#define BUTTON          IO_PC13    // Bind button + Blue Pill onboard LED (active LOW)
```

#### 5f. Serial/CLI Mode Switch
```c
// This is what controls whether PA2/PA3 runs as MAVLink or CLI at boot:
static bool _ser_or_com_is_serial = true; // Default: MAVLink mode

void ser_or_com_init(void) {
    button_init();
    _ser_or_com_is_serial = !button_pressed(); // Hold button at boot → CLI
}

bool ser_or_com_serial(void) {
    return _ser_or_com_is_serial;
}
```

#### 5g. Power Amplifier (PA)
```c
#define POWER_PA_E28_2G4M27SX
#include "../hal-power-pa.h"    // OlliW's standardised PA power tables for E28
```

**This single `#include` gives you automatic power level management (1mW to 500mW) for free!**

---

### File 6: `mLRS/CommonTx/jr_pin5_interface.h`
**Role:** Adds compile-time guard blocks for the future `JRPIN5_HALFDUPLEX_F103` mode.

**What was added (inactive, compile-guard only):**
```diff
+#if defined JRPIN5_HALFDUPLEX_F103
+    // STM32 HDSEL mode stubs — no register writes needed at runtime
+#endif
```

> [!NOTE]
> This file change is minimal and only adds empty `#if` guard blocks. It does not affect the working MAVLink / bind / CLI functionality.

---

## 5. Step-by-Step Porting Guide for Any Future Custom Board

### Prerequisites
- Fresh `git clone https://github.com/olliw42/mLRS` (or `git pull`)
- `arm-none-eabi-gcc 13.x` installed and in system PATH
- Python 3.x installed

---

### Step 1 — Choose Your Reference Board

Find the closest OlliW board to yours in `mLRS/Common/hal/stm32/`. Use this as your template:

| Your Hardware | Best Reference File to Copy |
|---|---|
| STM32F103 + E28 (SX1280) 2.4GHz | `tx-hal-diy-e28-board01-rj-f103cb.h` ← *This project* |
| STM32F103 + E22 (SX1262) 868/915MHz | `tx-hal-diy-e28dual-board02-f103cb.h` |
| STM32G431 + E28 (SX1280) 2.4GHz | `tx-hal-diy-e28-g431kb.h` |
| STM32G431 + E22 (SX1262) 868/915MHz | `tx-hal-diy-e22-g431kb.h` or `tx-hal-matek-mr900-30-g431kb.h` |
| STM32WLE5 (integrated sub-GHz) | `tx-hal-WioE5-Mini-wle5jc.h` |

---

### Step 2 — Create Your Board HAL File

```bash
# Copy the closest reference:
cp mLRS/Common/hal/stm32/tx-hal-diy-e28-board01-rj-f103cb.h \
   mLRS/Common/hal/stm32/tx-hal-myboard-f103cb.h
```

Open the new file and update **only** the pin definitions that differ on your PCB:

```c
// Change these to match YOUR schematic:
#define SPI_CS_IO       IO_PA4     // Your SPI CS pin
#define SX_RESET        IO_PB12    // Your RESET pin
#define SX_DIO1         IO_PB11    // Your DIO1/IRQ pin
#define SX_BUSY         IO_PB13    // Your BUSY pin
#define SX_TX_EN        IO_PC14    // Your TX_EN pin
#define SX_RX_EN        IO_PC15    // Your RX_EN pin
#define LED_GREEN       IO_PB5     // Your green LED pin
#define LED_RED         IO_PB4     // Your red LED pin
#define BUTTON          IO_PC13    // Your bind button pin
```

**Leave unchanged if using the same UART layout:**
- `UARTB_USE_UART2_PA2PA3` → CLI / MAVLink on PA2/PA3
- `UART_USE_UART1_PB6PB7` → JR Pin 5 on PB6/PB7

---

### Step 3 — Register in `device_conf.h`

Open `mLRS/Common/hal/device_conf.h`. Find the section for your RF chip and add:

```c
#ifdef TX_DIY_MYBOARD_F103CB
  #define DEVICE_NAME "My Custom Board"
  #define DEVICE_IS_TRANSMITTER
  #define DEVICE_HAS_SX128x       // ← Change to SX126x if using 868/915MHz
  #define FREQUENCY_BAND_2P4_GHZ  // ← Change to matching band
#endif
```

---

### Step 4 — Register in `hal.h`

Open `mLRS/Common/hal/hal.h`. Find the TX F103 include section and add:

```c
#ifdef TX_DIY_MYBOARD_F103CB
#include "stm32/tx-hal-myboard-f103cb.h"
#endif
```

---

### Step 5 — Register in `run_make_firmwares.py`

Open `tools/run_make_firmwares.py`. Find `TLIST` and add near other F103 TX entries:

```python
},{ 
    'target'      : 'tx-diy-myboard-f103cb',
    'target_D'    : 'TX_DIY_MYBOARD_F103CB',
    'extra_D_list': [],
    'appendix'    : ''
```

---

### Step 6 — Fix GCC Version Guard (once per repo clone)

Open `mLRS/Common/hal/glue.h` and change:

```c
// From:
#if __GNUC__ > 11

// To:
#if __GNUC__ > 13
```

---

### Step 7 — Build

```bash
python tools/run_make_firmwares.py -t tx-diy-myboard-f103cb
```

Output hex will be at:
```
tools/build/firmware/tx-diy-myboard-f103cb-v1.4.01-@<hash>.hex
```

---

### Step 8 — Flash

Use STM32CubeProgrammer or `stm32flash`:
```bash
# Via USB DFU (hold BOOT0 button, plug USB):
STM32_Programmer_CLI -c port=USB1 -w tx-diy-...hex -v -rst

# Via FTDI on UART1 (PA9/PA10):
stm32flash -w tx-diy-...hex -v -g 0x0 /dev/COM5
```

---

### Step 9 — Verify Boot

1. Power on board (**without** holding bind button for normal MAVLink mode)
2. Connect FTDI to `PA2` (TX→RX) / `PA3` (RX←TX) / GND
3. Open serial terminal at **115200 baud** → MAVLink data should stream
4. **Hold bind button at boot** → CLI mode → type `pl tx` to verify all parameters

---

### Step 10 — Bind

**TX side:** In CLI terminal, type:
```
bind
```

**RX side (BetaFPV SuperD / any mLRS RX):** Hold BOOT button for **4 seconds** while powered on until LED flashes rapidly, or connect via serial terminal and type:
```
bind
```

Both units exchange bind packets over the air and lock to each other automatically.

---

## 6. Serial Port Operating Modes

The `PA2/PA3` (UARTB) port on your board supports two modes, selected at boot time:

| Boot Condition | Mode | Use Case |
|---|---|---|
| **No button held** (default) | **Serial / MAVLink** | Mission Planner, QGroundControl over RF |
| **Hold bind button** at power-on | **CLI / COM** | Configuration, parameter tuning, binding |

In both modes, connect FTDI at **115200 baud** to the same physical `PA2/PA3` pins.

---

## 7. Summary of Confirmed Working Features

| Feature | Status | Notes |
|---|---|---|
| Board boot | ✅ Working | LEDs blink correctly on power-on |
| CLI terminal | ✅ Working | PA2/PA3 at 115200 baud — hold bind button at boot |
| RF binding | ✅ Working | OTA bind with BetaFPV SuperD RX v1.4.01 |
| MAVLink telemetry | ✅ Working | Full bidirectional, Mission Planner confirmed |

---

---

## 8. What We Changed From OlliW's Original HAL — and Why

OlliW's original HAL file (commit `e6c4289c`, path `hal/tx-hal-diy-e28-board01-rj-f103cb.h`) is a very old, minimal skeleton. The current repository version (in `hal/stm32/`) is a completely restructured, modern file. Below is a precise record of every difference between the two and the reason for each change.

---

### 8.1 File Structure: Old vs New Format

OlliW's original file (old) had no `#define UART_USE_*` blocks, no `DEVICE_HAS_JRPIN5`, and no proper UART buffering. The modern repository version is a full self-contained driver with all peripheral configuration inside the HAL file itself.

| Aspect | OlliW Original (e6c4289c) | Current Repo Version |
|---|---|---|
| Path | `hal/tx-hal-...h` | `hal/stm32/tx-hal-...h` |
| UART config style | Bare defines, no buffering | Full `UART_USE_*`, `TXBUFSIZE`, `RXBUFSIZE` |
| Power PA | Hardcoded `power_list[]` array | `#include "../hal-power-pa.h"` (OlliW's standard table) |
| Serial/CLI switch | Absent | `ser_or_com_init()` + `ser_or_com_serial()` |
| Device capabilities | Hardcoded `#define DEVICE_IS_TRANSMITTER` | Driven by `device_conf.h` externally |

---

### 8.2 Change 1: UARTB Serial Port — Complete Redefinition

**Original:**
```c
#define UARTC_USE_TX_ISR
//#define UARTC_USE_RX
//#define UARTC_RXBUFSIZE  512
```
(Used UARTC for CLI, no proper serial port defined)

**Our version:**
```c
#define UARTB_USE_UART2_PA2PA3     // Serial (MAVLink) + CLI both on PA2/PA3
#define UARTB_BAUD        TX_SERIAL_BAUDRATE
#define UARTB_USE_TX
#define UARTB_TXBUFSIZE   TX_SERIAL_TXBUFSIZE
#define UARTB_USE_TX_ISR
#define UARTB_USE_RX
#define UARTB_RXBUFSIZE   TX_SERIAL_RXBUFSIZE

#define UARTC_USE_UART2_PA2PA3     // COM port (CLI) on same UART as UARTB
#define UARTC_BAUD        TX_COM_BAUDRATE
#define UARTC_USE_TX
#define UARTC_TXBUFSIZE   TX_COM_TXBUFSIZE
#define UARTC_USE_TX_ISR
#define UARTC_USE_RX
#define UARTC_RXBUFSIZE   TX_COM_RXBUFSIZE
```

**Why:** The original used a generic `UARTC` which was incomplete. The modern mLRS architecture separates the physical UART into two logical roles: `UARTB` (serial/MAVLink data) and `UARTC` (COM/CLI). On this board they physically share the same UART2 pins (PA2/PA3), but the firmware selects which role to activate based on the boot-time `ser_or_com_serial()` result.

---

### 8.3 Change 2: UARTF Debug Port — Added

**Original:**
```c
// Not present
```

**Our version:**
```c
#define UARTF_USE_UART3_PB10PB11   // Debug TX on PB10 only
#define UARTF_BAUD        115200
#define UARTF_USE_TX
#define UARTF_TXBUFSIZE   512
#define UARTF_USE_TX_ISR
// NOTE: UARTF_USE_RX intentionally disabled — PB11 = SX_DIO1 (RF interrupt pin conflict!)
```

**Why:** PB11 is used as the SX1280's DIO1 interrupt pin (RF packet done IRQ). Using PB11 as UART3 RX simultaneously would cause the RF ISR and UART ISR to compete for the same pin. TX-only debug output on PB10 is safe.

---

### 8.4 Change 3: Power PA — Refactored to OlliW's Standard Table

**Original:**
```c
#define POWER_NUM   5
const uint16_t power_list[POWER_NUM][2] = {
    { POWER_0_DBM,  1   },
    { POWER_10_DBM, 10  },
    { POWER_20_DBM, 100 },
    { POWER_23_DBM, 200 },
    { POWER_27_DBM, 500 },
};
```

**Our version:**
```c
#define POWER_PA_E28_2G4M27SX
#include "../hal-power-pa.h"
```

**Why:** OlliW created `hal-power-pa.h` as a standardized power table for all known PA modules. By defining `POWER_PA_E28_2G4M27SX` and including this file, you automatically get:
- The correct power levels (1mW → 500mW, 7 steps)
- The correct SX1280 register values for each step
- Future updates from OlliW's repo automatically applied

**This is the correct modern pattern.** Never hardcode your own `power_list[]` unless you have a completely custom RF PA that OlliW hasn't defined yet.

---

### 8.5 Change 4: `ser_or_com_serial()` — Our Key Custom Addition

**Original (not present in OlliW's old file, but the current repo had):**
```c
void ser_or_com_init(void) {}

bool ser_or_com_serial(void) {
    return false;  // Always CLI mode
}
```

**Our version:**
```c
static bool _ser_or_com_is_serial = true; // Default: serial (MAVLink) mode

void ser_or_com_init(void) {
    button_init();
    // Hold bind button at boot → CLI mode
    // Release (default) at boot → Serial (MAVLink) mode
    _ser_or_com_is_serial = !button_pressed();
}

bool ser_or_com_serial(void) {
    return _ser_or_com_is_serial;
}
```

**Why this was the most critical change for making the board practically useful:**

OlliW's original code hardcoded `return false` which permanently locks the `PA2/PA3` UART into CLI (interactive terminal) mode. This means:
- MAVLink data from the receiver can NEVER flow to a GCS — it's swallowed by the CLI
- The board is only useful for configuration, not for actual flight operations

Our change uses the physical **bind button (`PC13`)** at boot time as a mode selector:
- **Normal operation (no button):** UART boots in serial/MAVLink mode → Mission Planner / QGroundControl connects directly
- **Configuration mode (hold button):** UART boots in CLI mode → serial terminal for parameter editing, bind commands, diagnostics

This pattern is identical to how OlliW implements it on professional boards like the MatekSys MR900/MR24 (`tx-hal-matek-mr900-30-g431kb.h`), which use a hardware switch or button for the same purpose.

---

### 8.6 Change 5: LEDs Initialization — Active-Low Correction

**Original:**
```c
void leds_init(void) {
    gpio_init(LED_GREEN, IO_MODE_OUTPUT_PP_LOW, IO_SPEED_DEFAULT);
    gpio_init(LED_RED, IO_MODE_OUTPUT_PP_LOW, IO_SPEED_DEFAULT);
    LED_GREEN_OFF;
    LED_RED_OFF;
}
```

**Our version:**
```c
void leds_init(void) {
    gpio_init(LED_GREEN, IO_MODE_OUTPUT_PP_HIGH, IO_SPEED_DEFAULT);
    gpio_init(LED_RED,   IO_MODE_OUTPUT_PP_HIGH, IO_SPEED_DEFAULT);
    gpio_high(LED_GREEN);  // OFF (active-low LED)
    gpio_high(LED_RED);    // OFF (active-low LED)
}
```

**Why:** Your board's LED circuit uses **active-low logic** — the LED turns ON when the GPIO pulls LOW, and turns OFF when the GPIO is HIGH. The original initialized LEDs with `OUTPUT_PP_LOW` (immediately drives LOW = LED ON at boot). The corrected version initializes with `OUTPUT_PP_HIGH` so both LEDs are OFF at startup, and the mLRS blink patterns start from a clean off state.

---

### 8.7 Summary Table: Original vs Our HAL

| Section | OlliW Original | Our Version | Impact |
|---|---|---|---|
| Serial UART (PA2/PA3) | Partial | ✅ Full UARTB + UARTC dual role | MAVLink + CLI works |
| Debug UART (PB10) | ❌ Not defined | ✅ TX-only on PB10 | Debug output works |
| Power PA table | Hardcoded array | ✅ Standard `hal-power-pa.h` | Correct power levels |
| Serial/CLI mode switch | Hardcoded CLI | ✅ Button-controlled at boot | Both modes accessible |
| LED init polarity | Active-HIGH (wrong) | ✅ Active-LOW (correct) | LEDs off at startup |
| SX1280 control pins | Minimal | ✅ Full RESET/BUSY/DIO1/TX_EN/RX_EN | RF module works |

---

*Generated: 2026-05-19 | mLRS v1.4.01 | Target: tx-diy-e28-board01-rj-f103cb*

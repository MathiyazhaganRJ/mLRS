# mLRS Firmware Build Guide
> Author: MathiyazhaganRJ | Branch: Karshak-e28-f103  
> Last Updated: May 2026

---

## Overview

mLRS supports three MCU families, each built with a **different toolchain**:

| MCU Family | Examples | Build Tool |
|---|---|---|
| **STM32** (F1, G4, WL) | F103CB, G431KB, WLE5CC | Custom Python script + ARM GCC |
| **ESP32 / ESP8266** | ESP32, ESP8266 | PlatformIO (Arduino framework) |
| **RP2040** | Waveshare RP2040-433 | PlatformIO (Arduino framework) |

---

## Part 1 — Prerequisites (Fresh Windows System)

### 1.1 Install Git
Download and install from: https://git-scm.com/download/win  
During install, select **"Git from the command line and also from 3rd-party software"**

### 1.2 Install Python 3
Download from: https://www.python.org/downloads/  
During install, **check "Add Python to PATH"**

Verify:
```powershell
python --version
# Should show Python 3.x.x
```

### 1.3 Clone the Repository
```powershell
git clone --recursive https://github.com/MathiyazhaganRJ/mLRS.git
cd mLRS
```

> **Important**: Use `--recursive` to also clone submodules (STM32 LL drivers, etc.)  
> If you forgot: `git submodule update --init --recursive`

### 1.4 Run Setup Script — MANDATORY FIRST STEP

> ⚠️ **This must be done once on every fresh clone before building. Skipping this will cause build errors.**

```powershell
python run_setup.py
```

This script does **4 things automatically**:

| Step | What it does |
|---|---|
| 1 | `git submodule update --init --recursive` — initializes all submodules |
| 2 | Copies ST LL driver files into the correct target folders |
| 3 | Generates MAVLink C library files from definitions |
| 4 | Generates DroneCAN C library files from definitions |

Without this step, the build will fail with errors like:
```
fatal error: stm32f1xx.h: No such file or directory
fatal error: mavlink.h: No such file or directory
```

**Selective setup (if you only need specific steps):**
```powershell
python run_setup.py --submodules   # only update git submodules
python run_setup.py --copy         # only copy ST drivers
python run_setup.py --mavlink      # only regenerate MAVLink library
python run_setup.py --dronecan     # only regenerate DroneCAN library
```

After `run_setup.py` completes successfully, you are ready to build firmware.

---

## Part 2 — STM32 Firmware Build

### 2.1 Toolchain Setup

The STM32 build uses **arm-none-eabi-gcc** (ARM bare-metal GCC cross-compiler).

**Option A — Install STM32CubeIDE (Recommended, Easiest)**

Download from: https://www.st.com/en/development-tools/stm32cubeide.html  
Install to default path: `C:\ST\STM32CubeIDE\`

The build script finds it automatically. No PATH setup needed.

> ⚠️ **Note**: The build script only restricts GCC version **< 12 when using CubeIDE-bundled GCC**.
> If using standalone `arm-none-eabi-gcc` from PATH, **any version works** (including GCC 13.x, confirmed working).

**Option B — Standalone ARM GCC (Alternative)**

Download from: https://developer.arm.com/downloads/-/gnu-rm  
Choose: `gcc-arm-none-eabi-10.3-2021.10-win32.exe`

After install, add to Windows PATH:
```
C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin
```

Verify:
```powershell
arm-none-eabi-gcc --version
# Should show arm-none-eabi-gcc 10.3.1
```

**Option C — Environment Variables (Advanced)**
```powershell
$env:MLRS_ST_DIR = "C:\path\to\ST\stm32cubeide\plugins"
$env:MLRS_GNU_DIR = "com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.10.3-..."
```

---

### 2.2 Build Commands

Navigate to the mLRS repo root:
```powershell
cd C:\path\to\mLRS
```

**Build a specific target:**
```powershell
python tools/run_make_firmwares.py --target <target-name> -np
```

**Build multiple targets at once:**
```powershell
python tools/run_make_firmwares.py --target <target1> <target2> -np
```

**Build all targets:**
```powershell
python tools/run_make_firmwares.py -np
```

> `-np` = "no pause" — skips the press-Enter wait at the end

---

### 2.3 Board01-RJ Targets

| Target Name | Description |
|---|---|
| `tx-diy-e28-board01-rj-f103cb` | TX module — DIY E28 Board-01 (STM32F103CB) |
| `rx-diy-e28-board01-rj-f103cb` | RX module — same hardware, RX firmware |

**Example:**
```powershell
# Build TX firmware
python tools/run_make_firmwares.py --target tx-diy-e28-board01-rj-f103cb -np

# Build RX firmware
python tools/run_make_firmwares.py --target rx-diy-e28-board01-rj-f103cb -np

# Build both at once
python tools/run_make_firmwares.py --target tx-diy-e28-board01-rj-f103cb rx-diy-e28-board01-rj-f103cb -np
```

---

### 2.4 Build Output Location

```
tools/
  build/
    tx-diy-e28-board01-rj-f103cb/
      tx-diy-e28-board01-rj-f103cb-v1.4.01-Karshak-e28-f103-@<hash>.hex  ← FLASH THIS
      tx-diy-e28-board01-rj-f103cb-v1.4.01-Karshak-e28-f103-@<hash>.elf
    rx-diy-e28-board01-rj-f103cb/
      rx-diy-e28-board01-rj-f103cb-v1.4.01-Karshak-e28-f103-@<hash>.hex
```

---

### 2.5 Reading Build Output

Successful build shows:
```
compiling
linking
   text    data     bss     dec     hex   filename
  84280     736   17368  102384   18ff0   tx-....elf
copying .hex files
```

- `text` = code size in flash
- `dec` must be less than **131072** (128KB flash limit for F103CB)
- Seeing `linking` + `.elf` = **SUCCESS**

Failed build shows `error:` with file name and line number.

---

### 2.6 Flash STM32 Firmware

Use **STM32CubeProgrammer** (free from ST):
1. Connect ST-Link to board: SWDIO, SWDCLK, GND, 3.3V
2. Open STM32CubeProgrammer → Connect → Erase → Open hex → Program

Or command line:
```powershell
STM32_Programmer_CLI.exe -c port=SWD -w "path\to\firmware.hex" -v -rst
```

---

## Part 3 — ESP32 / ESP8266 Firmware Build

### 3.1 Toolchain Setup

**Install VS Code:** https://code.visualstudio.com/  
**Install PlatformIO extension:** VS Code → Extensions → Search "PlatformIO IDE" → Install

PlatformIO automatically downloads the correct compiler for each target.

### 3.2 Build via VS Code
1. Open the mLRS repo folder in VS Code
2. PlatformIO icon in left sidebar → Project Tasks
3. Select your target environment
4. Click **Build**

### 3.3 Build via Command Line
```powershell
pip install platformio
pio run -e <environment-name>
```

**Example ESP32 targets:**
```powershell
pio run -e tx-radiomaster-bandit-900
pio run -e tx-betafpv-micro-1w-2400
```

### 3.4 Flash ESP32
```powershell
pio run -e <environment-name> --target upload
```

---

## Part 4 — RP2040 Firmware Build

### 4.1 Same as ESP32 — uses PlatformIO

```powershell
pio run -e tx-waveshare-rp2040-433
```

### 4.2 Flash RP2040
1. Hold **BOOTSEL** button, connect USB → appears as USB drive
2. Copy `.uf2` file from `.pio/build/<env>/` to the USB drive
3. Board reboots automatically

---

## Part 5 — How the STM32 Build Script Works

```
run_make_firmwares.py
  │
  ├─ 1. Finds ARM GCC:
  │      Checks C:\ST\ for STM32CubeIDE (GCC version < 12 only)
  │      Falls back to arm-none-eabi-gcc in PATH
  │      Can override with MLRS_ST_DIR / MLRS_GNU_DIR env vars
  │
  ├─ 2. Finds target definition in script:
  │      'target': 'tx-diy-e28-board01-rj-f103cb'
  │      'target_D': 'TX_DIY_E28_BOARD01_RJ_F103CB'
  │
  ├─ 3. Calls arm-none-eabi-g++ with:
  │      -D TX_DIY_E28_BOARD01_RJ_F103CB  → selects HAL in hal.h
  │      -mcpu=cortex-m3                   → for F103 (Cortex-M3)
  │      -mthumb
  │      all .cpp source files
  │
  ├─ 4. Links → .elf file
  ├─ 5. arm-none-eabi-objcopy → .hex file
  └─ 6. Copies hex to tools/build/<target-name>/
```

---

## Part 6 — Adding a Custom Board Target

### Step 1 — Create HAL files
```
mLRS/Common/hal/stm32/tx-hal-<your-board>.h
mLRS/Common/hal/stm32/rx-hal-<your-board>.h
```
Use `tx-hal-diy-e28-board01-rj-f103cb.h` as reference.

### Step 2 — Register in hal.h
In `mLRS/Common/hal/hal.h`:
```c
#elif defined TX_DIY_YOUR_BOARD
#include "stm32/tx-hal-your-board.h"
```

### Step 3 — Add build target in run_make_firmwares.py
```python
{
  'target': 'tx-your-board',
  'target_D': 'TX_YOUR_BOARD',
  # ... other fields (copy from existing entry)
},
```

### Step 4 — Build
```powershell
python tools/run_make_firmwares.py --target tx-your-board -np
```

---

## Part 7 — Troubleshooting

### "gnu-tools from STM32CubeIDE not found"
Build still works if `arm-none-eabi-gcc` is in PATH. Install STM32CubeIDE or standalone GCC 10.x.

### "gnu-tools ver >= 12 found but skipped"
The script skips GCC 12+ **from CubeIDE only**. It then falls back to `arm-none-eabi-gcc` in PATH.
**Fix**: Install standalone `arm-none-eabi-gcc` (any version, including 13.x) and add to PATH.
GCC 13.x from PATH is **confirmed working** on this project.

### "No such file or directory" for Drivers/STM32...
Submodules not cloned:
```powershell
git submodule update --init --recursive
```

### Build size too large (dec > 131072)
Flash is full. Disable debug features in HAL or use a chip with more flash.

### PlatformIO: environment not found
Check `platformio.ini` for exact environment name. Names are case-sensitive.

---

## Quick Reference Card

```
FRESH SETUP CHECKLIST:
[ ] Install Git
[ ] Install Python 3 (add to PATH)
[ ] Clone: git clone --recursive https://github.com/MathiyazhaganRJ/mLRS.git
[ ] RUN SETUP: python run_setup.py  ← MUST DO BEFORE FIRST BUILD
[ ] For STM32: Install STM32CubeIDE (C:\ST\) OR arm-none-eabi-gcc 10.x in PATH
[ ] For ESP/RP: Install VS Code + PlatformIO extension

BUILD COMMANDS:
STM32 TX:  python tools/run_make_firmwares.py --target tx-diy-e28-board01-rj-f103cb -np
STM32 RX:  python tools/run_make_firmwares.py --target rx-diy-e28-board01-rj-f103cb -np
ESP32:     pio run -e <environment-name>
RP2040:    pio run -e <environment-name>

OUTPUT (STM32):
tools/build/<target-name>/<target-name>-v<ver>-<branch>-@<hash>.hex
```

# mLRS RP2040 & RP2350 Firmware Build & Flashing Guide
> Author: MathiyazhaganRJ | Architecture: Raspberry Pi RP2040 / RP2350 (Pico SDK + Earle Philhower Arduino Core)  
> Supported Firmware Versions: mLRS v1.4.x (v1.4.05+) | Branch: `RP-v1.4` / `RP`  
> Last Updated: May 2026

---

## 1. Overview & Architecture

While STM32 mLRS targets rely on a custom Python build script with bare-metal GNU ARM toolchains, **RP2040 and RP2350** targets are built using **PlatformIO** leveraging the high-performance **Arduino-Pico core by Earle Philhower**.

### Key Architectural Features of mLRS on RP Microcontrollers:
1. **Dual-Core Execution**:
   - Core 0 runs the primary mLRS protocol engine, FHSS hopping loop, and RF transceiver management.
   - Core 1 handles intensive peripheral buffering, hardware USB CDC ACM emulation, and auxiliary tasks.
2. **Programmable I/O (PIO) Acceleration**:
   - **can2040 PIO Driver**: Implements a bit-exact CAN 2.0B controller entirely in software state machines, eliminating the need for an external CAN controller IC (e.g. MCP2515)—only a 3.3V CAN transceiver PHY (TJA1051 or SN65HVD230) is required.
   - **Half-Duplex CRSF/JR Bay**: Uses PIO state machines for single-wire bi-directional UART communication on JR Pin 5 with microsecond precision.
   - **WS2812 RGB LED**: Driven directly via PIO without blocking interrupts or CPU cycles.
3. **Zero-Programmer Drag-and-Drop Deployment (`.uf2`)**:
   - Microcontrollers boot with built-in ROM USB bootloaders. Firmware upload does not require ST-Link, J-Link, or UART bootloader dongles.

---

## 2. Supported RP Target Matrix

| Target Name (`platformio.ini` env) | MCU | RF Transceiver | Frequency | Primary Role / Interface |
|---|---|---|---|---|
| **`rx-waveshare-rp2040-433-can`** | RP2040 | SX1262 | 433 MHz | **Receiver with DroneCAN** (via can2040 PIO) |
| **`rx-waveshare-rp2040-433`** | RP2040 | SX1262 | 433 MHz | **Receiver** (CRSF / Serial MAVLink output) |
| **`tx-waveshare-rp2040-433`** | RP2040 | SX1262 | 433 MHz | **Transmitter** (JR Pin 5 CRSF + USB CLI) |
| **`rx-diy-2400-rp2040`** | RP2040 | SX1280 | 2.4 GHz | **Receiver with DroneCAN** (DIY Hardware) |
| **`tx-diy-2400-rp2040`** | RP2040 | SX1280 | 2.4 GHz | **Transmitter** (DIY 2.4GHz Module) |
| **`rx-diy-2400-rp2350`** | RP2350 | SX1280 | 2.4 GHz | **Receiver with DroneCAN** (Pico 2 Hardware) |
| **`tx-diy-2400-rp2350`** | RP2350 | SX1280 | 2.4 GHz | **Transmitter** (Pico 2 Hardware) |
| **`rx-waveshare-rp2040`** | RP2040 | SX1280 | 2.4 GHz | **Receiver** (Waveshare 2.4G Module) |
| **`tx-waveshare-rp2040`** | RP2040 | SX1280 | 2.4 GHz | **Transmitter** (Waveshare 2.4G Module) |

---

## 3. Prerequisites & Environment Setup

### 3.1 Install Git and Python 3
1. **Git**: Install from [git-scm.com](https://git-scm.com/download/win). Select *"Git from command line and 3rd-party software"*.
2. **Python 3**: Install from [python.org](https://www.python.org/downloads/). Ensure **"Add Python to PATH"** is checked.

### 3.2 Clone Repository (with Recursive Submodules)
```powershell
git clone --recursive https://github.com/MathiyazhaganRJ/mLRS.git
cd mLRS
git checkout RP-v1.4
```
> [!IMPORTANT]
> Always verify submodules are initialized:
> ```powershell
> git submodule update --init --recursive
> ```

### 3.3 Mandatory Setup Script (`run_setup.py`)
Run the repository generator before compiling any targets. This compiles the DroneCAN types and generates MAVLink headers:
```powershell
python run_setup.py
```
*If DroneCAN headers are missing, builds will fail with `#include <dronecan/out/include/...> not found`.*

---

## 4. Toolchain Setup: PlatformIO

You can build either using **VS Code GUI** or the **Standalone PlatformIO CLI**.

### Option A: PlatformIO CLI (Recommended for Quick Builds)
Open PowerShell or Terminal and install PlatformIO via pip:
```powershell
pip install -U platformio
```
Verify the installation:
```powershell
pio --version
# Output: PlatformIO Core, version 6.x.x
```

### Option B: VS Code GUI
1. Install [Visual Studio Code](https://code.visualstudio.com/).
2. Open VS Code Extensions (`Ctrl + Shift + X`).
3. Search for **PlatformIO IDE** and click **Install**.
4. Open the `mLRS` repository folder in VS Code (`File -> Open Folder`).

---

## 5. Building RP Firmware

All builds are orchestrated by `platformio.ini` located at the root of the repository.

### 5.1 Build Commands (PowerShell / Command Prompt)

Navigate to the repository root directory:
```powershell
cd C:\path\to\mLRS
```

#### Build Waveshare RP2040 433MHz DroneCAN Receiver:
```powershell
pio run -e rx-waveshare-rp2040-433-can
```

#### Build Waveshare RP2040 433MHz Standard Receiver:
```powershell
pio run -e rx-waveshare-rp2040-433
```

#### Build Waveshare RP2040 433MHz Transmitter:
```powershell
pio run -e tx-waveshare-rp2040-433
```

#### Build DIY RP2040 2.4GHz Targets:
```powershell
# Receiver with DroneCAN
pio run -e rx-diy-2400-rp2040

# Transmitter
pio run -e tx-diy-2400-rp2040
```

#### Clean Build Artifacts:
```powershell
pio run -e rx-waveshare-rp2040-433-can -t clean
```

---

## 6. Output Files & Binary Locations

Upon successful compilation, PlatformIO deposits compiled binaries in the `.pio` directory:

```
mLRS/
└── .pio/
    └── build/
        └── rx-waveshare-rp2040-433-can/
            ├── firmware.uf2    <-- PRIMARY FILE FOR FLASHING (Drag-and-Drop)
            ├── firmware.elf    <-- For SWD Debugging / Picoprobe
            └── firmware.bin    <-- Raw flat binary
```

---

## 7. Flashing & Firmware Upload Methods

### Method 1: UF2 USB Drag-and-Drop (Easiest, No Programmer Needed)

Every RP2040/RP2350 has a built-in ROM USB bootloader.

1. **Disconnect** the RP2040 board from USB.
2. Hold down the **BOOTSEL** (or `BOOT`) button on the board.
3. While holding the button, plug the board into your computer via a USB-C / Micro-USB data cable.
4. Release the button after 1–2 seconds.
5. A new USB mass storage drive named **`RPI-RP2`** (or `RP2350` for Pico 2) will appear in Windows Explorer.
6. Open `.pio/build/<target-name>/` and locate **`firmware.uf2`**.
7. **Copy or drag `firmware.uf2` onto the `RPI-RP2` drive**.
8. The drive will immediately unmount and the board will automatically reboot running the new mLRS firmware!

---

### Method 2: Direct Upload via PlatformIO CLI

If the board is connected and detected as a virtual serial port or mass storage drive, PlatformIO can upload directly:

```powershell
pio run -e rx-waveshare-rp2040-433-can -t upload
```

*Note: If the upload fails to automatically reset the board into bootloader mode, use Method 1 (hold BOOTSEL and plug in).*

---

### Method 3: Hardware SWD Debugging (Picoprobe / Raspberry Pi Debug Probe)

For real-time core debugging or step-by-step execution:
1. Connect SWD Pins: `SWDIO`, `SWCLK`, and `GND` to your Debug Probe.
2. Flash using OpenOCD or GDB:
```powershell
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "program .pio/build/rx-waveshare-rp2040-433-can/firmware.elf verify reset exit"
```

---

## 8. Hardware Wiring: Waveshare RP2040-LoRa 433MHz

### 8.1 Internal Pin Mapping

The Waveshare RP2040-LoRa board connects the RP2040 MCU to the on-board Semtech SX1262:

| Function | GPIO Pin | Hardware Notes |
|---|---|---|
| **SX1262 SCK** | `GP14` | SPI1 Bus (`SPI_USE_SPI1`) |
| **SX1262 MOSI** | `GP15` | SPI1 Bus (`SPI_USE_SPI1`) |
| **SX1262 MISO** | `GP24` | SPI1 Bus (`SPI_USE_SPI1`) |
| **SX1262 CS / NSS**| `GP13` | Chip Select |
| **SX1262 RESET**| `GP23` | Active-Low Hardware Reset |
| **SX1262 BUSY** | `GP18` | SX1262 Busy line |
| **SX1262 DIO1** | `GP16` | Packet Done / TxDone interrupt line |
| **SX1262 RF Switch**| `GP17` | Ant Switch (`LOW` = TX, `HIGH` = RX) |
| **LED Status** | `GP25` | Active-High Status LED |

> [!WARNING]
> **SPI0 vs SPI1 Constraint:**  
> On the Waveshare board, the SX1262 is routed to **SPI1** pins (`GP14/GP15/GP24`). Do not attempt to reassign to SPI0, as this causes bus collisions on the board's internal routing.

---

### 8.2 DroneCAN Transceiver Connection (`rx-waveshare-rp2040-433-can`)

The `can2040` driver runs on the RP2040's Programmable I/O (PIO) blocks and requires an external 3.3V CAN Transceiver (such as **SN65HVD230**, **TJA1051T/3**, or **MCP2551**):

```
RP2040 Board Pin           3.3V CAN Transceiver            Flight Controller (ArduPilot/PX4)
┌────────────────┐        ┌────────────────────┐          ┌────────────────────────────────┐
│   GP4 (CAN RX) ├───────►│ RXD                │          │                                │
│   GP5 (CAN TX) ├───────►│ TXD                │          │                                │
│   3V3          ├───────►│ VCC (3.3V)         │          │                                │
│   GND          ├───────►│ GND           CANH ├─────────►│ CANH (Pin 2 on JST-GH)         │
│                │        │               CANL ├─────────►│ CANL (Pin 3 on JST-GH)         │
└────────────────┘        └────────────────────┘          │ GND  (Pin 4 on JST-GH)         │
                                                          └────────────────────────────────┘
```

> [!TIP]
> Ensure a **120 Ω termination resistor** is installed across CANH and CANL at each end of the CAN bus line.

---

### 8.3 Transmitter Wiring (`tx-waveshare-rp2040-433`)

| Function | RP2040 Pin | External Radio / Bay Connection |
|---|---|---|
| **JR Bay Pin 5 (CRSF / Invertible)** | `GP9` | Handset JR Bay Pin 5 (Half-Duplex PIO UART) |
| **Serial / Telemetry TX** | `GP0` (UART0 TX) | External Serial Port / GCS Telemetry |
| **Serial / Telemetry RX** | `GP1` (UART0 RX) | External Serial Port / GCS Telemetry |
| **USB CDC Port** | Native USB | Interactive CLI Parameter Terminal |

---

## 9. Operating Modes & Configuration

### 9.1 USB CLI Terminal
When plugged into a computer via standard USB:
1. Open any serial terminal (PuTTY, Tera Term, Mission Planner Terminal, or Arduino Serial Monitor).
2. Select the virtual COM port assigned to the RP board (Baud rate: **115200**).
3. Press **Enter** to display the mLRS interactive menu:
   - Configure Bind Code, Frequency band, Power levels, and Channel outputs.
   - Run live link telemetry diagnostics.

### 9.2 ArduPilot DroneCAN Verification
1. Set the following ArduPilot parameters on your flight controller:
   - `CAN_P1_DRIVER = 1` (First CAN port enabled)
   - `CAN_D1_PROTOCOL = 1` (DroneCAN)
2. Reboot the flight controller.
3. In Mission Planner, navigate to **Initial Setup -> Optional Hardware -> DroneCAN/UAVCAN**.
4. You will see the **mLRS Receiver** identified as an active DroneCAN node transmitting RC channel override frames and bidirectional MAVLink telemetry streams!

---

## 10. Troubleshooting & Common Pitfalls

| Issue | Root Cause | Solution |
|---|---|---|
| **`fatal error: canard.h: No such file`** | Submodule `stm32-dronecan-lib` missing or uninitialized. | Run `git submodule update --init --recursive` and `python run_setup.py`. |
| **`RPI-RP2` USB drive does not show up** | Board not booted in bootloader mode. | Unplug USB, hold physical **BOOTSEL** button down firmly, reconnect USB, and wait 2 seconds before releasing. |
| **SX1262 initialization timeout (`Init Failed`)** | Crystal oscillator flag missing. | Waveshare board uses a standard quartz crystal, not a TCXO. Ensure `#define SX_USE_CRYSTALOSCILLATOR` is enabled in the HAL. |
| **DroneCAN frames not received by ArduPilot** | CAN bus baud rate mismatch or missing termination resistor. | DroneCAN defaults to **1 Mbps (1,000,000 baud)**. Verify 120 Ω resistor exists between CANH and CANL. |
| **Compilation failure: `Pico SDK version mismatch`** | Outdated platform packages in PlatformIO cache. | Run `pio pkg update` in terminal to refresh the Earle Philhower core to the latest release. |

---

## 11. Quick Reference Cheat Sheet

```powershell
# 1. First-time setup on fresh clone:
git submodule update --init --recursive
python run_setup.py

# 2. Build Commands:
pio run -e rx-waveshare-rp2040-433-can   # DroneCAN 433MHz RX
pio run -e rx-waveshare-rp2040-433       # Standard Serial 433MHz RX
pio run -e tx-waveshare-rp2040-433       # 433MHz TX
pio run -e rx-diy-2400-rp2040           # 2.4GHz DroneCAN RX

# 3. Flashing:
# Hold BOOTSEL -> Plug in USB -> Copy .pio/build/<env>/firmware.uf2 to RPI-RP2 drive
```

# mLRS HAL Architecture: A Deep Dive

This document explains the inner workings of the mLRS **Hardware Abstraction Layer (HAL)**, what it actually does under the hood, and the historical reasons why the HAL structure has changed significantly from OlliW's original implementations to the modern repository format.

---

## 1. What is the HAL?

In firmware development, a **HAL (Hardware Abstraction Layer)** is the bridge between the physical hardware (microcontroller pins, RF chips, buttons, LEDs) and the high-level logic of the software (the protocol, packets, telemetry routing).

In mLRS, the core code (e.g., `mlrs-tx.cpp` or `sx126x_driver.cpp`) is entirely hardware-agnostic. 
- The core code **does not know** if it is running on an STM32F103, an ESP32, or an RP2040. 
- The core code **does not know** what pin is connected to the antenna switch.

Instead, when the core needs to transmit a packet, it simply calls a function:
```c
sx_amp_transmit();
```
It is the **HAL file's job** (e.g., `tx-hal-diy-e28-board01-f103cb.h`) to intercept that call and translate it into the exact electrical signals required by your specific PCB. For example, in your F103 board, the HAL defines this as:
```c
void sx_amp_transmit(void) {
    gpio_low(SX_RX_EN);   // Turn off the listening amplifier
    gpio_high(SX_TX_EN);  // Turn on the transmitting power amplifier
}
```

The HAL is essentially a **translation dictionary** for your specific circuit board.

---

## 2. Why Did OlliW Change the HAL Structure? (Old vs. Modern)

If you compare older mLRS HAL files (from ~2022) to the modern HAL files, you will notice a massive shift in complexity.

### The Old Way: "Minimal Skeletons"
Originally, HAL files were very small. They only defined the absolute minimum required to make the board compile:
- Pin numbers for SPI.
- Simple arrays for power levels.
- A single UART for serial data.

**The Problem:** As mLRS grew to support advanced features (like CRSF telemetry on JR Pin 5, dual-role USB ports, programmatic CLI switches, and wildly different architectures like ESP32 and RP2040), the "minimal skeleton" approach broke down. Core code was getting polluted with `#ifdef ESP32` or `#ifdef F103` statements, which violates the purpose of a HAL.

### The Modern Way: "Self-Contained Drivers"
OlliW restructured the HAL so that **every single hardware decision is explicitly made inside the HAL file**. The HAL is no longer just a list of pins; it is a full configuration manifest.

Here is why specific things changed:

#### A. UART Definitions (The `UARTB`, `UARTC` Split)
- **Old:** Just defined `#define UARTC_USE_TX_ISR` and hoped the core knew what to do.
- **New:** You explicitly define the logical role, the buffer sizes, and the physical port.
- **Why:** Modern flight controllers send *massive* amounts of MAVLink data. The old system had fixed, small buffers. The new system allows a developer to say, "This ESP32 board has lots of RAM, give UARTB a 2048-byte buffer to prevent MAVLink bottlenecking." It also allowed splitting physical UARTs into logical roles (e.g., MAVLink vs CLI sharing the same port via the `ser_or_com_serial()` switch).

#### B. Power PA Tables (`hal-power-pa.h`)
- **Old:** Every single HAL file had a hardcoded `power_list[]` array. 
- **New:** `#include "../hal-power-pa.h"`
- **Why:** In the old way, if OlliW discovered a better register calibration for the SX1280 to improve signal purity at 500mW, he had to manually update 30 different board files. By moving this out of the board HAL and into a shared driver, every board inherits the most perfectly calibrated RF power tables automatically.

#### C. JR Pin 5 Handling
- **Old:** Non-existent. Handsets communicated via a standard UART.
- **New:** Complex macros like `JRPIN5_TX_XOR` or `JRPIN5_FULL_INTERNAL_ON_TX`.
- **Why:** The JR module bay only provides a **single wire** (Pin 5) for telemetry, but the CRSF protocol requires bi-directional data (half-duplex) *and* an inverted signal. Different microcontrollers handle this differently:
  - G431 can invert signals natively.
  - F103 requires physical XOR gates (U103/U105).
  - RP2040 uses PIO state machines to emulate it.
The modern HAL puts the burden of solving this hardware puzzle entirely on the board file, keeping the core mLRS protocol clean.

---

## 3. The 4 Pillars of a Modern mLRS HAL

When you read or write a modern HAL file, it is always broken down into 4 distinct pillars:

### Pillar 1: Identity & Capabilities
```c
#define DEVICE_HAS_JRPIN5
#define DEVICE_HAS_SINGLE_LED
#define DEVICE_HAS_SERIAL_OR_COM
```
These tell the core system what features to activate. If you don't declare `DEVICE_HAS_JRPIN5`, the Lua Script configurator simply won't compile into the firmware, saving flash memory.

### Pillar 2: Peripheral Routing (UARTs & SPI)
```c
#define UARTB_USE_UART2_PA2PA3
#define SPI_USE_SPI1
```
This routes logical functions to physical silicon. It tells the STM32/ESP32 internal multiplexer which physical legs of the chip to connect to the internal serial engines.

### Pillar 3: RF Chip Management (SX1262 / SX1280)
```c
#define SX_RESET        IO_PB12
#define SX_DIO1         IO_PB11
```
The LoRa module communicates over SPI, but it needs side-band signals. `SX_DIO1` is arguably the most important pin on the board: it is the hardware interrupt line. When the SX1280 finishes receiving a packet over the air, it immediately pulls this pin HIGH. The HAL maps this pin to an interrupt (EXTI) so the microcontroller stops whatever it is doing to process the incoming radio packet instantly (achieving mLRS's incredibly low latency).

### Pillar 4: Human Interface (LEDs & Buttons)
```c
void ser_or_com_init(void) {
    button_init();
    tx_ser_or_com_serial = !button_pressed();
}
```
This bridges human physical input to software state. Because every custom board designer wires buttons differently (active-high vs active-low, internal pull-up vs pull-down), the HAL abstracts this away so the core firmware just asks `button_pressed()` and gets a simple true/false answer.

---

## Summary
The evolution of the mLRS HAL reflects the project's transition from a simple DIY experiment into a professional, multi-architecture (STM32, ESP32, RP2040) RF ecosystem. By strictly enforcing that **all hardware quirks must live in the HAL**, OlliW ensures that the complex RF scheduling logic at the heart of mLRS remains untouched and perfectly stable across dozens of different DIY and commercial boards.

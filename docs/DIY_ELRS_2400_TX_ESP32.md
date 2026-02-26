# ExpressLRS DIY 2400 TX (ESP32)

This target adds support for the popular ExpressLRS DIY 2400 TX hardware to mLRS.

## Hardware Configuration

The following pinout is used for the ESP32 and SX1280 E28 module:

| Component | ESP32 Pin |
|---|---|
| **SX1280 CS** | GPIO 5 |
| **SX1280 SCK** | GPIO 18 |
| **SX1280 MOSI** | GPIO 23 |
| **SX1280 MISO** | GPIO 19 |
| **SX1280 RESET** | GPIO 14 |
| **SX1280 BUSY** | GPIO 21 |
| **SX1280 DIO1** | GPIO 4 |
| **TX_EN** | GPIO 26 |
| **RX_EN** | GPIO 27 |
| **LED** | GPIO 2 |
| **JR Pin 5 (CRSF)** | GPIO 13 |

## Special Features

### Programmatic CLI Switch ("###")
In addition to holding the **BOOT** button (GPIO 0) at power-up, you can now enter CLI mode programmatically.
- Open your serial monitor at **115200 baud**.
- Type `###` (three hash characters) and press Enter.
- The module will automatically switch from MAVLink/Serial mode to CLI mode.

## Flashing

The target is configured in PlatformIO as `tx-elrs-diy-2400-esp32`.
For initial flashing, it is recommended to use a merged binary at offset `0x0` to ensure the bootloader and partitions are correctly set up.

//*******************************************************
// ExpressLRS DIY 2400MHz TX (ESP32 + SX1280 E28) HAL for mLRS
// Pinout matches the official ELRS DIY_2400_TX_ESP32_SX1280_E28 schematic
// See: https://github.com/ExpressLRS/ExpressLRS-Hardware
// GPL3
//*******************************************************

#define DEVICE_HAS_JRPIN5
#define DEVICE_HAS_SINGLE_LED
#define DEVICE_HAS_SERIAL_OR_COM  // Serial and CLI share USB
#define DEVICE_HAS_DEBUG
#undef DEVICE_HAS_NO_DEBUG
#define UARTF_USE_SERIAL
#define UARTF_BAUD 115200

//-- UARTs

// Serial (USB via CP2102/CH340 on GPIO1/3) -- used for telemetry passthrough and config
#define UARTB_USE_SERIAL
#define UARTB_BAUD              TX_SERIAL_BAUDRATE
#define UARTB_USE_TX_IO         IO_P1
#define UARTB_USE_RX_IO         IO_P3
#define UARTB_TXBUFSIZE         TX_SERIAL_TXBUFSIZE
#define UARTB_RXBUFSIZE         TX_SERIAL_RXBUFSIZE

// COM port for CLI (same USB pins)
#define UARTC_USE_SERIAL
#define UARTC_BAUD              115200
#define UARTC_USE_TX_IO         IO_P1
#define UARTC_USE_RX_IO         IO_P3
#define UARTC_TXBUFSIZE         0
#define UARTC_RXBUFSIZE         TX_COM_RXBUFSIZE

// JR bay pin 5 -- Standard ELRS DIY uses GPIO 13
#define UART_USE_SERIAL1
#define UART_BAUD               400000
#define UART_USE_TX_IO          IO_P13
#define UART_USE_RX_IO          IO_P13
#define UART_TXBUFSIZE          0
#define UART_RXBUFSIZE          0

//-- SX1280 SPI (standard ESP32 VSPI)
// ELRS DIY 2400 TX E28 schematic pinout:
//   CS   = GPIO 5
//   SCK  = GPIO 18
//   MOSI = GPIO 23
//   MISO = GPIO 19
//   RESET = GPIO 14
//   BUSY  = GPIO 21
//   DIO1  = GPIO 4
#define SPI_CS_IO               IO_P5
#define SPI_MISO                IO_P19
#define SPI_MOSI                IO_P23
#define SPI_SCK                 IO_P18
#define SPI_FREQUENCY           18000000L
#define SX_RESET                IO_P14
#define SX_BUSY                 IO_P21
#define SX_DIO1                 IO_P4

// PA/LNA enable pins for E28 module
// ELRS DIY 2400 TX E28: TX_EN=GPIO26, RX_EN=GPIO27
#define SX_TX_EN                IO_P26
#define SX_RX_EN                IO_P27

IRQHANDLER(void SX_DIO_EXTI_IRQHandler(void);)

void sx_init_gpio(void)
{
    gpio_init(SX_DIO1, IO_MODE_INPUT_PU);
    gpio_init(SX_BUSY, IO_MODE_INPUT_PU);
    gpio_init(SX_TX_EN, IO_MODE_OUTPUT_PP_LOW);
    gpio_init(SX_RX_EN, IO_MODE_OUTPUT_PP_LOW);
    gpio_init(SX_RESET, IO_MODE_OUTPUT_PP_HIGH);
}

IRAM_ATTR bool sx_busy_read(void) { return (gpio_read_activehigh(SX_BUSY)) ? true : false; }

IRAM_ATTR void sx_amp_transmit(void)
{
    gpio_low(SX_RX_EN);
    gpio_high(SX_TX_EN);
}

IRAM_ATTR void sx_amp_receive(void)
{
    gpio_low(SX_TX_EN);
    gpio_high(SX_RX_EN);
}

void sx_dio_enable_exti_isr(void) { attachInterrupt(SX_DIO1, SX_DIO_EXTI_IRQHandler, RISING); }
void sx_dio_init_exti_isroff(void) { detachInterrupt(SX_DIO1); }
void sx_dio_exti_isr_clearflag(void) {}

//-- Button (GPIO 0 = BOOT button)
#define BUTTON_IO               IO_P0
void button_init(void) { gpio_init(BUTTON_IO, IO_MODE_INPUT_PU); }
IRAM_ATTR bool button_pressed(void) { return gpio_read_activelow(BUTTON_IO); }

//-- LED
// ELRS DIY 2400 TX E28 uses GPIO 2 for LED
#define LED_RED                 IO_P2
void leds_init(void) { gpio_init(LED_RED, IO_MODE_OUTPUT_PP_HIGH); }
IRAM_ATTR void led_red_off(void) { gpio_low(LED_RED); }
IRAM_ATTR void led_red_on(void) { gpio_high(LED_RED); }
IRAM_ATTR void led_red_toggle(void) { gpio_toggle(LED_RED); }

//-- Serial or Com switch (BOOT button)
bool tx_ser_or_com_serial = true;
void ser_or_com_init(void)
{
    button_init();
    tx_ser_or_com_serial = !button_pressed();
}
IRAM_ATTR bool ser_or_com_serial(void) { return tx_ser_or_com_serial; }
IRAM_ATTR void ser_or_com_set_to_com(void) { tx_ser_or_com_serial = false; }

//-- POWER
#define POWER_PA_E28_2G4M27SX
#include "../hal-power-pa.h"

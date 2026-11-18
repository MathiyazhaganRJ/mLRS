//*******************************************************
// Waveshare RP2040-LoRa HAL for mLRS (RX)
// Based on jl repo + TCXO 3.3V fix + SPI1 for GP24
// GPL3
//*******************************************************

// Board features
#define DEVICE_HAS_SINGLE_LED
#define DEVICE_HAS_OUT
#define DEVICE_HAS_NO_DEBUG

//-- UARTS
// UARTB = serial port (MAVLink to FC) via UART0 GP0/GP1
#define UARTB_USE_SERIAL1
#define UARTB_BAUD                RX_SERIAL_BAUDRATE
#define UARTB_TXBUFSIZE           RX_SERIAL_TXBUFSIZE
#define UARTB_RXBUFSIZE           RX_SERIAL_RXBUFSIZE
#define UARTB_TX_PIN              IO_P0
#define UARTB_RX_PIN              IO_P1

// UART = SBUS/CRSF output via UART1 GP8
#define UART_USE_SERIAL2
#define UART_BAUD                 416666
#define UART_TXBUFSIZE            1024
#define UART_TX_PIN               IO_P8

//-- SX1: SX1262 & SPI1
// GP24 is only valid as MISO on SPI1 (not SPI0) on RP2040
// Full SPI1 pinset: MISO=GP24, MOSI=GP15, SCK=GP14, CS=GP13
#define SPI_USE_SPI1
#define SPI_MISO                  IO_P24
#define SPI_MOSI                  IO_P15
#define SPI_SCK                   IO_P14
#define SPI_CS_IO                 IO_P13
#define SPI_FREQUENCY             8000000L  // 8 MHz

// CRITICAL: Waveshare RP2040-LoRa uses a CRYSTAL oscillator (NO TCXO)
// Setting SX_USE_CRYSTALOSCILLATOR skips SetDio3AsTcxoControl — required for crystal boards
#define SX_USE_CRYSTALOSCILLATOR

#define SX_RESET                  IO_P23
#define SX_BUSY                   IO_P18
#define SX_DIO1                   IO_P16
#define SX_ANT_SW                 IO_P17

IRQHANDLER(void SX_DIO_EXTI_IRQHandler(void);)

void sx_init_gpio(void)
{
    gpio_init(SX_RESET, IO_MODE_OUTPUT_PP_HIGH);
    gpio_init(SX_DIO1, IO_MODE_INPUT_PD);
    gpio_init(SX_BUSY, IO_MODE_INPUT_PU);
    gpio_init(SX_ANT_SW, IO_MODE_OUTPUT_PP_HIGH); // high = RX mode
}

bool sx_busy_read(void) { return (gpio_read_activehigh(SX_BUSY)) ? true : false; }

void sx_amp_transmit(void) { gpio_low(SX_ANT_SW); }
void sx_amp_receive(void)  { gpio_high(SX_ANT_SW); }

void sx_dio_init_exti_isroff(void) { detachInterrupt(SX_DIO1); }
void sx_dio_enable_exti_isr(void)  { attachInterrupt(digitalPinToInterrupt(SX_DIO1), SX_DIO_EXTI_IRQHandler, RISING); }
void sx_dio_exti_isr_clearflag(void) {}

//-- Out port
void out_init_gpio(void) {}
void out_set_normal(void)   { gpio_set_outover(UART_TX_PIN, GPIO_OVERRIDE_NORMAL); }
void out_set_inverted(void) { gpio_set_outover(UART_TX_PIN, GPIO_OVERRIDE_INVERT); }

//-- Button (stub)
#define BUTTON                    IO_P22
void button_init(void) { gpio_init(BUTTON, IO_MODE_INPUT_PU); }
bool button_pressed(void) { return false; }

//-- LED
#define LED_RED                   IO_P25
void leds_init(void)       { gpio_init(LED_RED, IO_MODE_OUTPUT_PP_LOW); }
void led_red_off(void)     { gpio_low(LED_RED); }
void led_red_on(void)      { gpio_high(LED_RED); }
void led_red_toggle(void)  { gpio_toggle(LED_RED); }

//-- POWER
#define POWER_PA_NONE_SX126X
#include "../hal-power-pa.h"

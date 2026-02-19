//*******************************************************
// Waveshare RP2040-LoRa HAL for mLRS (TX)
// SPI1 + Crystal Oscillator + JR CRSF + Tx In Mode
// GPL3
//*******************************************************

// Board features
#define DEVICE_HAS_SINGLE_LED
#define DEVICE_HAS_JRPIN5            // board has JR bay Pin5 (enables Tx Ch Source in CLI)
#define DEVICE_HAS_SERIAL_OR_COM
#define DEVICE_HAS_NO_DEBUG

//-- UARTs (USB shared between Serial and COM)
#define UARTB_USE_SERIAL
#define UARTC_USE_SERIAL

//-- UART: JR bay Pin5 half-duplex on GP9 (CRSF from radio)
#define UART_USE_PIO_HALF_DUPLEX
#define UART_BAUD                    400000   // CRSF baud rate
#define UART_TX_PIN                  IO_P9    // GP9 = JR bay signal wire

//-- SX1: SX1262 & SPI1
// GP24 is only valid as MISO on SPI1 on RP2040 — SPI0 crashes the board
#define SPI_USE_SPI1
#define SPI_MISO                  IO_P24
#define SPI_MOSI                  IO_P15
#define SPI_SCK                   IO_P14
#define SPI_CS_IO                 IO_P13
#define SPI_FREQUENCY             8000000L   // 8 MHz

// Waveshare RP2040-LoRa uses a CRYSTAL oscillator (NO TCXO)
// Must skip SetDio3AsTcxoControl — confirmed by RadioLib community
#define SX_USE_CRYSTALOSCILLATOR

#define SX_RESET                  IO_P23
#define SX_BUSY                   IO_P18
#define SX_DIO1                   IO_P16
#define SX_ANT_SW                 IO_P17    // LOW=TX, HIGH=RX (per Waveshare wiki)

IRQHANDLER(void SX_DIO_EXTI_IRQHandler(void);)

void sx_init_gpio(void)
{
    gpio_init(SX_RESET, IO_MODE_OUTPUT_PP_HIGH);
    gpio_init(SX_DIO1, IO_MODE_INPUT_PD);
    gpio_init(SX_BUSY, IO_MODE_INPUT_PU);
    gpio_init(SX_ANT_SW, IO_MODE_OUTPUT_PP_HIGH); // start in RX mode
}

bool sx_busy_read(void) { return (gpio_read_activehigh(SX_BUSY)) ? true : false; }

void sx_amp_transmit(void) { gpio_low(SX_ANT_SW); }
void sx_amp_receive(void)  { gpio_high(SX_ANT_SW); }

void sx_dio_init_exti_isroff(void) { detachInterrupt(SX_DIO1); }
void sx_dio_enable_exti_isr(void)  { attachInterrupt(digitalPinToInterrupt(SX_DIO1), SX_DIO_EXTI_IRQHandler, RISING); }
void sx_dio_exti_isr_clearflag(void) {}

//-- Button (stub — no physical button on Waveshare board)
#define BUTTON                    IO_P22
void button_init(void) { gpio_init(BUTTON, IO_MODE_INPUT_PU); }
bool button_pressed(void) { return false; }

//-- LED
#define LED_RED                   IO_P25
void leds_init(void)      { gpio_init(LED_RED, IO_MODE_OUTPUT_PP_LOW); }
void led_red_off(void)    { gpio_low(LED_RED); }
void led_red_on(void)     { gpio_high(LED_RED); }
void led_red_toggle(void) { gpio_toggle(LED_RED); }

//-- Serial/COM switch (USB shared — default to COM/CLI mode)
bool tx_ser_or_com_serial = false;
void ser_or_com_init(void)       { tx_ser_or_com_serial = false; }
bool ser_or_com_serial(void)     { return tx_ser_or_com_serial; }
void ser_or_com_set_to_com(void) { tx_ser_or_com_serial = false; }

//-- POWER
#define POWER_PA_NONE_SX126X
#include "../hal-power-pa.h"

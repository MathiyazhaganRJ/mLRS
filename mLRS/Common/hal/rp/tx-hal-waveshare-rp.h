//*******************************************************
// Waveshare RP2040-LoRa HAL for mLRS (TX)
// Maps SX1262 pins to RP2040 GPIOs per Waveshare wiki
// GPL3
//*******************************************************

// Board features
#define DEVICE_HAS_SINGLE_LED
#define DEVICE_HAS_SERIAL_OR_COM

//-- UARTs (USB)
#define UARTB_USE_SERIAL
#define UARTC_USE_SERIAL

//-- SX1: SX1262 & SPI
// Waveshare mapping (Pico GP numbers)
#define SPI_MISO                  IO_P24
#define SPI_MOSI                  IO_P15
#define SPI_SCK                   IO_P14
#define SPI_CS_IO                 IO_P13
#define SPI_FREQUENCY             8000000L  // 8 MHz safe for SX1262
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
    gpio_init(SX_ANT_SW, IO_MODE_OUTPUT_PP_HIGH); // high = RX
}

bool sx_busy_read(void) { return (gpio_read_activehigh(SX_BUSY)) ? true : false; }

void sx_amp_transmit(void) {
    // ANT_SW low for transmit
    gpio_low(SX_ANT_SW);
}

void sx_amp_receive(void)
{
    // ANT_SW high for receive
    gpio_high(SX_ANT_SW);
}

void sx_dio_init_exti_isroff(void) { detachInterrupt(SX_DIO1); }
void sx_dio_enable_exti_isr(void) { attachInterrupt(digitalPinToInterrupt(SX_DIO1), SX_DIO_EXTI_IRQHandler, RISING); }
void sx_dio_exti_isr_clearflag(void) {}


//-- LEDs

#define LED_RED                   IO_P25

void leds_init(void)
{ 
    gpio_init(LED_RED, IO_MODE_OUTPUT_PP_LOW);
}
void led_red_off(void) { gpio_low(LED_RED); }
void led_red_on(void) { gpio_high(LED_RED); }
void led_red_toggle(void) { gpio_toggle(LED_RED); }

//-- Serial/Com switch: use serial (USB) by default
bool tx_ser_or_com_serial = true;
void ser_or_com_init(void) { tx_ser_or_com_serial = true; }
bool ser_or_com_serial(void) { return tx_ser_or_com_serial; }
void ser_or_com_set_to_com(void) { tx_ser_or_com_serial = false; }

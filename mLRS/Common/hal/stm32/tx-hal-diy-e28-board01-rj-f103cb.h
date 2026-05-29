//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// hal
//********************************************************

//-------------------------------------------------------
// TX DIY E28 BOARD01 v004 STM32F103CB
//-------------------------------------------------------

// #define DEVICE_HAS_IN
#define DEVICE_HAS_JRPIN5
#define DEVICE_HAS_SERIAL_OR_COM

//-- Timers, Timing, EEPROM, and such stuff

#define DELAY_USE_DWT

#define SYSTICK_TIMESTEP 1000
#define SYSTICK_DELAY_MS(x)                                                    \
  (uint16_t)(((uint32_t)(x) * (uint32_t)1000) / SYSTICK_TIMESTEP)

#define EE_START_PAGE 124 // 128 kB flash, 1 kB page

#define MICROS_TIMx TIM3
#define MICROS_TIM_NAMEPREFIX TIM3_

//-- UARTS
// UARTB = serial port
// UARTC = COM (CLI)
// UART = SPORT (pin5) on JR bay
// UARTE = in port, SBus or whatever
// UARTF = debug port

#define UARTB_USE_UART2_PA2PA3 // serial
#define UARTB_BAUD TX_SERIAL_BAUDRATE
#define UARTB_USE_TX
#define UARTB_TXBUFSIZE TX_SERIAL_TXBUFSIZE
#define UARTB_USE_TX_ISR
#define UARTB_USE_RX
#define UARTB_RXBUFSIZE TX_SERIAL_RXBUFSIZE

#ifndef DEBUG_ENABLED
#error This board needs redesign, COM (CLI) cannot be used since PB11 overlaps!
#endif
// #define UARTC_USE_UART3_PB10PB11 // COM (CLI)
// #define UARTC_BAUD                TX_COM_BAUDRATE
// #define UARTC_USE_TX
// #define UARTC_TXBUFSIZE           TX_COM_TXBUFSIZE
// #define UARTC_USE_TX_ISR
// #define UARTC_USE_RX
// #define UARTC_RXBUFSIZE           TX_COM_RXBUFSIZE

#ifdef DEVICE_HAS_IN
#define UARTE_USE_UART1_PB6PB7 // SBus
#define UARTE_BAUD 100000 // SBus normal baud rate, is being set later anyhow
// #define UARTE_USE_TX
// #define UARTE_TXBUFSIZE           512
// #define UARTE_USE_TX_ISR
#define UARTE_USE_RX
#define UARTE_RXBUFSIZE 512
#endif
#ifdef DEVICE_HAS_JRPIN5  
#define UART_USE_UART1_PB6PB7 // JR pin5, MBridge
#define UART_BAUD 400000
#define UART_USE_TX
#define UART_TXBUFSIZE 512
#define UART_USE_TX_ISR
#define UART_USE_RX
#define UART_RXBUFSIZE 512

#define JRPIN5_TX_XOR IO_PB3
#define JRPIN5_TX_SET_NORMAL gpio_low(JRPIN5_TX_XOR)
#define JRPIN5_TX_SET_INVERTED                                                 \
  gpio_low(JRPIN5_TX_XOR) // Boxer: no HW inverter, keep pass-through
#define JRPIN5_RX_XOR IO_PA15
#define JRPIN5_RX_SET_NORMAL gpio_low(JRPIN5_RX_XOR)
#define JRPIN5_RX_SET_INVERTED                                                 \
  gpio_low(JRPIN5_RX_XOR) // Boxer: no HW inverter, keep pass-through
#endif

#define UARTF_USE_UART3_PB10PB11 // debug
#define UARTF_BAUD 115200
#define UARTF_USE_TX
#define UARTF_TXBUFSIZE 512
#define UARTF_USE_TX_ISR
// #define UARTF_USE_RX
// #define UARTF_RXBUFSIZE           512

//-- SX1: SX12xx & SPI

#define SPI_USE_SPI1 // PA65, PA6, PA7
#define SPI_CS_IO IO_PA4
#define SPI_USE_CLK_LOW_1EDGE // datasheet says CPHA = 0  CPOL = 0
#define SPI_USE_CLOCKSPEED_9MHZ

#define SX_RESET IO_PB12
#define SX_DIO1 IO_PB11
#define SX_BUSY IO_PB1
#define SX_RX_EN IO_PB0
#define SX_TX_EN IO_PB13
// #define SX_ANT_SELECT
// #define SX_USE_DCDC

#define SX_DIO1_GPIO_AF_EXTI_PORTx LL_GPIO_AF_EXTI_PORTB
#define SX_DIO1_GPIO_AF_EXTI_LINEx LL_GPIO_AF_EXTI_LINE11
#define SX_DIO_EXTI_LINE_x LL_EXTI_LINE_11
#define SX_DIO_EXTI_IRQn EXTI15_10_IRQn
#define SX_DIO_EXTI_IRQHandler EXTI15_10_IRQHandler
// #define SX_DIO_EXTI_IRQ_PRIORITY    11

void sx_init_gpio(void) {
  gpio_init(SX_RESET, IO_MODE_OUTPUT_PP_HIGH, IO_SPEED_VERYFAST);
  gpio_init(SX_DIO1, IO_MODE_INPUT_PD, IO_SPEED_VERYFAST);
#ifdef SX_BUSY
  gpio_init(SX_BUSY, IO_MODE_INPUT_PU, IO_SPEED_VERYFAST);
#endif
  gpio_init(SX_TX_EN, IO_MODE_OUTPUT_PP_LOW, IO_SPEED_VERYFAST);
  gpio_init(SX_RX_EN, IO_MODE_OUTPUT_PP_LOW, IO_SPEED_VERYFAST);
}

bool sx_dio_read(void) {
  return (gpio_read_activehigh(SX_DIO1)) ? true : false;
}

#ifdef SX_BUSY
bool sx_busy_read(void) {
  return (gpio_read_activehigh(SX_BUSY)) ? true : false;
}
#endif

void sx_amp_transmit(void) {
  gpio_low(SX_RX_EN);
  gpio_high(SX_TX_EN);
}

void sx_amp_receive(void) {
  gpio_low(SX_TX_EN);
  gpio_high(SX_RX_EN);
}

void sx_dio_init_exti_isroff(void) {
  LL_GPIO_AF_SetEXTISource(SX_DIO1_GPIO_AF_EXTI_PORTx,
                           SX_DIO1_GPIO_AF_EXTI_LINEx);

  // let's not use LL_EXTI_Init(), but let's do it by hand, is easier to allow
  // enabling isr later
  LL_EXTI_DisableEvent_0_31(SX_DIO_EXTI_LINE_x);
  LL_EXTI_DisableIT_0_31(SX_DIO_EXTI_LINE_x);
  LL_EXTI_DisableFallingTrig_0_31(SX_DIO_EXTI_LINE_x);
  LL_EXTI_EnableRisingTrig_0_31(SX_DIO_EXTI_LINE_x);

  NVIC_SetPriority(SX_DIO_EXTI_IRQn, SX_DIO_EXTI_IRQ_PRIORITY);
  NVIC_EnableIRQ(SX_DIO_EXTI_IRQn);
}

void sx_dio_enable_exti_isr(void) {
  LL_EXTI_ClearFlag_0_31(SX_DIO_EXTI_LINE_x);
  LL_EXTI_EnableIT_0_31(SX_DIO_EXTI_LINE_x);
}

void sx_dio_exti_isr_clearflag(void) {
  LL_EXTI_ClearFlag_0_31(SX_DIO_EXTI_LINE_x);
}

//-- SBus input pin

#define IN IO_PB7 // UART1 RX
#define IN_XOR IO_PA15

void in_init_gpio(void) {
  gpio_init(IN_XOR, IO_MODE_OUTPUT_PP_LOW, IO_SPEED_VERYFAST);
  gpio_low(IN_XOR);
}

void in_set_normal(void) { gpio_low(IN_XOR); }

void in_set_inverted(void) { gpio_high(IN_XOR); }

//-- Button

#define BUTTON IO_PC13

void button_init(void) {
  gpio_init(BUTTON, IO_MODE_INPUT_PU, IO_SPEED_DEFAULT);
}

bool button_pressed(void) { return gpio_read_activelow(BUTTON); }

//-- LEDs

#define LED_GREEN IO_PB5
#define LED_RED IO_PB4

void leds_init(void) {
  gpio_init(LED_GREEN, IO_MODE_OUTPUT_PP_HIGH, IO_SPEED_DEFAULT);
  gpio_init(LED_RED, IO_MODE_OUTPUT_PP_HIGH, IO_SPEED_DEFAULT);
  gpio_high(LED_GREEN); // LED_GREEN_OFF
  gpio_high(LED_RED);   // LED_RED_OFF
}

void led_green_off(void) { gpio_high(LED_GREEN); }
void led_green_on(void) { gpio_low(LED_GREEN); }
void led_green_toggle(void) { gpio_toggle(LED_GREEN); }

void led_red_off(void) { gpio_high(LED_RED); }
void led_red_on(void) { gpio_low(LED_RED); }
void led_red_toggle(void) { gpio_toggle(LED_RED); }

//-- Position Switch

void pos_switch_init(void) {}

//-- Serial or Com Switch

static bool _ser_or_com_is_serial = true; // default: serial (MAVLink) mode

void ser_or_com_init(void) {
  button_init();
  // Hold bind button at boot → CLI mode
  // Release (or no button) at boot → Serial (MAVLink) mode
  _ser_or_com_is_serial = !button_pressed();
}

bool ser_or_com_serial(void) { return _ser_or_com_is_serial; }

//-- POWER

#define POWER_PA_E28_2G4M27SX
#include "../hal-power-pa.h"

//-- TEST

#define PORTA_N 9

uint32_t porta[PORTA_N] = {
    LL_GPIO_PIN_0, LL_GPIO_PIN_1, LL_GPIO_PIN_2, LL_GPIO_PIN_3,  LL_GPIO_PIN_4,
    LL_GPIO_PIN_5, LL_GPIO_PIN_6, LL_GPIO_PIN_7, LL_GPIO_PIN_15,
};

#define PORTB_N 12

uint32_t portb[PORTB_N] = {
    LL_GPIO_PIN_0,  LL_GPIO_PIN_1,  LL_GPIO_PIN_3,  LL_GPIO_PIN_4,
    LL_GPIO_PIN_5,  LL_GPIO_PIN_6,  LL_GPIO_PIN_7,  LL_GPIO_PIN_10,
    LL_GPIO_PIN_11, LL_GPIO_PIN_12, LL_GPIO_PIN_13, LL_GPIO_PIN_15,
};

#define PORTC_N 1

uint32_t portc[PORTC_N] = {
    LL_GPIO_PIN_13,
};

#ifndef BSP_H
#define BSP_H


#define CPU_BUS_CLK_HZ 48000000ul

#define RESULT_ERROR       1
#define RESULT_OK          0 

#include   "stdio.h"
#include   "stdlib.h"
#include   "stdarg.h"
#include   "stdbool.h"
#include   "string.h"
#include   "ctype.h"
#include   "math.h"
#include   "limits.h"
#include   "arm_itm.h"

#include   "OS.h"

#include   "MKS22F12.h"
#include   "MKS22F12_features.h"
#include   "ANSIN01_INIT_SYS.h"
#include   "ANSIN01_Flash.h"
#include   "ANSIN01_PIT.h"
#include   "ANSIN01_DMA.h"
#include   "ANSIN01_ADC.h"
#include   "ANSIN01_SPI.h"
#include   "ANSIN01_LDC1101.h"
#include   "ANSIN01_SPI_HOST.h"

typedef struct
{
  GPIO_Type *gpio;
  PORT_Type *port;
  unsigned char  pin_num;
  unsigned char  irqc; //  Interrupt Configuration
                       //  0000 Interrupt/DMA Request disabled.
                       //  0001 DMA Request on rising edge.
                       //  0010 DMA Request on falling edge.
                       //  0011 DMA Request on either edge.
                       //  0100 Reserved.
                       //  1000 Interrupt when logic zero.
                       //  1001 Interrupt on rising edge.
                       //  1010 Interrupt on falling edge.
                       //  1011 Interrupt on either edge.
                       //  1100 Interrupt when logic one.
  unsigned char  lock; //  if 1 Pin Control Register bits [15:0] are locked and cannot be updated until the next System Reset.
  unsigned char  mux;  //  Pin Mux Control
                       //  000 Pin Disabled (Analog).
                       //  001 Alternative 1 (GPIO).
                       //  010 Alternative 2 (chip specific).
                       //  011 Alternative 3 (chip specific).
                       //  100 Alternative 4 (chip specific).
                       //  101 Alternative 5 (chip specific).
                       //  110 Alternative 6 (chip specific).
                       //  111 Alternative 7 (chip specific / JTAG / NMI).
  unsigned char  DSE; // 0 Low drive strength is configured on the corresponding pin, if pin is configured as a digital output.
                      // 1 High drive strength is configured on the corresponding pin, if pin is configured as a digital output.
  unsigned char  SRE;  // 0 Fast slew rate is configured on the corresponding pin, if pin is configured as a digital output.
                       // 1 Slow slew rate is configured on the corresponding pin, if pin is configured as a digital output.
  unsigned char  ODE;  // 0 Open Drain output is disabled on the corresponding pin.
                       // 1 Open Drain output is enabled on the corresponding pin, provided pin is configured as a digital output.
  unsigned char  PFE;  // 0 Passive Input Filter is disabled on the corresponding pin.
                       // 1 Passive Input Filter is enabled on the corresponding pin.
  unsigned char  PUPD; // 00 Internal pull-up or pull-down resistor is not enabled on the corresponding pin.
                       // 10 Internal pull-down resistor is enabled on the corresponding pin, if the corresponding Port Pull Enable Register bit is set.
                       // 11 Internal pull-up resistor is enabled on the corresponding pin, if the corresponding Port Pull Enable Register bit is set.
  unsigned char  dir;  // 0 Pin is configured as general purpose input, if configured for the GPIO function
                       // 1 Pin is configured for general purpose output, if configured for the GPIO function
  unsigned char  init; // Init state

} T_IO_pins_configuration;

#define   ANAL          0  // Pin Disabled (Analog).
#define   ALT0          0  // Pin Disabled (Analog).
#define   GPIO          1  // Alternative 1 (GPIO).
#define   ALT1          1  // Alternative 1 (GPIO).
#define   ALT2          2  //
#define   ALT3          3  //
#define   ALT4          4  //
#define   ALT5          5  //
#define   ALT6          6  //
#define   ALT7          7  //

#define   DSE_LO        0 // 0 Low drive strength is configured on the corresponding pin, if pin is configured as a digital output.
#define   DSE_HI        1 // 1 High drive strength is configured on the corresponding pin, if pin is configured as a digital output.

#define   OD_DIS        0 // 0 Open Drain output is disabled on the corresponding pin.
#define   OD__EN        1 // 1 Open Drain output is enabled on the corresponding pin, provided pin is configured as a digital output.

#define   PFE_DIS       0 // 0 Passive Input Filter is disabled on the corresponding pin.
#define   PFE__EN       1 // 1 Passive Input Filter is enabled on the corresponding pin.

#define   FAST_SLEW     0 // 0 Fast slew rate is configured on the corresponding pin, if pin is configured as a digital output.
#define   SLOW_SLEW     1 // 1 Slow slew rate is configured on the corresponding pin, if pin is configured as a digital output.


#define   PUPD_DIS      0 // 00 Internal pull-up or pull-down resistor is not enabled on the corresponding pin.
#define   PULL__DN      2 // 10 Internal pull-down resistor is enabled on the corresponding pin, if the corresponding Port Pull Enable Register bit is set.
#define   PULL__UP      3 // 11 Internal pull-up resistor is enabled on the corresponding pin, if the corresponding Port Pull Enable Register bit is set.

#define   GP_INP        0 // 0 Pin is configured as general purpose input, if configured for the GPIO function
#define   GP_OUT        1 // 1 Pin is configured for general purpose output, if configured for the GPIO function

#define   IRQ_DIS       0  // Interrupt Status Flag (ISF) is disabled.
#define   IRQ_IDR       1  // ISF flag and DMA request on rising edge.
#define   IRQ_IDF       2  // ISF flag and DMA request on falling edge.
#define   IRQ_IDE       3  // ISF flag and DMA request on either edge.
#define   IRQ_IL0       8  // ISF flag and Interrupt when logic 0.
#define   IRQ_IRE       9  // ISF flag and Interrupt on rising-edge.
#define   IRQ_IFE       10 // ISF flag and Interrupt on falling-edge.
#define   IRQ_IEE       11 // ISF flag and Interrupt on either edge.
#define   IRQ_IL1       12 // ISF flag and Interrupt when logic 1.


#define BIT(n) (1u << n)
#define LSHIFT(v,n) (((unsigned int)(v) << n))

int  Init_pins(void);
void BOARD_BootClockRUN(void);

void LED_set_state(uint32_t state);
void SPI0_CS0_set_state(uint32_t state);
void SPI0_CS1_set_state(uint32_t state);
void SPI0_CS2_set_state(uint32_t state);
void SPI0_CS3_set_state(uint32_t state);
void READY_set_state(uint32_t state);

#endif // BSP_H




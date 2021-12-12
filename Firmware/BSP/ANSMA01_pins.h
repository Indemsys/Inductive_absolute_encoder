#ifndef ANSMA01_PINS_H
  #define ANSMA01_PINS_H

// Настройки линий ввода/вывода платы ANSMA01 на базе микроконтроллера MKS22FN256VFT12 
const T_IO_pins_configuration ANSMA01_pins_conf[] =
{
//  gpio          port              num  irqc      lock  mux   DSE     SRE        ODE     PFE      PUPD      dir     init
  { PTA,          PORTA,            0,   IRQ_DIS,   0,   ALT7, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // SWD_CLK          PTA0            Default=(JTAG_TCLK/SWD_CLK)  ALT0=()  ALT1=(PTA0)  ALT2=(UART0_CTS_b)  ALT3=(TPM0_CH5)  ALT4=()  ALT5=(EWM_IN)  ALT6=()  ALT7=(JTAG_TCLK/SWD_CLK)  
  { PTA,          PORTA,            1,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //                  PTA1            Default=(JTAG_TDI)  ALT0=()  ALT1=(PTA1)  ALT2=(UART0_RX)  ALT3=()  ALT4=(CMP0_OUT)  ALT5=(LPI2C1_HREQ)  ALT6=(TPM1_CH1)  ALT7=(JTAG_TDI)  
  { PTA,          PORTA,            2,   IRQ_DIS,   0,   ALT7, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // SWO              PTA2            Default=(JTAG_TDO/TRACE_SWO)  ALT0=()  ALT1=(PTA2)  ALT2=(UART0_TX)  ALT3=()  ALT4=()  ALT5=()  ALT6=(TPM1_CH0)  ALT7=(JTAG_TDO/TRACE_SWO)  
  { PTA,          PORTA,            3,   IRQ_DIS,   0,   ALT7, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // SWD_DIO          PTA3            Default=(JTAG_TMS/SWD_DIO)  ALT0=()  ALT1=(PTA3)  ALT2=(UART0_RTS_b)  ALT3=(TPM0_CH0)  ALT4=()  ALT5=(EWM_OUT_b)  ALT6=()  ALT7=(JTAG_TMS/SWD_DIO)  
  { PTA,          PORTA,            4,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //                  PTA4/LLWU_P3    Default=(NMI_b)  ALT0=()  ALT1=(PTA4/LLWU_P3)  ALT2=()  ALT3=(TPM0_CH1)  ALT4=()  ALT5=()  ALT6=(I2S0_MCLK)  ALT7=(NMI_b)  
  { PTA,          PORTA,            18,  IRQ_DIS,   0,   ALT0, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // EXTAL0           PTA18           Default=(EXTAL0)  ALT0=(EXTAL0)  ALT1=(PTA18)  ALT2=()  ALT3=()  ALT4=(TPM_CLKIN0)  ALT5=()  ALT6=()  ALT7=()  
  { PTA,          PORTA,            19,  IRQ_DIS,   0,   ALT0, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // XTAL0            PTA19           Default=(XTAL0)  ALT0=(XTAL0)  ALT1=(PTA19)  ALT2=()  ALT3=()  ALT4=(TPM_CLKIN1)  ALT5=()  ALT6=(LPTMR0_ALT1)  ALT7=()  
                                                                                                                                                
  { PTB,          PORTB,            0,   IRQ_DIS,   0,   ALT6, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // SCK              PTB0/LLWU_P5    Default=(ADC0_SE8)  ALT0=(ADC0_SE8)  ALT1=(PTB0/LLWU_P5)  ALT2=(LPI2C0_SCL)  ALT3=(TPM1_CH0)  ALT4=()  ALT5=()  ALT6=(FXIO0_D4)  ALT7=(UART0_RX)
  { PTB,          PORTB,            1,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   1 }, // CSQ              PTB1            Default=(ADC0_SE9)  ALT0=(ADC0_SE9)  ALT1=(PTB1)  ALT2=(LPI2C0_SDA)  ALT3=(TPM1_CH1)  ALT4=()  ALT5=(EWM_IN)  ALT6=(FXIO0_D5)  ALT7=(UART0_TX)  
  { PTB,          PORTB,            2,   IRQ_DIS,   0,   ALT6, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // DATA             PTB2            Default=(ADC0_SE12)  ALT0=(ADC0_SE12)  ALT1=(PTB2)  ALT2=(LPI2C0_SCL)  ALT3=(UART0_RTS_b)  ALT4=()  ALT5=()  ALT6=(FXIO0_D6)  ALT7=(CAN1_RX)  
  { PTB,          PORTB,            3,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //                  PTB3            Default=(ADC0_SE13)  ALT0=(ADC0_SE13)  ALT1=(PTB3)  ALT2=(LPI2C0_SDA)  ALT3=(UART0_CTS_b)  ALT4=()  ALT5=()  ALT6=(FXIO0_D7)  ALT7=(CAN1_TX)  
  { PTB,          PORTB,            16,  IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //                  PTB16           Default=(DISABLED)  ALT0=()  ALT1=(PTB16)  ALT2=(SPI1_SOUT)  ALT3=(UART0_RX)  ALT4=(TPM_CLKIN0)  ALT5=()  ALT6=(EWM_IN)  ALT7=(I2S1_TXD0(Note:100LQFPonly))  
  { PTB,          PORTB,            18,  IRQ_DIS,   0,   ALT2, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // CAN0_TX          PTB18           Default=(DISABLED)  ALT0=()  ALT1=(PTB18)  ALT2=(CAN0_TX)  ALT3=(TPM2_CH0)  ALT4=(I2S0_TX_BCLK)  ALT5=()  ALT6=()  ALT7=(FXIO0_D1)  
  { PTB,          PORTB,            19,  IRQ_DIS,   0,   ALT2, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // CAN0_RX          PTB19           Default=(DISABLED)  ALT0=()  ALT1=(PTB19)  ALT2=(CAN0_RX)  ALT3=(TPM2_CH1)  ALT4=(I2S0_TX_FS)  ALT5=()  ALT6=()  ALT7=(FXIO0_D2)  
                                                                                                                                                
  { PTC,          PORTC,            1,   IRQ_DIS,   0,   ALT4, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // TMP0_CH0         PTC1/LLWU_P6    Default=(ADC0_SE15)  ALT0=(ADC0_SE15)  ALT1=(PTC1/LLWU_P6)  ALT2=(SPI0_PCS3)  ALT3=(UART1_RTS_b)  ALT4=(TPM0_CH0)  ALT5=()  ALT6=(I2S0_TXD0)  ALT7=(LPUART0_RTS_b)
  { PTC,          PORTC,            2,   IRQ_DIS,   0,   ALT4, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // TMP0_CH1         PTC2            Default=(ADC0_SE4b)  ALT0=(ADC0_SE4b)  ALT1=(PTC2)  ALT2=(SPI0_PCS2)  ALT3=(UART1_CTS_b)  ALT4=(TPM0_CH1)  ALT5=()  ALT6=(I2S0_TX_FS)  ALT7=(LPUART0_CTS_b)  
  { PTC,          PORTC,            3,   IRQ_DIS,   0,   ALT4, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // TMP0_CH2         PTC3/LLWU_P7    Default=(DISABLED)  ALT0=()  ALT1=(PTC3/LLWU_P7)  ALT2=(SPI0_PCS1)  ALT3=(UART1_RX)  ALT4=(TPM0_CH2)  ALT5=(CLKOUT)  ALT6=(I2S0_TX_BCLK)  ALT7=(LPUART0_RX)  
  { PTC,          PORTC,            4,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, // LED              PTC4/LLWU_P8    Default=(DISABLED)  ALT0=()  ALT1=(PTC4/LLWU_P8)  ALT2=(SPI0_PCS0)  ALT3=(UART1_TX)  ALT4=(TPM0_CH3)  ALT5=()  ALT6=(LPI2C0_HREQ)  ALT7=(LPUART0_TX)  
  { PTC,          PORTC,            5,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //                  PTC5/LLWU_P9    Default=(DISABLED)  ALT0=()  ALT1=(PTC5/LLWU_P9)  ALT2=(SPI0_SCK)  ALT3=(LPTMR0_ALT2)  ALT4=(I2S0_RXD0)  ALT5=()  ALT6=(CMP0_OUT)  ALT7=(TPM0_CH2)  
  { PTC,          PORTC,            6,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //                  PTC6/LLWU_P10   Default=(CMP0_IN0)  ALT0=(CMP0_IN0)  ALT1=(PTC6/LLWU_P10)  ALT2=(SPI0_SOUT)  ALT3=(PDB0_EXTRG)  ALT4=(I2S0_RX_BCLK)  ALT5=()  ALT6=(I2S0_MCLK)  ALT7=(LPI2C0_SCL)  
  { PTC,          PORTC,            7,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //                  PTC7            Default=(CMP0_IN1)  ALT0=(CMP0_IN1)  ALT1=(PTC7)  ALT2=(SPI0_SIN)  ALT3=(USB_SOF_OUT)  ALT4=(I2S0_RX_FS)  ALT5=()  ALT6=()  ALT7=(LPI2C0_SDA)  
                                                                                                                                                
  { PTD,          PORTD,            0,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   1 }, //                  PTD0/LLWU_P12   Default=(DISABLED)  ALT0=()  ALT1=(PTD0/LLWU_P12)  ALT2=(SPI0_PCS0)  ALT3=(UART2_RTS_b)  ALT4=()  ALT5=()  ALT6=(LPUART0_RTS_b)  ALT7=(FXIO0_D6)
  { PTD,          PORTD,            1,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //                  PTD1            Default=(ADC0_SE5b)  ALT0=(ADC0_SE5b)  ALT1=(PTD1)  ALT2=(SPI0_SCK)  ALT3=(UART2_CTS_b)  ALT4=()  ALT5=()  ALT6=(LPUART0_CTS_b)  ALT7=(FXIO0_D7)  
  { PTD,          PORTD,            2,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //                  PTD2/LLWU_P13   Default=(DISABLED)  ALT0=()  ALT1=(PTD2/LLWU_P13)  ALT2=(SPI0_SOUT)  ALT3=(UART2_RX)  ALT4=()  ALT5=()  ALT6=(LPUART0_RX)  ALT7=(LPI2C0_SCL)  
  { PTD,          PORTD,            3,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PULL__UP, GP_INP,   0 }, //                  PTD3            Default=(DISABLED)  ALT0=()  ALT1=(PTD3)  ALT2=(SPI0_SIN)  ALT3=(UART2_TX)  ALT4=()  ALT5=()  ALT6=(LPUART0_TX)  ALT7=(LPI2C0_SDA)  
  { PTD,          PORTD,            4,   IRQ_DIS,   0,   ALT7, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   1 }, // SPI1_CS0         PTD4/LLWU_P14   Default=(DISABLED)  ALT0=()  ALT1=(PTD4/LLWU_P14)  ALT2=(SPI0_PCS1)  ALT3=(UART0_RTS_b)  ALT4=(TPM0_CH4)  ALT5=()  ALT6=(EWM_IN)  ALT7=(SPI1_PCS0)  
  { PTD,          PORTD,            5,   IRQ_DIS,   0,   ALT7, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   1 }, // SPI1_CLK         PTD5            Default=(ADC0_SE6b)  ALT0=(ADC0_SE6b)  ALT1=(PTD5)  ALT2=(SPI0_PCS2)  ALT3=(UART0_CTS_b)  ALT4=(TPM0_CH5)  ALT5=()  ALT6=(EWM_OUT_b)  ALT7=(SPI1_SCK)  
  { PTD,          PORTD,            6,   IRQ_DIS,   0,   ALT7, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   1 }, // SPI1_MISO        PTD6/LLWU_P15   Default=(ADC0_SE7b)  ALT0=(ADC0_SE7b)  ALT1=(PTD6/LLWU_P15)  ALT2=(SPI0_PCS3)  ALT3=(UART0_RX)  ALT4=()  ALT5=()  ALT6=()  ALT7=(SPI1_SOUT)  
  { PTD,          PORTD,            7,   IRQ_DIS,   0,   ALT7, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // SPI1_MOSI        PTD7            Default=(DISABLED)  ALT0=()  ALT1=(PTD7)  ALT2=()  ALT3=(UART0_TX)  ALT4=()  ALT5=()  ALT6=()  ALT7=(SPI1_SIN)  
                                                                                                                                                
  { PTE,          PORTE,            0,   IRQ_DIS,   0,   ALT3, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // UART1_TX         PTE0/CLKOUT32K  Default=(ADC0_SE4a)  ALT0=(ADC0_SE4a)  ALT1=(PTE0/CLKOUT32K)  ALT2=(SPI1_PCS1)  ALT3=(UART1_TX)  ALT4=()  ALT5=()  ALT6=(LPI2C1_SDA)  ALT7=(RTC_CLKOUT)
  { PTE,          PORTE,            1,   IRQ_DIS,   0,   ALT3, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // UART1_RX         PTE1/LLWU_P0    Default=(ADC0_SE5a)  ALT0=(ADC0_SE5a)  ALT1=(PTE1/LLWU_P0)  ALT2=(SPI1_SOUT)  ALT3=(UART1_RX)  ALT4=()  ALT5=()  ALT6=(LPI2C1_SCL)  ALT7=(SPI1_SIN)  
  { PTE,          PORTE,            2,   IRQ_DIS,   0,   ANAL, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // TEMP             PTE2/LLWU_P1    Default=(ADC0_SE6a)  ALT0=(ADC0_SE6a)  ALT1=(PTE2/LLWU_P1)  ALT2=(SPI1_SCK)  ALT3=(UART1_CTS_b)  ALT4=()  ALT5=()  ALT6=()  ALT7=()  
  { PTE,          PORTE,            3,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, // CANCTRL          PTE3            Default=(ADC0_SE7a)  ALT0=(ADC0_SE7a)  ALT1=(PTE3)  ALT2=(SPI1_SIN)  ALT3=(UART1_RTS_b)  ALT4=()  ALT5=()  ALT6=()  ALT7=(SPI1_SOUT)  
  { PTE,          PORTE,            4,   IRQ_DIS,   0,   ALT7, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // LPI2C1_SDA       PTE4/LLWU_P2    Default=(DISABLED)  ALT0=()  ALT1=(PTE4/LLWU_P2)  ALT2=(SPI1_PCS0)  ALT3=(LPUART0_TX)  ALT4=()  ALT5=()  ALT6=()  ALT7=(LPI2C1_SDA)  
  { PTE,          PORTE,            5,   IRQ_DIS,   0,   ALT7, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, // LPI2C1_SCL       PTE5            Default=(DISABLED)  ALT0=()  ALT1=(PTE5)  ALT2=(SPI1_PCS2)  ALT3=(LPUART0_RX)  ALT4=()  ALT5=()  ALT6=()  ALT7=(LPI2C1_SCL)  
};

#endif // 



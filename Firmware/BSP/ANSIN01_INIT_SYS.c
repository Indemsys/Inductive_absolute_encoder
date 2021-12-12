// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-18
// 12:49:19
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "BSP.h"


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Init_WDOG(void)
{
  // Выключать WATCHDOG надо сразу, иначе может возникнуть нестабильный запуск кода после холодного рестарта
  //--------------------------------------------------------------------------------------------------------------------------------------
  WDOG->UNLOCK = 0xC520; // Откроем доступ на запись в регитры управления WDOG
  WDOG->UNLOCK = 0xD928;
  WDOG->STCTRLH = 0
    + LSHIFT(0x00, 14) // DISTESTWDOG | Allows the WDOG’s functional test mode to be disabled permanently| 0 WDOG functional test mode is not disabled.
    + LSHIFT(0x00, 12) // BYTESEL[1:0]| This 2-bit field select the byte to be tested ...                | 00 Byte 0 selected
    + LSHIFT(0x00, 11) // TESTSEL     | Selects the test to be run on the watchdog timer                 | 0 Quick test
    + LSHIFT(0x00, 10) // TESTWDOG    | Puts the watchdog in the functional test mode                    |
    + LSHIFT(0x01, 8)  // Reserved    |
    + LSHIFT(0x01, 7)  // WAITEN      | Enables or disables WDOG in wait mode.                           | 1 WDOG is enabled in CPU wait mode.
    + LSHIFT(0x01, 6)  // STOPEN      | Enables or disables WDOG in stop mode                            | 1 WDOG is enabled in CPU stop mode.
    + LSHIFT(0x00, 5)  // DBGEN       | Enables or disables WDOG in Debug mode                           | 0 WDOG is disabled in CPU Debug mode.
    + LSHIFT(0x01, 4)  // ALLOWUPDATE | Enables updates to watchdog write once registers                 | 1 WDOG write once registers can be unlocked for updating
    + LSHIFT(0x00, 3)  // WINEN       | Enable windowing mode.                                           | 0 Windowing mode is disabled.
    + LSHIFT(0x00, 2)  // IRQRSTEN    | Used to enable the debug breadcrumbs feature                     | 0 WDOG time-out generates reset only.
    + LSHIFT(0x01, 1)  // CLKSRC      | Selects clock source for the WDOG                                | 1 WDOG clock sourced from alternate clock source
    + LSHIFT(0x00, 0)  // WDOGEN      | Enables or disables the WDOG’s operation                         | 1 WDOG is enabled.
  ;

}

//-------------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------------
void Init_cpu(void)
{

  //--------------------------------------------------------------------------------------------------------------------------------------
  SIM->SCGC6 |= BIT(29); // RTC | RTC clock gate control | 1 Clock is enabled.

  RTC->CR = 0;

  //--------------------------------------------------------------------------------------------------------------------------------------
  // Загрузка регистров из области чипа с заводскими установками
  if (*((uint8_t *)0x03FFU) != 0xFFU)
  {
    MCG->C3 = *((uint8_t *)0x03FFU);
    MCG->C4 = (MCG->C4 & 0xE0U) | ((*((uint8_t *)0x03FEU)) & 0x1FU);
  }


  //--------------------------------------------------------------------------------------------------------------------------------------
  // Разрешаем работу на высокой частоте
  SMC->PMPROT = 0
    + LSHIFT(0x1, 7) // AHSRUN  | Allow High Speed Run mode        | 1 HSRUN is allowed
    + LSHIFT(0x0, 5) // AVLP    | Allow Very-Low-Power Modes       | 1 VLPR, VLPW, and VLPS are allowed.
    + LSHIFT(0x0, 3) // ALLS    | Allow Low-Leakage Stop Mode      | 1 Any LLSx mode is allowed
    + LSHIFT(0x0, 1) // AVLLS   | Allow Very-Low-Leakage Stop Mode | 1 Any VLLSx mode is allowed
  ;

  SMC->PMCTRL = 0
    + LSHIFT(0x3, 5) // RUNM    | 11 High Speed Run mode (HSRUN)
    + LSHIFT(0x0, 0) // STOPM   | 000 Normal Stop (STOP)
  ;

  // Ожидаем пока перейдем в заданный режим
  while (SMC->PMSTAT !=  0x80)
  {
  }

  //--------------------------------------------------------------------------------------------------------------------------------------
  SIM->CLKDIV1 = 0
    + LSHIFT(0x00, 28) // OUTDIV1 | Divide value for the core/system clock                                  | 0000 Divide-by-1.  | core/system clock = 120 MHz
    + LSHIFT(0x01, 24) // OUTDIV2 | Divide value for the peripheral clock                                   | 0010 Divide-by-3.  | bus clock = 60 MHz
    + LSHIFT(0x04, 16) // OUTDIV4 | Divide value for the flash clock                                        | 0110 Divide-by-7.  | flash clock = 120/50= 24
  ;


  // Установка емкостей на осциляторе 0
  OSC->CR = 0
    + LSHIFT(0x01, 7) // ERCLKEN  | External Reference Enable (OSCERCLK)       | 1 External reference clock is enabled.
    + LSHIFT(0x00, 5) // EREFSTEN | External Reference Stop Enable             | 0 External reference clock is disabled in Stop mode.
    + LSHIFT(0x00, 3) // SC2P     | Oscillator 2  pF Capacitor Load Configure  | 1 Add 2 pF capacitor to the oscillator load.
    + LSHIFT(0x00, 2) // SC4P     | Oscillator 4  pF Capacitor Load Configure  | 1 Add 4 pF capacitor to the oscillator load.
    + LSHIFT(0x00, 1) // SC8P     | Oscillator 8  pF Capacitor Load Configure  | 1 Add 8 pF capacitor to the oscillator load.
    + LSHIFT(0x00, 0) // SC16P    | Oscillator 16 pF Capacitor Load Configure  | 1 Add 16 pF capacitor to the oscillator load.
  ;

  // Конфигурируем осцилятор 0 (кварц 16 МГц)
  // При конфигурировании обязательно обращать внимание на бит  HGO0
  // При установке данного бита в 1 несмотря на запуск осциляции може не произойти стабилизация PLL
  MCG->C2 = 0
    + LSHIFT(0x00, 7) // LOCRE0 | Loss of Clock Reset Enable     | 0 Interrupt request is generated on a loss of OSC0 external reference clock.
    + LSHIFT(0x00, 6) // FCFTRIM| Fast Internal Reference Clock Fine Trim     | FCFTRIM controls the smallest adjustment of the fast internal reference clock frequency
    + LSHIFT(0x02, 4) // RANGE0 | Frequency Range Select         | 1X Encoding 2 — Very high frequency range selected for the crystal oscillator ..
    + LSHIFT(0x00, 3) // HGO0   | High Gain Oscillator Select    | 1 Configure crystal oscillator for high-gain operation.
    + LSHIFT(0x01, 2) // EREFS0 | External Reference Select      | 1 Oscillator requested.
    + LSHIFT(0x00, 1) // LP     | Low Power Select               | 0 FLL (or PLL) is not disabled in bypass modes.
    + LSHIFT(0x01, 0) // IRCS   | Internal Reference Clock Select| 1 Fast internal reference clock selected.
  ;

  while ((MCG->S & BIT(1)) == 0) // OSCINIT0 Ждем стабилизации осцилятора OSC0
  {
  }


  //
  SIM->SOPT2 = 0
    + LSHIFT(0x01, 26) // LPUARTSRC   | LPUART clock source select            | 01 MCGFLLCLK , or MCGPLLCLK, or IRC48M, or USB1 PFD
    + LSHIFT(0x01, 24) // TPMSRC      | TPM clock source select               | 01 MCGFLLCLK , or MCGPLLCLK, or IRC48M, or USB1 PFD
    + LSHIFT(0x01, 18) // USBSRC      | USB clock source select               | 1 MCGFLLCLK, or MCGPLLCLK, or IRC48M, or USB1 PFD
    + LSHIFT(0x00, 16) // PLLFLLSEL   | PLL/FLL clock select                  | 00 MCGFLLCLK clock, 01 MCGPLL0CLK !!!, 10 Reserved, 11 IRC48 MHz clock
    + LSHIFT(0x01, 12) // TRACECLKSEL | Debug trace clock select              | 0 MCGCLKOUT
    + LSHIFT(0x00, 10) // LPI2C0SRC   | LPI2C0 source                         | 00 Clock disabled
    + LSHIFT(0x06, 5)  // CLKOUTSEL   | Clock out select                      | 110 OSCERCLK0
    + LSHIFT(0x00, 4)  // RTCCLKOUTSEL| RTC clock out select                  | 0 RTC 1 Hz clock drives RTC CLKOUT.
    + LSHIFT(0x00, 2)  // LPI2C1SRC   | LPI2C1 source                         | 00 Clock disabled
  ;
  SIM->SOPT1 = 0
    + LSHIFT(0x00, 18) // OSC32KSEL | 32K oscillator clock select | 00 System oscillator (OSC32KCLK)
    + LSHIFT(0x00, 16) // OSC32KOUT | 32K Oscillator Clock Output | 00 ERCLK32K is not output.
  ;


  //--------------------------------------------------------------------------------------------------------------------------------------
  // Назначаем источником тактирования FLL и PLL сигнал тактирования  OSCLK приходящий от осциллятора

  MCG->C7 = 0; // OSCSEL | MCG OSC Clock Select | 0 Selects System Oscillator (OSCCLK). 1 Selects 32 kHz RTC Oscillator.


  MCG->SC = 0
    + LSHIFT(0x0, 7) // ATME      | Automatic Trim Machine Enable          | 0 Auto Trim Machine disabled
    + LSHIFT(0x0, 6) // ATMS      | Automatic Trim Machine Select          | 0 32 kHz Internal Reference Clock selected.
    + LSHIFT(0x0, 5) // ATMF      | Automatic Trim Machine Fail Flag       | 0 Automatic Trim Machine completed normally.
    + LSHIFT(0x0, 4) // FLTPRSRV  | FLL Filter Preserve Enable             | 0 FLL filter and FLL frequency will reset on changes to currect clock mode.
    + LSHIFT(0x0, 1) // FCRDIV    | Fast Clock Internal Reference Divider  | 000 Divide Factor is 1, 001 Divide Factor is 2.
    + LSHIFT(0x0, 0) // LOCS0     | OSC0 Loss of Clock Status              | 0 Loss of OSC0 has not occurred.
  ;

  // Переход в режим FBE - FLL Bypassed External ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  MCG->C1 = 0
    + LSHIFT(0x02, 6) // CLKS     | Clock Source Select             | 10 Encoding 2 — External reference clock is selected.
    + LSHIFT(0x04, 3) // FRDIV    | FLL External Reference Divider  | 100 If RANGE 0 = 0 or OSCSEL=1 , Divide Factor is 16; for all other RANGE 0 values, Divide Factor is 512.
    + LSHIFT(0x00, 2) // IREFS    | Internal Reference Select       | 0 External reference clock is selected.
    + LSHIFT(0x01, 1) // IRCLKEN  | Internal Reference Clock Enable | 1 MCGIRCLK active.
    + LSHIFT(0x00, 0) // IREFSTEN | Internal Reference Stop Enable  | 0 Internal reference clock is disabled in Stop mode.
  ;
  while ((MCG->S & BIT(4)) != 0) // IREFST Ждем пока FLL начнет работать от внешнего источника тактирования
  {
  }
  while ((MCG->S & 0x0CU) != 0x08U)  // CLKST  Ждем пока мультиплексор CLKS установиться в режим External reference clock is selected.
  {
  }

  // Настройка FLL
  MCG->C4 = 0
    + LSHIFT(0x00, 7) // DMX32    | DCO Maximum Frequency with 32.768 kHz Reference  | 0 DCO has a default range of 25%.
    + LSHIFT(0x00, 5) // DRST_DRS | DCO Range Select                                 | 00 Encoding 0 — Low range (reset default).
    + LSHIFT(0x00, 1) // FCTRIM   | Fast Internal Reference Clock Trim Setting       |
    + LSHIFT(0x00, 0) // SCFTRIM  | Slow Internal Reference Clock Fine Trim          |
  ;

  // Конфигурация PLL0 на 120 МГц
  MCG->C5 = 0
    + LSHIFT(0x00, 6) // PLLCLKEN  | PLL Clock Enable                | 0 MCGPLL0CLK and MCGPLL0CLK2X are inactive
    + LSHIFT(0x00, 5) // PLLSTEN   | PLL Stop Enable                 | 0 MCGPLL0CLK and MCGPLL0CLK2X are disabled in any of the Stop modes.
    + LSHIFT(0x03, 0) // PRDIV     | PLL External Reference Divider  | 011 Divide Factor 4. Selects the amount to divide down the external reference clock for the PLL0. The resulting frequency must be in the range of 2 MHz to 4 MHz.
  ;

  MCG->C6 = 0
    + LSHIFT(0x00, 7) // LOLIE0  | Loss of Lock Interrrupt Enable | 0 No interrupt request is generated on loss of lock.
    + LSHIFT(0x00, 6) // PLLS    | PLL Select                     | 0 FLL is selected.
    + LSHIFT(0x01, 5) // CME0    | Clock Monitor Enable           | 0 External clock monitor is disabled for OSC0.
    + LSHIFT(0x06, 0) // VDIV    | VCO Divider                    | Умножаем на 30 (чтобы получить 120 МГц от 16 МГц кварца)
  ;


  // Переход в режим PBE - PLL Bypassed External ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  MCG->C6 = 0
    + LSHIFT(0x00, 7) // LOLIE0  | Loss of Lock Interrrupt Enable | 0 No interrupt request is generated on loss of lock.
    + LSHIFT(0x01, 6) // PLLS    | PLL Select                     | 1 PLLCS output clock is selected
    + LSHIFT(0x01, 5) // CME0    | Clock Monitor Enable           | 0 External clock monitor is disabled for OSC0.
    + LSHIFT(0x06, 0) // VDIV0   | VCO0 Divider                   | Умножаем на 30 (чтобы получить 120 МГц от 16 МГц кварца)
  ;
  while ((MCG->S & BIT(5)) == 0x00U) // PLLST Ждем пока PLLS не переключится на PLL
  {
  }
  while ((MCG->S & BIT(6)) == 0x00U) // LOCK0 Ждем пока PLL0 защелкнется
  {
  }

  // Переход в режим PEE - PLL Engaged External ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  MCG->C1 = 0
    + LSHIFT(0x00, 6) // CLKS     | Clock Source Select             | 00 Encoding 0 — Output of FLL or PLLCS is selected (depends on PLLS control bit).
    + LSHIFT(0x04, 3) // FRDIV    | FLL External Reference Divider  | 101 If RANGE 0 = 0 or OSCSEL=1 , Divide Factor is 32; for all other RANGE 0 values, Divide Factor is 1024.
    + LSHIFT(0x00, 2) // IREFS    | Internal Reference Select       | 0 External reference clock is selected.
    + LSHIFT(0x01, 1) // IRCLKEN  | Internal Reference Clock Enable | 1 MCGIRCLK active.
    + LSHIFT(0x00, 0) // IREFSTEN | Internal Reference Stop Enable  | 0 Internal reference clock is disabled in Stop mode.
  ;

  while ((MCG->S & 0x0CU) != 0x0CU)  // CLKST  Ждем пока мультиплексор CLKS установиться в режим Output of the PLL is selected.
  {
  }

  MCG->C6 |= BIT(5);     // CME0 = 1 | 1 External clock monitor is enabled for OSC0.


  //--------------------------------------------------------------------------------------------------------------------------------------
  // Установки  Reset Control Module (RCM)
  RCM->RPFW = 0
    + LSHIFT(0x1F, 0) // RSTFLTSEL | Selects the reset pin bus clock filter width.| 11111 Bus clock filter count is 32
  ;
  RCM->RPFC = 0
    + LSHIFT(0x00, 2) // RSTFLTSS  | Selects how the reset pin filter is enabled in STOP and VLPS modes. | 0 All filtering disabled
    + LSHIFT(0x01, 0) // RSTFLTSRW | Selects how the reset pin filter is enabled in run and wait modes.  | 01 Bus clock filter enabled for normal operation
  ;

  if ((PMC->REGSC & BIT(3)) != 0) PMC->REGSC |= BIT(3); // Сброс бита ACKISO, чтобы разблокировать некоторую периферию

  //--------------------------------------------------------------------------------------------------------------------------------------
  // Установки Power Management Controller (PMC)
  PMC->REGSC = 0
    + LSHIFT(0x00, 3) // ACKISO | Acknowledge Isolation | 0 Peripherals and I/O pads are in normal run state|
                      //          Writing one to this bit when it is set releases the I/O pads and certain peripherals to their normal run mode state
    + LSHIFT(0x00, 0) // BGBE   | Bandgap Buffer Enable | 0 Bandgap buffer not enabled
  ;


  PMC->LVDSC1 = 0
    + LSHIFT(0x01, 6) // LVDACK | Low-Voltage Detect Acknowledge     | This write-only bit is used to acknowledge low voltage detection errors (write 1 to clear LVDF). Reads always return 0.
    + LSHIFT(0x00, 5) // LVDIE  | Low-Voltage Detect Interrupt Enable| 0 Hardware interrupt disabled (use polling)
    + LSHIFT(0x01, 4) // LVDRE  | Low-Voltage Detect Reset Enable    | 1 Force an MCU reset when LVDF = 1
    + LSHIFT(0x00, 0) // LVDV   | Low-Voltage Detect Voltage Select  | 00 Low trip point selected (V LVD = V LVDL )
  ;

  PMC->LVDSC2 = 0
    + LSHIFT(0x01, 6) // LVWACK | Low-Voltage Warning Acknowledge      |
    + LSHIFT(0x00, 5) // LVWIE  | Low-Voltage Warning Interrupt Enable | 0 Hardware interrupt disabled (use polling)
    + LSHIFT(0x00, 0) // LVWV   | Low-Voltage Warning Voltage Select   | 00 Low trip point selected (V LVW = V LVW1 )
  ;

}

/*-----------------------------------------------------------------------------------------------------
 Инициализируем достум мастеров к FLASH чтобы DMA и USB имели возможность читать данные из FLASH памяти  
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void Init_Flash_Access_Protection_Register(void)
{
  //--------------------------------------------------------------------------------------------------------------------------------------
  // Инициализация Flash Access Protection Register для разрешения доступа к Flash по DMA и от прочих мастеров
  FMC->PFAPR = 0
               + LSHIFT(0x00, 23) // M7PFD     | 1 Prefetching for this master is disabled.                       7 ---
               + LSHIFT(0x00, 22) // M6PFD     | 1 Prefetching for this master is disabled.                       6 ---
               + LSHIFT(0x00, 21) // M5PFD     | 1 Prefetching for this master is disabled.                       5 ---
               + LSHIFT(0x01, 20) // M4PFD     | 1 Prefetching for this master is enabled.                        4 (USB  OTG)
               + LSHIFT(0x00, 19) // M3PFD     | 0 Prefetching for this master is disabled.                       3 ---
               + LSHIFT(0x01, 18) // M2PFD     | 1 Prefetching for this master is enabled.                        2 (DMA)
               + LSHIFT(0x01, 17) // M1PFD     | 1 Prefetching for this master is enabled.                        1 (ARM core system bus)
               + LSHIFT(0x01, 16) // M0PFD     | 1 Prefetching for this master is enabled.                        0 (ARM core code bus)
               + LSHIFT(0x00, 14) // M7AP[1:0] | 00 No access may be performed by this master.                    7 ---
               + LSHIFT(0x00, 12) // M6AP[1:0] | 00 No access may be performed by this master.                    6 ---
               + LSHIFT(0x00, 10) // M5AP[1:0] | 00 No access may be performed by this master.                    5 ---
               + LSHIFT(0x03, 8)  // M4AP[1:0] | 11 Both read and write accesses may be performed by this master  4 (USB OTG)
               + LSHIFT(0x00, 6)  // M3AP[1:0] | 00 No access may be performed by this master.                    3 ---
               + LSHIFT(0x03, 4)  // M2AP[1:0] | 11 Both read and write accesses may be performed by this master  2 (DMA)
               + LSHIFT(0x03, 2)  // M1AP[1:0] | 11 Both read and write accesses may be performed by this master  1 (ARM core system bus)
               + LSHIFT(0x03, 0)  // M0AP[1:0] | 11 Both read and write accesses may be performed by this master  0 (ARM core code bus)
  ;

}

/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * How to setup clock using clock driver functions:
 *
 * 1. CLOCK_SetSimSafeDivs, to make sure core clock, bus clock, flexbus clock
 *    and flash clock are in allowed range during clock mode switch.
 *
 * 2. Call CLOCK_Osc0Init to setup OSC clock, if it is used in target mode.
 *
 * 3. Set MCG configuration, MCG includes three parts: FLL clock, PLL clock and
 *    internal reference clock(MCGIRCLK). Follow the steps to setup:
 *
 *    1). Call CLOCK_BootToXxxMode to set MCG to target mode.
 *
 *    2). If target mode is FBI/BLPI/PBI mode, the MCGIRCLK has been configured
 *        correctly. For other modes, need to call CLOCK_SetInternalRefClkConfig
 *        explicitly to setup MCGIRCLK.
 *
 *    3). Don't need to configure FLL explicitly, because if target mode is FLL
 *        mode, then FLL has been configured by the function CLOCK_BootToXxxMode,
 *        if the target mode is not FLL mode, the FLL is disabled.
 *
 *    4). If target mode is PEE/PBE/PEI/PBI mode, then the related PLL has been
 *        setup by CLOCK_BootToXxxMode. In FBE/FBI/FEE/FBE mode, the PLL could
 *        be enabled independently, call CLOCK_EnablePll0 explicitly in this case.
 *
 * 4. Call CLOCK_SetSimConfig to set the clock configuration in SIM.
 */

/* TEXT BELOW IS USED AS SETTING FOR THE CLOCKS TOOL *****************************
!!ClocksProfile
product: Clocks v1.0
processor: MKS22FN256xxx12
package_id: MKS22FN256VLL12
mcu_data: ksdk2_0
processor_version: 1.0.1
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE CLOCKS TOOL **/

#include "fsl_smc.h"
#include "clock_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define OSC_CAP0P                                         0U  /*!< Oscillator 0pF capacitor load */
#define SIM_CLKOUT_SEL_OSCERCLK_CLK                       6U  /*!< CLKOUT pin clock select: OSCERCLK clock */
#define SIM_FLEXIOS0_CLK_SEL_CORE_SYSTEM_CLK              0U  /*!< FLEXIOS0 clock select: Core/system clock */
#define SIM_FLEXIO_CLK_SEL_FLEXIOS0_CLK                   0U  /*!< FLEXIO clock select: FLEXIOS0 output clock */
#define SIM_LPUART_CLK_SEL_PLLFLLSEL_CLK                  1U  /*!< LPUART clock select: PLLFLLSEL output clock */
#define SIM_OSC32KSEL_OSC32KCLK_CLK                       0U  /*!< OSC32KSEL select: OSC32KCLK clock */
#define SIM_PLLFLLSEL_DIV_1                               1U  /*!< PLLFLLSEL clock divider divisor: divided by 2 */
#define SIM_PLLFLLSEL_FRAC_0                              0U  /*!< PLLFLLSEL clock divider fraction: multiplied by 1 */
#define SIM_PLLFLLSEL_MCGPLLCLK_CLK                       1U  /*!< PLLFLL select: MCGPLLCLK clock */
#define SIM_TPM_CLK_SEL_PLLFLLSEL_CLK                     1U  /*!< TPM clock select: PLLFLLSEL output clock */
#define SIM_TRACE_CLK_SEL_CORE_SYSTEM_CLK                 1U  /*!< Trace clock select: Core/system clock */
#define SIM_USB_CLK_120000000HZ                   120000000U  /*!< Input SIM frequency for USB: 120000000Hz */

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* System clock frequency. */
extern uint32_t SystemCoreClock;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_CONFIG_SetPllFllSelDivFrac
 * Description   : Configure PLLFLLSEL divider divison and fraction in SIM
 * Param div     : The value to set the divider division.
 * Param frac    : The value to set the divider fraction.
 *
 *END**************************************************************************/
static void CLOCK_CONFIG_SetPllFllSelDivFrac(uint8_t div, uint8_t frac)
{
    SIM->CLKDIV3 = ((SIM->CLKDIV3 & ~(SIM_CLKDIV3_PLLFLLDIV_MASK | SIM_CLKDIV3_PLLFLLFRAC_MASK))
                   | SIM_CLKDIV3_PLLFLLDIV(div)
                   | SIM_CLKDIV3_PLLFLLFRAC(frac));
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_CONFIG_SetFlexioS0Clock
 * Description   : Set FlexIOS0 clock source.
 * Param src     : The value to set FlexIOS0 clock source.
 *
 *END**************************************************************************/
static void CLOCK_CONFIG_SetFlexioS0Clock(uint8_t src)
{
    SIM->MISCCTL = ((SIM->MISCCTL & ~SIM_MISCCTL_FlexIOS0_MASK) | SIM_MISCCTL_FlexIOS0(src));
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_CONFIG_SetFllExtRefDiv
 * Description   : Configure FLL external reference divider (FRDIV).
 * Param frdiv   : The value to set FRDIV.
 *
 *END**************************************************************************/
static void CLOCK_CONFIG_SetFllExtRefDiv(uint8_t frdiv)
{
    MCG->C1 = ((MCG->C1 & ~MCG_C1_FRDIV_MASK) | MCG_C1_FRDIV(frdiv));
}

/*******************************************************************************
 ********************** Configuration BOARD_BootClockRUN ***********************
 ******************************************************************************/
/* TEXT BELOW IS USED AS SETTING FOR THE CLOCKS TOOL *****************************
!!Configuration
name: BOARD_BootClockRUN
outputs:
- {id: Bus_clock.outFreq, value: 60 MHz}
- {id: CLKOUT.outFreq, value: 16 MHz}
- {id: Core_clock.outFreq, value: 120 MHz}
- {id: FLEXIOCLK.outFreq, value: 120 MHz}
- {id: Flash_clock.outFreq, value: 24 MHz}
- {id: LPO_clock.outFreq, value: 1 kHz}
- {id: LPUARTCLK.outFreq, value: 120 MHz}
- {id: MCGIRCLK.outFreq, value: 4 MHz}
- {id: OSCERCLK.outFreq, value: 16 MHz}
- {id: OSCERCLK_UNDIV.outFreq, value: 16 MHz}
- {id: PLLFLLCLK.outFreq, value: 120 MHz}
- {id: System_clock.outFreq, value: 120 MHz}
- {id: TPMCLK.outFreq, value: 60 MHz}
- {id: TRACECLKIN.outFreq, value: 120 MHz}
- {id: USB48MCLK.outFreq, value: 48 MHz}
settings:
- {id: MCGMode, value: PEE}
- {id: powerMode, value: HSRUN}
- {id: CLKOUTConfig, value: 'yes'}
- {id: FLEXIOClkConfig, value: FlexIO}
- {id: LPUARTClkConfig, value: 'yes'}
- {id: MCG.FCRDIV.scale, value: '1'}
- {id: MCG.FRDIV.scale, value: '512'}
- {id: MCG.IRCS.sel, value: MCG.FCRDIV}
- {id: MCG.IREFS.sel, value: MCG.FRDIV}
- {id: MCG.PLLS.sel, value: MCG.PLL}
- {id: MCG.PRDIV.scale, value: '4'}
- {id: MCG.VDIV.scale, value: '30'}
- {id: MCG_C1_IRCLKEN_CFG, value: Enabled}
- {id: MCG_C2_OSC_MODE_CFG, value: ModeOscLowPower}
- {id: MCG_C2_RANGE0_CFG, value: Very_high}
- {id: MCG_C2_RANGE0_FRDIV_CFG, value: Very_high}
- {id: MCG_C5_PLLCLKEN0_CFG, value: Enabled}
- {id: OSC_CR_ERCLKEN_CFG, value: Enabled}
- {id: OSC_CR_ERCLKEN_UNDIV_CFG, value: Enabled}
- {id: SIM.CLKOUTSEL.sel, value: OSC.OSCERCLK}
- {id: SIM.LPUARTSRCSEL.sel, value: SIM.PLLFLLSEL}
- {id: SIM.OUTDIV2.scale, value: '2'}
- {id: SIM.OUTDIV4.scale, value: '5'}
- {id: SIM.PLLFLLDIV.scale, value: '2'}
- {id: SIM.PLLFLLSEL.sel, value: MCG.MCGPLLCLK}
- {id: SIM.TPMSRCSEL.sel, value: SIM.PLLFLLDIV}
- {id: SIM.USBDIV.scale, value: '5'}
- {id: SIM.USBFRAC.scale, value: '2'}
- {id: SIM.USBSRCSEL.sel, value: SIM.USBDIV}
- {id: TPMClkConfig, value: 'yes'}
- {id: TraceClkConfig, value: 'yes'}
- {id: USBClkConfig, value: 'yes'}
sources:
- {id: OSC.OSC.outFreq, value: 16 MHz, enabled: true}
- {id: SIM.USBCLK_EXT.outFreq, value: 48 MHz, enabled: true}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE CLOCKS TOOL **/

/*******************************************************************************
 * Variables for BOARD_BootClockRUN configuration
 ******************************************************************************/
const mcg_config_t mcgConfig_BOARD_BootClockRUN =
    {
        .mcgMode = kMCG_ModePEE,                  /* PEE - PLL Engaged External */
        .irclkEnableMode = kMCG_IrclkEnable,      /* MCGIRCLK enabled, MCGIRCLK disabled in STOP mode */
        .ircs = kMCG_IrcFast,                     /* Fast internal reference clock selected */
        .fcrdiv = 0x0U,                           /* Fast IRC divider: divided by 1 */
        .frdiv = 0x4U,                            /* FLL reference clock divider: divided by 512 */
        .drs = kMCG_DrsLow,                       /* Low frequency range */
        .dmx32 = kMCG_Dmx32Default,               /* DCO has a default range of 25% */
        .oscsel = kMCG_OscselOsc,                 /* Selects System Oscillator (OSCCLK) */
        .pll0Config =
            {
                .enableMode = kMCG_PllEnableIndependent,/* MCGPLLCLK enabled independent of MCG clock mode, MCGPLLCLK disabled in STOP mode */
                .prdiv = 0x3U,                    /* PLL Reference divider: divided by 4 */
                .vdiv = 0x6U,                     /* VCO divider: multiplied by 30 */
            },
    };
const sim_clock_config_t simConfig_BOARD_BootClockRUN =
    {
        .pllFllSel = SIM_PLLFLLSEL_MCGPLLCLK_CLK, /* PLLFLL select: MCGPLLCLK clock */
        .er32kSrc = SIM_OSC32KSEL_OSC32KCLK_CLK,  /* OSC32KSEL select: OSC32KCLK clock */
        .clkdiv1 = 0x1040000U,                    /* SIM_CLKDIV1 - OUTDIV1: /1, OUTDIV2: /2, OUTDIV4: /5 */
    };
const osc_config_t oscConfig_BOARD_BootClockRUN =
    {
        .freq = 16000000U,                        /* Oscillator frequency: 16000000Hz */
        .capLoad = (OSC_CAP0P),                   /* Oscillator capacity load: 0pF */
        .workMode = kOSC_ModeOscLowPower,         /* Oscillator low power */
        .oscerConfig =
            {
                .enableMode = kOSC_ErClkEnable,   /* Enable external reference clock, disable external reference clock in STOP mode */
                .erclkDiv = 0,                    /* Divider for OSCERCLK: divided by 1 */
            }
    };

/*******************************************************************************
 * Code for BOARD_BootClockRUN configuration
 ******************************************************************************/
void BOARD_BootClockRUN(void)
{
    /* Set HSRUN power mode */
    SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeAll);
    SMC_SetPowerModeHsrun(SMC);
    while (SMC_GetPowerModeState(SMC) != kSMC_PowerStateHsrun)
    {
    }
    /* Set the system clock dividers in SIM to safe value. */
    CLOCK_SetSimSafeDivs();
    /* Initializes OSC0 according to board configuration. */
    CLOCK_InitOsc0(&oscConfig_BOARD_BootClockRUN);
    CLOCK_SetXtal0Freq(oscConfig_BOARD_BootClockRUN.freq);
    /* Configure the Internal Reference clock (MCGIRCLK). */
    CLOCK_SetInternalRefClkConfig(mcgConfig_BOARD_BootClockRUN.irclkEnableMode,
                                  mcgConfig_BOARD_BootClockRUN.ircs, 
                                  mcgConfig_BOARD_BootClockRUN.fcrdiv);
    /* Configure FLL external reference divider (FRDIV). */
    CLOCK_CONFIG_SetFllExtRefDiv(mcgConfig_BOARD_BootClockRUN.frdiv);
    /* Set MCG to PEE mode. */
    CLOCK_BootToPeeMode(mcgConfig_BOARD_BootClockRUN.oscsel,
                        kMCG_PllClkSelPll0,
                        &mcgConfig_BOARD_BootClockRUN.pll0Config);
    /* Set the clock configuration in SIM module. */
    CLOCK_SetSimConfig(&simConfig_BOARD_BootClockRUN);
    /* Set the PLLFLLDIV and PLLFLLFRAC in SIM module. */
    CLOCK_CONFIG_SetPllFllSelDivFrac(SIM_PLLFLLSEL_DIV_1, 
                                     SIM_PLLFLLSEL_FRAC_0);
    /* Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;
    /* Enable USB FS clock. */
    CLOCK_EnableUsbfs0Clock(kCLOCK_UsbSrcPll0, SIM_USB_CLK_120000000HZ);
    /* Set LPUART clock source. */
    CLOCK_SetLpuart0Clock(SIM_LPUART_CLK_SEL_PLLFLLSEL_CLK);
    /* Set FLEXIOS0 clock source. */
    CLOCK_CONFIG_SetFlexioS0Clock(SIM_FLEXIOS0_CLK_SEL_CORE_SYSTEM_CLK);
    /* Set FLEXIO clock source. */
    CLOCK_SetFlexio0Clock(SIM_FLEXIO_CLK_SEL_FLEXIOS0_CLK);
    /* Set TPM clock source. */
    CLOCK_SetTpmClock(SIM_TPM_CLK_SEL_PLLFLLSEL_CLK);
    /* Set CLKOUT source. */
    CLOCK_SetClkOutClock(SIM_CLKOUT_SEL_OSCERCLK_CLK);
    /* Set debug trace clock source. */
    CLOCK_SetTraceClock(SIM_TRACE_CLK_SEL_CORE_SYSTEM_CLK);
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-06
// 15:05:01
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "app.h"

OS_MUTEX             ldc1101_mitex;
OS_FLAG_GRP          ldc1101_flags;
static uint32_t      portd_pcr_cs;
static uint32_t      portd_pcr3;

static T_DMA_SPI_cbl DS_cbl;


extern void FMSTR_Recorder(void);


T_LDC1101_registers      LDC1101_regs[LDC1101_DEVS_CNT];
#ifdef COIL_RESEARCH
static T_LDC1101_chip_cs ic[LDC1101_DEVS_CNT] = { SPI_CS0 };
#else
static T_LDC1101_chip_cs ic[LDC1101_DEVS_CNT] = { SPI_CS0, SPI_CS1, SPI_CS2, SPI_CS3 };
#endif
uint32_t                 g_spi_wr_err;
uint32_t                 g_spi_rd_err;

#define RP_SET_ADDR            0x01
#define TC1_ADDR               0x02
#define TC2_ADDR               0x03
#define DIG_CONFIG_ADDR        0x04
#define ALT_CONFIG_ADDR        0x05
#define RP_THRESH_H_LSB_ADDR   0x06
#define RP_THRESH_H_MSB_ADDR   0x07
#define RP_THRESH_L_LSB_ADDR   0x08
#define RP_THRESH_L_MSB_ADDR   0x09
#define INTB_MODE_ADDR         0x0A
#define START_CONFIG_ADDR      0x0B
#define D_CONF_ADDR            0x0C
#define L_THRESH_HI_LSB_ADDR   0x16
#define L_THRESH_HI_MSB_ADDR   0x17
#define L_THRESH_LO_LSB_ADDR   0x18
#define L_THRESH_LO_MSB_ADDR   0x19
#define STATUS_ADDR            0x20
#define RP_DATA_LSB_ADDR       0x21
#define RP_DATA_MSB_ADDR       0x22
#define L_DATA_LSB_ADDR        0x23
#define L_DATA_MSB_ADDR        0x24
#define LHR_RCOUNT_LSB_ADDR    0x30
#define LHR_RCOUNT_MSB_ADDR    0x31
#define LHR_OFFSET_LSB_ADDR    0x32
#define LHR_OFFSET_MSB_ADDR    0x33
#define LHR_CONFIG_ADDR        0x34
#define LHR_DATA_LSB_ADDR      0x38
#define LHR_DATA_MID_ADDR      0x39
#define LHR_DATA_MSB_ADDR      0x3A
#define LHR_STATUS_ADDR        0x3B
#define RID_ADDR               0x3E
#define CHIP_ID_ADDR           0x3F








const char               RP_SET_descr[] =           "RP_SET          - Configure RP Measurement Dynamic Range                ";
const char               TC1_descr[] =              "TC1             - Configure Internal Time Constant 1                    ";
const char               TC2_descr[] =              "TC2             - Configure Internal Time Constant 2                    ";
const char               DIG_CONFIG_descr[] =       "DIG_CONFIG      - Configure RP+L conversion interval                    ";
const char               ALT_CONFIG_descr[] =       "ALT_CONFIG      - Configure additional device settings                  ";
const char               RP_THRESH_H_LSB_descr[] =  "RP_THRESH_H_LSB - RP_THRESHOLD High Setting.                 bits 7..0  ";
const char               RP_THRESH_H_MSB_descr[] =  "RP_THRESH_H_MSB - RP_THRESHOLD High Setting.                 bits 15..8 ";
const char               RP_THRESH_L_LSB_descr[] =  "RP_THRESH_L_LSB - RP_THRESHOLD Low Setting.                  bits 7..0  ";
const char               RP_THRESH_L_MSB_descr[] =  "RP_THRESH_L_MSB - RP_THRESHOLD Low Setting.                  bits 15..8 ";
const char               INTB_MODE_descr[] =        "INTB_MODE       - Configure INTB reporting on SDO pin                   ";
const char               START_CONFIG_descr[] =     "START_CONFIG    - Configure Power State                                 ";
const char               D_CONF_descr[] =           "D_CONF          - Sensor Amplitude Control Requirement                  ";
const char               L_THRESH_HI_LSB_descr[] =  "L_THRESH_HI_LSB - L_THRESHOLD High Setting.                  bits 7..0  ";
const char               L_THRESH_HI_MSB_descr[] =  "L_THRESH_HI_MSB - L_THRESHOLD High Setting.                  bits 15..8 ";
const char               L_THRESH_LO_LSB_descr[] =  "L_THRESH_LO_LSB - L_THRESHOLD Low Setting.                   bits 7..0  ";
const char               L_THRESH_LO_MSB_descr[] =  "L_THRESH_LO_MSB - L_THRESHOLD Low Setting.                   bits 15..8 ";
const char               STATUS_descr[] =           "STATUS          - Report RP+L measurement status                        ";
const char               RP_DATA_LSB_descr[] =      "RP_DATA_LSB     - RP Conversion Result Data Output.          bits 7..0  ";
const char               RP_DATA_MSB_descr[] =      "RP_DATA_MSB     - RP Conversion Result Data Output.          bits 15..8 ";
const char               L_DATA_LSB_descr[] =       "L_DATA_LSB      - L Conversion Result Data Output.           bits 7..0  ";
const char               L_DATA_MSB_descr[] =       "L_DATA_MSB      - L Conversion Result Data Output.           bits 15..8 ";
const char               LHR_RCOUNT_LSB_descr[] =   "LHR_RCOUNT_LSB  - High Resolution L Reference Count.         bits 7..0  ";
const char               LHR_RCOUNT_MSB_descr[] =   "LHR_RCOUNT_MSB  - High Resolution L Reference Count.         bits 15..8 ";
const char               LHR_OFFSET_LSB_descr[] =   "LHR_OFFSET_LSB  - High Resolution L Offset.                  bits 7..0  ";
const char               LHR_OFFSET_MSB_descr[] =   "LHR_OFFSET_MSB  - High Resolution L Offset.                  bits 15..8 ";
const char               LHR_CONFIG_descr[] =       "LHR_CONFIG      - High Resolution L Configuration                       ";
const char               LHR_DATA_LSB_descr[] =     "LHR_DATA_LSB    - High Resolution L Conversion Data output.  bits 7..0  ";
const char               LHR_DATA_MID_descr[] =     "LHR_DATA_MID    - High Resolution L Conversion Data output.  bits 15..8 ";
const char               LHR_DATA_MSB_descr[] =     "LHR_DATA_MSB    - High Resolution L Conversion Data output.  bits 23..16";
const char               LHR_STATUS_descr[] =       "LHR_STATUS      - High Resolution L Measurement Status                  ";
const char               RID_descr[] =              "RID             - Device RID value                                      ";
const char               CHIP_ID_descr[] =          "CHIP_ID         - Device ID value                                       ";

const T_LDC1101_map      LDC1101_map[LDC1101_DEVS_CNT][LDC1101_REGS_CNT] =
{
   {
      { RP_SET_ADDR,  RP_SET_descr, &LDC1101_regs[0].RP_SET, 0x07 },
      { TC1_ADDR,  TC1_descr, &LDC1101_regs[0].TC1, 0x90 },
      { TC2_ADDR,  TC2_descr, &LDC1101_regs[0].TC2, 0xA0 },
      { DIG_CONFIG_ADDR,  DIG_CONFIG_descr, &LDC1101_regs[0].DIG_CONFIG, 0x03 },
      { ALT_CONFIG_ADDR,  ALT_CONFIG_descr, &LDC1101_regs[0].ALT_CONFIG, 0x01 },
      { RP_THRESH_H_LSB_ADDR,  RP_THRESH_H_LSB_descr, &LDC1101_regs[0].RP_THRESH_H_LSB, 0x00 },
      { RP_THRESH_H_MSB_ADDR,  RP_THRESH_H_MSB_descr, &LDC1101_regs[0].RP_THRESH_H_MSB, 0x00 },
      { RP_THRESH_L_LSB_ADDR,  RP_THRESH_L_LSB_descr, &LDC1101_regs[0].RP_THRESH_L_LSB, 0x00 },
      { RP_THRESH_L_MSB_ADDR,  RP_THRESH_L_MSB_descr, &LDC1101_regs[0].RP_THRESH_L_MSB, 0x00 },
      { INTB_MODE_ADDR,  INTB_MODE_descr, &LDC1101_regs[0].INTB_MODE, 0x00 },
      { START_CONFIG_ADDR,  START_CONFIG_descr, &LDC1101_regs[0].START_CONFIG, 0x01 },
      { D_CONF_ADDR,  D_CONF_descr, &LDC1101_regs[0].D_CONF, 0x00 },
      { L_THRESH_HI_LSB_ADDR,  L_THRESH_HI_LSB_descr, &LDC1101_regs[0].L_THRESH_HI_LSB, 0x00 },
      { L_THRESH_HI_MSB_ADDR,  L_THRESH_HI_MSB_descr, &LDC1101_regs[0].L_THRESH_HI_MSB, 0x00 },
      { L_THRESH_LO_LSB_ADDR,  L_THRESH_LO_LSB_descr, &LDC1101_regs[0].L_THRESH_LO_LSB, 0x00 },
      { L_THRESH_LO_MSB_ADDR,  L_THRESH_LO_MSB_descr, &LDC1101_regs[0].L_THRESH_LO_MSB, 0x00 },
      { STATUS_ADDR,  STATUS_descr, &LDC1101_regs[0].STATUS, 0x00 },
      { RP_DATA_LSB_ADDR,  RP_DATA_LSB_descr, &LDC1101_regs[0].RP_DATA_LSB, 0x00 },
      { RP_DATA_MSB_ADDR,  RP_DATA_MSB_descr, &LDC1101_regs[0].RP_DATA_MSB, 0x00 },
      { L_DATA_LSB_ADDR,  L_DATA_LSB_descr, &LDC1101_regs[0].L_DATA_LSB, 0x00 },
      { L_DATA_MSB_ADDR,  L_DATA_MSB_descr, &LDC1101_regs[0].L_DATA_MSB, 0x00 },
      { LHR_RCOUNT_LSB_ADDR,  LHR_RCOUNT_LSB_descr, &LDC1101_regs[0].LHR_RCOUNT_LSB, 0x00 },
      { LHR_RCOUNT_MSB_ADDR,  LHR_RCOUNT_MSB_descr, &LDC1101_regs[0].LHR_RCOUNT_MSB, 0x00 },
      { LHR_OFFSET_LSB_ADDR,  LHR_OFFSET_LSB_descr, &LDC1101_regs[0].LHR_OFFSET_LSB, 0x00 },
      { LHR_OFFSET_MSB_ADDR,  LHR_OFFSET_MSB_descr, &LDC1101_regs[0].LHR_OFFSET_MSB, 0x00 },
      { LHR_CONFIG_ADDR,  LHR_CONFIG_descr, &LDC1101_regs[0].LHR_CONFIG, 0x00 },
      { LHR_DATA_LSB_ADDR,  LHR_DATA_LSB_descr, &LDC1101_regs[0].LHR_DATA_LSB, 0x00 },
      { LHR_DATA_MID_ADDR,  LHR_DATA_MID_descr, &LDC1101_regs[0].LHR_DATA_MID, 0x00 },
      { LHR_DATA_MSB_ADDR,  LHR_DATA_MSB_descr, &LDC1101_regs[0].LHR_DATA_MSB, 0x00 },
      { LHR_STATUS_ADDR,  LHR_STATUS_descr, &LDC1101_regs[0].LHR_STATUS, 0x00 },
      { RID_ADDR,  RID_descr, &LDC1101_regs[0].RID, 0x02 },
      { CHIP_ID_ADDR,  CHIP_ID_descr, &LDC1101_regs[0].CHIP_ID, 0xD4 },
   },
#ifndef COIL_RESEARCH
   {
      { RP_SET_ADDR,  RP_SET_descr, &LDC1101_regs[1].RP_SET, 0x07 },
      { TC1_ADDR,  TC1_descr, &LDC1101_regs[1].TC1, 0x90 },
      { TC2_ADDR,  TC2_descr, &LDC1101_regs[1].TC2, 0xA0 },
      { DIG_CONFIG_ADDR,  DIG_CONFIG_descr, &LDC1101_regs[1].DIG_CONFIG, 0x03 },
      { ALT_CONFIG_ADDR,  ALT_CONFIG_descr, &LDC1101_regs[1].ALT_CONFIG, 0x01 },
      { RP_THRESH_H_LSB_ADDR,  RP_THRESH_H_LSB_descr, &LDC1101_regs[1].RP_THRESH_H_LSB, 0x00 },
      { RP_THRESH_H_MSB_ADDR,  RP_THRESH_H_MSB_descr, &LDC1101_regs[1].RP_THRESH_H_MSB, 0x00 },
      { RP_THRESH_L_LSB_ADDR,  RP_THRESH_L_LSB_descr, &LDC1101_regs[1].RP_THRESH_L_LSB, 0x00 },
      { RP_THRESH_L_MSB_ADDR,  RP_THRESH_L_MSB_descr, &LDC1101_regs[1].RP_THRESH_L_MSB, 0x00 },
      { INTB_MODE_ADDR,  INTB_MODE_descr, &LDC1101_regs[1].INTB_MODE, 0x00 },
      { START_CONFIG_ADDR,  START_CONFIG_descr, &LDC1101_regs[1].START_CONFIG, 0x01 },
      { D_CONF_ADDR,  D_CONF_descr, &LDC1101_regs[1].D_CONF, 0x00 },
      { L_THRESH_HI_LSB_ADDR,  L_THRESH_HI_LSB_descr, &LDC1101_regs[1].L_THRESH_HI_LSB, 0x00 },
      { L_THRESH_HI_MSB_ADDR,  L_THRESH_HI_MSB_descr, &LDC1101_regs[1].L_THRESH_HI_MSB, 0x00 },
      { L_THRESH_LO_LSB_ADDR,  L_THRESH_LO_LSB_descr, &LDC1101_regs[1].L_THRESH_LO_LSB, 0x00 },
      { L_THRESH_LO_MSB_ADDR,  L_THRESH_LO_MSB_descr, &LDC1101_regs[1].L_THRESH_LO_MSB, 0x00 },
      { STATUS_ADDR,  STATUS_descr, &LDC1101_regs[1].STATUS, 0x00 },
      { RP_DATA_LSB_ADDR,  RP_DATA_LSB_descr, &LDC1101_regs[1].RP_DATA_LSB, 0x00 },
      { RP_DATA_MSB_ADDR,  RP_DATA_MSB_descr, &LDC1101_regs[1].RP_DATA_MSB, 0x00 },
      { L_DATA_LSB_ADDR,  L_DATA_LSB_descr, &LDC1101_regs[1].L_DATA_LSB, 0x00 },
      { L_DATA_MSB_ADDR,  L_DATA_MSB_descr, &LDC1101_regs[1].L_DATA_MSB, 0x00 },
      { LHR_RCOUNT_LSB_ADDR,  LHR_RCOUNT_LSB_descr, &LDC1101_regs[1].LHR_RCOUNT_LSB, 0x00 },
      { LHR_RCOUNT_MSB_ADDR,  LHR_RCOUNT_MSB_descr, &LDC1101_regs[1].LHR_RCOUNT_MSB, 0x00 },
      { LHR_OFFSET_LSB_ADDR,  LHR_OFFSET_LSB_descr, &LDC1101_regs[1].LHR_OFFSET_LSB, 0x00 },
      { LHR_OFFSET_MSB_ADDR,  LHR_OFFSET_MSB_descr, &LDC1101_regs[1].LHR_OFFSET_MSB, 0x00 },
      { LHR_CONFIG_ADDR,  LHR_CONFIG_descr, &LDC1101_regs[1].LHR_CONFIG, 0x00 },
      { LHR_DATA_LSB_ADDR,  LHR_DATA_LSB_descr, &LDC1101_regs[1].LHR_DATA_LSB, 0x00 },
      { LHR_DATA_MID_ADDR,  LHR_DATA_MID_descr, &LDC1101_regs[1].LHR_DATA_MID, 0x00 },
      { LHR_DATA_MSB_ADDR,  LHR_DATA_MSB_descr, &LDC1101_regs[1].LHR_DATA_MSB, 0x00 },
      { LHR_STATUS_ADDR,  LHR_STATUS_descr, &LDC1101_regs[1].LHR_STATUS, 0x00 },
      { RID_ADDR,  RID_descr, &LDC1101_regs[1].RID, 0x02 },
      { CHIP_ID_ADDR,  CHIP_ID_descr, &LDC1101_regs[1].CHIP_ID, 0xD4 },
   },
   {
      { RP_SET_ADDR,  RP_SET_descr, &LDC1101_regs[2].RP_SET, 0x07 },
      { TC1_ADDR,  TC1_descr, &LDC1101_regs[2].TC1, 0x90 },
      { TC2_ADDR,  TC2_descr, &LDC1101_regs[2].TC2, 0xA0 },
      { DIG_CONFIG_ADDR,  DIG_CONFIG_descr, &LDC1101_regs[2].DIG_CONFIG, 0x03 },
      { ALT_CONFIG_ADDR,  ALT_CONFIG_descr, &LDC1101_regs[2].ALT_CONFIG, 0x01 },
      { RP_THRESH_H_LSB_ADDR,  RP_THRESH_H_LSB_descr, &LDC1101_regs[2].RP_THRESH_H_LSB, 0x00 },
      { RP_THRESH_H_MSB_ADDR,  RP_THRESH_H_MSB_descr, &LDC1101_regs[2].RP_THRESH_H_MSB, 0x00 },
      { RP_THRESH_L_LSB_ADDR,  RP_THRESH_L_LSB_descr, &LDC1101_regs[2].RP_THRESH_L_LSB, 0x00 },
      { RP_THRESH_L_MSB_ADDR,  RP_THRESH_L_MSB_descr, &LDC1101_regs[2].RP_THRESH_L_MSB, 0x00 },
      { INTB_MODE_ADDR,  INTB_MODE_descr, &LDC1101_regs[2].INTB_MODE, 0x00 },
      { START_CONFIG_ADDR,  START_CONFIG_descr, &LDC1101_regs[2].START_CONFIG, 0x01 },
      { D_CONF_ADDR,  D_CONF_descr, &LDC1101_regs[2].D_CONF, 0x00 },
      { L_THRESH_HI_LSB_ADDR,  L_THRESH_HI_LSB_descr, &LDC1101_regs[2].L_THRESH_HI_LSB, 0x00 },
      { L_THRESH_HI_MSB_ADDR,  L_THRESH_HI_MSB_descr, &LDC1101_regs[2].L_THRESH_HI_MSB, 0x00 },
      { L_THRESH_LO_LSB_ADDR,  L_THRESH_LO_LSB_descr, &LDC1101_regs[2].L_THRESH_LO_LSB, 0x00 },
      { L_THRESH_LO_MSB_ADDR,  L_THRESH_LO_MSB_descr, &LDC1101_regs[2].L_THRESH_LO_MSB, 0x00 },
      { STATUS_ADDR,  STATUS_descr, &LDC1101_regs[2].STATUS, 0x00 },
      { RP_DATA_LSB_ADDR,  RP_DATA_LSB_descr, &LDC1101_regs[2].RP_DATA_LSB, 0x00 },
      { RP_DATA_MSB_ADDR,  RP_DATA_MSB_descr, &LDC1101_regs[2].RP_DATA_MSB, 0x00 },
      { L_DATA_LSB_ADDR,  L_DATA_LSB_descr, &LDC1101_regs[2].L_DATA_LSB, 0x00 },
      { L_DATA_MSB_ADDR,  L_DATA_MSB_descr, &LDC1101_regs[2].L_DATA_MSB, 0x00 },
      { LHR_RCOUNT_LSB_ADDR,  LHR_RCOUNT_LSB_descr, &LDC1101_regs[2].LHR_RCOUNT_LSB, 0x00 },
      { LHR_RCOUNT_MSB_ADDR,  LHR_RCOUNT_MSB_descr, &LDC1101_regs[2].LHR_RCOUNT_MSB, 0x00 },
      { LHR_OFFSET_LSB_ADDR,  LHR_OFFSET_LSB_descr, &LDC1101_regs[2].LHR_OFFSET_LSB, 0x00 },
      { LHR_OFFSET_MSB_ADDR,  LHR_OFFSET_MSB_descr, &LDC1101_regs[2].LHR_OFFSET_MSB, 0x00 },
      { LHR_CONFIG_ADDR,  LHR_CONFIG_descr, &LDC1101_regs[2].LHR_CONFIG, 0x00 },
      { LHR_DATA_LSB_ADDR,  LHR_DATA_LSB_descr, &LDC1101_regs[2].LHR_DATA_LSB, 0x00 },
      { LHR_DATA_MID_ADDR,  LHR_DATA_MID_descr, &LDC1101_regs[2].LHR_DATA_MID, 0x00 },
      { LHR_DATA_MSB_ADDR,  LHR_DATA_MSB_descr, &LDC1101_regs[2].LHR_DATA_MSB, 0x00 },
      { LHR_STATUS_ADDR,  LHR_STATUS_descr, &LDC1101_regs[2].LHR_STATUS, 0x00 },
      { RID_ADDR,  RID_descr, &LDC1101_regs[2].RID, 0x02 },
      { CHIP_ID_ADDR,  CHIP_ID_descr, &LDC1101_regs[2].CHIP_ID, 0xD4 },
   },
   {
      { RP_SET_ADDR,  RP_SET_descr, &LDC1101_regs[3].RP_SET, 0x07 },
      { TC1_ADDR,  TC1_descr, &LDC1101_regs[3].TC1, 0x90 },
      { TC2_ADDR,  TC2_descr, &LDC1101_regs[3].TC2, 0xA0 },
      { DIG_CONFIG_ADDR,  DIG_CONFIG_descr, &LDC1101_regs[3].DIG_CONFIG, 0x03 },
      { ALT_CONFIG_ADDR,  ALT_CONFIG_descr, &LDC1101_regs[3].ALT_CONFIG, 0x01 },
      { RP_THRESH_H_LSB_ADDR,  RP_THRESH_H_LSB_descr, &LDC1101_regs[3].RP_THRESH_H_LSB, 0x00 },
      { RP_THRESH_H_MSB_ADDR,  RP_THRESH_H_MSB_descr, &LDC1101_regs[3].RP_THRESH_H_MSB, 0x00 },
      { RP_THRESH_L_LSB_ADDR,  RP_THRESH_L_LSB_descr, &LDC1101_regs[3].RP_THRESH_L_LSB, 0x00 },
      { RP_THRESH_L_MSB_ADDR,  RP_THRESH_L_MSB_descr, &LDC1101_regs[3].RP_THRESH_L_MSB, 0x00 },
      { INTB_MODE_ADDR,  INTB_MODE_descr, &LDC1101_regs[3].INTB_MODE, 0x00 },
      { START_CONFIG_ADDR,  START_CONFIG_descr, &LDC1101_regs[3].START_CONFIG, 0x01 },
      { D_CONF_ADDR,  D_CONF_descr, &LDC1101_regs[3].D_CONF, 0x00 },
      { L_THRESH_HI_LSB_ADDR,  L_THRESH_HI_LSB_descr, &LDC1101_regs[3].L_THRESH_HI_LSB, 0x00 },
      { L_THRESH_HI_MSB_ADDR,  L_THRESH_HI_MSB_descr, &LDC1101_regs[3].L_THRESH_HI_MSB, 0x00 },
      { L_THRESH_LO_LSB_ADDR,  L_THRESH_LO_LSB_descr, &LDC1101_regs[3].L_THRESH_LO_LSB, 0x00 },
      { L_THRESH_LO_MSB_ADDR,  L_THRESH_LO_MSB_descr, &LDC1101_regs[3].L_THRESH_LO_MSB, 0x00 },
      { STATUS_ADDR,  STATUS_descr, &LDC1101_regs[3].STATUS, 0x00 },
      { RP_DATA_LSB_ADDR,  RP_DATA_LSB_descr, &LDC1101_regs[3].RP_DATA_LSB, 0x00 },
      { RP_DATA_MSB_ADDR,  RP_DATA_MSB_descr, &LDC1101_regs[3].RP_DATA_MSB, 0x00 },
      { L_DATA_LSB_ADDR,  L_DATA_LSB_descr, &LDC1101_regs[3].L_DATA_LSB, 0x00 },
      { L_DATA_MSB_ADDR,  L_DATA_MSB_descr, &LDC1101_regs[3].L_DATA_MSB, 0x00 },
      { LHR_RCOUNT_LSB_ADDR,  LHR_RCOUNT_LSB_descr, &LDC1101_regs[3].LHR_RCOUNT_LSB, 0x00 },
      { LHR_RCOUNT_MSB_ADDR,  LHR_RCOUNT_MSB_descr, &LDC1101_regs[3].LHR_RCOUNT_MSB, 0x00 },
      { LHR_OFFSET_LSB_ADDR,  LHR_OFFSET_LSB_descr, &LDC1101_regs[3].LHR_OFFSET_LSB, 0x00 },
      { LHR_OFFSET_MSB_ADDR,  LHR_OFFSET_MSB_descr, &LDC1101_regs[3].LHR_OFFSET_MSB, 0x00 },
      { LHR_CONFIG_ADDR,  LHR_CONFIG_descr, &LDC1101_regs[3].LHR_CONFIG, 0x00 },
      { LHR_DATA_LSB_ADDR,  LHR_DATA_LSB_descr, &LDC1101_regs[3].LHR_DATA_LSB, 0x00 },
      { LHR_DATA_MID_ADDR,  LHR_DATA_MID_descr, &LDC1101_regs[3].LHR_DATA_MID, 0x00 },
      { LHR_DATA_MSB_ADDR,  LHR_DATA_MSB_descr, &LDC1101_regs[3].LHR_DATA_MSB, 0x00 },
      { LHR_STATUS_ADDR,  LHR_STATUS_descr, &LDC1101_regs[3].LHR_STATUS, 0x00 },
      { RID_ADDR,  RID_descr, &LDC1101_regs[3].RID, 0x02 },
      { CHIP_ID_ADDR,  CHIP_ID_descr, &LDC1101_regs[3].CHIP_ID, 0xD4 },
   },
#endif
};

uint32_t                 tx_buf[LDC1101_DEVS_CNT * 3];
uint32_t                 rx_buf[LDC1101_DEVS_CNT * 3];

/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void LDC1101_init_sync_obj(void)
{
   OS_ERR err;
   OSMutexCreate(&ldc1101_mitex, "LDC1101 mutex", &err);
   OSFlagCreate(&ldc1101_flags, "LDC1101_flags", (OS_FLAGS)0, &err);
}

/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void LDC1101_pend_mutex(void)
{
   OS_ERR err;
   OSMutexPend(&ldc1101_mitex, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
}

/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void LDC1101_post_mutex(void)
{
   OS_ERR err;
   OSMutexPost(&ldc1101_mitex, OS_OPT_POST_NONE, &err);
}

/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_read_all_devs(void)
{
   uint32_t res = RESULT_OK;
   uint32_t i;

   for (i = 0; i < LDC1101_DEVS_CNT; i++)
   {
      if (LDC1101_read_all(i) != RESULT_OK) res = RESULT_ERROR;
   }
   return res;
}

/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void LDC1101_write_all_devs(void)
{
   uint32_t i;

   for (i = 0; i < LDC1101_DEVS_CNT; i++)
   {
      LDC1101_write_all(i);
   }
}

/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void LDC1101_update_all_devs(void)
{

   LDC1101_write_all_devs();
   LDC1101_read_all_devs();
}


/*-----------------------------------------------------------------------------------------------------

 \param void

 \return T_LDC1101_map*
-----------------------------------------------------------------------------------------------------*/
const T_LDC1101_map* LDC1101_get_map(uint32_t ic_num, uint32_t  *psz)
{
   if (psz != 0) *psz = LDC1101_REGS_CNT;
   return LDC1101_map[ic_num];
}

/*-----------------------------------------------------------------------------------------------------

 \param ic_num

 \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_cs(uint32_t ic_num)
{
   return ic[ic_num].cs;
}

/*-----------------------------------------------------------------------------------------------------

 \param addr
 \param pval
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_read_register(uint32_t chip_cs, uint8_t addr, uint8_t *pval)
{
   uint16_t cmdw;
   uint32_t res;

   cmdw = BIT(15);
   cmdw |= ((addr & 0x7F) << 8);

   SPI_push_data(LDC1101_SPI, chip_cs, cmdw);
   res = SPI_wait_tx_complete(LDC1101_SPI, pval, 10);
   if (res != RESULT_OK) g_spi_rd_err++;
   return res;
}


/*-----------------------------------------------------------------------------------------------------

 \param addr
 \param pval
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_write_register(uint32_t chip_cs, uint8_t addr, uint8_t val)
{
   uint16_t cmdw;
   uint32_t res;

   cmdw = ((addr & 0x7F) << 8);
   cmdw |=  val & 0xFF;

   SPI_push_data(LDC1101_SPI, chip_cs, cmdw);
   res = SPI_wait_tx_complete(LDC1101_SPI, 0, 10);
   if (res != RESULT_OK) g_spi_wr_err++;
   return res;
}

/*-----------------------------------------------------------------------------------------------------

 \param addr
 \param pval
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_read_register_DMA(uint32_t chip_cs, uint8_t addr, uint8_t *pval)
{
   uint32_t rxbuf[1];
   uint32_t txbuf[1];
   uint16_t cmdw;
   uint32_t res   = RESULT_OK;

   cmdw = BIT(15);
   cmdw |= ((addr & 0x7F) << 8);

   txbuf[0] = cmdw | chip_cs;
   if (LDC1101_SPI_write_read_buf(txbuf, rxbuf, 1, 2) != 0)
   {
      g_spi_rd_err++;
      res = RESULT_ERROR;
   }
   *pval = rxbuf[0] & 0x000000FF;
   return res;
}

/*-----------------------------------------------------------------------------------------------------

 \param addr
 \param pval
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_write_register_DMA(uint32_t chip_cs, uint8_t addr, uint8_t val)
{
   uint32_t rxbuf[1];
   uint32_t txbuf[1];
   uint16_t cmdw;
   uint32_t res   = RESULT_OK;;

   cmdw = ((addr & 0x7F) << 8);
   cmdw |=  val & 0xFF;

   txbuf[0] = cmdw | chip_cs;
   if (LDC1101_SPI_write_read_buf(txbuf, rxbuf, 1, 2) != 0)
   {
      g_spi_wr_err++;
      res = RESULT_ERROR;
   }
   return res;
}


/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_read_all(uint32_t ic_num)
{
   uint32_t i;

   LDC1101_pend_mutex();
   for (i = 0; i < LDC1101_REGS_CNT; i++)
   {
      if (LDC1101_read_register_DMA(ic[ic_num].cs, LDC1101_map[ic_num][i].addr, LDC1101_map[ic_num][i].pval) != RESULT_OK)
      {
         LDC1101_post_mutex();
         return RESULT_ERROR;
      }
   }
   LDC1101_post_mutex();
   return RESULT_OK;
}

/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_write_all(uint32_t ic_num)
{
   uint32_t i;
   uint8_t  tmp;

   LDC1101_pend_mutex();

   tmp = LDC1101_regs[ic_num].START_CONFIG;
   // Оснанавливаем работу измерителя записывая 1 в START_CONFIG
   LDC1101_write_register_DMA(ic[ic_num].cs, START_CONFIG_ADDR, 1);
   LDC1101_regs[ic_num].START_CONFIG = 1;

   for (i = 0; i < LDC1101_REGS_CNT; i++)
   {
      if (LDC1101_write_register_DMA(ic[ic_num].cs, LDC1101_map[ic_num][i].addr, *LDC1101_map[ic_num][i].pval) != RESULT_OK)
      {
         LDC1101_post_mutex();
         return RESULT_ERROR;
      }
   }

   // Восстанавливаем работу измерителя
   LDC1101_write_register_DMA(ic[ic_num].cs, START_CONFIG_ADDR, tmp);

   LDC1101_post_mutex();
   return RESULT_OK;
}


/*-----------------------------------------------------------------------------------------------------
 Записать во все регистры инициализационые значения
 \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_write_def_all(uint32_t ic_num)
{
   uint32_t i;

   for (i = 0; i < LDC1101_REGS_CNT; i++)
   {
      if (LDC1101_write_register(ic[ic_num].cs, LDC1101_map[ic_num][i].addr, LDC1101_map[ic_num][i].defv) != RESULT_OK)
      {
         return RESULT_ERROR;
      }
   }
   return RESULT_OK;
}

/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void LDC1101_Gates_enable(void)
{
   PTA->PSOR = BIT(25);
}
/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void LDC1101_Gates_disable(void)
{
   PTA->PCOR = BIT(25);
}


/*-----------------------------------------------------------------------------------------------------

 \param void

 \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_read_all_LHR_DATA(void)
{
   uint32_t n;
   for (n = 0; n < LDC1101_DEVS_CNT; n++)
   {
      lhr_data[n] = LDC1101_read_LHR_DATA(n);
   }
   return 0;
}

/*-----------------------------------------------------------------------------------------------------

 \param void

 \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_read_all_LHR_DATA_DMA(void)
{
   uint32_t n;
   uint32_t dsum = 0;

   LDC1101_pend_mutex();
   for (n = 0; n < LDC1101_DEVS_CNT; n++)
   {
      tx_buf[n * 3 + 0] = BIT(15) | ((LHR_DATA_LSB_ADDR & 0x7F) << 8) | ic[n].cs; // | BIT(31);
      tx_buf[n * 3 + 1] = BIT(15) | ((LHR_DATA_MID_ADDR & 0x7F) << 8) | ic[n].cs; // | BIT(31);
      tx_buf[n * 3 + 2] = BIT(15) | ((LHR_DATA_MSB_ADDR & 0x7F) << 8) | ic[n].cs;
   }

   //ITM_EVENT8(1, 4);
   if (LDC1101_SPI_write_read_buf(tx_buf, rx_buf, LDC1101_DEVS_CNT * 3, 10) != 0)
   {
      g_spi_rd_err++;
      LDC1101_post_mutex();
      //ITM_EVENT8(2, 6);
      return RESULT_ERROR;
   }

   for (n = 0; n < LDC1101_DEVS_CNT; n++)
   {
      lhr_data[n] = ((rx_buf[n * 3 + 2] & 0xFF) << 16) + ((rx_buf[n * 3 + 1] & 0xFF) << 8) + (rx_buf[n * 3 + 0] & 0xFF);
      dsum += lhr_data[n];
   }
   lhr_data_sum = dsum;

   LDC1101_post_mutex();
   //ITM_EVENT8(1, 6);
   return RESULT_OK;
}


/*-----------------------------------------------------------------------------------------------------

 \param ic_num

 \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_read_LHR_DATA(uint32_t ic_num)
{
   uint8_t  lsb;
   uint8_t  mid;
   uint8_t  msb;
   uint32_t res;

   LDC1101_read_register_DMA(ic[ic_num].cs, LHR_DATA_LSB_ADDR, &lsb);
   LDC1101_read_register_DMA(ic[ic_num].cs, LHR_DATA_MID_ADDR, &mid);
   LDC1101_read_register_DMA(ic[ic_num].cs, LHR_DATA_MSB_ADDR, &msb);

   res = (msb << 16) + (mid << 8) + lsb;
   return res;
}

/*-----------------------------------------------------------------------------------------------------

 \param ic_num

 \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_update_RCOUNT_OFFS(uint32_t ic_num, uint16_t rcount, uint16_t offset)
{
   // Оснанавливаем работу измерителя записывая 1 в START_CONFIG
   LDC1101_write_register_DMA(ic[ic_num].cs, START_CONFIG_ADDR, 1);

   LDC1101_write_register_DMA(ic[ic_num].cs, D_CONF_ADDR,          0x01); // LDC continues to convert even if sensor amplitude is unable to maintain regulation.
   LDC1101_write_register_DMA(ic[ic_num].cs, ALT_CONFIG_ADDR,      0x01); // Only perform LHR and/or L-only measurements. RP measurements are invalid.
   LDC1101_write_register_DMA(ic[ic_num].cs, LHR_CONFIG_ADDR,      0x01); // Sensor Frequency divided by 2

   // Последняя микросхема в схеме генерирует прерывания
   if (ic_num == LDC1101_DEVS_CNT - 1)
   {
      LDC1101_write_register_DMA(ic[ic_num].cs, INTB_MODE_ADDR,       0xA0); // Report INTB on SDO pin, b100000: -Report LHR Data Ready
   }


   LDC1101_write_register_DMA(ic[ic_num].cs, LHR_RCOUNT_LSB_ADDR,  rcount       & 0xFF);
   LDC1101_write_register_DMA(ic[ic_num].cs, LHR_RCOUNT_MSB_ADDR, (rcount >> 8) & 0xFF);
   LDC1101_write_register_DMA(ic[ic_num].cs, LHR_OFFSET_LSB_ADDR,  offset       & 0xFF);
   LDC1101_write_register_DMA(ic[ic_num].cs, LHR_OFFSET_MSB_ADDR, (offset >> 8) & 0xFF);

   // Стартуем работу измерителя записывая 0 в START_CONFIG
   LDC1101_write_register_DMA(ic[ic_num].cs, START_CONFIG_ADDR, 0);

   return 0;
}


/*-----------------------------------------------------------------------------------------------------

 \param void

 \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_update_RCOUNT_OFFS_for_All(void)
{
   uint32_t n;

   LDC1101_pend_mutex();
   for (n = 0; n < LDC1101_DEVS_CNT; n++)
   {
      LDC1101_update_RCOUNT_OFFS(n, cdata.rcount, cdata.offset);
   }
   g_angle_smpl_period   = Get_angle_smpl_time();
   g_fmstr_smpls_period  = g_angle_smpl_period;
   LDC1101_post_mutex();
   return 0;
}


/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void LDC1101_config_interupts(void)
{
   NVIC_SetPriority(PORTD_IRQn, 3);
}

/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void LDC1101_prepare_to_wait_rdy(uint32_t cs_line_num)
{

   PORTD->ISFR = 0xFFFFFFFF;         // Сбросим флаги прерываний на порту D
   NVIC_ClearPendingIRQ(PORTD_IRQn); // Сбросим запрос на прерывание в NVIC от порта D

   // Установим свойство линии PTD3 (линия SPI0_SIN на схеме ANSIN v0.1) для генерации прерываний
   portd_pcr3 = PORTD->PCR[3];
   PORTD->PCR[3] = (portd_pcr3 & ~LSHIFT(0x0F,   16)) | LSHIFT(IRQ_IFE,   16);

   // Установим свойство линии PTD6 (один из сигналов SPI0_CSx чипов  LDC1101 на схеме ANSIN v0.1)  как выход с низким уровнем
   portd_pcr_cs = PORTD->PCR[cs_line_num];
   PORTD->PCR[cs_line_num] = LSHIFT(IRQ_DIS,   16) |
      LSHIFT(0,         15) |
      LSHIFT(GPIO,      8) |
      LSHIFT(DSE_HI,    6) |
      LSHIFT(OD_DIS,    5) |
      LSHIFT(PFE_DIS,   4) |
      LSHIFT(FAST_SLEW, 2) |
      LSHIFT(PUPD_DIS,  0);

   GPIOD->PDDR = (GPIOD->PDDR & ~LSHIFT(1, cs_line_num)) | LSHIFT(GP_OUT, cs_line_num);
   GPIOD->PCOR = LSHIFT(1, cs_line_num);
   NVIC_EnableIRQ(PORTD_IRQn);
}

/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void LDC1101_restore_from_rdy(uint32_t cs_line_num)
{
   PORTD->PCR[cs_line_num] = portd_pcr_cs;
   GPIOD->PDDR = (GPIOD->PDDR & ~LSHIFT(1, cs_line_num)) | LSHIFT(GP_INP, cs_line_num);
   GPIOD->PSOR = LSHIFT(1, cs_line_num);
   PORTD->PCR[3] = portd_pcr3;
   NVIC_DisableIRQ(PORTD_IRQn);
}

/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void PORTD_IRQHandler(void)
{
   CPU_SR_ALLOC();
   OS_ERR      err;

   CPU_CRITICAL_ENTER();
   OSIntEnter();                                               /* Tell uC/OS-III that we are starting an ISR           */
   CPU_CRITICAL_EXIT();

   //ITM_EVENT8(1, 1);

   PORTD->ISFR = 0xFFFFFFFF; // Сбросим флаг прерывания
   OSFlagPost(&ldc1101_flags, LDC1101_READY, OS_OPT_POST_FLAG_SET, &err);

   if (g_fmstr_rate_src == FMSTR_SMPS_PORTD)
   {
      FMSTR_Recorder(); // Вызываем функцию записи сигнала для инструмента FreeMaster
   }


   OSIntExit();
}

/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void LDC1101_wait_results(void)
{
   OS_ERR      err;
   OSFlagPend(&ldc1101_flags, (OS_FLAGS)LDC1101_READY, 0, OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME, (CPU_TS *)0U, &err);
   //ITM_EVENT8(1, 2);
}



/*------------------------------------------------------------------------------
  Обработчик прерывания от модуля DMA по завершению чтения данных из SPI связанного с LDC1101

 \param user_isr_ptr
 ------------------------------------------------------------------------------*/
void LDC1101_DMA_ISR(void)
{
   CPU_SR_ALLOC();

   OS_ERR      err;

   CPU_CRITICAL_ENTER();
   OSIntEnter();                                               /* Tell uC/OS-III that we are starting an ISR           */
   CPU_CRITICAL_EXIT();

   //ITM_EVENT8(1, 5);

   DMA0->INT = BIT(DMA_LDC1101_FM_CH); // Сбрасываем флаг прерываний  канала

   // Очистим FIFO приемника и передатчика
   spi_mods[LDC1101_SPI].spi->MCR  |= BIT(CLR_RXF) + BIT(CLR_TXF);
   // Сбросим все флаги у SPI
   spi_mods[LDC1101_SPI].spi->SR =  spi_mods[LDC1101_SPI].spi->SR;


   OSFlagPost(&spi_cbl[LDC1101_SPI].spi_flags, SPI_TX_COMPLETE, OS_OPT_POST_FLAG_SET, &err);

   OSIntExit();
}


/*------------------------------------------------------------------------------
  Конфигурирование 2-х каналов DMA на прием и на передачу для работы с модулем SPI

 ------------------------------------------------------------------------------*/
static void _LDC1101_Init_SPI_DMA(void)
{
   T_DMA_SPI_RX_config rx_cfg;
   T_DMA_SPI_TX_config tx_cfg;

   tx_cfg.ch             = DMA_LDC1101_MF_CH;                              // номер канала DMA
   tx_cfg.spi_pushr      = (uint32_t)&(spi_mods[LDC1101_SPI].spi->PUSHR);  // адрес регистра PUSHR SPI
   tx_cfg.minor_tranf_sz = DMA_4BYTE_MINOR_TRANSFER;                       // номер входа периферии для выбранного мультиплексора DMAMUX для передачи на DMA.
   tx_cfg.int_en         = 0;

   Config_DMA_for_SPI_TX(&tx_cfg, &DS_cbl);
   Set_DMA_MUX_chanel(DMAMUX, DMA_LDC1101_DMUX_TX_SRC, DMA_LDC1101_MF_CH);


   rx_cfg.ch             = DMA_LDC1101_FM_CH;                              // номер канала DMA
   rx_cfg.spi_popr       = (uint32_t)&(spi_mods[LDC1101_SPI].spi->POPR);   // адрес регистра POPR SPI
   rx_cfg.minor_tranf_sz = DMA_4BYTE_MINOR_TRANSFER;                       // номер входа периферии для выбранного мультиплексора DMAMUX для передачи на DMA.
   rx_cfg.int_en         = 1;

   Config_DMA_for_SPI_RX(&rx_cfg, &DS_cbl);
   Set_DMA_MUX_chanel(DMAMUX, DMA_LDC1101_DMUX_RX_SRC, DMA_LDC1101_FM_CH);

   NVIC_SetPriority((IRQn_Type)DMA_LDC1101_RX_INT_NUM, LDC1101_DMA_SPI_PRIO);
   NVIC_EnableIRQ((IRQn_Type)DMA_LDC1101_RX_INT_NUM);
}


/*-----------------------------------------------------------------------------------------------------

 \param void
-----------------------------------------------------------------------------------------------------*/
void LDC1101_Init_communication(void)
{
   SPI_master_init(LDC1101_SPI, SPI_16_BITS, 0, 0, SPI_BAUD_6MHZ, 0); // Инициализация интерфейса к чипам LDC1101, без прерываний, обмен по DMA
   _LDC1101_Init_SPI_DMA();   // Инициализируем структуры и прерываний DMA для обмена по SPI с чипами LDC1101
}

/*-----------------------------------------------------------------------------------------------------
  Прием данных по SPI с использованием DMA

 \param wbuff  - буфер с высылаемыми данными
 \param rbuff  - буфер с принимаемыми данными
 \param sz     - количество мажорных пересылок, в данном случае количество 32-х битных слов в буфере
 \param ticks

 \return uint32_t Возвращает не 0 если произошла ошибка
-----------------------------------------------------------------------------------------------------*/
uint32_t LDC1101_SPI_write_read_buf(uint32_t *wbuff,  uint32_t *rbuff, uint32_t sz, uint32_t ticks)
{
   uint32_t res = 0;
   uint32_t s;
   OS_ERR   err;


   while (sz > 0)
   {
      if (sz > MAX_DMA_SPI_BUFF) s = MAX_DMA_SPI_BUFF;
      else s = sz;

      Start_DMA_for_SPI_RXTX(&DS_cbl, wbuff, rbuff, s);
      // Ожидаем флага окончания передачи буфера по DMA
      OSFlagPend(&spi_cbl[LDC1101_SPI].spi_flags, (OS_FLAGS)SPI_TX_COMPLETE, ticks, OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME, (CPU_TS *)0U, &err);

      if (err != OS_ERR_NONE)
      {
         spi_cbl[LDC1101_SPI].tx_err_cnt++;
         res = 1;
      }
      wbuff = wbuff + s;
      sz -= s;
   }

   return res;
}


#ifndef __ANSIN01_LDC1101_H
#define __ANSIN01_LDC1101_H


#define  LDC1101_READY  BIT(0)  // Флаг готовности результата 

#ifdef COIL_RESEARCH
#define  LDC1101_DEVS_CNT 1
#else
#define  LDC1101_DEVS_CNT 4
#endif

#define  LDC1101_REGS_CNT 32

typedef struct
{
  uint8_t  RP_SET;              // Configure RP Measurement Dynamic Range
  uint8_t  TC1;                 // Configure Internal Time Constant 1
  uint8_t  TC2;                 // Configure Internal Time Constant 2
  uint8_t  DIG_CONFIG;          // Configure RP+L conversion interval
  uint8_t  ALT_CONFIG;          // Configure additional device settings
  uint8_t  RP_THRESH_H_LSB;     // RP_THRESHOLD High Setting – bits 7:0
  uint8_t  RP_THRESH_H_MSB;     // RP_THRESHOLD High Setting – bits 15:8
  uint8_t  RP_THRESH_L_LSB;     // RP_THRESHOLD Low Setting – bits 7:0
  uint8_t  RP_THRESH_L_MSB;     // RP_THRESHOLD Low Setting – bits 15:8
  uint8_t  INTB_MODE;           // Configure INTB reporting on SDO pin
  uint8_t  START_CONFIG;        // Configure Power State
  uint8_t  D_CONF;              // Sensor Amplitude Control Requirement
  uint8_t  L_THRESH_HI_LSB;     // L_THRESHOLD High Setting – bits 7:0
  uint8_t  L_THRESH_HI_MSB;     // L_THRESHOLD High Setting – bits 15:8
  uint8_t  L_THRESH_LO_LSB;     // L_THRESHOLD Low Setting – bits 7:0
  uint8_t  L_THRESH_LO_MSB;     // L_THRESHOLD Low Setting – bits 15:8
  uint8_t  STATUS;              // Report RP+L measurement status
  uint8_t  RP_DATA_LSB;         // RP Conversion Result Data Output - bits 7:0
  uint8_t  RP_DATA_MSB;         // RP Conversion Result Data Output - bits 15:8
  uint8_t  L_DATA_LSB;          // L Conversion Result Data Output - bits 7:0
  uint8_t  L_DATA_MSB;          // L Conversion Result Data Output - bits 15:8
  uint8_t  LHR_RCOUNT_LSB;      // High Resolution L Reference Count – bits 7:0
  uint8_t  LHR_RCOUNT_MSB;      // High Resolution L Reference Count – bits 15:8
  uint8_t  LHR_OFFSET_LSB;      // High Resolution L Offset – bits 7:0
  uint8_t  LHR_OFFSET_MSB;      // High Resolution L Offset – bits 15:8
  uint8_t  LHR_CONFIG;          // High Resolution L Configuration
  uint8_t  LHR_DATA_LSB;        // High Resolution L Conversion Result Data output - bits 7:0
  uint8_t  LHR_DATA_MID;        // High Resolution L Conversion Result Data output - bits 15:8
  uint8_t  LHR_DATA_MSB;        // High Resolution L Conversion Result Data output - bits 23:16
  uint8_t  LHR_STATUS;          // High Resolution L Measurement Status
  uint8_t  RID;                 // Device RID value
  uint8_t  CHIP_ID;             // Device ID value

} T_LDC1101_registers;


typedef struct
{
  uint8_t       addr;   // Адрес регистра в чипе 
  const char    *vname; // Имя переменной
  uint8_t       *pval;  // Указатель на переменную содержащую  значение регистра 
  uint8_t       defv;   // Инициализационная сонстанта
} T_LDC1101_map;

typedef struct
{
  uint32_t cs;

} T_LDC1101_chip_cs;


void           LDC1101_Init_communication(void);

uint32_t       LDC1101_cs(uint32_t ic_num);
uint32_t       LDC1101_read_all_devs(void);
void           LDC1101_write_all_devs(void);
void           LDC1101_update_all_devs(void);
uint32_t       LDC1101_read_register(uint32_t chip_cs, uint8_t addr, uint8_t *pval);
uint32_t       LDC1101_read_register_DMA(uint32_t chip_cs, uint8_t addr, uint8_t *pval);
uint32_t       LDC1101_write_register(uint32_t chip_cs, uint8_t addr, uint8_t val);
uint32_t       LDC1101_write_register_DMA(uint32_t chip_cs, uint8_t addr, uint8_t val);
uint32_t       LDC1101_read_all(uint32_t ic_num);
uint32_t       LDC1101_write_all(uint32_t ic_num);
uint32_t       LDC1101_write_def_all(uint32_t ic_num);
const T_LDC1101_map* LDC1101_get_map(uint32_t ic_num, uint32_t  *psz);


void           LDC1101_Gates_enable(void);
void           LDC1101_Gates_disable(void);
               
void           LDC1101_init_sync_obj(void);
void           LDC1101_pend_mutex(void);
void           LDC1101_post_mutex(void);
uint32_t       LDC1101_read_LHR_DATA(uint32_t ic_num);
uint32_t       LDC1101_read_all_LHR_DATA(void);
uint32_t       LDC1101_read_all_LHR_DATA_DMA(void);
uint32_t       LDC1101_update_RCOUNT_OFFS(uint32_t ic_num, uint16_t rcount, uint16_t offset);
uint32_t       LDC1101_update_RCOUNT_OFFS_for_All(void);
void           LDC1101_config_interupts(void);
void           LDC1101_prepare_to_wait_rdy(uint32_t cs_line_num);
void           LDC1101_restore_from_rdy(uint32_t cs_line_num);
void           LDC1101_wait_results(void);
               
uint32_t       LDC1101_SPI_write_read_buf( uint32_t *wbuff,  uint32_t *rbuff, uint32_t sz, uint32_t ticks);

#endif // ANSIN01_LDC1101_H




#ifndef ANSIN01_FLASH_H
#define ANSIN01_FLASH_H

// Микроконтроллер MKS22FN256VFT12 имеет 256 КБ Flash и 64 КБ RAM
#define FLASH_SECT_SIZE    2048  // Размер стираемого сегмента.

/* error code */
#define FLASH_OK           0x00
#define FLASH_FACCERR      0x01
#define FLASH_FPVIOL       0x02
#define FLASH_MGSTAT0      0x04
#define FLASH_RDCOLERR     0x08
#define FLASH_NOT_ERASED   0x10
#define FLASH_CONTENTERR   0x11
#define FLASH_PROGRERR     0x12

/* flash commands */
#define FLASH_CMD_PROGR_LWORD   0x06
#define FLASH_CMD_SECTOR_ERASE  0x09


void           Flash_reset_errors(void);
uint32_t       Flash_Erase_Sector(uint32_t addr);
uint32_t       Flash_Erase_Area(uint32_t addr, uint32_t sz);
uint32_t       Flash_program_buf(uint32_t addr, uint8_t *buff, uint32_t sz, uint32_t verify_en);
uint32_t       Flash_Read_IFR_ID(uint32_t addr, uint8_t resource_code, uint32_t *val);
uint32_t       Flash_check_of_clean(uint32_t addr, uint8_t margine, uint32_t n_of_phrases);
__ramfunc void FLASH_exec_cmd(void);




#endif // ANSIN01_FLASH_H




// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017-03-09
// 10:47:19
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "App.h"

#define FSTAT_CCIF_BIT     BIT(7)  // Command Complete Interrupt Flag
#define FSTAT_RDCOLERR_BIT BIT(6)  // Flash Read Collision Error Flag
#define FSTAT_ACCERR_BIT   BIT(5)  // Flash Access Error Flag
#define FSTAT_FPVIOL_BIT   BIT(4)  // Flash Protection Violation Flag
#define FSTAT_MGSTAT0_BIT  BIT(0)  // Memory Controller Command Completion Status Flag
/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void Flash_reset_errors(void)
{
  // Сбрасываем ошибки
  if (FTFA->FSTAT & (FSTAT_ACCERR_BIT | FSTAT_FPVIOL_BIT | FSTAT_RDCOLERR_BIT))
  {
    FTFA->FSTAT |= (FSTAT_ACCERR_BIT | FSTAT_FPVIOL_BIT | FSTAT_RDCOLERR_BIT);
  }
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
static uint32_t Flash_get_error_code(void)
{
  uint32_t ret = FLASH_OK;

  if (FTFA->FSTAT & FSTAT_ACCERR_BIT)
  {
    FTFA->FSTAT |= FSTAT_ACCERR_BIT;
    ret |= FLASH_FACCERR;
  }
  else if (FTFA->FSTAT & FSTAT_FPVIOL_BIT)
  {
    FTFA->FSTAT |= FSTAT_FPVIOL_BIT;
    ret |= FLASH_FPVIOL;
  }
  else if (FTFA->FSTAT & FSTAT_RDCOLERR_BIT)
  {
    FTFA->FSTAT |= FSTAT_RDCOLERR_BIT;
    ret |= FLASH_RDCOLERR;
  }
  else if (FTFA->FSTAT & FSTAT_MGSTAT0_BIT)
  {
    ret |= FLASH_MGSTAT0;
  }
  return ret;
}
/*-----------------------------------------------------------------------------------------------------
  Стирание сектора Flash
 
 
 \param addr  - адрес в пространстве стираемого сектора 
 
 \return unsigned char 
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_Erase_Sector(uint32_t addr)
{

  while (!(FTFA->FSTAT & FSTAT_CCIF_BIT))  // Ждем готовности к записи в регистры
  {__no_operation();}
  Flash_reset_errors();

  FTFA->FCCOB0 = FLASH_CMD_SECTOR_ERASE;
  FTFA->FCCOB1 = (uint8_t)(addr >> 16);
  FTFA->FCCOB2 = (uint8_t)((addr >> 8) & 0xFF);
  FTFA->FCCOB3 = (uint8_t)(addr & 0xFF);

  FLASH_exec_cmd();  // Выполняем командк
  return  Flash_get_error_code();
}


/*-----------------------------------------------------------------------------------------------------
  Стирание прсекторное области во  Flash
  Пользователь должен учитывать, что стирание будет посекторным и можеть быть стерто болше чем указано в параметре sz 
 
 \param addr - адрес начала области 
 \param sz   - размер области
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_Erase_Area(uint32_t addr, uint32_t sz)
{
  uint32_t res;
  uint32_t endadr = addr + sz - 1;
  do
  {
    res = Flash_Erase_Sector(addr);
    if (res != FLASH_OK)
    {
      return res;
    }
    addr += FLASH_SECT_SIZE;
    if (addr > endadr)
    {
      return FLASH_OK;
    }
  }
  while (1);

}

/*-----------------------------------------------------------------------------------------------------
 byte program the flash
 
 \param addr 
 \param buff 
 \param sz 
 \param verify_en : 1 - выполнять проверку 
 
 \return unsigned char 
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_program_buf(uint32_t addr, uint8_t *buff, uint32_t sz, uint32_t verify_en)
{
  uint32_t res = FLASH_OK;
  uint8_t  b1,
           b2,
           b3,
           b4;

  if (sz == 0) return FLASH_CONTENTERR;


  while (!(FTFA->FSTAT & FSTAT_CCIF_BIT))  // Ждем готовности к записи в регистры
  {__no_operation();}
  Flash_reset_errors();

  while ((sz != 0) && (res == FLASH_OK))
  {
    b1 = *(buff + 0);
    sz--;
    if (sz != 0)
    {
      b2 = *(buff + 1);
      sz--;
      if (sz != 0)
      {
        b3 = *(buff + 2);
        sz--;
        if (sz != 0)
        {
          b4 = *(buff + 3);
          sz--;
        }
        else
        {
          b4 = 0xFF;
        }
      }
      else
      {
        b3 = 0xFF;
        b4 = 0xFF;
      }
    }
    else
    {
      b2 = 0xFF;
      b3 = 0xFF;
      b4 = 0xFF;
    }

    FTFA->FCCOB0 = FLASH_CMD_PROGR_LWORD;
    FTFA->FCCOB1 = (uint8_t)(addr >> 16);
    FTFA->FCCOB2 = (uint8_t)((addr >> 8) & 0xFF);
    FTFA->FCCOB3 = (uint8_t)(addr & 0xFF);
    FTFA->FCCOB7 = b1;
    FTFA->FCCOB6 = b2;
    FTFA->FCCOB5 = b3;
    FTFA->FCCOB4 = b4;

    FLASH_exec_cmd();
    res = Flash_get_error_code();
    if (res != FLASH_OK) break;

    if (verify_en != 0)
    {
      FTFA->FCCOB0 = 0x02; // Program Check Command
      FTFA->FCCOB1 = (uint8_t)(addr >> 16);
      FTFA->FCCOB2 = (uint8_t)((addr >> 8) & 0xFF);
      FTFA->FCCOB3 = (uint8_t)(addr & 0xFF);
      FTFA->FCCOB4 = 0x01; // Read at 'User' margin-1 and 'User' margin-0
      FTFA->FCCOBB = b1;
      FTFA->FCCOBA = b2;
      FTFA->FCCOB9 = b3;
      FTFA->FCCOB8 = b4;

      FLASH_exec_cmd();
      res = Flash_get_error_code();
      if (res != FLASH_OK)
      {
        if (res == FLASH_MGSTAT0) res = FLASH_PROGRERR;
        break;
      }
    }

    buff += 4;
    addr += 4;
  }
  return  res;
}

/*-----------------------------------------------------------------------------------------------------
  The Read Resource command allows the user to read data from special-purpose memory resources located within the flash memory module.
  The special-purpose memory resources available include program flash IFR space and the Version ID field.
 
 \param addr 
 \param resource_code 
 \param val 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_Read_IFR_ID(uint32_t addr, uint8_t resource_code, uint32_t *val)
{
  uint32_t res = FLASH_OK;
  uint32_t v;

  while (!(FTFA->FSTAT & FSTAT_CCIF_BIT))  // Ждем готовности к записи в регистры
  {__no_operation();}
  Flash_reset_errors();

  FTFA->FCCOB0 = 0x03;  // Команда чтения ресурса
  FTFA->FCCOB1 = (uint8_t)(addr >> 16);
  FTFA->FCCOB2 = (uint8_t)((addr >> 8) & 0xFF);
  FTFA->FCCOB3 = (uint8_t)(addr & 0xFF);
  FTFA->FCCOB8 = resource_code;

  FLASH_exec_cmd();
  res = Flash_get_error_code();

  if (res == FLASH_OK)
  {
    v = FTFA->FCCOB7;
    v |= (FTFA->FCCOB6 << 8);
    v |= (FTFA->FCCOB5 << 16);
    v |= (FTFA->FCCOB4 << 24);
    *val = v;
  }
  return  res;
}

/*-----------------------------------------------------------------------------------------------------
  The Read 1s Section command checks if a section of program flash memory is erased to the specified read margin level.
 
 \param addr: адрес должен быть выровнен по границе 8
 \param margine: 0- normal margine, 1 - User margine, 2-  Factory margine
 \param n_of_phrases: количество проверяемых фраз. Одна фраза это 8 байт 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t Flash_check_of_clean(uint32_t addr, uint8_t margine, uint32_t n_of_phrases)
{
  uint32_t    res = FLASH_OK;

  while (!(FTFA->FSTAT & FSTAT_CCIF_BIT))  // Ждем готовности к записи в регистры
  {__no_operation();}
  Flash_reset_errors();

  FTFA->FCCOB0 = 0x01;  // Команда проверки секции на чистоту
  FTFA->FCCOB1 = (uint8_t)(addr >> 16);
  FTFA->FCCOB2 = (uint8_t)((addr >> 8) & 0xFF);
  FTFA->FCCOB3 = (uint8_t)(addr & 0xFF);
  FTFA->FCCOB4 = (uint8_t)((n_of_phrases >> 8) & 0xFF);
  FTFA->FCCOB5 = (uint8_t)((n_of_phrases >> 0) & 0xFF);
  FTFA->FCCOB6 = margine;

  FLASH_exec_cmd();
  res = Flash_get_error_code();

  return  res;
}

/*-----------------------------------------------------------------------------------------------------
  Иницируем выполнение команды
 
 \param void 
 
 \return __ramfunc void 
-----------------------------------------------------------------------------------------------------*/
__ramfunc void FLASH_exec_cmd(void)
{
  uint8_t  tmp;

  __disable_interrupt();
  tmp = FTFA->FSTAT;
  tmp = tmp | BIT(7);  // Запись бита CCIF = 1 приводит к старту выпонения предварительно заданной команды
  FTFA->FSTAT = tmp;

  while (!(FTFA->FSTAT & BIT(7))) // После записи CCIF = 1 при старте этот бит сбрасывается в 0 и снова устанавливается в 1 после окончания выпонения команды
  {
    __no_operation();
  };
  __enable_interrupt();
}


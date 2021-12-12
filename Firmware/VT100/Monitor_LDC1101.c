// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-24
// 20:01:43
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


/*-----------------------------------------------------------------------------------------------------
 
 \param keycode 
-----------------------------------------------------------------------------------------------------*/
void Do_LDC1101_diagnostic(uint8_t keycode)
{
  const T_LDC1101_map *drvmap;
  uint32_t            sz;
  uint32_t            devn    = 0;
  uint32_t            i;
  uint32_t            itemn;
  uint32_t            row;
  uint32_t            val;

  uint8_t             b;
  T_monitor_cbl       *mcbl;
  OS_ERR              err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0, &err);

  itemn  = 0;


  do
  {

    row = 0;
    mcbl->_printf("\033[%.2d;%.2dH", row, 0);
    mcbl->_printf("LDC1101 diagnostic.\n\r"); row++;
    mcbl->_printf("Press 'R' to exit. 'E' - edit, '<''>' - Up and Down selection \n\r"); row++;
    mcbl->_printf("\n\r"); row++;


    LDC1101_read_all(devn);
    drvmap = LDC1101_get_map(devn, &sz);
    mcbl->_printf("Devise %d\n\r", devn); row++;

    for (i = 0; i < sz; i++)
    {
      mcbl->_printf(VT100_CLR_LINE);
      if (itemn == i) mcbl->_printf("> ");
      else mcbl->_printf("  ");

      mcbl->_printf("%s = %04X\n\r", drvmap[i].vname, *drvmap[i].pval);
      row++;
    }

    mcbl->_printf("\n\r Communication errors counter =%d\n\r", spi_cbl[LDC1101_SPI].tx_err_cnt);
    row++;
    row++;

    if (mcbl->_wait_char(&b, 100) == RESULT_OK)
    {
      switch (b)
      {
      case 'M':
      case 'm':
        devn++;
        if (devn >= 4) devn = 0;
        break;
      case 'N':
      case 'n':
        if (devn == 0) devn = 3;
        else devn--;
        break;

      case '>':
      case '.':
        itemn++;
        if (itemn >= sz) itemn = 0;
        break;
      case '<':
      case ',':
        if (itemn == 0) itemn = sz - 1;
        else itemn--;
        break;
      case 'E':
      case 'e':
        {
          uint32_t tmpr = row;
          mcbl->_printf("\n\r"); row++;
          mcbl->_printf("Register [%s] editing:\n\r", drvmap[itemn].vname); row++;
          val = *drvmap[itemn].pval;
          if (Edit_integer_hex_val(row + 1, &val, 0, 0x7FF) == RESULT_OK)
          {
            if (LDC1101_write_register_DMA(LDC1101_cs(devn),  drvmap[itemn].addr, val) != RESULT_OK)
            {
              mcbl->_printf("Write error!\n\r");
              _time_delay(1000);
            }
          }
          mcbl->_printf("\033[%.2d;%.2dH", tmpr, 0);
          mcbl->_printf(VT100_CLR_FM_CRSR);
        }
        break;
      case 'R':
      case 'r':
        return;
      }
    }

  }
  while (1);


}

/*-----------------------------------------------------------------------------------------------------
 
 \param keycode 
-----------------------------------------------------------------------------------------------------*/
void     Do_TLI5012_diagnostic(uint8_t keycode)
{

  uint8_t             b;
  T_monitor_cbl       *mcbl;
  OS_ERR              err;

  uint32_t            row;

  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0, &err);

  do
  {
    row = 0;
    mcbl->_printf("\033[%.2d;%.2dH", row, 0);
    mcbl->_printf("TLI5012 diagnostic.\n\r"); row++;
    mcbl->_printf("Press 'R' to exit. 'E' - edit, '<''>' - Up and Down selection \n\r"); row++;
    mcbl->_printf("\n\r"); row++;



    if (mcbl->_wait_char(&b, 100) == RESULT_OK)
    {
      switch (b)
      {
      case 'R':
      case 'r':
        return;
      }
    }

  }
  while (1);

}

/*-----------------------------------------------------------------------------------------------------
 
 \param keycode 
-----------------------------------------------------------------------------------------------------*/
void Do_FlashMemoryDiagnostic(uint8_t keycode)
{
  uint8_t       b;
  uint32_t      i;
  uint32_t      res;
  T_monitor_cbl *mcbl;
  OS_ERR        err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0, &err);



  do
  {
    mcbl->_printf(VT100_CLEAR_AND_HOME"uC Flash diagnostic.\n\r");
    mcbl->_printf("Press 'R' to exit. \n\r");
    mcbl->_printf("------------------------------------------------\n\r");


    // Читаем область IFR во Flash
    mcbl->_printf("IFR:\n\r");
    for (i = 0; i < 64; i++)
    {
      uint32_t val;
      res = Flash_Read_IFR_ID(4 * i, 0, &val);
      if (res == FLASH_OK)
      {
        if (((i % 4) == 0) && (i != 0)) mcbl->_printf("\n\r", res);
        mcbl->_printf(" %02X %02X %02X %02X", (val >> 24) & 0xFF, (val >> 16) & 0xFF, (val >> 8) & 0xFF, (val >> 0) & 0xFF);
      }
      else
      {
        mcbl->_printf("IFR read error = %d\n\r", res);
        break;
      }
    }
    mcbl->_printf("\n\r", res);


    // Читаем область Version ID во Flash
    mcbl->_printf("Version ID:\n\r");
    for (i = 0; i < 2; i++)
    {
      uint32_t val;
      res = Flash_Read_IFR_ID(4 * i, 1, &val);
      if (res == FLASH_OK)
      {
        if (((i % 4) == 0) && (i != 0)) mcbl->_printf("\n\r", res);
        mcbl->_printf(" %02X %02X %02X %02X", (val >> 24) & 0xFF, (val >> 16) & 0xFF, (val >> 8) & 0xFF, (val >> 0) & 0xFF);
      }
      else
      {
        mcbl->_printf("Version ID read error = %d\n\r", res);
        break;
      }
    }
    mcbl->_printf("\n\r\n\r", res);

    // Прверяем чистоту области данных во Flash
    {
      uint32_t n = (uint32_t)__DATA_FLASH_SIZE / 8; // Находим количество фраз в области данных
      res = Flash_check_of_clean((uint32_t)__CODE_FLASH_END + 1, 0, n);
      if (res == 0)
      {
        mcbl->_printf("Data area in Flash is BLANK!\n\r", res);
      }
      else if (res == FLASH_MGSTAT0)
      {
        mcbl->_printf("Data area in Flash is NOT blank!\n\r", res);
      }
      else
      {
        mcbl->_printf("Data area in Flash reading error!\n\r", res);
      }
    }



    if (mcbl->_wait_char(&b, 10000) == RESULT_OK)
    {
      switch (b)
      {
      case 'E':
      case 'e':
        {
          // Стираем все сектора области данных
          OS_ERR   err;
          uint32_t saddr = (uint32_t)__CODE_FLASH_END + 1;
          uint32_t endaddr = (uint32_t)__DATA_FLASH_SIZE + (uint32_t)__CODE_FLASH_END;
          while (saddr <= endaddr)
          {
            res = Flash_Erase_Sector(saddr);
            if (res != FLASH_OK)
            {
              mcbl->_printf("\n\rErasing failed! Res=%d\n\r", res);
              break;
            }
            saddr += FLASH_SECT_SIZE;
          }
          if (res == FLASH_OK )
          {
            mcbl->_printf("\n\rErasing was successful!\n\r");
          }
          OSTimeDly(2000, OS_OPT_TIME_DLY, &err);
        }
        break;
      case 'T':
      case 't':
        {
          OS_ERR   err;
          // Проверяем функцию записи
          uint32_t saddr = (uint32_t)__CODE_FLASH_END + 1;
          uint8_t  buf[11]   = {1,2,3,4,5,6,7,8,9,10,11};

          res = Flash_program_buf(saddr, buf, 11, 1);
          if (res == FLASH_OK)
          {
            mcbl->_printf("\n\rProgramming was successful!\n\r");
          }
          else
          {
            mcbl->_printf("\n\rProgramming failed! Res=%d\n\r", res);
          }
          OSTimeDly(2000, OS_OPT_TIME_DLY, &err);
        }
        break;
      case 'R':
      case 'r':
        return;
      }
    }

  }
  while (1);


}


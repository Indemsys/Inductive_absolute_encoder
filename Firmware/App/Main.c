// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-17
// 23:43:51
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define    _GLOBALS_
#include   "App.h"




void Task_app(void *handle);
/*-------------------------------------------------------------------------------------------------------------
   Функция main использует стек по указателю в регистре MSP (SP_main)  ядра ARM Cortex-M4
   Этот же стек используют функции обработчики прерываний 
-------------------------------------------------------------------------------------------------------------*/
int main(void)
{
  OS_ERR   error;

  Init_pins();
  BOARD_BootClockRUN(); // Инициализация подсистемы тактирования. Сгенерирована автоматически иструментом Kinetis Expert Tools
  Init_Flash_Access_Protection_Register();
  Init_PIT_module();
  Init_PIT0(PIT_INT_PERIOD);

  Init_DMA(); // Используем DMA для обмена по SPI

  LDC1101_Init_communication();



  ADC_calibr_config_start();

  RTT_printf("Main application started.\r\n");



  OSInit(&error);

  OSTaskCreate(&task_app_tcb,      /* Task control block handle */
               "App task",         /* Task Name */
               Task_app,           /* pointer to the task */
               0,                  /* Task input parameter */
               TASK_APP_PRIO,      /* Task priority */
               task_app_stack,     /* Task stack base address */
               0U,                 /* Task stack limit */
               TASK_APP_STACK_SZ,  /* Task stack size */
               0U,                 /* Mumber of messages can be sent to the task */
               0U,                 /* Default time_quanta */
               0U,                 /* Task extension */
               OS_OPT_TASK_STK_CHK, /* Task save FP */
               &error);
  if (error != OS_ERR_NONE)
  {
    RTT_printf("VCOM_task create failed!\r\n");
  }

  OS_CPU_SysTickInit(SystemCoreClock);
  CPU_IntSrcPrioSet(CPU_INT_SYSTICK, APP_SYSTICK_PRIO);

  OSStart(&error);
}



/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
uint32_t Load_app_settings(void)
{
  uint16_t crc,
           rcrc;
  uint32_t data_adr = (uint32_t)__CODE_FLASH_END + 1; // Адрес начала области данных приложения во Flash памяти микроконтроллера
  uint32_t data_sz  = (uint32_t)__DATA_FLASH_SIZE;    // Размер области данных приложения во Flash памяти микроконтроллера

  // Проверяем целостность данных
  crc = *(uint32_t *)(data_adr + data_sz - 4);
  rcrc = Get_CRC_of_block((void *)data_adr, data_sz - 4, 0xFFFF);
  if (crc != rcrc)
  {
    g_flash_crc_errors++;
    return RESULT_ERROR;
  }
  memcpy(&cdata, (void *)data_adr, sizeof(cdata));

  return RESULT_OK;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
uint32_t Save_app_settings(void)
{
  uint32_t res;
  uint16_t crc;

  uint32_t data_adr = (uint32_t)__CODE_FLASH_END + 1; // Адрес начала области данных приложения во Flash памяти микроконтроллера
  uint32_t data_sz  = (uint32_t)__DATA_FLASH_SIZE;    // Размер области данных приложения во Flash памяти микроконтроллера

  // Стираем все данные
  if (Flash_Erase_Area(data_adr, data_sz) != FLASH_OK)
  {
    g_flash_erase_errors++;
    return RESULT_ERROR;
  }

  // Записываем данные во Flash с проверкой
  res = Flash_program_buf(data_adr, (uint8_t *)&cdata, sizeof(cdata), 1);
  if (res != FLASH_OK)
  {
    g_flash_write_errors++;
    return RESULT_ERROR;
  }
  // Расчитываем контрольную сумму
  crc = Get_CRC_of_block((void *)data_adr, data_sz - 4, 0xFFFF);
  // Записываем контрольную сумму
  res = Flash_program_buf(data_adr + data_sz - 4, (uint8_t *)&crc, sizeof(crc), 1);
  if (res != FLASH_OK)
  {
    g_flash_write_errors++;
    return RESULT_ERROR;
  }

  return RESULT_OK;

}




/*-----------------------------------------------------------------------------------------------------
 Эта задача и все другие использует стек по указателю в регистре PSP (SP_process)  ядра ARM Cortex-M4
 
 \param handle 
-----------------------------------------------------------------------------------------------------*/
void Task_app(void *handle)
{
  OS_ERR   error;


  #if (OS_CFG_STAT_TASK_EN == DEF_ENABLED)
  OSStatTaskCPUUsageInit(&error);
  #endif


  Set_default_settings();
  Load_app_settings();

  Frmstr_Set_recorder_smpls_src(FMSTR_SMPS_PORTD); // Установим определитель источника данных для recorder-а FreeMaster
  LDC1101_init_sync_obj();
  Init_measur_sync_obj();

  USB_init_device();


  OSTaskCreate(&task_vt100_1_tcb,     /* Task control block handle */
               "VT100_1",             /* Task Name */
               Task_VT100,          /* pointer to the task */
               Mnudrv_get_USB_vcom_driver1(), /* Task input parameter */
               TASK_VT100_PRIO,     /* Task priority */
               task_vt100_1_stack,    /* Task stack base address */
               0U,                  /* Task stack limit */
               TASK_VT100_STACK_SZ, /* Task stack size */
               0U,                  /* Mumber of messages can be sent to the task */
               0U,                  /* Default time_quanta */
               0U,                  /* Task extension */
               OS_OPT_TASK_STK_CHK, /* Task save FP */
               &error);
  if (error != OS_ERR_NONE)
  {
    RTT_printf("VCOM_task create failed!\r\n");
  }

//  OSTaskCreate(&task_vt100_2_tcb,     /* Task control block handle */
//               "VT100_2",             /* Task Name */
//               Task_VT100,          /* pointer to the task */
//               Mnudrv_get_USB_vcom_driver2(), /* Task input parameter */
//               TASK_VT100_PRIO,     /* Task priority */
//               task_vt100_2_stack,    /* Task stack base address */
//               0U,                  /* Task stack limit */
//               TASK_VT100_STACK_SZ, /* Task stack size */
//               0U,                  /* Mumber of messages can be sent to the task */
//               0U,                  /* Default time_quanta */
//               0U,                  /* Task extension */
//               OS_OPT_TASK_STK_CHK, /* Task save FP */
//               &error);
//  if (error != OS_ERR_NONE)
//  {
//    RTT_printf("VCOM_task create failed!\r\n");
//  }

  OSTaskCreate(&task_freem_tcb,    /* Task control block handle */
               "FreeMaster",       /* Task Name */
               Task_FreeMaster,    /* pointer to the task */
               0,                  /* Task input parameter */
               TASK_FREEM_PRIO,    /* Task priority */
               task_freem_stack,   /* Task stack base address */
               0U,                 /* Task stack limit */
               TASK_FREEM_STACK_SZ, /* Task stack size */
               0U,                 /* Mumber of messages can be sent to the task */
               0U,                 /* Default time_quanta */
               0U,                 /* Task extension */
               OS_OPT_TASK_STK_CHK, /* Task save FP */
               &error);
  if (error != OS_ERR_NONE)
  {
    RTT_printf("FreeMaster create failed!\r\n");
  }

  OSTaskCreate(&task_measur_tcb,        /* Task control block handle */
               "Measurement",           /* Task Name */
               Task_Measurement,        /* pointer to the task */
               0,                       /* Task input parameter */
               TASK_MEASUR_PRIO,        /* Task priority */
               task_measur_stack,       /* Task stack base address */
               0U,                      /* Task stack limit */
               TASK_MEASUR_STACK_SZ,    /* Task stack size */
               0U,                      /* Mumber of messages can be sent to the task */
               0U,                      /* Default time_quanta */
               0U,                      /* Task extension */
               OS_OPT_TASK_STK_CHK,     /* Task save FP */
               &error);
  if (error != OS_ERR_NONE)
  {
    RTT_printf("Measurement task create failed!\r\n");
  }

  OSTaskCreate(&task_host_chan_tcb,     /* Task control block handle */
               "Host_channel",          /* Task Name */
               Task_Host_channel,       /* pointer to the task */
               0,                       /* Task input parameter */
               TASK_HOST_CHAN_PRIO,     /* Task priority */
               task_host_chan_stack,    /* Task stack base address */
               0U,                      /* Task stack limit */
               TASK_HOST_CHAN_STACK_SZ, /* Task stack size */
               0U,                      /* Mumber of messages can be sent to the task */
               0U,                      /* Default time_quanta */
               0U,                      /* Task extension */
               OS_OPT_TASK_STK_CHK,     /* Task save FP */
               &error);
  if (error != OS_ERR_NONE)
  {
    RTT_printf("Host channel task create failed!\r\n");
  }


  #if APP_PLATF == ANSIN
  ANSIN_task_function();
  #elif APP_PLATF == ANSMA
  ANSMA_task_function();
  #else
    #error  Must be selected platform
  #endif
}



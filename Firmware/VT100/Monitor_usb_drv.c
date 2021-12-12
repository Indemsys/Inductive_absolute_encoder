// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.06.23
// 11:31:24
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

#define  VCOM_PORT_NUM 2

static int Mnudrv1_init(void **pcbl, void *pdrv);
static int Mnudrv1_send_buf(const void *buf, unsigned int len);
static int Mnudrv1_wait_ch(unsigned char *b, int ticks);
static int Mnudrv1_printf(const char  *fmt_ptr, ...);
static int Mnudrv1_deinit(void *pcbl);

static int Mnudrv2_init(void **pcbl, void *pdrv);
static int Mnudrv2_send_buf(const void *buf, unsigned int len);
static int Mnudrv2_wait_ch(unsigned char *b, int ticks);
static int Mnudrv2_printf(const char  *fmt_ptr, ...);
static int Mnudrv2_deinit(void *pcbl);

static T_monitor_driver vcom_driver1 =
{
  MN_USB_VCOM_DRIVER1,
  Mnudrv1_init,
  Mnudrv1_send_buf,
  Mnudrv1_wait_ch,
  Mnudrv1_printf,
  Mnudrv1_deinit,
};

static T_monitor_driver vcom_driver2 =
{
  MN_USB_VCOM_DRIVER2,
  Mnudrv2_init,
  Mnudrv2_send_buf,
  Mnudrv2_wait_ch,
  Mnudrv2_printf,
  Mnudrv2_deinit,
};

#define MAX_STR_SZ  128
typedef struct
{
  uint8_t          str[MAX_STR_SZ];
} T_vcom_drv_cbl;



/*------------------------------------------------------------------------------
  Инициализация драйвера.
  Вызывается из задачи терминала при ее старте


 \param pcbl  -  указатель на указатель на внутреннюю управляющую структуру драйвера
 \param pdrv   - указатель  на структуру драйвера типа T_monitor_driver

 \return int
 ------------------------------------------------------------------------------*/
static int Mnudrv1_init(void **pcbl, void *pdrv)
{
  uint8_t   b;

  T_vcom_drv_cbl *p;

  // Выделяем память для управляющей структуры драйвера
  p = (T_vcom_drv_cbl *)_mem_alloc_zero(sizeof(T_vcom_drv_cbl));

  if (((T_monitor_driver *)pdrv)->driver_type == MN_USB_VCOM_DRIVER1)
  {
    *pcbl = p; //  Устанавливаем в управляющей структуре драйвера задачи указатель на управляющую структуру драйвера
  }
  else
  {
    //LOG("Incorrect driver type", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    _mem_free(p);
    return RESULT_ERROR;
  }

  // Ждем подключения по  Virtual COM
  return Mnudrv1_wait_ch(&b, INT_MAX);

}

/*------------------------------------------------------------------------------
  Инициализация драйвера.
  Вызывается из задачи терминала при ее старте


 \param pcbl  -  указатель на указатель на внутреннюю управляющую структуру драйвера
 \param pdrv   - указатель  на структуру драйвера типа T_monitor_driver

 \return int
 ------------------------------------------------------------------------------*/
static int Mnudrv2_init(void **pcbl, void *pdrv)
{
  uint8_t   b;

  T_vcom_drv_cbl *p;

  // Выделяем память для управляющей структуры драйвера
  p = (T_vcom_drv_cbl *)_mem_alloc_zero(sizeof(T_vcom_drv_cbl));

  if (((T_monitor_driver *)pdrv)->driver_type == MN_USB_VCOM_DRIVER2)
  {
    *pcbl = p; //  Устанавливаем в управляющей структуре драйвера задачи указатель на управляющую структуру драйвера
  }
  else
  {
    //LOG("Incorrect driver type", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    _mem_free(p);
    return RESULT_ERROR;
  }

  // Ждем подключения по  Virtual COM
  return Mnudrv2_wait_ch(&b, INT_MAX);

}

/*------------------------------------------------------------------------------

 \param pcbl - указатель на внутреннюю управляющую структуру драйвера

 \return int
 ------------------------------------------------------------------------------*/
static int Mnudrv1_deinit(void *pcbl)
{
  // Освобождаем память упраляющей структуры
  _mem_free(pcbl);
  return RESULT_OK;
}

/*------------------------------------------------------------------------------

 \param pcbl - указатель на внутреннюю управляющую структуру драйвера

 \return int
 ------------------------------------------------------------------------------*/
static int Mnudrv2_deinit(void *pcbl)
{
  // Освобождаем память упраляющей структуры
  _mem_free(pcbl);
  return RESULT_OK;
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
T_monitor_driver* Mnudrv_get_USB_vcom_driver1(void)
{
  return &vcom_driver1;
}
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
T_monitor_driver* Mnudrv_get_USB_vcom_driver2(void)
{
  return &vcom_driver2;
}

/*-------------------------------------------------------------------------------------------------------------
  Вывод форматированной строки в коммуникационный канал порта
-------------------------------------------------------------------------------------------------------------*/
static int Mnudrv1_send_buf(const void *buf, unsigned int len)
{
  uint32_t res;

  res = Virtual_com1_send_data((uint8_t *)buf, len, 2);

  if (res == RESULT_OK) Virtual_com1_wait_sending_end(100);
  
  return res;
}
/*-------------------------------------------------------------------------------------------------------------
  Вывод форматированной строки в коммуникационный канал порта
-------------------------------------------------------------------------------------------------------------*/
static int Mnudrv2_send_buf(const void *buf, unsigned int len)
{
  uint32_t res;

  res = Virtual_com2_send_data((uint8_t *)buf, len, 100);

  if (res == RESULT_OK) Virtual_com2_wait_sending_end(100);
  
  return res;
}

/*------------------------------------------------------------------------------

 \param b
 \param timeout

 \return int возвращает RESULT__OK в случае состоявшегося приема байта
 ------------------------------------------------------------------------------*/
static int Mnudrv1_wait_ch(unsigned char *b, int ticks)
{
  return Virtual_com1_get_data(b, ticks);
}
/*------------------------------------------------------------------------------

 \param b
 \param timeout

 \return int возвращает RESULT__OK в случае состоявшегося приема байта
 ------------------------------------------------------------------------------*/
static int Mnudrv2_wait_ch(unsigned char *b, int ticks)
{
  return Virtual_com2_get_data(b, ticks);
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static int Mnudrv1_printf(const char  *fmt_ptr, ...)
{
  int32_t            res;
  va_list            ap;
  T_vcom_drv_cbl     *p;
  T_monitor_cbl      *mcbl;
  OS_ERR             err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0, &err);
  p = (T_vcom_drv_cbl *)(mcbl->pdrvcbl);

  va_start(ap, fmt_ptr);
  res = vsnprintf((char *)p->str, MAX_STR_SZ - 1, fmt_ptr, ap);
  va_end(ap);
  if (res < 0) return RESULT_ERROR;
  res = Mnudrv1_send_buf(p->str, res);
  return res;
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static int Mnudrv2_printf(const char  *fmt_ptr, ...)
{
  int32_t            res;
  va_list            ap;
  T_vcom_drv_cbl     *p;
  T_monitor_cbl      *mcbl;
  OS_ERR             err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0, &err);
  p = (T_vcom_drv_cbl *)(mcbl->pdrvcbl);

  va_start(ap, fmt_ptr);
  res = vsnprintf((char *)p->str, MAX_STR_SZ - 1, fmt_ptr, ap);
  va_end(ap);
  if (res < 0) return RESULT_ERROR;
  res = Mnudrv2_send_buf(p->str, res);
  return res;
}


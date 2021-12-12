// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017.03.14
// 14:03:09
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

OS_FLAG_GRP              meas_flags;
#define  MEASUR_READY    BIT(0)  // Флаг готовности результата

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void Set_default_settings(void)
{
  cdata.rcount = 248;//490;   //496 для 2 КГц, 996 для 1 Кгц
  cdata.offset = 0;
  cdata.ax     = 294142;
  cdata.ox     = -34265;
  cdata.ay     = 338621;
  cdata.oy     = -45759;
  g_angle_smpl_period = Get_angle_smpl_time();
}


#define MIN_ANGLE_CHANGE 0.2
#define MAX_SMPLS_CNT_FOR_MEAS  100
/*-----------------------------------------------------------------------------------------------------
 Расчет угла поворота
 Вызывается из главной задачи по сигналу от прерывания на линии порта 3-го чипа  
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void Angle_calculation(void)
{
  static uint32_t start    = 0;
  static uint32_t cnt      = 0;
  static float    fixangls = 0;


  float x;
  float y;
  float f;


  {
    // Этот участок выполняется 8954 циклов(186 мкс при частоте системы 48 МГц) при максимальной оптимизации и 11418 при минимальной оптимизации
    x = lhr_data[0] - lhr_data[2];
    y = lhr_data[1] - lhr_data[3];
    x = (x - (float)cdata.ox) / (float)cdata.ax;
    y = (y - (float)cdata.oy) / (float)cdata.ay;
    xsig = x;
    ysig = y;
    if (y > 0)
    {
      f = atan(x / y) * 180.0 / PI + 270.0 ;
    }
    else
    {
      f = atan(x / y) * 180.0 / PI + 90.0;
    }

    // Корректировка для соответствия шкале на печатной плате
    if (f < 260)
      f = 260 - f; 
    else
      f = 260 - f + 360;
   
    g_angle = f;
  }

  // Определяем скорость вращения

  
  if (start == 0)
  {
    // Запоминаем величину угла и время фиксации величины угла при первом проходе
    fixangls = g_angle;
    start = 1;
  }
  else
  {
    float dangle = g_angle - fixangls;
    cnt++;

    if (fabs(dangle) > MIN_ANGLE_CHANGE)
    {
      g_RPS = dangle / ((float)cnt * g_angle_smpl_period * 360.0);
      fixangls = g_angle;
      cnt = 0;
    }

    if (cnt > MAX_SMPLS_CNT_FOR_MEAS) // Если спустя MAC_SMPLS_CNT_FOR_MEAS отсчетов не зафиксировано значащего изменения угла, то считать скорость равной нулю 
    {
      g_RPS = 0;
      fixangls = g_angle;
      cnt = 0;
    }
  }

  // Режимом работы светодиода сообщаем об измерении скорости
  if (g_RPS==0)
  {
    Set_LED_pattern(LED_ON, 0);
  }
  else
  {
    Set_LED_pattern(LED_BLINK_FAST, 0);
  }
}
/*-----------------------------------------------------------------------------------------------------
 
 \param val 
 
 \return float 
-----------------------------------------------------------------------------------------------------*/
float ANSIN01_Convert_int_temp(uint16_t val)
{
  #define VREF     3.3
  #define ADC_PREC 4096.0
  return (25 - ((val * VREF / ADC_PREC) - 0.719) / 0.001715);
}
/*-----------------------------------------------------------------------------------------------------
 
 \param val 
 
 \return float 
-----------------------------------------------------------------------------------------------------*/
float ANSIN01_Convert_ext_temp(uint16_t val)
{
  #define   B1  (3988.0)
  #define   r0  (10000.0)
  float r;
  r = val * 10000.0 / (0xFFF - val);
  r = (25 + 273.15) / ((log(r / r0) * (25 + 273.15)) / B1 + 1) - 273.15;

  return r;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return float 
-----------------------------------------------------------------------------------------------------*/
float Get_angle_smpl_time(void)
{
  return (cdata.rcount * 16.0 + 55.0) / (float)(16000000.0);
}


/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void Init_measur_sync_obj(void)
{
  OS_ERR err;
  OSFlagCreate(&meas_flags, "Measur_flags", (OS_FLAGS)0, &err);
}


/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
OS_ERR Measurement_wait_flag(void)
{
  OS_ERR      err;
  OSFlagPend(&meas_flags, (OS_FLAGS)MEASUR_READY, 0, OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME, (CPU_TS *)0U, &err);
  return err;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void Measurement_post_flag(void)
{
  OS_ERR err;
  OSFlagPost(&meas_flags, MEASUR_READY, OS_OPT_POST_FLAG_SET, &err);
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
-----------------------------------------------------------------------------------------------------*/
void Task_Measurement(void *handle)
{
  OS_ERR err;
  do
  {
    if (Measurement_wait_flag() == OS_ERR_NONE)
    {
      // Вычисляем внутреннюю температуру
      //ITM_EVENT8(1, 0);
      // Вычисления длятся 3744 цикла (78 мкс при частоте ядра 48 МГц)
      g_int_temp = ANSIN01_Convert_int_temp((adcs.smpl_ITEMP_1 + adcs.smpl_ITEMP_2 + adcs.smpl_ITEMP_3) / 3);
      g_ext_temp = ANSIN01_Convert_ext_temp((adcs.smpl_TERM_1 + adcs.smpl_TERM_2 + adcs.smpl_TERM_3) / 3);
      //ITM_EVENT8(1, 1);
    }
    else
    {
      // Делаем паузу в 1 сек в случае ошибки при ожидании флага
      OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
    }
  }
  while (1);

}

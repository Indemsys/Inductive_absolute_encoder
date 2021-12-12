// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-18
// 12:06:59
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "App.h"

extern void LEDS_state_automat(void);

uint32_t  g_PIT_counter;

/*-------------------------------------------------------------------------------------------------------------
  Инициализация модуля Periodic Interrupt Timer (PIT)
  

  Тактируется модуль от Bus Clock -> CPU_BUS_CLK_HZ = 60 МГц
-------------------------------------------------------------------------------------------------------------*/
void Init_PIT_module(void)
{
//  SIM_Type*  SIM  = SIM;
//  PIT_TYpe*  PIT = PIT_BASE_PTR;


  SIM->SCGC6 |= BIT(23); // PIT | PIT clock gate control | 1 Clock is enabled.
  PIT->MCR = 0
    + LSHIFT(0, 1) // MDIS | Module Disable  | 1 Clock for PIT Timers is disabled.
    + LSHIFT(1, 0) // FRZ  | Freeze          | 1 Timers are stopped in debug mode.
;
}

/*-------------------------------------------------------------------------------------------------------------
   Инициализация таймера 0 модуля PIT
   Используется для генерации периодических сигналов 

   period - период подачи сигнала в мкс

   Тактируется модуль от Bus Clock -> CPU_BUS_CLK_HZ = 60 МГц
 -------------------------------------------------------------------------------------------------------------*/
void Init_PIT0(uint32_t period)
{
//  PIT_MemMapPtr PIT = PIT_BASE_PTR;

  // Timer Load Value Register
  PIT->CHANNEL[0].LDVAL = ((CPU_BUS_CLK_HZ/1000000ul) * period); // Загружаем период таймера

  PIT->CHANNEL[0].TCTRL = 0
    + LSHIFT(1, 1) // TIE | Timer Interrupt Enable Bit. | 0 Interrupt requests from Timer n are disabled.
    + LSHIFT(1, 0) // TEN | Timer Enable Bit.           | 1 Timer n is active.
;

  NVIC_SetPriority(PIT0_IRQn, PIT0_PRIO);
  NVIC_EnableIRQ(PIT0_IRQn);
}


/*-------------------------------------------------------------------------------------------------------------
   Получить текущее значение таймера. Таймер ведет счет до 0 и обносляет свое занчение величиной из PIT->CHANNEL[0].LDVAL
 -------------------------------------------------------------------------------------------------------------*/
uint32_t Get_PIT0_curr_val(void)
{
//  PIT_MemMapPtr PIT = PIT_BASE_PTR;
  return PIT->CHANNEL[0].CVAL;
}

/*-------------------------------------------------------------------------------------------------------------
   Получить загрузочное  значение таймера.
 -------------------------------------------------------------------------------------------------------------*/
uint32_t Get_PIT0_load_val(void)
{
//  PIT_MemMapPtr PIT = PIT_BASE_PTR;
  return PIT->CHANNEL[0].LDVAL;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void PIT0_IRQHandler(void)
{
  PIT->CHANNEL[0].TFLG = 1; // Сброс флага прерываний
  LEDS_state_automat();
  g_PIT_counter++;
}

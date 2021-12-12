// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-22
// 22:42:26
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



/*------------------------------------------------------------------------------
 Сброс системы
 ------------------------------------------------------------------------------*/
void Reset_system(void)
{
  __disable_interrupt();
  // System Control Block -> Application Interrupt and Reset Control Register
  SCB->AIRCR = 0x05FA0000  // Обязательный шаблон при записи в этот регистр
    | BIT(2);    // Установка бита SYSRESETREQ
  for (;;)
  {
    __no_operation();
  }
}

/*-----------------------------------------------------------------------------------------------------
 Задержка в миллисекундах
 Аналог функции из MQX
 
 \param millisec 
-----------------------------------------------------------------------------------------------------*/
void _time_delay(uint32_t millisec)
{
  OS_ERR       err;

  OSTimeDlyHMSM(0, 0, 0, millisec, OS_OPT_TIME_HMSM_NON_STRICT, &err);
}


/*-----------------------------------------------------------------------------------------------------
 Аналог функции из MQX
 
 \param size 
 
 \return void* 
-----------------------------------------------------------------------------------------------------*/
void* _mem_alloc_zero(uint32_t size)
{
  void *ptr;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  ptr = malloc(size);
  CPU_CRITICAL_EXIT();
  if (ptr != NULL)
  {
    memset(ptr, 0, size);
  }
  return ptr;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param ptr 
-----------------------------------------------------------------------------------------------------*/
void _mem_free(void *ptr)
{
  CPU_SR_ALLOC(); 
  CPU_CRITICAL_ENTER();
  free(ptr);
  CPU_CRITICAL_EXIT();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017.05.09
// 14:09:01
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void  Coil_research_task(void)
{
   LDC1101_read_all_devs();
   Set_LED_pattern(LED_BLINK, 0);
   LDC1101_update_RCOUNT_OFFS_for_All();
   LDC1101_config_interupts();
   for (;;)
   {
      LDC1101_pend_mutex();
      LDC1101_prepare_to_wait_rdy(CS_U2); // Подготовим линии портов для ожидания прерывания от чипа LDC1101
      LDC1101_wait_results();         // Ждем события о завершении преобразования
      LDC1101_restore_from_rdy(CS_U2);     // Восстанавливаем линии портов для обмена по DMA
      LDC1101_post_mutex();
      LDC1101_read_all_LHR_DATA_DMA(); // Читаем результат по DMA
   }
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void ANSIN_task_function(void)
{
#ifdef COIL_RESEARCH
   Coil_research_task();
#endif

   if (LDC1101_read_all_devs() != RESULT_OK) Set_LED_pattern(LED_ON, 0);
   else Set_LED_pattern(LED_BLINK, 0);

   LDC1101_update_RCOUNT_OFFS_for_All();

   LDC1101_config_interupts();
   for (;;)
   {
      LDC1101_pend_mutex();
      LDC1101_prepare_to_wait_rdy(CS_U5); // Подготовим линии портов для ожидания прерывания от чипа LDC1101
      LDC1101_wait_results();             // Ждем события о завершении преобразования
      LDC1101_restore_from_rdy(CS_U5);    // Восстанавливаем линии портов для обмена по DMA
      LDC1101_post_mutex();
      //ITM_EVENT8(1, 3);
      LDC1101_read_all_LHR_DATA_DMA();    // Читаем результат по DMA

      // Вычисляем угол
      Angle_calculation();
      Host_ch_set_ready_sig();
   }

}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
// 23:02:14
// 2017.03.14

typedef struct
{
  uint32_t indx;
  uint8_t  *pval;
  uint8_t  sz;

} T_host_protocol_rw_vars;

#define RW_VARS_COUNT 18
// Массив читаемых и записываемых переменных по протоколу с хост контроллером
T_host_protocol_rw_vars vrs[RW_VARS_COUNT] =
{
  {0   ,(uint8_t *)&g_angle              , sizeof(g_angle)},
  {1   ,(uint8_t *)&cdata.ax             , sizeof(cdata.ax)},
  {2   ,(uint8_t *)&cdata.ox             , sizeof(cdata.ox)},
  {3   ,(uint8_t *)&cdata.ay             , sizeof(cdata.ay)},
  {4   ,(uint8_t *)&cdata.oy             , sizeof(cdata.oy)},
  {5   ,(uint8_t *)&cdata.rcount         , sizeof(cdata.rcount)},
  {6   ,(uint8_t *)&cdata.offset         , sizeof(cdata.offset)},
  {7   ,(uint8_t *)&g_flash_erase_errors , sizeof(g_flash_erase_errors)},
  {8   ,(uint8_t *)&g_flash_write_errors , sizeof(g_flash_write_errors)},
  {9   ,(uint8_t *)&g_flash_crc_errors   , sizeof(g_flash_crc_errors)},
  {10  ,(uint8_t *)&g_int_temp           , sizeof(g_int_temp)},
  {11  ,(uint8_t *)&g_ext_temp           , sizeof(g_ext_temp)},
  {12  ,(uint8_t *)&xsig                 , sizeof(xsig)},
  {13  ,(uint8_t *)&ysig                 , sizeof(ysig)},
  {14  ,(uint8_t *)&g_fmstr_rate_src     , sizeof(g_fmstr_rate_src)},
  {15  ,(uint8_t *)&g_fmstr_smpls_period , sizeof(g_fmstr_smpls_period)},
  {16  ,(uint8_t *)&g_angle_smpl_period  , sizeof(g_angle_smpl_period)},
  {17  ,(uint8_t *)&g_RPS                , sizeof(g_RPS)},
};


/*-----------------------------------------------------------------------------------------------------
 Подготавливаем к отправке пакет с данными для чтения хостам
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void _Prepare_packet_to_host(void)
{
  uint32_t i;
  uint8_t  crc  = 0;
  uint8_t  *buf;
  uint8_t  v;
  uint32_t sz;

  pcbl.packet[0] = pcbl.var_indx; // Первый байт пакета - индекс переменной или идентификатор пакета
  crc += pcbl.packet[0];

  // Передаем  данные
  buf = pcbl.pvar;
  sz  = pcbl.var_sz;

  for (i = 0; i < sz; i++)
  {
    v = *buf;
    pcbl.packet[1 + i] = v;
    buf++;
    crc += v;
  }
  crc = 0x100 - crc;
  pcbl.packet[1 + sz] = crc;
  pcbl.pack_len = sz + 2;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _Set_default_read_var(void)
{
  pcbl.var_indx = 0;
  pcbl.pvar      = (uint8_t *)&g_angle; // По умолчанию передаем хосту величину узмеренного угла
  pcbl.var_sz   = sizeof(g_angle);

}


/*-----------------------------------------------------------------------------------------------------
 
     
     Структура пакетов при обмене с хост контроллером:
     Номер байта    Количество          Назначение
     в пакете       байт          
     0              1                   [ID]         - идентификатор пакета 
     1              1                   [data0]      - первый байт данных
     ...                                
     N+1            1                   [dataN]      - последний байт данных
     N+2            1                   [CRC]        - контрольная сумма 
 
 
 
 \param val 
-----------------------------------------------------------------------------------------------------*/
static void _Protocol_parser(void)
{

}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _Reset_protocol(void)
{
  _Set_default_read_var();
}

/*-----------------------------------------------------------------------------------------------------
  Функция устанавливающая сигнал READY по готовности результата для выдачи хост контроллеру  
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void Host_ch_set_ready_sig(void)
{
  Host_SPI_set_data_ready();
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
-----------------------------------------------------------------------------------------------------*/
void Task_Host_channel(void *handle)
{
  READY_set_state(0);
  Host_SPI_init();  

  _Reset_protocol();


  do
  {
    // Подготавливаем пакет для выдачи хосту и устанавливаем после этого сигнал READY

    _Prepare_packet_to_host();
    if (HOST_SPI_send_packet(pcbl.packet, pcbl.pack_len, 10) == RESULT_OK)
    {
      // Данные хосту переданы
      // Готовимся к приему данных от хоста
      switch (Host_SPI_wait_packet(pcbl.packet, pcbl.pack_len, 10))
      {
      case HOSTSPI_PACKET_WAIT_ERROR:
        // Ошибка приема пакета или таймаут
        _time_delay(2);
        break;
      case HOSTSPI_PACKET_RECEIVED:
        _Protocol_parser();
        // Ждать готовности данных для отправки
        Host_SPI_wait_data_ready(10);
        break;
      case HOSTSPI_TX_DATA_READY:
        // Пакет получить не успели поскольку пришло время отправки новых данных
        break;
      }
    }
    else
    {
      // Данные хосту не переданы, сбросим флаг готовности к передачи и снова взведем
      READY_set_state(0);
      _time_delay(2);
    }

  }
  while (1);


}


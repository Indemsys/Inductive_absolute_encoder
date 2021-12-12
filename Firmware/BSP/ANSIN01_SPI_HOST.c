// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.07.29
// 13:50:42
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


#define  HOST_SPI_RXEND           BIT(0)  // Флаг передаваемый из ISR DMA об окончании приема по SPI
#define  HOST_SPI_TXEND           BIT(1)  // Флаг передаваемый из ISR DMA об окончании приема по SPI
#define  HOST_SPI_TX_DATA_READY   BIT(2)  // Флаг сигнализирующий о необходимости прекращения прооцедуры приема


static T_DMA_SPI_cbl     DS_cbl;


/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void HOSTSPI_RX_DMA_ISR(void)
{
  CPU_SR_ALLOC();
  OS_ERR      err;

  DMA0->INT = BIT(DMA_HOSTSPI_FM_CH); // Сбрасываем флаг прерываний  канала


  CPU_CRITICAL_ENTER();
  OSIntEnter();                                               /* Tell uC/OS-III that we are starting an ISR           */
  CPU_CRITICAL_EXIT();

  OSFlagPost(&spi_cbl[HOST_SPI].spi_flags, HOST_SPI_RXEND, OS_OPT_POST_FLAG_SET, &err);

  OSIntExit();
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void HOSTSPI_TX_DMA_ISR(void)
{
  CPU_SR_ALLOC();
  OS_ERR      err;

  DMA0->INT = BIT(DMA_HOSTSPI_MF_CH); // Сбрасываем флаг прерываний  канала



  CPU_CRITICAL_ENTER();
  OSIntEnter();                                               /* Tell uC/OS-III that we are starting an ISR           */
  CPU_CRITICAL_EXIT();

  OSFlagPost(&spi_cbl[HOST_SPI].spi_flags, HOST_SPI_TXEND, OS_OPT_POST_FLAG_SET, &err);

  OSIntExit();
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _Host_SPI_Init_SPI_DMA(void)
{
  T_DMA_SPI_RX_config rx_cfg;
  T_DMA_SPI_TX_config tx_cfg;

  tx_cfg.ch             = DMA_HOSTSPI_MF_CH;                              // номер канала DMA
  tx_cfg.spi_pushr      = (uint32_t)&(spi_mods[HOST_SPI].spi->PUSHR);     // адрес регистра PUSHR SPI
  tx_cfg.minor_tranf_sz = DMA_1BYTE_MINOR_TRANSFER;                       // Количество байт в единичной операции записи в SPI
  tx_cfg.int_en         = 1;                                              // Разрешаем прерывания после цикоа пересылки данных по DMA

  Config_DMA_for_SPI_TX(&tx_cfg, &DS_cbl);
  Set_DMA_MUX_chanel(DMAMUX, DMA_HOSTSPI_DMUX_TX_SRC, DMA_HOSTSPI_MF_CH);

  NVIC_SetPriority((IRQn_Type)DMA_HOSTSPI_TX_INT_NUM, HOST_DMA_SPI_PRIO);
  NVIC_EnableIRQ((IRQn_Type)DMA_HOSTSPI_TX_INT_NUM);

  
  rx_cfg.ch             = DMA_HOSTSPI_FM_CH;                              // номер канала DMA
  rx_cfg.spi_popr       = (uint32_t)&(spi_mods[HOST_SPI].spi->POPR);      // адрес регистра POPR SPI
  rx_cfg.minor_tranf_sz = DMA_1BYTE_MINOR_TRANSFER;                       // Количество байт в единичной операции чтения из SPI
  rx_cfg.int_en         = 1;                                              // Разрешаем прерывания после цикоа пересылки данных по DMA

  Config_DMA_for_SPI_RX(&rx_cfg, &DS_cbl);
  Set_DMA_MUX_chanel(DMAMUX, DMUX_SRC_DISABLED, DMA_HOSTSPI_FM_CH);

  NVIC_SetPriority((IRQn_Type)DMA_HOSTSPI_RX_INT_NUM, HOST_DMA_SPI_PRIO);
  NVIC_EnableIRQ((IRQn_Type)DMA_HOSTSPI_RX_INT_NUM);
}

/*-------------------------------------------------------------------------------------------------------------
   Инициализируем DMA для приема данных по SPI
  Данные передаются 1-о байтами словами
  sz - количество передаваемых слов (Это не размер буфера в байтах!)

  Может быть передано не более 2048 слов (MAX_DMA_SPI_BUFF) !!!
-------------------------------------------------------------------------------------------------------------*/
static void _Start_HOSTSPI_DMA_RX(T_DMA_SPI_cbl *pDS_cbl, uint8_t *inbuf, uint32_t sz)
{
  DMA_Type            *DMA    = DMA0;
  T_DMA_TCD  volatile *pTDCfm;

  //CPU_SR_ALLOC();
  //CPU_CRITICAL_ENTER();

  DMA->CERQ = pDS_cbl->tx_ch; // Запрещаем запросы по каналу передачи

  spi_mods[HOST_SPI].spi->RSER = (spi_mods[HOST_SPI].spi->RSER & ~BIT(25)) | BIT(17);  // Запрещаем запросы DMA от SPI от сигнала Transmit FIFO Fill Flag (TFFF) и разрешаем от сигнала FIFO Drain Request Enable (RFDF)
  spi_mods[HOST_SPI].spi->MCR  |= BIT(CLR_RXF) + BIT(CLR_TXF); // Очистим FIFO приемника и передатчика
  spi_mods[HOST_SPI].spi->SR =  spi_mods[HOST_SPI].spi->SR;    // Сбросим все флаги у SPI

  // Подготовка дескриптора DMA предназначенного для чтения из SPI
  Set_DMA_MUX_chanel(DMAMUX, DMUX_SRC_DISABLED, DMA_HOSTSPI_MF_CH);
  Set_DMA_MUX_chanel(DMAMUX, DMA_HOSTSPI_DMUX_RX_SRC, DMA_HOSTSPI_FM_CH);

  // Сбрасываем бит DONE
  DMA->CDNE = pDS_cbl->rx_ch;

  // Программируем дескриптор  чтения
  pTDCfm = (T_DMA_TCD *)&DMA->TCD[pDS_cbl->rx_ch];
  pTDCfm->DADDR         = (uint32_t)inbuf;
  pTDCfm->DOFF          = pDS_cbl->minor_tranf_sz;
  pTDCfm->BITER_ELINKNO = (pTDCfm->BITER_ELINKNO & ~0x7FF) |  sz;
  pTDCfm->CITER_ELINKNO = pTDCfm->BITER_ELINKNO;

  DMA->SERQ = pDS_cbl->rx_ch;

  //CPU_CRITICAL_EXIT();
}

/*-------------------------------------------------------------------------------------------------------------
   Инициализируем DMA для отправки данных по SPI
  Данные передаются 1-о байтами словами
  sz - количество передаваемых слов (Это не размер буфера в байтах!)

  Может быть передано не более 2048 слов (MAX_DMA_SPI_BUFF) !!!
-------------------------------------------------------------------------------------------------------------*/
static void _Start_HOSTSPI_DMA_TX(T_DMA_SPI_cbl *pDS_cbl, void *outbuf, uint32_t sz)
{
  DMA_Type            *DMA    = DMA0;
  T_DMA_TCD  volatile *pTDCmf;

  //CPU_SR_ALLOC();
  //CPU_CRITICAL_ENTER();

  DMA->CERQ = pDS_cbl->rx_ch; // Запрещаем запросы по каналу приема

  spi_mods[HOST_SPI].spi->RSER = (spi_mods[HOST_SPI].spi->RSER & ~BIT(17)) | BIT(25);  // Разрешаем запросы DMA от SPI от сигнала Transmit FIFO Fill Flag (TFFF) и запрещаем от сигнала FIFO Drain Request Enable (RFDF)
  spi_mods[HOST_SPI].spi->MCR  |= BIT(CLR_RXF) + BIT(CLR_TXF); // Очистим FIFO приемника и передатчика
  spi_mods[HOST_SPI].spi->SR =  spi_mods[HOST_SPI].spi->SR;    // Сбросим все флаги у SPI

  // Подготовка дескриптора DMA предназначенных для записи буфера в SPI
  Set_DMA_MUX_chanel(DMAMUX, DMA_HOSTSPI_DMUX_TX_SRC, DMA_HOSTSPI_MF_CH);
  Set_DMA_MUX_chanel(DMAMUX, DMUX_SRC_DISABLED, DMA_HOSTSPI_FM_CH);

  // Сбрасываем бит DONE
  DMA->CDNE = pDS_cbl->tx_ch;

  pTDCmf = (T_DMA_TCD *)&DMA->TCD[pDS_cbl->tx_ch];
  pTDCmf->SADDR         = (uint32_t)outbuf;
  pTDCmf->SOFF          = pDS_cbl->minor_tranf_sz;
  pTDCmf->BITER_ELINKNO = (pTDCmf->BITER_ELINKNO & ~0x7FF) |  sz;
  pTDCmf->CITER_ELINKNO = pTDCmf->BITER_ELINKNO;

  DMA->SERQ = pDS_cbl->tx_ch;

  //CPU_CRITICAL_EXIT();
}




/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void Host_SPI_init(void)
{
  SPI_slave_init(HOST_SPI, SPI_8_BITS, 0, 1, 0);  // Инициализация интерфейса SPI с хост контроллером с использованием DMA
  _Host_SPI_Init_SPI_DMA(); // Инициализируем структуры и прерываний DMA для обмена по SPI с хост контроллером
}


/*-----------------------------------------------------------------------------------------------------
  Установить флаг готовности данных к отправке 
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void Host_SPI_set_data_ready(void)
{
  OS_ERR   err;
  OSFlagPost(&spi_cbl[HOST_SPI].spi_flags, HOST_SPI_TX_DATA_READY, OS_OPT_POST_FLAG_SET, &err);
}

/*-----------------------------------------------------------------------------------------------------
  Ожидать флага готовности данных к отправке
 
 \param void 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t Host_SPI_wait_data_ready(uint32_t timeout)
{
  OS_ERR   err;

  OSFlagPend(&spi_cbl[HOST_SPI].spi_flags, (OS_FLAGS)HOST_SPI_TX_DATA_READY, timeout, OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME, (CPU_TS *)0U, &err);
  if (err != OS_ERR_NONE)
  {
    return HOSTSPI_PACKET_WAIT_ERROR;
  }
  return HOSTSPI_TX_DATA_READY;
}

/*-------------------------------------------------------------------------------------------------------------
  Ожидание приема данных из SPI slave интерфейса подключенного к хост контроллеру
 
  Возвращает 0 если произошел таймаут или шибка
 -------------------------------------------------------------------------------------------------------------*/
uint32_t Host_SPI_wait_packet(void *buf, uint32_t sz, uint32_t timeout)
{
  OS_ERR   err;
  OS_FLAGS flags;

  _Start_HOSTSPI_DMA_RX(&DS_cbl, buf, sz);
  READY_set_state(0); // Информируем хост о готовности к приему от него данных
                      // Ожидаем флага окончания передачи
  flags = OSFlagPend(&spi_cbl[HOST_SPI].spi_flags, (OS_FLAGS)HOST_SPI_RXEND + (OS_FLAGS)HOST_SPI_TX_DATA_READY, timeout, OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME, (CPU_TS *)0U, &err);
  if (err != OS_ERR_NONE)
  {
    spi_cbl[HOST_SPI].rx_err_cnt++;
    return HOSTSPI_PACKET_WAIT_ERROR;
  }
  if (flags & HOST_SPI_TX_DATA_READY)
  {
    return HOSTSPI_TX_DATA_READY;
  }
  return HOSTSPI_PACKET_RECEIVED;
}


/*-----------------------------------------------------------------------------------------------------
 Посылаем байтовый пакет с данными хосту
-----------------------------------------------------------------------------------------------------*/
uint32_t HOST_SPI_send_packet(void *buf, uint32_t sz, uint32_t timeout)
{
  OS_ERR   err;
  uint32_t res = RESULT_OK;

  _Start_HOSTSPI_DMA_TX(&DS_cbl, buf, sz);
  READY_set_state(1); // Информируем хост о готовности к передаче ему данных
                      // Ожидаем флага окончания передачи
  OSFlagPend(&spi_cbl[HOST_SPI].spi_flags, (OS_FLAGS)HOST_SPI_TXEND, timeout, OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME, (CPU_TS *)0U, &err);
  if (err != OS_ERR_NONE)
  {
    spi_cbl[HOST_SPI].tx_err_cnt++;
    res = RESULT_ERROR;
  }
  return res;
}

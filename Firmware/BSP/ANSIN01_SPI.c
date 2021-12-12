#define _SPI_GLOBAL_
#include "App.h"




// /*-------------------------------------------------------------------------------------------------------------
//   Обработчик прерывания от модуля SPI с номером modn
// -------------------------------------------------------------------------------------------------------------*/
// static void SPI_isr(uint8_t modn)
// {
//   SPI_Type  *SPI   = spi_mods[modn].spi;
//   unsigned int   sr    = SPI->SR;
// 
//   if ((sr & BIT(TCF)) != 0) // Проверяем и сбрасываем флаг  Transfer complete
//   {
//     OS_ERR     err; 
//     SPI->SR = BIT(TCF);
//     OSFlagPost(&spi_cbl[modn].spi_flags, SPI_TX_COMPLETE, OS_OPT_POST_FLAG_SET, &err);
//   }
//   if ((sr & BIT(EOQF)) != 0) // Проверяем и сбрасываем флаг  End of Queue Flag
//   {
//     SPI->SR = BIT(EOQF);
//   }
//   if ((sr & BIT(TFUF)) != 0) // Проверяем и сбрасываем флаг  Transmit FIFO Underflow Flag
//   {
//     SPI->SR = BIT(TFUF);
//     spi_errors[modn].tx_uderfl_cnt++;
//   }
//   if ((sr & BIT(RFOF)) != 0) // Проверяем и сбрасываем флаг  Receive FIFO Overflow Flag
//   {
//     SPI->SR = BIT(RFOF);
//     spi_errors[modn].rx_overfl_cnt++;
//   }
// }

// /*-------------------------------------------------------------------------------------------------------------
// 
// -------------------------------------------------------------------------------------------------------------*/
// void SPI0_IRQHandler(void *user_isr_ptr)
// {
//   SPI_isr(0);
// }
// /*-------------------------------------------------------------------------------------------------------------
// 
// -------------------------------------------------------------------------------------------------------------*/
// void SPI1_IRQHandler(void *user_isr_ptr)
// {
//   SPI_isr(1);
// }


/*-------------------------------------------------------------------------------------------------------------

  Тактирование SPI производится от bus clock/2   BSP_BUS_CLOCK (60 МГц)
  Размер FIFO - 4-е 32-х разрядных слова

  modn  - новмер SPI модуля (0,1,2)
  fsz   - размер передаваемых данных ( 3 < fsz < 17 )
  cpol  - начальный уровень. 0 The inactive state value of SCK is low.
                            1 The inactive state value of SCK is high.

  cpha  - выбор среза. 0 Data is captured on the leading edge of SCK and changed on the following edge.
                      1 Data is changed on the leading edge of SCK and captured on the following edge.

  baud  - частота тактирования SPI  для BSP_CORE_CLOCK = 120 МГц

  enint - флаг разрешения прерываний генерируемых модулем SPI. В процедуре обслуживания прерываний ведем счетчик ошибок и выполняем другие вспомогательные функции

-------------------------------------------------------------------------------------------------------------*/
void SPI_master_init(uint8_t modn, T_SPI_bits fsz, uint8_t cpol, uint8_t cpha, T_SPI_bauds baud, int enint)
{
  SPI_Type  *SPI   = spi_mods[modn].spi;

  uint8_t dbr    = 1;
  uint8_t pbr    = 3;
  uint8_t br     = 2;
  uint8_t pcssck = 1;
  uint8_t cssck  = 1;

  switch (modn)
  {
  case 0:
    SIM->SCGC6 |= BIT(12); break;
  case 1:
    SIM->SCGC6 |= BIT(13); break;
  }

  SPI->MCR |= BIT(HALT); // Остановить модуль
  while (SPI->SR & BIT(TXRXS)); // Ожидание пока бит 30 (TXRXS) не станет равным нулю


  SPI->MCR = 0
    + LSHIFT(1,    31) // MSTR      | Master/Slave Mode Select                     | 1 Enables Master mode
    + LSHIFT(0,    30) // CONT_SCKE | Continuous SCK Enable                        | 0 Continuous SCK disabled
    + LSHIFT(0,    28) // DCONF     | SPI Configuration.                           | 00 SPI
    + LSHIFT(1,    27) // FRZ       | Freeze                                       | 0 Do not halt serial transfers in Debug mode.
    + LSHIFT(0,    26) // MTFE      | Modified Timing Format Enable                | 0 Modified SPI transfer format disabled.
    + LSHIFT(0,    25) // PCSSE     | Peripheral Chip Select Strobe Enable         | 0 PCS5/ PCSS is used as the Peripheral Chip Select[5] signal.
    + LSHIFT(0,    24) // ROOE      | Receive FIFO Overflow Overwrite Enable       | 0 Incoming data is ignored.
    + LSHIFT(0x3F, 16) // PCSIS     | Peripheral Chip Select x Inactive State      | 1 The inactive state of PCSx is high.
    + LSHIFT(0,    15) // DOZE      | Doze Enable                                  | 0 Doze mode has no effect on the module.
    + LSHIFT(0,    14) // MDIS      | Module Disable                               | 0 Enables the module clocks.
    + LSHIFT(0,    13) // DIS_TXF   | Disable Transmit FIFO                        | 0 TX FIFO is enabled.
    + LSHIFT(0,    12) // DIS_RXF   | Disable Receive FIFO                         | 0 RX FIFO is enabled.
    + LSHIFT(1,    11) // CLR_TXF   | Writing a 1 to CLR_TXF clears the TX FIFO Counter
    + LSHIFT(1,    10) // CLR_RXF   | Writing a 1 to CLR_RXF clears the RX FIFO Counter
    + LSHIFT(0,    8)  // SMPL_PT   | Controls when the module master samples SIN in Modified Transfer Format
    + LSHIFT(1,    0)  // HALT      | The HALT bit starts and stops frame transfers| 1 Stop transfers.
  ;



// Поиск делителей для обеспечения частоты тактирования baud = (CPU_BUS_CLK_HZ /pbr) x [(1+dbr)/br]
// dbr примем = 1
// Задержка  подачи тактового сигнала после установки сигнала выборки должна быть равна половине длительности тактового сигнала
// Значения приведены для частоты 60 Мгц
  switch (baud)
  {
  case SPI_BAUD_20MHZ   :
    pbr = PBR_3; br = BR_2; pcssck = PCSSCK_1; cssck = CSSCK_2;  break;   //
  case SPI_BAUD_15MHZ   :
    pbr = PBR_2; br = BR_4; pcssck = PCSSCK_1; cssck = CSSCK_2;  break;   //
  case SPI_BAUD_10MHZ   :
    pbr = PBR_3; br = BR_4; pcssck = PCSSCK_1; cssck = CSSCK_2;  break;   //
  case SPI_BAUD_6MHZ    :
    pbr = PBR_5; br = BR_4; pcssck = PCSSCK_1; cssck = CSSCK_4;  break;   //
  case SPI_BAUD_4MHZ    :
    pbr = PBR_5; br = BR_6; pcssck = PCSSCK_3; cssck = CSSCK_2;  break;   //
  case SPI_BAUD_3MHZ  :
    pbr = PBR_5; br = BR_8; pcssck = PCSSCK_5; cssck = CSSCK_2;  break;   //
  case SPI_BAUD_1_5MHZ :
    pbr = PBR_5; br = BR_16; pcssck = PCSSCK_5; cssck = CSSCK_4;  break;   //
  }

  // Программируем первый набор аттрибутов, будем использовать только его
  SPI->CTAR[0] = 0
    + LSHIFT(dbr,     31) // DBR    | Double Baud Rate                       | 0 The baud rate is computed normally with a 50/50 duty cycle.
    + LSHIFT(fsz - 1, 27) // FMSZ   | Frame Size                             |
    + LSHIFT(cpol,    26) // CPOL   | Clock Polarity
    + LSHIFT(cpha,    25) // CPHA   | Clock Phase
    + LSHIFT(0,       24) // LSBFE  | LSB First                              | 0 Data is transferred MSB first.
    + LSHIFT(pcssck,  22) // PCSSCK | PCS to SCK Delay Prescaler             | 01 PCS to SCK Prescaler value is 3.
    + LSHIFT(0,       20) // PASC   | After SCK Delay Prescaler              | 00 Delay after Transfer Prescaler value is 1.
    + LSHIFT(0,       18) // PDT    | Delay after Transfer Prescaler         | 00 Delay after Transfer Prescaler value is 1.
    + LSHIFT(pbr,     16) // PBR    | Baud Rate Prescaler. The baud rate is the frequency of the SCK (0 -> 2, 1 -> 3, 2 -> 5, 3 -> 7)
    + LSHIFT(cssck,   12) // CSSCK  | PCS to SCK Delay Scaler. The PCS to SCK Delay is the delay between the assertion of PCS and the first edge of the SCK
    + LSHIFT(0,       8)  // ASC    | After SCK Delay Scaler. The After SCK Delay is the delay between the last edge of SCK and the negation of PCS
    + LSHIFT(0,       4)  // DT     | Delay After Transfer Scaler. The Delay after Transfer is the time between the negation of the PCS signal at the end
                          //        |   of a frame and the assertion of PCS at the beginning of the next frame.
    + LSHIFT(br,      0)  // BR     | Baud Rate Scaler. The baud rate is computed according to the following equation:
                          //        |  SCK baud rate = (fP /PBR) x [(1+DBR)/BR]
  ;

  SPI->SR = 0
    + LSHIFT(1,   31) // TCF       | Transfer Complete Flag
    + LSHIFT(1,   30) // TXRXS     | TX and RX Status
    + LSHIFT(1,   28) // EOQF      | End of Queue Flag
    + LSHIFT(1,   27) // TFUF      | Transmit FIFO Underflow Flag
    + LSHIFT(1,   25) // TFFF      | Transmit FIFO Fill Flag
    + LSHIFT(1,   19) // RFOF      | Receive FIFO Overflow Flag
    + LSHIFT(1,   17) // RFDF      | Receive FIFO Drain Flag
    + LSHIFT(0,   12) // TXCTR     | TX FIFO Counter (read only)
    + LSHIFT(0,    8) // TXNXTPTR  | Transmit Next Pointer
    + LSHIFT(0,    4) // RXCTR     | RX FIFO Counter
    + LSHIFT(0,    0) // POPNXTPTR | Pop Next Pointer
  ;

  if (enint == 0)
  {
    SPI->RSER = 0
      + LSHIFT(0,   31) // TCF_RE    | Transmission Complete Request Enable. Enables TCF flag in the SR to generate an interrupt request                   | 0 TCF interrupt requests are disabled.
      + LSHIFT(0,   28) // EOQF_RE   | Finished Request Enable. Enables the EOQF flag in the SR to generate an interrupt request                           | 0 EOQF interrupt requests are disabled.
      + LSHIFT(0,   27) // TFUF_RE   | Transmit FIFO Underflow Request Enable. Enables the TFUF flag in the SR to generate an interrupt request            | 0 TFUF interrupt requests are disabled.
      + LSHIFT(1,   25) // TFFF_RE   | Transmit FIFO Fill Request Enable. Enables the TFFF flag in the SR to generate a request                            | 1 TFFF interrupts or DMA requests are enabled
      + LSHIFT(1,   24) // TFFF_DIRS | Transmit FIFO Fill DMA or Interrupt Request Select. Selects between generating a DMA request or an interrupt request| 1 TFFF flag generates DMA requests.
      + LSHIFT(1,   19) // RFOF_RE   | Receive FIFO Overflow Request Enable. Enables the RFOF flag in the SR to generate an interrupt request              | 1 RFOF interrupt requests are enabled.
      + LSHIFT(1,   17) // RFDF_RE   | Receive FIFO Drain Request Enable. Enables the RFDF flag in the SR to generate a request.                           | 1 RFDF interrupt or DMA requests are enabled.
      + LSHIFT(1,   16) // RFDF_DIRS | Receive FIFO Drain DMA or Interrupt Request Select                                                                  | 1 DMA request.
    ;
  }
  else
  {
    SPI->RSER = 0
      + LSHIFT(1,   31) // TCF_RE    | Transmission Complete Request Enable. Enables TCF flag in the SR to generate an interrupt request                   | 0 TCF interrupt requests are disabled.
      + LSHIFT(0,   28) // EOQF_RE   | Finished Request Enable. Enables the EOQF flag in the SR to generate an interrupt request                           | 0 EOQF interrupt requests are disabled.
      + LSHIFT(0,   27) // TFUF_RE   | Transmit FIFO Underflow Request Enable. Enables the TFUF flag in the SR to generate an interrupt request            | 0 TFUF interrupt requests are disabled.
      + LSHIFT(1,   25) // TFFF_RE   | Transmit FIFO Fill Request Enable. Enables the TFFF flag in the SR to generate a request                            | 1 TFFF interrupts or DMA requests are enabled
      + LSHIFT(0,   24) // TFFF_DIRS | Transmit FIFO Fill DMA or Interrupt Request Select. Selects between generating a DMA request or an interrupt request| 1 TFFF flag generates DMA requests.
      + LSHIFT(0,   19) // RFOF_RE   | Receive FIFO Overflow Request Enable. Enables the RFOF flag in the SR to generate an interrupt request              | 1 RFOF interrupt requests are enabled.
      + LSHIFT(1,   17) // RFDF_RE   | Receive FIFO Drain Request Enable. Enables the RFDF flag in the SR to generate a request.                           | 1 RFDF interrupt or DMA requests are enabled.
      + LSHIFT(0,   16) // RFDF_DIRS | Receive FIFO Drain DMA or Interrupt Request Select                                                                  | 1 DMA request.
    ;
  }

  {
    OS_ERR     err; 
    OSFlagCreate(&spi_cbl[modn].spi_flags, "SPI_flags", (OS_FLAGS)0, &err); 
  }


  if (enint != 0)
  {
    spi_errors[modn].rx_overfl_cnt = 0;
    spi_errors[modn].tx_uderfl_cnt = 0;

    NVIC_SetPriority((IRQn_Type)spi_mods[modn].int_num, spi_mods[modn].prio);
    NVIC_EnableIRQ((IRQn_Type)spi_mods[modn].int_num);
  }

  SPI->MCR &= ~BIT(HALT); // Запустить
}

/*------------------------------------------------------------------------------
 Инициализация SPI в slave режиме


 \param modn
 \param fsz
 \param cpol
 \param cpha
 \param baud
 \param enint
 ------------------------------------------------------------------------------*/
void SPI_slave_init(uint8_t modn, T_SPI_bits fsz, uint8_t cpol, uint8_t cpha,  int enint)
{
  SPI_Type  *SPI   = spi_mods[modn].spi;

  switch (modn)
  {
  case 0:
    SIM->SCGC6 |= BIT(12); break;
  case 1:
    SIM->SCGC6 |= BIT(13); break;
  }

  SPI->MCR |= BIT(HALT); // Остановить модуль
  while (SPI->SR & BIT(TXRXS)); // Ожидание пока бит 30 (TXRXS) не станет равным нулю
  
  SPI->MCR = BIT(HALT); // Предварительно обнуляем бит MDIS, чтобы можно было программировать все остальные биты

  SPI->MCR = 0
    + LSHIFT(0,    31) // MSTR      | Master/Slave Mode Select                     | 1 Enables Master mode
    + LSHIFT(0,    30) // CONT_SCKE | Continuous SCK Enable                        | 0 Continuous SCK disabled
    + LSHIFT(0,    28) // DCONF     | SPI Configuration.                           | 00 SPI
    + LSHIFT(1,    27) // FRZ       | Freeze                                       | 0 Do not halt serial transfers in Debug mode.
    + LSHIFT(0,    26) // MTFE      | Modified Timing Format Enable                | 0 Modified SPI transfer format disabled.
    + LSHIFT(0,    25) // PCSSE     | Peripheral Chip Select Strobe Enable         | 0 PCS5/ PCSS is used as the Peripheral Chip Select[5] signal.
    + LSHIFT(1,    24) // ROOE      | Receive FIFO Overflow Overwrite Enable       | 0 Incoming data is ignored.
    + LSHIFT(0x3F, 16) // PCSIS     | Peripheral Chip Select x Inactive State      | 1 The inactive state of PCSx is high.
    + LSHIFT(0,    15) // DOZE      | Doze Enable                                  | 0 Doze mode has no effect on the module.
    + LSHIFT(0,    14) // MDIS      | Module Disable                               | 0 Enables the module clocks.
    + LSHIFT(0,    13) // DIS_TXF   | Disable Transmit FIFO                        | 0 TX FIFO is enabled.
    + LSHIFT(0,    12) // DIS_RXF   | Disable Receive FIFO                         | 0 RX FIFO is enabled.
    + LSHIFT(1,    11) // CLR_TXF   | Writing a 1 to CLR_TXF clears the TX FIFO Counter
    + LSHIFT(1,    10) // CLR_RXF   | Writing a 1 to CLR_RXF clears the RX FIFO Counter
    + LSHIFT(0,    8)  // SMPL_PT   | Controls when the module master samples SIN in Modified Transfer Format
    + LSHIFT(1,    0)  // HALT      | The HALT bit starts and stops frame transfers| 1 Stop transfers.
  ;

  // Программируем первый набор аттрибутов, будем использовать только его
  SPI->CTAR_SLAVE[0] = 0
    + LSHIFT(fsz - 1, 27) // FMSZ   | Frame Size                             
    + LSHIFT(cpol,    26) // CPOL   | Clock Polarity
    + LSHIFT(cpha,    25) // CPHA   | Clock Phase
  ;

  SPI->SR = 0
    + LSHIFT(1,   31)   // TCF       | Transfer Complete Flag
    + LSHIFT(1,   30)   // TXRXS     | TX and RX Status
    + LSHIFT(1,   28)   // EOQF      | End of Queue Flag
    + LSHIFT(1,   27)   // TFUF      | Transmit FIFO Underflow Flag
    + LSHIFT(1,   25)   // TFFF      | Transmit FIFO Fill Flag
    + LSHIFT(1,   19)   // RFOF      | Receive FIFO Overflow Flag
    + LSHIFT(1,   17)   // RFDF      | Receive FIFO Drain Flag
    + LSHIFT(0,   12)   // TXCTR     | TX FIFO Counter (read only)
    + LSHIFT(0,    8)   // TXNXTPTR  | Transmit Next Pointer
    + LSHIFT(0,    4)   // RXCTR     | RX FIFO Counter
    + LSHIFT(0,    0)   // POPNXTPTR | Pop Next Pointer
  ;

  if (enint == 0)
  {
    SPI->RSER = 0
      + LSHIFT(0,   31) // TCF_RE    | Transmission Complete Request Enable. Enables TCF flag in the SR to generate an interrupt request                   | 0 TCF interrupt requests are disabled.
      + LSHIFT(0,   28) // EOQF_RE   | Finished Request Enable. Enables the EOQF flag in the SR to generate an interrupt request                           | 0 EOQF interrupt requests are disabled.
      + LSHIFT(0,   27) // TFUF_RE   | Transmit FIFO Underflow Request Enable. Enables the TFUF flag in the SR to generate an interrupt request            | 0 TFUF interrupt requests are disabled.
      + LSHIFT(0,   25) // TFFF_RE   | Transmit FIFO Fill Request Enable. Enables the TFFF flag in the SR to generate a request                            | 1 TFFF interrupts or DMA requests are enabled
      + LSHIFT(1,   24) // TFFF_DIRS | Transmit FIFO Fill DMA or Interrupt Request Select. Selects between generating a DMA request or an interrupt request| 1 TFFF flag generates DMA requests.
      + LSHIFT(0,   19) // RFOF_RE   | Receive FIFO Overflow Request Enable. Enables the RFOF flag in the SR to generate an interrupt request              | 1 RFOF interrupt requests are enabled.
      + LSHIFT(0,   17) // RFDF_RE   | Receive FIFO Drain Request Enable. Enables the RFDF flag in the SR to generate a request.                           | 1 RFDF interrupt or DMA requests are enabled.
      + LSHIFT(1,   16) // RFDF_DIRS | Receive FIFO Drain DMA or Interrupt Request Select                                                                  | 1 DMA request.
    ;
  }
  else
  {
    SPI->RSER = 0
      + LSHIFT(0,   31) // TCF_RE    | Transmission Complete Request Enable. Enables TCF flag in the SR to generate an interrupt request                   | 0 TCF interrupt requests are disabled.
      + LSHIFT(0,   28) // EOQF_RE   | Finished Request Enable. Enables the EOQF flag in the SR to generate an interrupt request                           | 0 EOQF interrupt requests are disabled.
      + LSHIFT(0,   27) // TFUF_RE   | Transmit FIFO Underflow Request Enable. Enables the TFUF flag in the SR to generate an interrupt request            | 0 TFUF interrupt requests are disabled.
      + LSHIFT(0,   25) // TFFF_RE   | Transmit FIFO Fill Request Enable. Enables the TFFF flag in the SR to generate a request                            | 1 TFFF interrupts or DMA requests are enabled
      + LSHIFT(0,   24) // TFFF_DIRS | Transmit FIFO Fill DMA or Interrupt Request Select. Selects between generating a DMA request or an interrupt request| 1 TFFF flag generates DMA requests.
      + LSHIFT(0,   19) // RFOF_RE   | Receive FIFO Overflow Request Enable. Enables the RFOF flag in the SR to generate an interrupt request              | 1 RFOF interrupt requests are enabled.
      + LSHIFT(1,   17) // RFDF_RE   | Receive FIFO Drain Request Enable. Enables the RFDF flag in the SR to generate a request.                           | 1 RFDF interrupt or DMA requests are enabled.
      + LSHIFT(0,   16) // RFDF_DIRS | Receive FIFO Drain DMA or Interrupt Request Select                                                                  | 1 DMA request.
    ;
  }

  {
    OS_ERR     err; 
    OSFlagCreate(&spi_cbl[modn].spi_flags, "SPI_flags", (OS_FLAGS)0, &err);
  }

  if (enint != 0)
  {
    spi_errors[modn].rx_overfl_cnt = 0;
    spi_errors[modn].tx_uderfl_cnt = 0;

    NVIC_SetPriority((IRQn_Type)spi_mods[modn].int_num, spi_mods[modn].prio);
    NVIC_EnableIRQ((IRQn_Type)spi_mods[modn].int_num);
  }

  SPI->MCR &= ~BIT(HALT); // Запустить
}




/*-----------------------------------------------------------------------------------------------------
 
 \param modn 
 \param cs - маска активизируемого сигнала CS
 \param data 
-----------------------------------------------------------------------------------------------------*/
void SPI_push_data(uint8_t modn, uint32_t cs, uint16_t data)
{
  SPI_Type  *SPI   = spi_mods[modn].spi;
  SPI->PUSHR = (uint32_t)(cs | (uint32_t)data);
}
/*-----------------------------------------------------------------------------------------------------
 
 \param modn 
 
 \return _mqx_uint 
-----------------------------------------------------------------------------------------------------*/
uint32_t SPI_wait_tx_complete(uint8_t modn, uint8_t *pdata, uint32_t ticks)
{
  SPI_Type  *SPI   = spi_mods[modn].spi;
  OS_ERR   err;

  OSFlagPend(&spi_cbl[modn].spi_flags, (OS_FLAGS)SPI_TX_COMPLETE, ticks, OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME, (CPU_TS *)0U, &err);
  if (err != OS_ERR_NONE)
  {
    spi_cbl[modn].tx_err_cnt++;
    return  RESULT_ERROR;
  }
  if (pdata != 0) *pdata = SPI->POPR;
  else SPI->POPR;

  return RESULT_OK;
}


/*------------------------------------------------------------------------------



 \param modn
 ------------------------------------------------------------------------------*/
void SPI_clear_FIFO(uint8_t modn)
{
  SPI_Type  *SPI   = spi_mods[modn].spi;

  SPI->MCR |= BIT(HALT); // Остановить модуль
  while (SPI->SR & BIT(TXRXS)); // Ожидание пока бит 30 (TXRXS) не станет равным нулю

  SPI->SR = SPI->SR;
  SPI->MCR |= BIT(CLR_TXF) + BIT(CLR_RXF); // Установить флаги CLR_TXF и CLR_RXF для очистки обоих FIFO
  SPI->MCR &= ~BIT(HALT); // Запустить модуль
}


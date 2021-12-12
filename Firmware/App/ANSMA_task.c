// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017.05.09
// 14:10:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


uint32_t          flexio_verid;

         #define REGS_CNT 0x30
volatile uint32_t reg_data[REGS_CNT];

volatile uint32_t rd_cnt;
volatile uint32_t cmd;

volatile int16_t  angle_int;
volatile float    angle_float;
/*-----------------------------------------------------------------------------------------------------
 
 \param state 
-----------------------------------------------------------------------------------------------------*/
void CSQ_set_state(uint32_t state)
{
  if (state == 0) PTB->PCOR = BIT(1);
  else PTB->PSOR = BIT(1);
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void ANSMA_delay(uint32_t val)
{
  uint32_t i;
  for (i = 0; i < val; i++)
  {
    __no_operation();
  }
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void ANSMA_task_function(void)
{
  Set_LED_pattern(LED_BLINK, 0);


  SIM->SOPT2 = (SIM->SOPT2 & ~LSHIFT(3, 22)) | LSHIFT(1, 22); // FlexIO Module Clock Source Select. 01 MCGFLLCLK , or MCGPLLCLK, or IRC48M clock as selected by SOPT2[PLLFLLSEL].
  SIM->SCGC5 = SIM->SCGC5 | BIT(31); // FlexIO Clock Gate Control. 1 - Clock enabled

  flexio_verid = FLEXIO->VERID;

  FLEXIO->CTRL |= BIT(0); // 1 FlexIO module is enabled.

  // Конфигурирование FlexIO в режим CPHA= 1 (синхронизация по заднему фронту) и CPOL=0 ( сигнал клока начинается с нуля)

  // -------- Регистр 0 работает на передачу
  FLEXIO->SHIFTCFG[0] = 0 // 0x00000021; // Start bit loads data on first shift.
                        + LSHIFT(0,   8) // INSRC   | Input Source | 0 - Источник сигнала - внешний пин, 1 - Источник сигнала - выход следующего шифтера
                        + LSHIFT(2,   4) // SSTOP   | 10 Transmitter outputs stop bit value 0 on store, receiver/match store sets error flag if stop bit is not 0
                        + LSHIFT(1,   0) // SSTART  | 01 Start bit disabled for transmitter/receiver/match store, transmitter loads data on first shift
  ;
  FLEXIO->SHIFTCTL[0] = 0 // 0x00030002; // Configure transmit using Timer 0 on posedge of clock with output data on Pin 6.
                        + LSHIFT(0,   24) // TIMSEL | Selects which Timer is used for controlling the logic/shift register and generating the Shift clock.
                        + LSHIFT(0,   23) // TIMPOL | Timer Polarity | Shift on posedge of Shift clock
                        + LSHIFT(3,   16) // PINCFG | Shifter Pin Configuration | 11 Shifter pin output
                        + LSHIFT(6,   8)  // PINSEL | Selects which pin is used by the Shifter input or output.
                        + LSHIFT(0,   7)  // PINPOL | Shifter Pin Polarity | 0 Pin is active high
                        + LSHIFT(2,   0)  // SMOD   | Shifter Mode | Transmit mode. Load SHIFTBUF contents into the Shifter on expiration of the Timer.
  ;


  // -------- Регистр 1 работает на прием
  FLEXIO->SHIFTCFG[1] = 0 // 0x00000000; // Start and stop bit disabled.
                        + LSHIFT(0,   8) // INSRC   | Input Source | 0 - Источник сигнала - внешний пин, 1 - Источник сигнала - выход следующего шифтера
                        + LSHIFT(0,   4) // SSTOP   | 00 Stop bit disabled for transmitter/receiver/match store
                        + LSHIFT(0,   0) // SSTART  | 00 Start bit disabled for transmitter/receiver/match store, transmitter loads data on enable
  ;

  FLEXIO->SHIFTCTL[1] = 0 //0x00800101; // Configure receive using Timer 0 on negedge of clock with input data on Pin 7.
                        + LSHIFT(0,   24) // TIMSEL | Selects which Timer is used for controlling the logic/shift register and generating the Shift clock.
                        + LSHIFT(1,   23) // TIMPOL | Timer Polarity | 1 Shift on negedge of Shift clock
                        + LSHIFT(0,   16) // PINCFG | Shifter Pin Configuration | 00 Shifter pin output disabled
                        + LSHIFT(6,   8)  // PINSEL | Selects which pin is used by the Shifter input or output.
                        + LSHIFT(0,   7)  // PINPOL | Shifter Pin Polarity | 0 Pin is active high
                        + LSHIFT(1,   0)  // SMOD   | Shifter Mode | 001 Receive mode. Captures the current Shifter content into the SHIFTBUF on expiration of the Timer.
  ;


  // -------- Таймер 0 формирует тактовые импульсы
  FLEXIO->TIMCMP[0] = 0 //0x00003F01; // Configure 16-bit transfer with baud rate of divide by 4 of the FlexIO clock. Set TIMCMP[15:8] = (number of bits x 2) - 1. Set TIMCMP[7:0] = (baud rate divider /2) - 1.
                      + LSHIFT(0x1F,  8) // number of bits =  (N + 1) / 2.
                      + LSHIFT(2,    1) // baud rate divider = (N + 1) * 2
  ;
  FLEXIO->TIMCFG[0] = 0 // 0x01002222; // Configure start bit, stop bit, enable on trigger high and disable on compare, initial clock state is logic 0. Set PINPOL to invert the output shift clock. Set TIMDIS=3 to keep slave select asserted for as long as there is data in the transmit buffer.
                      + LSHIFT(1,   24) // TIMOUT  | Timer Output    | 01 Timer output is logic zero when enabled and is not affected by timer reset
                      + LSHIFT(0,   20) // TIMDEC  | Timer Decrement | 00 Decrement counter on FlexIO clock, Shift clock equals Timer output.
                      + LSHIFT(0,   16) // TIMRST  | Timer Reset     | 000 Timer never reset
                      + LSHIFT(2,   12) // TIMDIS  | Timer Disable   | 010 Timer disabled on Timer compare
                      + LSHIFT(2,   8)  // TIMENA  | Timer Enable    | 010 Timer enabled on Trigger high
                      + LSHIFT(2,   4)  // TSTOP   | Timer Stop Bit  | 10 Stop bit is enabled on timer disable
                      + LSHIFT(1,   1)  // TSTART  | Timer Start Bit | 1 Start bit enabled
  ;
  FLEXIO->TIMCTL[0] = 0 //0x01C30201; // Configure dual 8-bit counter using Pin 4 output (shift clock), with Shifter 0 flag as the inverted trigger.
                      + LSHIFT(1,   24) // TRGSEL  | Trigger Select   | 01 - shifter 0 status flag
                      + LSHIFT(1,   23) // TRGPOL  | Trigger Polarity | 1 Trigger active low
                      + LSHIFT(1,   22) // TRGSRC  | Trigger Source   | 1 Internal trigger selected
                      + LSHIFT(3,   16) // PINCFG  | Timer Pin Configuration | 11 Timer pin output
                      + LSHIFT(4,   8)  // PINSEL  | Timer Pin Select | Selects which pin is used by the Timer input or output.
                      + LSHIFT(0,   7)  // PINPOL  | Timer Pin Polarity | 0 Pin is active high
                      + LSHIFT(1,   0)  // TIMOD   | Timer Mode | 01 Dual 8-bit counters baud/bit mode.
  ;

  // -------- Таймер 1 формирует сигнал CS
  FLEXIO->TIMCMP[1] = 0x0000FFFF; // Never compare.
  FLEXIO->TIMCFG[1] = 0 //0x00001100; // Enable when Timer 0 is enabled and disable when Timer 0 is disabled.
                      + LSHIFT(0,   24) // TIMOUT  | Timer Output    | 00 Timer output is logic one when enabled and is not affected by timer reset
                      + LSHIFT(0,   20) // TIMDEC  | Timer Decrement | 00 Decrement counter on FlexIO clock, Shift clock equals Timer output.
                      + LSHIFT(0,   16) // TIMRST  | Timer Reset     | 000 Timer never reset
                      + LSHIFT(1,   12) // TIMDIS  | Timer Disable   | 001 Timer disabled on Timer N-1 disable
                      + LSHIFT(1,   8)  // TIMENA  | Timer Enable    | 001 Timer enabled on Timer N-1 enable
                      + LSHIFT(0,   4)  // TSTOP   | Timer Stop Bit  | 10 Stop bit is enabled on timer disable
                      + LSHIFT(0,   1)  // TSTART  | Timer Start Bit | 1 Start bit enabled
  ;
  FLEXIO->TIMCTL[1] = 0 //0x00030383; // Configure 16-bit counter (never compare) using inverted Pin 5 output (as slave select).
                      + LSHIFT(0,   24) // TRGSEL  | Trigger Select   | 00 - pin 2 input
                      + LSHIFT(0,   23) // TRGPOL  | Trigger Polarity | 0 Trigger active high
                      + LSHIFT(0,   22) // TRGSRC  | Trigger Source   | 0 External trigger selected
                      + LSHIFT(3,   16) // PINCFG  | Timer Pin Configuration | 11 Timer pin output
                      + LSHIFT(5,   8)  // PINSEL  | Timer Pin Select | Selects which pin is used by the Timer input or output.
                      + LSHIFT(1,   7)  // PINPOL  | Timer Pin Polarity | 1 Pin is active low
                      + LSHIFT(3,   0)  // TIMOD   | Timer Mode | 11 Single 16-bit counter mode.
  ;

//  FLEXIO->SHIFTBUF[0] = 0; // Transmit data can be written to SHIFTBUF, use the Shifter Status Flag to indicate when data can be written using interrupt or DMA request. Can support MSB first transfer by writing to SHIFTBUFBBS register instead.
//  FLEXIO->SHIFTBUF[1] = 0; // Received data can be read from SHIFTBUFBYS, use the Shifter Status Flag to indicate when data can be read using interrupt or DMA request. Can support MSB first transfer by reading from SHIFTBUFBIS register instead.

  rd_cnt = 4;
  cmd = 0x80030000;
  for (;;)
  {
    uint32_t i;
    _time_delay(1);


    CSQ_set_state(0);
    // Включаем выход чтобы начать запись
    FLEXIO->SHIFTCTL[0] = (FLEXIO->SHIFTCTL[0] & ~LSHIFT(3, 16)) | LSHIFT(3, 16);

    FLEXIO->SHIFTBUFBIS[0] = cmd; // Передача кода команды чтения регистра 0

    // Ожидаем окончания передачи
    // while ((FLEXIO->SHIFTSTAT & 0x01) == 0) __no_operation();
    ANSMA_delay(50);
    FLEXIO->SHIFTBUFBIS[1];


    //Отключаем выход чтобы начать чтение
    FLEXIO->SHIFTCTL[0] = (FLEXIO->SHIFTCTL[0] & ~LSHIFT(3, 16)) | LSHIFT(0, 16);

    for (i = 0; i < rd_cnt; i++)
    {
      FLEXIO->SHIFTBUFBIS[0] = 0x0; // Инициируем обмен путем начала передачи незначащих данных

      // Ожидаем окончания передачи
      // while ((FLEXIO->SHIFTSTAT & 0x02) == 0) __no_operation();
      ANSMA_delay(50);

      reg_data[i] = FLEXIO->SHIFTBUFBIS[1]; // Читаем приянятые данные

    }
    CSQ_set_state(1);
    
    //ANSMA_delay(1000);

    angle_int = (int16_t)(reg_data[2] & 0x7FFF);
    angle_float = (360.0 / 32768.0) * (float)angle_int;
  }


}

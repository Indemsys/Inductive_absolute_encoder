#include "BSP.h"

void Config_pin(const T_IO_pins_configuration pinc);


#include "ANSIN01_pins.h"
#include "ANSMA01_pins.h"

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void Config_pin(const T_IO_pins_configuration pinc)
{
  pinc.port->PCR[pinc.pin_num] = LSHIFT(pinc.irqc, 16) |
                                 LSHIFT(pinc.lock, 15) |
                                 LSHIFT(pinc.mux, 8) |
                                 LSHIFT(pinc.DSE, 6) |
                                 LSHIFT(pinc.ODE, 5) |
                                 LSHIFT(pinc.PFE, 4) |
                                 LSHIFT(pinc.SRE, 2) |
                                 LSHIFT(pinc.PUPD, 0);

  if (pinc.init == 0) pinc.gpio->PCOR = LSHIFT(1, pinc.pin_num);
  else pinc.gpio->PSOR = LSHIFT(1, pinc.pin_num);
  pinc.gpio->PDDR = (pinc.gpio->PDDR & ~LSHIFT(1, pinc.pin_num)) | LSHIFT(pinc.dir, pinc.pin_num);
}


/*------------------------------------------------------------------------------



 \return int
 ------------------------------------------------------------------------------*/
int Init_pins(void)
{
  int i;

  // Включаем тактирование на всех портах
  SIM->SCGC5 |=   SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;

  for (i = 0; i < (sizeof(ANSIN01_pins_conf) / sizeof(ANSIN01_pins_conf[0])); i++)
  {
#if APP_PLATF == ANSIN
    Config_pin(ANSIN01_pins_conf[i]);
#elif APP_PLATF == ANSMA
    Config_pin(ANSMA01_pins_conf[i]);
#else
   #error  Must be selected platform
#endif
 
  }

  return 0;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param state 
-----------------------------------------------------------------------------------------------------*/
void SPI0_CS0_set_state(uint32_t state)
{
  if (state == 0)
    PTD->PCOR = BIT(0);
  else
    PTD->PSOR = BIT(0);
}
/*-----------------------------------------------------------------------------------------------------
 
 \param state 
-----------------------------------------------------------------------------------------------------*/
void SPI0_CS1_set_state(uint32_t state)
{
  if (state == 0)
    PTD->PCOR = BIT(4);
  else
    PTD->PSOR = BIT(4);
}
/*-----------------------------------------------------------------------------------------------------
 
 \param state 
-----------------------------------------------------------------------------------------------------*/
void SPI0_CS2_set_state(uint32_t state)
{
  if (state == 0)
    PTD->PCOR = BIT(5);
  else
    PTD->PSOR = BIT(5);
}
/*-----------------------------------------------------------------------------------------------------
 
 \param state 
-----------------------------------------------------------------------------------------------------*/
void SPI0_CS3_set_state(uint32_t state)
{
  if (state == 0)
    PTD->PCOR = BIT(6);
  else
    PTD->PSOR = BIT(6);
}

/*-----------------------------------------------------------------------------------------------------
 
 \param state 
-----------------------------------------------------------------------------------------------------*/
void READY_set_state(uint32_t state)
{
  if (state == 0)
  {
    PTB->PCOR = BIT(1);
    //ITM_EVENT8(3, 0);
  }
  else
  {
    PTB->PSOR = BIT(1);
    //ITM_EVENT8(3, 1);
  }
}


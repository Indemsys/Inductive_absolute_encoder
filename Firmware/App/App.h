#ifndef APP_H
#define APP_H



#define PIT_INT_PERIOD  1000ul  // Сигнал от PIT каждую 1 мс
#define PI (3.14159265359)


// Здесь сведены в одно место все настройки всех каналов DMA 
// Настройка DMA включает:
//   - Выбор канала DMA. Каналов может быть 16. Канал выбирается произвольно, но у каналов есть приоритеты 
//   - Выбор мультиплексора DMA если их в чипе больше одного 
//   - Выбор входа у мультиплексора. Входв мультиплексоров жестко привязаны к определенной периферии
//   - Выбор номера прерывания DMA если использовано несколько каналов DMA для одного процесса обмена  
//


// -----------------------------------------------------------------------------
//  Статическое конфигурирование каналов DMA и DMA MUX для работы с ADC
#define ADC_SCAN_SZ                6
                                   
#define DMA_ADC0_RES_CH            3               // Занятый канал DMA для обслуживания ADC
#define DMA_ADC0_CFG_CH            2               // Занятый канал DMA для обслуживания ADC
#define DMA_ADC_DMUX_PTR           DMAMUX          // Указатель на модуль DMUX который используется для передачи сигналов от ADC к DMA
#define DMA_ADC0_DMUX_SRC          DMUX_SRC_ADC0   // Входы DMUX используемые для выбранного ADC
#define DMA_ADC_INT_NUM            DMA3_IRQn       // Номер вектора прерывания используемый в DMA для обслуживания ADC

// -----------------------------------------------------------------------------
//  Статическое конфигурирование каналов DMA и DMA MUX для работы с интерфейсом SPI микросхемы LDC1101 в проекте ANSIN

#define LDC1101_SPI                0                // Номер SPI модуля используемый для коммуникации с LDC1101
#define DMA_LDC1101_FM_CH          0                // Канал DMA для обслуживания приема по   SPI. FIFO->Память.
#define DMA_LDC1101_MF_CH          1                // Канал DMA для обслуживания передачи по SPI. Память->FIFO.
#define DMA_LDC1101_DMUX_PTR       DMAMUX           // Указатель на модуль DMUX который используется для передачи сигналов от контроллера SPI к DMA
#define DMA_LDC1101_DMUX_TX_SRC    DMUX_SRC_SPI0_TX // Вход DMUX используемый для вызова передачи по DMA
#define DMA_LDC1101_DMUX_RX_SRC    DMUX_SRC_SPI0_RX // Вход DMUX используемый для вызова приема по DMA
                                   
#define DMA_LDC1101_RX_INT_NUM     DMA0_IRQn       // Номер вектора прерывания используемый в DMA для обслуживания приема по SPI
#define LDC1101_DMA_ISR            DMA0_IRQHandler

// -----------------------------------------------------------------------------
//  Статическое конфигурирование каналов DMA и DMA MUX для работы с интерфейсом SPI связывающим с хост контроллером
//  Поскольку у SPI1 только один канал запроса DMA для приема и для передачи, то обмен ведется только симплексный 
//  
#define HOST_SPI                   1                   // Номер SPI модуля используемый для коммуникации с хостом
#define DMA_HOSTSPI_FM_CH          4                   // Канал DMA для обслуживания приема по   SPI. FIFO->Память.
#define DMA_HOSTSPI_MF_CH          5                   // Канал DMA для обслуживания передачи по SPI. Память->FIFO.
#define DMA_HOSTSPI_DMUX_PTR       DMAMUX              // Указатель на модуль DMUX который используется для передачи сигналов от контроллера SPI к DMA
#define DMA_HOSTSPI_DMUX_TX_SRC    DMUX_SRC_SPI1_RX_TX // Вход DMUX используемый для вызова передачи по DMA
#define DMA_HOSTSPI_DMUX_RX_SRC    DMUX_SRC_SPI1_RX_TX // Вход DMUX используемый для вызова приема по DMA

#define DMA_HOSTSPI_RX_INT_NUM     DMA4_IRQn           // Номер вектора прерывания используемый в DMA для обслуживания приема по SPI
#define HOSTSPI_RX_DMA_ISR         DMA4_IRQHandler

#define DMA_HOSTSPI_TX_INT_NUM     DMA5_IRQn           // Номер вектора прерывания используемый в DMA для обслуживания передачи по SPI
#define HOSTSPI_TX_DMA_ISR         DMA5_IRQHandler


#define FMSTR_SMPS_PORTD        0   // Источник сымплирования для recorder FreeMaster - прерывания на порте D от чипа LDC1101 

// Уровни приоритетов аппаратных прерываний.   Меньшая величина означает больший приоритет!!!
#define  APP_SYSTICK_PRIO          0x20  // (было 0x20)Аппаратный приоритет прерываний от системного таймера (эквивалент 2 для функции NVIC_SetPriority)
#define  USB_DEVICE_PRIO           0     // (было 2)   Аппратный приоритет прерываний от USB
#define  PIT0_PRIO                 0     // Аппратный приоритет прерываний от PIT0
#define  ADC_PRIO                  0     // Аппратный приоритет прерываний от ADC
#define  DMA_ADC_PRIO              0     // Аппратный приоритет прерываний от DMA 3
#define  LDC1101_DMA_SPI_PRIO      0     // Аппратный приоритет прерываний от DMA 0
#define  HOST_DMA_SPI_PRIO         0     // (было 1) Аппратный приоритет прерываний от DMA 5


#define  TASK_HOST_CHAN_PRIO      1
#define  TASK_HOST_CHAN_STACK_SZ  300
                                  
#define  TASK_APP_PRIO            2
#define  TASK_APP_STACK_SZ        300

#define  TASK_USBDEV_PRIO         3
#define  TASK_USBDEV_STACK_SZ     300
                                  
#define  TASK_FREEM_PRIO          4
#define  TASK_FREEM_STACK_SZ      300
                                  
#define  TASK_VT100_PRIO          5
#define  TASK_VT100_STACK_SZ      300
                                  
#define  TASK_MEASUR_PRIO         6
#define  TASK_MEASUR_STACK_SZ     300



#include "BSP.h"

#include "SEGGER_RTT.h"
#include "Debug_io.h"
#include "Task_FreeMaster.h"
#include "MonitorVT100.h"
#include "Monitor_usb_drv.h"
#include "OS_misc.h"
#include "Monitor_LDC1101.h"
#include "CRC_utils.h"
#include "LED_control.h"
#include "Mesurements.h"
#include "Host_channel.h"
#include "ANSIN_task.h"
#include "ANSMA_task.h"



typedef struct
{
  int32_t  ax; // Амплитуда X 
  int32_t  ox; // Смещение X
  int32_t  ay; // Амплитуда Y    
  int32_t  oy; // Смещение Y   
  uint16_t rcount;  
  uint16_t offset;            

} T_calibr_data;
//---------------------------------------------------
// Определение глобальных переменных
//---------------------------------------------------

#ifdef   _GLOBALS_

OS_TCB         task_app_tcb;
uint32_t       task_app_stack[TASK_APP_STACK_SZ];
               
OS_TCB         task_freem_tcb;
uint32_t       task_freem_stack[TASK_FREEM_STACK_SZ];
               
OS_TCB         task_usbdev_tcb;
uint32_t       task_usbdev_stack[TASK_USBDEV_STACK_SZ];
               
OS_TCB         task_vt100_1_tcb;
uint32_t       task_vt100_1_stack[TASK_VT100_STACK_SZ];

OS_TCB         task_measur_tcb;
uint32_t       task_measur_stack[TASK_MEASUR_STACK_SZ];

OS_TCB         task_host_chan_tcb;
uint32_t       task_host_chan_stack[TASK_HOST_CHAN_STACK_SZ];

//  OS_TCB   task_vt100_2_tcb;
//  uint32_t task_vt100_2_stack[TASK_VT100_STACK_SZ];

T_ADC_res      adcs;
int32_t        lhr_data[LDC1101_DEVS_CNT];
int32_t        lhr_data_sum;
               
float          xsig;
float          ysig;
uint32_t       g_fmstr_rate_src;
float          g_fmstr_smpls_period;
float          g_angle_smpl_period; // Период измерения  угла 
float          g_RPS ;              // Угловая скорость в оборотах в секунду 
 

T_calibr_data  cdata;  // Калибровочные данные 

float          g_angle; // Рассчитанный угол поворота

uint32_t       g_flash_erase_errors;
uint32_t       g_flash_write_errors;
uint32_t       g_flash_crc_errors;

float          g_int_temp;
float          g_ext_temp;

T_host_channel_cbl pcbl;
 
#else

extern OS_TCB         task_app_tcb;
extern uint32_t       task_app_stack[TASK_APP_STACK_SZ];
                      
extern OS_TCB         task_freem_tcb;
extern uint32_t       task_freem_stack[TASK_FREEM_STACK_SZ];
                      
extern OS_TCB         task_usbdev_tcb;
extern uint32_t       task_usbdev_stack[TASK_USBDEV_STACK_SZ];
                      
extern OS_TCB         task_vt100_1_tcb;
extern uint32_t       task_vt100_1_stack[TASK_VT100_STACK_SZ];

extern OS_TCB         task_measur_tcb;
extern uint32_t       task_measur_stack[TASK_MEASUR_STACK_SZ];

extern OS_TCB         task_host_chan_tcb;
extern uint32_t       task_host_chan_stack[TASK_HOST_CHAN_STACK_SZ];

//  extern OS_TCB   task_vt100_2_tcb;
//  extern uint32_t task_vt100_2_stack[TASK_VT100_STACK_SZ];

extern   T_ADC_res    adcs;
extern   int32_t      lhr_data[LDC1101_DEVS_CNT];
extern   int32_t      lhr_data_sum;
extern   float        xsig;
extern   float        ysig;
                      
extern   uint32_t     g_fmstr_rate_src;
extern   float        g_fmstr_smpls_period;
extern   float        g_angle_smpl_period; 
extern   float        g_RPS ;              // Угловая скорость в оборотах в секунду 


extern T_calibr_data  cdata;  // Калибровочные данные 

extern float          g_angle; // Рассчитанный угол поворота

extern uint32_t       g_flash_erase_errors;
extern uint32_t       g_flash_write_errors;
extern uint32_t       g_flash_crc_errors;
  
extern float          g_int_temp;
extern float          g_ext_temp;
  
extern T_host_channel_cbl pcbl;
  
  
#endif

extern uint32_t  __DATA_FLASH_SIZE[];
extern uint32_t  __CODE_FLASH_END[];


void USB_init_device(void);

//typedef struct
//{
//  OS_TCB                 tcb,          // task's TCB
//  CPU_CHAR              *p_name,       // is a pointer to an ASCII string to provide a name to the task.
//  OS_TASK_PTR            p_task,       // is a pointer to the task's code
//  void                  *p_arg,        // is a pointer to an optional data area which can be used to pass parameters
//  OS_PRIO                prio,         // is the task's priority. The lower the number, the higher the priority.
//  CPU_STK               *p_stk_base,   // is a pointer to the base address of the stack (i.e. low address).
//  CPU_STK_SIZE           stk_limit,    // is the number of stack elements to set as 'watermark' limit for the stack.
//  CPU_STK_SIZE           stk_size,     // is the size of the stack in number of elements.
//  OS_MSG_QTY             q_size,       // is the maximum number of messages that can be sent to the task
//  OS_TICK                time_quanta,  // amount of time (in ticks) for time slice when round-robin between tasks.  Specify 0 to use the default.
//  void                  *p_ext,        // is a pointer to a user supplied memory location which is used as a TCB extension.
//  OS_OPT                 opt,          // contains additional information (or options) about the behavior of the task.
//}
//T_task_descriptor;


uint32_t Save_app_settings(void);
uint32_t Load_app_settings(void);


#endif // APP_H




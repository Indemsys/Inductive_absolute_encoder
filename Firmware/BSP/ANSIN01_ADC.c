#include "App.h"


#define ADC_a_group  0
#define ADC_b_group  1

#define ADC_sw_trig  0 
#define ADC_hw_trig  1 

#define ADC0_config  0 
#define ADC1_config  1 

#define ADC_enable_int   1 
#define ADC_disable_int  0 

/*
  Инициализация ADC для режима работы синхронно с генератором PWM 
  Применяется аппаратный триггер запуска и DMA
  Программируем на 12-и битное преобразование
 
  Порядок взаимодействия периферии:
  - Инициализируется ADC с флагом DMA и стартом от аппаратного тригера
  - Инициализируется DMA.
      DMA использует два связанных канала. Первый канала пересылает результат ADC в память,
      следующий связанный канал загружает регистр ADC->SC1[0] и запускает следующее преобразование   
  - Инициализируется FTM (генератор PWM) с генерацией внешнего сигнала тригероа.
      FTM конфигурируется с центрально симметричным PWM. На серединах импульсов PWM генерируется сигнал тригера запускающий ADC
  - ADC от тригера FTM выполняет первое преобразование и генерирует прерывание.
  - В ADC прерывании считывается результат первого преобразования и конфигурируется запуск ADC от программного тригера.
      В том же прерывании программно запускается следующее преобразование ADC. 
  - После преобразования ADC запускает DMA которое последовательно несколько раз без пауз выполняет считывание результатов
      и запуск преобразования в остальных каналах ADC по заданной цепочке.
 
 
*/ 


// Массив байт для конфигурации регистра ADC->SC1[0]
#pragma data_alignment= 64 // Выравнивание необязательно
// Массив конфигурационный констант загружаемых в регистр ADC->SC1[0] каналом DMA перед началом каждого следующего преобразования
// Размер массива должен быть степенью двойки и согласован с параметром DEST_MODULO
const uint8_t  adc0_ch_cfg[ADC_SCAN_SZ+1] =
{
// --------------------------------------------------------------------------
//  Канал ADC0    Сигнал                                     Вход ADC 
// --------------------------------------------------------------------------
  12,     //       TEMP        - Термистор                  | ADC0_SE12
  12,     //       TEMP        - Термистор                  | ADC0_SE12
  12,     //       TEMP        - Термистор                  | ADC0_SE12
  26,     //                   - Temperature Sensor (S.E)   | 
  26,     //                   - Temperature Sensor (S.E)   | 
  26,     //                   - Temperature Sensor (S.E)   | 
};


T_DMA_TCD  sw_trig_tcd; 


#define ADC_DMA_DEST_ADDR_MODULO 4    // Количество вариабельных бит адреса назначения при DMA пересылке. 
                                      // 4 бит соответствует адресации  8 выборок по 2 байта каждая

// Массив результатов преобразования ADC0 заполняемый по DMA 
#pragma data_alignment= 64 // Выравнивание
volatile uint16_t adc0_results[ADC_SCAN_SZ];


// Массив структур для инициализации каналов DMA обслуживающих 2-а модуля ADC.
const T_init_ADC_DMA_cbl init_ADC_DMA_cbl[1] =
{
  { ADC0, adc0_results, adc0_ch_cfg, DMA_ADC0_RES_CH, DMA_ADC0_CFG_CH, DMA_ADC0_DMUX_SRC },
};

static T_ADC_state adc_state;


static void ADC_start_DMA(uint8_t n, uint8_t enint);


/*-----------------------------------------------------------------------------------------------------
  Прерывание после первого преобразования ADC
-----------------------------------------------------------------------------------------------------*/
void ADC0_IRQHandler(void)
{
  // Результат прошедшего преобразования уже скопирован в память модулем DMA

  // Устанавливаем программный триггер у обоих ADC
  ADC0->SC2 &= ~BIT(6);

  // Запрещаем прерывания и запускаем следующее преобразование
  ADC0->SC1[0] = adc0_ch_cfg[1];

  // Далее начинает работу DMA и завершает автоматически преобразование всех остальных каналов 
}

/*-------------------------------------------------------------------------------------------------------------
  Прерывание после окончания мажорного цикла канала  DMA
-------------------------------------------------------------------------------------------------------------*/
void DMA3_IRQHandler(void)
{
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();                                               /* Tell uC/OS-III that we are starting an ISR           */
  CPU_CRITICAL_EXIT();

  DMA0->INT = BIT(DMA_ADC0_RES_CH); // Сбрасываем флаг прерываний  канала DMA 

  // Копируем результаты и памяти приема по DMA в рабочую переменную adcs
  adcs.smpl_TERM_1   = adc0_results[0];
  adcs.smpl_TERM_2   = adc0_results[1];
  adcs.smpl_TERM_3   = adc0_results[2];
  adcs.smpl_ITEMP_1  = adc0_results[3];
  adcs.smpl_ITEMP_2  = adc0_results[4];
  adcs.smpl_ITEMP_3  = adc0_results[5];

  // Подготавливаемся к первому преобразованию ADC с вызовом прерываний 
  // Устанавливаем аппаратный триггер у обоих ADC
  ADC0->SC2 |= BIT(6);

  ADC0->SC1[0] = BIT(6) + adc0_ch_cfg[0];


  ADC_start_DMA(ADC0_config, ADC_enable_int);  // Разрешаем прерывания от этого канала DMA


  Measurement_post_flag();

  OSIntExit();
}

/*-------------------------------------------------------------------------------------------------------------
  Инициализируем DMA для автоматической выборки из заданных ADC каналов
  По мотивам Document Number: AN4590 "Using DMA to Emulate ADC Flexible Scan Mode on Kinetis K Series"

  Здесь используется механизм связанного вызова минорного цикла одного канала DMA по окончании минорного цикла другого канала DMA


  n     -  номер конфигурационной записи в массиве init_ADC_DMA_cbl
  enint - флаг разрешения прерывания от данной конфигурации
-------------------------------------------------------------------------------------------------------------*/
static void ADC_start_DMA(uint8_t n, uint8_t enint)
{


  ADC_Type          *ADC    = init_ADC_DMA_cbl[n].ADC;
  uint8_t            res_ch = init_ADC_DMA_cbl[n].results_channel; // Канал res_ch DMA (более приоритетный) считывает данные из ADC
  uint8_t            cfg_ch = init_ADC_DMA_cbl[n].config_channel;  // Канал cfg_ch DMA (менее приоритетный) загружает новым значением регистр ADC->SC1[0]


  // Конфигурируем более приоритетный канал пересылки данных из ADC
  DMA0->TCD[res_ch].SADDR = (uint32_t)&ADC->R[0];      // Источник - регистр данных ADC
  DMA0->TCD[res_ch].SOFF = 0;                        // Адрес источника не изменяется после чтения
  DMA0->TCD[res_ch].SLAST = 0;                       // Не корректируем адрес источника после завершения всего цикла DMA (окончания мажорного цикла)
  DMA0->TCD[res_ch].DADDR = (uint32_t)&init_ADC_DMA_cbl[n].results_vals[0]; // Адрес приемника данных из ADC - массив отсчетов
  DMA0->TCD[res_ch].DOFF = 2;                        // После каждой записи смещаем указатель приемника на два байта
  DMA0->TCD[res_ch].DLAST_SGA = 0;                   // Здесь не корректируем адреса приемника после мажорного цикла. Для коррекции используем поле DMOD в регистре ATTR
                                                    // DMA->TCD[res_ch].DLAST_SGA = (uint32_t)(-ADC_SCAN_SZ*2);  // Здесь корректируем адреса приемника после мажорного цикла.
  DMA0->TCD[res_ch].NBYTES_MLNO = 2;                 // Количество байт пересылаемых за один запрос DMA (в минорном цикле)
  DMA0->TCD[res_ch].BITER_ELINKNO = 0
    + LSHIFT(1, 15)           // ELINK  | Включаем линковку к минорному циклу другого канала
    + LSHIFT(cfg_ch, 9)       // LINKCH | Линкуемся к каналу cfg_ch
    + LSHIFT(ADC_SCAN_SZ, 0)  // BITER  | Количество итераций в мажорном цикле
  ;
  DMA0->TCD[res_ch].CITER_ELINKNO = 0
    + LSHIFT(1, 15)           // ELINK  | Включаем линковку к минорному циклу другого канала
    + LSHIFT(cfg_ch, 9)       // LINKCH | Линкуемся к каналу cfg_ch
    + LSHIFT(ADC_SCAN_SZ, 0)  // BITER  | Количество итераций в мажорном цикле
  ;
  DMA0->TCD[res_ch].ATTR = 0
    + LSHIFT(0, 11) // SMOD  | Модуль адреса источника. Не используем
    + LSHIFT(1, 8)  // SSIZE | 16-и битная пересылка из источника
    + LSHIFT(0, 3)  // DMOD  | Модуль адреса приемника. Не используем
    + LSHIFT(1, 0)  // DSIZE | 16-и битная пересылка в приемник
  ;
  DMA0->TCD[res_ch].CSR = 0
    + LSHIFT(3, 14) // BWC         | Bandwidth Control. 00 No eDMA engine stalls
    + LSHIFT(0, 8)  // MAJORLINKCH | Линк мажорного цикла. Минорный линк не срабатывает на последней минорной пересылке. Если нужно, то линк надо настроить и для мажорного цикла
    + LSHIFT(0, 7)  // DONE        | This flag indicates the eDMA has completed the major loop.
    + LSHIFT(0, 6)  // ACTIVE      | This flag signals the channel is currently in execution
    + LSHIFT(0, 5)  // MAJORELINK  | После завершения мажорного цикла не линкуеся ни к чему
    + LSHIFT(0, 4)  // ESG         | Enable Scatter/Gather Processing
    + LSHIFT(1, 3)  // DREQ        | Disable Request. If this flag is set, the eDMA hardware automatically clears the corresponding ERQ bit when the current major iteration count reaches zero.
    + LSHIFT(0, 2)  // INTHALF     | Enable an interrupt when major counter is half complete
    + LSHIFT(enint, 1)  // INTMAJOR    | Разрешаем прерывание после мажорного цикла
    + LSHIFT(0, 0)  // START       | Channel Start. If this flag is set, the channel is requesting service.
  ;

  // Конфигурируем менее приоритетный канала пересылки настройки в ADC
  DMA0->TCD[cfg_ch].SADDR = (uint32_t)&init_ADC_DMA_cbl[n].config_vals[1];     // Источник - массив настроек для ADC
  DMA0->TCD[cfg_ch].SOFF = 1;                        // Адрес источника после прочтения смещается на 1 байт вперед
  DMA0->TCD[cfg_ch].SLAST = 0;                       // Корректируем адрес источника после завершения всего цикла DMA (окончания мажорного цикла)
  DMA0->TCD[cfg_ch].DADDR = (uint32_t)&ADC->SC1[0];  // Адрес приемника данных - регистр управления ADC
  DMA0->TCD[cfg_ch].DOFF = 0;                        // Указатель приемника неменяем после записи
  DMA0->TCD[cfg_ch].DLAST_SGA = 0;                   // Коррекцию адреса приемника не производим после окончания всей цепочки сканирования (окончания мажорного цикла)
  DMA0->TCD[cfg_ch].NBYTES_MLNO = 1;                 // Количество байт пересылаемых за один запрос DMA (в минорном цикле)
  DMA0->TCD[cfg_ch].BITER_ELINKNO = 0
    + LSHIFT(0, 15)           // ELINK  | Линковка выключена
    + LSHIFT(0, 9)            // LINKCH |
    + LSHIFT(ADC_SCAN_SZ, 0)  // BITER  | Количество итераций в мажорном цикле
  ;
  DMA0->TCD[cfg_ch].CITER_ELINKNO = 0
    + LSHIFT(0, 15)           // ELINK  | Линковка выключена
    + LSHIFT(0, 9)            // LINKCH |
    + LSHIFT(ADC_SCAN_SZ, 0)  // BITER  | Количество итераций в мажорном цикле
  ;
  DMA0->TCD[cfg_ch].ATTR = 0
    + LSHIFT(0, 11) // SMOD  | Модуль адреса источника не используем
    + LSHIFT(0, 8)  // SSIZE | 8-и битная пересылка из источника
    + LSHIFT(0, 3)  // DMOD  | Модуль адреса приемника не используем
    + LSHIFT(0, 0)  // DSIZE | 8-и битная пересылка в приемник
  ;
  DMA0->TCD[cfg_ch].CSR = 0
    + LSHIFT(3, 14) // BWC         | Bandwidth Control. 00 No eDMA engine stalls
    + LSHIFT(0, 8)  // MAJORLINKCH | Link Channel Number
    + LSHIFT(0, 7)  // DONE        | This flag indicates the eDMA has completed the major loop.
    + LSHIFT(0, 6)  // ACTIVE      | This flag signals the channel is currently in execution
    + LSHIFT(0, 5)  // MAJORELINK  | Линкуемся к каналу 0  после завершения мажорного цикла
    + LSHIFT(0, 4)  // ESG         | Enable Scatter/Gather Processing
    + LSHIFT(0, 3)  // DREQ        | Disable Request. If this flag is set, the eDMA hardware automatically clears the corresponding ERQ bit when the current major iteration count reaches zero.
    + LSHIFT(0, 2)  // INTHALF     | Enable an interrupt when major counter is half complete
    + LSHIFT(0, 1)  // INTMAJOR    | Прерывания не используем
    + LSHIFT(0, 0)  // START       | Channel Start. If this flag is set, the channel is requesting service.
  ;

  DMA0->SERQ = res_ch;                                           // Разрешаем запросы от внешней периферии для канала DMA с номером res_ch
}

/*-----------------------------------------------------------------------------------------------------
 
 \param n  - номер конфигурационной записи в массиве init_ADC_DMA_cbl
-----------------------------------------------------------------------------------------------------*/
void ADC_config_DMA_MUX(uint8_t n)
{
  uint8_t            res_ch = init_ADC_DMA_cbl[n].results_channel; // Канал res_ch DMA (более приоритетный) считывает данные из ADC
  DMAMUX->CHCFG[res_ch] = init_ADC_DMA_cbl[n].req_src + BIT(7);    // Через мультиплексор связываем сигнал от внешней периферии (здесь от канала ADC) с входом выбранного канала DMA
                                                                   // BIT(7) означает DMA Channel Enable
}

/*-------------------------------------------------------------------------------------------------------------
  Инициализируем DMA для автоматической выборки из заданных ADC каналов
-------------------------------------------------------------------------------------------------------------*/
void ADC_config_start_DMA(void)
{

  NVIC_SetPriority((IRQn_Type)DMA_ADC_INT_NUM, DMA_ADC_PRIO);
  NVIC_EnableIRQ((IRQn_Type)DMA_ADC_INT_NUM);

  ADC_config_DMA_MUX(ADC0_config);

  ADC_start_DMA(ADC0_config, ADC_enable_int);  
}


/*-------------------------------------------------------------------------------------------------------------
 Включаем тактирование всех ADC
-------------------------------------------------------------------------------------------------------------*/
void ADC_switch_on_all(void)
{
  SIM->SCGC6 |= BIT(27); // Тактирование ADC0
}



/*-------------------------------------------------------------------------------------------------------------

 


  ab - Выбор между каналами с суфксами a или b. 0 - a, 1 - b
  hw_trig - флаг включения аппаратного тригера 
-------------------------------------------------------------------------------------------------------------*/
static void ADC_config(ADC_Type *ADC, uint32_t ab, uint32_t hw_trig)
{
  uint8_t        adiv;
  // Подбираем делитель для тактовой частоты ADC
  if (CPU_BUS_CLK_HZ <= 48)
  {
    adiv = 2; // Деление на 4. Тактовая равна 12 МГц 
  }
  else
  {
    adiv = 3; // Деление на 8. Тактовая равна 7.5 МГц если  CPU_BUS_CLK_HZ = 60 МГц 
  }

  // Для 16 битного преобразования частота тактирования должна быть в пределах 2-12 МГц для семейства KS22
  // Для 13 битного преобразования частота тактирования должна быть в пределах 2-24 МГц для семейства KS22
  // Оптимальная частота для получения максимальной точности 6 МГц 
  ADC->CFG1 = 0
              + LSHIFT(0, 7) // ADLPC.  Low power configuration. The power is reduced at the expense of maximum clock speed. 0 Normal power configuration
              + LSHIFT(adiv, 5) // ADIV.   Clock divide select. 11 - The divide ratio is 8 and the clock rate is (input clock)/8. = 4.6875 MHz
              + LSHIFT(0, 4) // ADLSMP. Sample time configuration. 0 Short sample time.
              + LSHIFT(1, 2) // MODE.   Conversion mode selection. When DIFF=0:It is single-ended 12-bit conversion ; when DIFF=1, it is differential 13-bit conversion with 2's complement output.
              + LSHIFT(0, 0) // ADICLK. Input clock select. 00 Bus clock 
  ;

  ADC->CFG2 = 0
              + LSHIFT(ab & 1, 4) // MUXSEL.  0 ADxxa channels are selected. Выбор между каналами с суфксами a или b
              + LSHIFT(0, 3) // ADACKEN. Asynchronous clock output enable
              + LSHIFT(1, 2) // ADHSC.   High speed configuration. 0 Normal conversion sequence selected.
              + LSHIFT(0, 0) // ADLSTS.  Default longest sample time (20 extra ADCK cycles; 24 ADCK cycles total). If ADLSMP = 1
  ;

  // Регистр статуса и управления. Выбор типа тригера, управление функцией сравнения, разрешение DMA, выбор типа опоры
  // Выбор источника сигнала  тригера для ADC производится в регистре SIM_SOPT7.
  // Источниками сигнала триггера могут быть PDB 0-3, High speed comparator 0-3, PIT 0-3, FTM 0-3, RTC, LP Timer,

  ADC->SC2 = 0
             + LSHIFT(0, 7) // ADACT.   Read only. 1 Conversion in progress.
             + LSHIFT(hw_trig, 6) // ADTRG.   Conversion trigger select. 0 Software trigger selected. 1 Hardware trigger selected.
             + LSHIFT(0, 5) // ACFE.    Compare function enable. 0 Compare function disabled.
             + LSHIFT(0, 4) // ACFGT.   Compare function greater than enable
             + LSHIFT(0, 3) // ACREN.   Compare function range enable
             + LSHIFT(1, 2) // DMAEN.   DMA enable
             + LSHIFT(0, 0) // REFSEL.  Voltage reference selection. 00 Default voltage reference pin pair (external pins VREFH and VREFL)
  ;
  // Регистр статуса и управления. Управление и статус калибровки, управление усреднением, управление непрерывным преобразованием
  ADC->SC3 = 0
             + LSHIFT(0, 7) // CAL.     CAL begins the calibration sequence when set.
             + LSHIFT(0, 3) // ADCO.    Continuous conversion enable
             + LSHIFT(0, 2) // AVGE.
             + LSHIFT(0, 0) // AVGS.
  ;


  // Управляющий регистр B
  // Результат управляемый этим регистром будет находиться в регистре ADC->RB
  // Используеться только при включенном аппаратном триггере
  ADC->SC1[1] = 0
                + LSHIFT(0, 7) // COCO. Read only. 1 Conversion completed.
                + LSHIFT(0, 6) // AIEN. 1 Conversion complete interrupt enabled.
                + LSHIFT(0, 5) // DIFF. 1 Differential conversions and input channels are selected.
                + LSHIFT(0, 0) // ADCH. Input channel select. 11111 Module disabled.
                               //                             00000 When DIFF=0, DADP0 is selected as input; when DIFF=1, DAD0 is selected as input.
  ;
  // Управляющий регистр A
  // Результат управляемый этим регистром будет находиться в регистре ADC0->RA
  // Запись в данный регист либо прерывает текущее преобразование либо инициирует новое (если установлен флаг софтварныого тригера - ADTRG = 0)
  ADC->SC1[0] = 0
                + LSHIFT(0, 7) // COCO. Read only. 1 Conversion completed.
                + LSHIFT(0, 6) // AIEN. 1 Conversion complete interrupt enabled.
                + LSHIFT(0, 5) // DIFF. 1 Differential conversions and input channels are selected.
                + LSHIFT(0, 0) // ADCH. Input channel select. 11111 Module disabled.
                               //                             00000 When DIFF=0, DADP0 is selected as input; when DIFF=1, DAD0 is selected as input.
  ;

}

/*-------------------------------------------------------------------------------------------------------------


   Устанавливаем ADICLK = 0, т.е. тактирование от  Bus clock он же Peripheral Clock поделенной на 2 = 30 Mhz

   Во время калибровки рекомендуется установить чатоту тактирования меньшей или равной 4 МГц
    и установить максимальную глубину усреднения
-------------------------------------------------------------------------------------------------------------*/
int32_t ADC_calibrating(ADC_Type *ADC)
{

  unsigned short tmp;
  uint8_t        adiv;

  // Подбираем делитель для тактовой частоты ADC
  if (CPU_BUS_CLK_HZ <= 48)
  {
    adiv = 3; // Деление на 8. Тактовая равна 6 МГц 
  }
  else
  {
    adiv = 3; // Деление на 8. Тактовая равна 7.5 МГц если  CPU_BUS_CLK_HZ = 60 МГц 
  }


  ADC->SC1[0] = 0
                + LSHIFT(0, 7) // COCO. Read only. 1 Conversion completed.
                + LSHIFT(0, 6) // AIEN. 1 Conversion complete interrupt enabled.
                + LSHIFT(1, 5) // DIFF. 1 Differential conversions and input channels are selected.
                + LSHIFT(0, 0) // ADCH. Input channel select. 11111 Module disabled.
                               //                             00000 When DIFF=0, DADP0 is selected as input; when DIFF=1, DAD0 is selected as input.
  ;
  // Для 16 битного преобразования частота тактирования должна быть в пределах 2-12 МГц для семейства KS22
  // Для 13 битного преобразования частота тактирования должна быть в пределах 2-24 МГц для семейства KS22
  // Оптимальная частота для получения максимальной точности 6 МГц 
  ADC->CFG1 = 0
              + LSHIFT(0, 7) // ADLPC.  Low power configuration. The power is reduced at the expense of maximum clock speed. 0 Normal power configuration
              + LSHIFT(adiv, 5) // ADIV.   Clock divide select. 11 - The divide ratio is 8 and the clock rate is (input clock)/8. = 4.6875 MHz
              + LSHIFT(0, 4) // ADLSMP. Sample time configuration. 0 Short sample time.
              + LSHIFT(1, 2) // MODE.   Conversion mode selection. When DIFF=0:It is single-ended 12-bit conversion ; when DIFF=1, it is differential 13-bit conversion with 2's complement output.
              + LSHIFT(0, 0) // ADICLK. Input clock select. 00 Bus clock 
  ;
  ADC->CFG2 = 0
              + LSHIFT(0, 4) // MUXSEL.  0 ADxxa channels are selected.
              + LSHIFT(0, 3) // ADACKEN. Asynchronous clock output enable
              + LSHIFT(1, 2) // ADHSC.   High speed configuration. 0 Normal conversion sequence selected.
              + LSHIFT(0, 0) // ADLSTS.  Long sample time select
  ;
  // Регистр статуса и управления. Выбор типа тригера, управление функцией сравнения, разрешение DMA, выбор типа опоры
  ADC->SC2 = 0
             + LSHIFT(0, 7) // ADACT.   Read only. 1 Conversion in progress.
             + LSHIFT(0, 6) // ADTRG.   Conversion trigger select. 0 Software trigger selected.
             + LSHIFT(0, 5) // ACFE.    Compare function enable. 0 Compare function disabled.
             + LSHIFT(0, 4) // ACFGT.   Compare function greater than enable
             + LSHIFT(0, 3) // ACREN.   Compare function range enable
             + LSHIFT(0, 2) // DMAEN.   DMA enable
             + LSHIFT(0, 0) // REFSEL.  Voltage reference selection. 00 Default voltage reference pin pair (external pins VREFH and VREFL)
  ;
  ADC->SC3 = 0
             + LSHIFT(1, 7) // CAL.     CAL begins the calibration sequence when set.
             + LSHIFT(1, 6) // CALF.    Read Only. 1 Calibration failed.
             + LSHIFT(0, 3) // ADCO.    Continuous conversion enable
             + LSHIFT(1, 2) // AVGE.    1 Hardware average function enabled.
             + LSHIFT(3, 0) // AVGS.    11 32 samples averaged.
  ;

  // Ожидать завершения калибровки
  while (ADC->SC3 & BIT(7));

  tmp = ADC->CLP0;
  tmp += ADC->CLP1;
  tmp += ADC->CLP2;
  tmp += ADC->CLP3;
  tmp += ADC->CLP4;
  tmp += ADC->CLPS;
  tmp /= 2;
  ADC->PG = tmp | 0x8000;

  tmp =  ADC->CLM0;
  tmp += ADC->CLM1;
  tmp += ADC->CLM2;
  tmp += ADC->CLM3;
  tmp += ADC->CLM4;
  tmp += ADC->CLMS;
  tmp /= 2;
  ADC->MG = tmp | 0x8000;

  if (ADC->SC3 & BIT(6))
  {
    return RESULT_ERROR;
  }
  else
  {
    return RESULT_OK;
  }
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
T_ADC_state* ADC_get_state(void)
{
  return &adc_state;
}


/*-------------------------------------------------------------------------------------------------------------
  Подготовка подсистемы  ADC к работе.
  Калибровка и старт работы всех конвертеров ADC
-------------------------------------------------------------------------------------------------------------*/
void ADC_calibr_config_start(void)
{

  ADC_switch_on_all();
  adc_state.adc0_cal_res = ADC_calibrating(ADC0);  // Проводим процедуру калибровки модулей ADC

  ADC_config(ADC0, ADC_b_group, ADC_hw_trig);  // 

  // Конфигурируем номера каналов измеряемых первым преобразованием после аппаратного триггера
  ADC0->SC1[0] = BIT(6) + adc0_ch_cfg[0];

  NVIC_SetPriority((IRQn_Type)ADC0_IRQn, ADC_PRIO);
  NVIC_EnableIRQ((IRQn_Type)ADC0_IRQn);

  
  
  ADC_config_start_DMA();

  // Устновка источника сигналов аппаратных триггеров для ADC. Выбираем PIT 0 тригер
  SIM->SOPT7 = 0
               + LSHIFT(1,  7) // ADC0ALTTRGEN  | ADC alternate trigger enable | 1 Alternate trigger selected for ADC0.
               + LSHIFT(0,  4) // ADC0PRETRGSEL | ADC pre-trigger select       | 0 Pre-trigger A selected for ADC0. Выбор управляющего регистра ADC0 (A или B), который будет активизировать тригер
               + LSHIFT(4,  0) // ADC0TRGSEL    | ADC trigger select           | 0100 PIT trigger 0, 1011 FTM3 trigger
  ;


}


/*-------------------------------------------------------------------------------------------------------------
  Получить массив отсчетов из других задач
-------------------------------------------------------------------------------------------------------------*/
void Get_ADC_samples(T_ADC_res **pp_adc_res)
{
  *pp_adc_res = &adcs;
}



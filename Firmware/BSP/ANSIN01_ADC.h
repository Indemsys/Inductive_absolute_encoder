#ifndef ANSIN01_ADC
  #define ANSIN01_ADC



#define ADC_AVER_4   1
#define ADC_AVER_8   2
#define ADC_AVER_16  3
#define ADC_AVER_32  4



typedef struct
{
  ADC_Type                    *ADC;                           //  Указатель на модуль ADC
  unsigned short volatile     *results_vals;   //  Указатель на буффер с результатами ADC
  unsigned char const         *config_vals;    //  Указатель на массив констант содержащих конфигурацию ADC
  uint8_t                     results_channel; //  Номер канала DMA пересылающий резульат ADC.     Более приоритетный. По умолчанию канал с большим номером имеет больший приоритет.
  uint8_t                     config_channel;  //  Номер канала DMA пересылающий конфигурацию ADC. Менее приоритетный
  uint8_t                     req_src;         //  Номер источника запросов для конфигурирования мультиплексора DMAMUX
} T_init_ADC_DMA_cbl;


typedef struct
{
  // Результаты калибровки модулей ADC
  int32_t adc0_cal_res;
} T_ADC_state;


typedef struct
{
  uint16_t smpl_TERM_1   ; 
  uint16_t smpl_TERM_2   ; 
  uint16_t smpl_TERM_3   ; 
  uint16_t smpl_ITEMP_1  ; 
  uint16_t smpl_ITEMP_2  ; 
  uint16_t smpl_ITEMP_3  ; 

}
T_ADC_res;




typedef struct
{
  const char   *name;
  float        (*int_converter)(int);
  float        (*flt_converter)(float);

} T_vals_scaling;

typedef void(*T_adc_isr_callback)(void); 

void ADC_config_start_DMA(void);

void ADC_calibr_config_start(void);
void ADC_switch_on_all(void);
int  ADC_calibrating(ADC_Type *ADC);


T_ADC_state *ADC_get_state(void);

void Get_ADC_samples(T_ADC_res **pp_adc_res);
void Copy_to_ADC_res(uint8_t n, uint16_t *buf);



#endif

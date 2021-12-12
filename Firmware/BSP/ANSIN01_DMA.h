#ifndef ANSIN01_DMA_H
#define ANSIN01_DMA_H

#define DMA_CHANNELS_COUNT  16

#define MAX_DMA_SPI_BUFF    2048

// Номера источников запросов для мультиплесора MUX запросов DMA
// 32-а регистра DMAMUX->CHCFG управляют мультиплексированием запросов с 63 периферийных сигналов на 32 входа контролера DMA

#define DMUX_SRC_DISABLED               0   // Channel disabled1
#define DMUX_SRC_RESERV1                1   //
#define DMUX_SRC_UART0_RX               2   // Receive
#define DMUX_SRC_UART0_TX               3   // Transmit
#define DMUX_SRC_UART1_RX               4   // Receive
#define DMUX_SRC_UART1_TX               5   // Transmit
#define DMUX_SRC_UART2_RX               6   // Receive
#define DMUX_SRC_UART2_TX               7   // Transmit
#define DMUX_SRC_RESERV2                8   //
#define DMUX_SRC_RESERV3                9   //
#define DMUX_SRC_RESERV4                10  //
#define DMUX_SRC_RESERV5                11  //
#define DMUX_SRC_I2S0_RX                12  // Receive
#define DMUX_SRC_I2S0_TX                13  // Transmit
#define DMUX_SRC_SPI0_RX                14  // Receive
#define DMUX_SRC_SPI0_TX                15  // Transmit
#define DMUX_SRC_SPI1_RX_TX             16  // Receive
#define DMUX_SRC_RESERV6                17  //
#define DMUX_SRC_LPI2C0                 18  //
#define DMUX_SRC_LPI2C1                 19  //
#define DMUX_SRC_FTM0_CH0               20  // Channel 0
#define DMUX_SRC_FTM0_CH1               21  // Channel 1
#define DMUX_SRC_FTM0_CH2               22  // Channel 2
#define DMUX_SRC_FTM0_CH3               23  // Channel 3
#define DMUX_SRC_FTM0_CH4               24  // Channel 4
#define DMUX_SRC_FTM0_CH5               25  // Channel 5
#define DMUX_SRC_LPI2C0_                26  // Channel 6
#define DMUX_SRC_LPI2C1_                27  // Channel 7
#define DMUX_SRC_FTM1_TPM1_CH0          28  // Channel 0
#define DMUX_SRC_FTM1_TPM1_CH1          29  // Channel 1
#define DMUX_SRC_FTM2_TPM2_CH0          30  // Channel 0
#define DMUX_SRC_FTM2_TPM2_CH1          31  // Channel 1
#define DMUX_SRC_RESERV7                32  //
#define DMUX_SRC_RESERV8                33  //
#define DMUX_SRC_RESERV9                34  //
#define DMUX_SRC_RESERV10               35  //
#define DMUX_SRC_RESERV11               36  //
#define DMUX_SRC_RESERV12               37  //
#define DMUX_SRC_FLEXIO_0               38  //
#define DMUX_SRC_FLEXIO_1               39  //
#define DMUX_SRC_ADC0                   40  // —
#define DMUX_SRC_FLEXCAN1               41  // —
#define DMUX_SRC_CMP0                   42  // —
#define DMUX_SRC_FLEXIO2                43  // —
#define DMUX_SRC_FLEXIO3                44  // —
#define DMUX_SRC_DAC0                   45  // —
#define DMUX_SRC_I2S1_RX                46  // —
#define DMUX_SRC_I2S1_TX                47  // —
#define DMUX_SRC_PDB                    48  // —
#define DMUX_SRC_PortA_control_module   49  // Port A
#define DMUX_SRC_PortB_control_module   50  // Port B
#define DMUX_SRC_PortC_control_module   51  // Port C
#define DMUX_SRC_PortD_control_module   52  // Port D
#define DMUX_SRC_PortE_control_module   53  // Port E
#define DMUX_SRC_TPM0                   54  //
#define DMUX_SRC_TPM1                   55  //
#define DMUX_SRC_TPM2                   56  //
#define DMUX_SRC_RESERV13               57  //
#define DMUX_SRC_LPUART0_RX             58  //
#define DMUX_SRC_LPUART0_TX             59  //
#define DMUX_SRC_Always_enabled1        60  // Always enabled
#define DMUX_SRC_Always_enabled2        61  // Always enabled
#define DMUX_SRC_Always_enabled3        62  // Always enabled
#define DMUX_SRC_Always_enabled4        63  // Always enabled

#define DMA_1BYTE_MINOR_TRANSFER    1
#define DMA_2BYTE_MINOR_TRANSFER    2
#define DMA_4BYTE_MINOR_TRANSFER    4



typedef struct {
  uint32_t SADDR;
  uint16_t SOFF;
  uint16_t ATTR;
  union {
    uint32_t NBYTES_MLNO;
    uint32_t NBYTES_MLOFFNO;
    uint32_t NBYTES_MLOFFYES;
  };
  uint32_t SLAST;
  uint32_t DADDR;
  uint16_t DOFF;
  union {
    uint16_t CITER_ELINKNO;
    uint16_t CITER_ELINKYES;
  };
  uint32_t DLAST_SGA;
  uint16_t CSR;
  union {
    uint16_t BITER_ELINKNO;
    uint16_t BITER_ELINKYES;
  };
} T_DMA_TCD;

typedef struct
{
  uint8_t             ch;         // номер канала DMA пересылающий данные из FIFO SPI в буффер с данными
  uint8_t             *databuf;   // указатель на исходный буфер с данными
  uint32_t            datasz;     // размер исходного буфера данных
  uint32_t            spi_popr;   // адрес регистра POPR SPI
  uint32_t            dmux_src;   // номер входа периферии для выбранного мультиплексора DMAMUX для передачи на DMA. Например DMUX0_SRC_SPI0_RX
  uint8_t             minor_tranf_sz; // Количество пересылаемых байт в минорном цикле
  uint8_t             int_en;         // Разрешение прерываний по окончании главного цикла пересылки 

} T_DMA_SPI_RX_config;

typedef struct
{
  uint8_t             ch;         // номер канала DMA пересылающий данные из буффер с данными в FIFO SPI
  uint8_t             *databuf;   // указатель на исходный буфер с данными
  uint32_t            datasz;     // размер исходного буфера данных
  uint32_t            spi_pushr;  // адрес регистра PUSHR SPI
  uint32_t            dmux_src;   // номер входа периферии для выбранного мультиплексора DMAMUX для передачи на DMA. Например DMUX0_SRC_SPI0_TX
  uint8_t             minor_tranf_sz; // Количество пересылаемых байт в минорном цикле
  uint8_t             int_en;         // Разрешение прерываний по окончании главного цикла пересылки 
} T_DMA_SPI_TX_config;



typedef struct
{
  uint8_t       tx_ch;            // Номер дескриптора DMA на который указывает mem_ch
  uint8_t       rx_ch;            // Номер дескриптора DMA предлназначенного для приема
  uint8_t       minor_tranf_sz;   // Количество пересылаемых байт в минорном цикле

} T_DMA_SPI_cbl;


void Init_DMA(void);


void Config_DMA_for_SPI_TX(T_DMA_SPI_TX_config *cfg, T_DMA_SPI_cbl *pDS_cbl);
void Config_DMA_for_SPI_RX(T_DMA_SPI_RX_config *cfg, T_DMA_SPI_cbl *pDS_cbl);
void Set_DMA_MUX_chanel(DMAMUX_Type  *DMA_MUX, uint32_t mux_ch, uint32_t dma_ch);
void Start_DMA_for_SPI_TX(T_DMA_SPI_cbl *pDS_cbl, const uint32_t *buf, uint32_t sz);
void Start_DMA_for_SPI_RX(T_DMA_SPI_cbl *pDS_cbl, const uint32_t *buf, uint32_t sz);
void Start_DMA_for_SPI_RXTX(T_DMA_SPI_cbl *pDS_cbl, const uint32_t *outbuf, uint32_t *inbuf, uint32_t sz);



#endif // ANSIN01_DMA_H




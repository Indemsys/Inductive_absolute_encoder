#ifndef USB_COMPOSITE_DEV_H
#define USB_COMPOSITE_DEV_H


#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
  #define CONTROLLER_ID kUSB_ControllerEhci0
  #define DATA_BUFF_SIZE HS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0)
  #define CONTROLLER_ID kUSB_ControllerKhci0
  #define DATA_BUFF_SIZE FS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#endif

/* Currently configured line coding */
#define LINE_CODING_SIZE            (0x07)
#define LINE_CODING_DTERATE         (115200)
#define LINE_CODING_CHARFORMAT      (0x00)
#define LINE_CODING_PARITYTYPE      (0x00)
#define LINE_CODING_DATABITS        (0x08)

/* Communications feature */
#define COMM_FEATURE_DATA_SIZE      (0x02)
#define STATUS_ABSTRACT_STATE       (0x0000)
#define COUNTRY_SETTING             (0x0000)

/* Notification of serial state */
#define NOTIF_PACKET_SIZE           (0x08)
#define UART_BITMAP_SIZE            (0x02)
#define NOTIF_REQUEST_TYPE          (0xA1)


#define VCOM_FLAG_RECEIVED    BIT( 0 )  // Флаг OS обозначающий завершившийся прием пакета
#define VCOM_FLAG_TRANSMITED  BIT( 1 )  // Флаг OS обозначающий завершившуюся передачу пакета


#define  IN_BUF_QUANTITY   2  // Количество приемных буферов



// Управляющая стуктура принятого пакета
typedef struct
{
  uint8_t   *buff; // Буфер с пакетов
  uint32_t  len;   // Длина пакета
  uint32_t  pos;   // Текущая позиция считывания байта

}
T_rx_pack_cbl;

/* Define the infomation relates to abstract control model */
typedef struct _usb_cdc_acm_info
{
  uint8_t       serialStateBuf[NOTIF_PACKET_SIZE + UART_BITMAP_SIZE]; /* Serial state buffer of the CDC device to notify the serial state to host. */
  bool          dtePresent;          /* A flag to indicate whether DTE is present.         */
  uint16_t      breakDuration;       /* Length of time in milliseconds of the break signal */
  uint8_t       dteStatus;           /* Status of data terminal equipment                  */
  uint8_t       currentInterface;    /* Current interface index.                           */
  uint16_t      uartState;           /* UART state of the CDC device.                      */
}
T_usb_cdc_acm_info;


/* Define the types for application */
typedef struct _usb_cdc_vcom_struct
{
  class_handle_t        class_handle;                                                      /* USB CDC ACM class handle.                                                         */
  volatile uint8_t      start_transactions;                                                /* A flag to indicate whether a CDC device is ready to transmit and receive data.    */
  T_usb_cdc_acm_info    g_usb_cdc_acm_info; /* CDC ACM information */

  uint8_t               rx_buf[IN_BUF_QUANTITY][DATA_BUFF_SIZE]; //  Указатель на область с буферами приема
  T_rx_pack_cbl         rx_pack_cbl[IN_BUF_QUANTITY];            //  Массив управляющих структур приема-обработки входящих пакетов

  volatile uint32_t     rx_head;      //  Индекс головы циклической очереди буферов приема
  volatile uint32_t     rx_tail;      //  Индекс головы циклической очереди буферов приема
  volatile uint32_t     rx_full;      //  Флаг заполненнной очереди приемных буфферов
  uint32_t              rx_errors;    //  Счетчик ошибок процедур приема
  uint32_t              tx_errors;    //  Счетчик ошибок процедур передачи

  OS_FLAG_GRP           os_flags;
}
T_usb_cdc_vcom_struct;


/* Define the types for application */
typedef struct 
{
  volatile uint8_t      attach;                                                            /* A flag to indicate whether a usb device is attached. 1: attached, 0: not attached */
  uint8_t               speed;                                                             /* Speed of USB device. USB_SPEED_FULL/USB_SPEED_LOW/USB_SPEED_HIGH.                 */
  uint8_t               current_configuration;                                             /* Current configuration value. */
  uint8_t               current_interface_alternate_setting[CDC_VCOM1_INTERFACE_COUNT]; /* Current alternate setting value for each interface. */
}
T_composite_dev;


void USB_init_device(void);


#endif // USB_COMPOSITE_DEV_H




#ifndef MONITOR_USB_DRV_H
  #define MONITOR_USB_DRV_H


T_monitor_driver *Mnudrv_get_USB_vcom_driver1(void);
T_monitor_driver *Mnudrv_get_USB_vcom_driver2(void);



uint32_t Virtual_com1_send_data(uint8_t *buff, uint32_t size, uint32_t ticks);
uint32_t Virtual_com1_get_data(uint8_t *b, uint32_t ticks);
uint32_t Virtual_com1_wait_sending_end(uint32_t ticks);

uint32_t Virtual_com2_send_data(uint8_t *buff, uint32_t size, uint32_t ticks);
uint32_t Virtual_com2_send_data_sync(uint8_t *buff, uint32_t size, uint32_t ticks);
uint32_t Virtual_com2_get_data(uint8_t *b, uint32_t ticks);
uint32_t Virtual_com2_wait_sending_end(uint32_t ticks);


#endif // MONITOR_USB_DRV_H




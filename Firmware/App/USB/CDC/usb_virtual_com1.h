#ifndef USB_VIRTUAL_COM1_H
#define USB_VIRTUAL_COM1_H

usb_status_t            USB_DeviceCdcVcomCallback(class_handle_t class_handle, uint32_t event, void *param);
T_usb_cdc_vcom_struct*  VCOM1_get_cbl(void);

#endif /* _USB_CDC_VCOM_H_ */

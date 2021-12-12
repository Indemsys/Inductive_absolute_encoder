#ifndef USB_VIRTUAL_COM2_H
#define USB_VIRTUAL_COM2_H


usb_status_t            USB_DeviceCdcVcom2Callback(class_handle_t class_handle, uint32_t event, void *param);
T_usb_cdc_vcom_struct * VCOM2_get_cbl(void);

#endif /* _USB_CDC_VCOM_H_ */

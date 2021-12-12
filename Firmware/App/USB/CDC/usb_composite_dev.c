// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-25
// 15:37:58
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "App.h"

#include "fsl_device_registers.h"
#include "clock_config.h"

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "usb_composite_dev.h"
#include "usb_virtual_com1.h"
#include "usb_virtual_com2.h"

usb_device_handle     g_usb_dev_handle;    /* USB device handle. */

extern usb_device_class_struct_t     g_UsbDeviceCdcVcom1Config;
extern usb_device_class_struct_t     g_UsbDeviceCdcVcom2Config;

extern usb_device_endpoint_struct_t  g_cdcVcom1DicEndpoints[];
extern usb_device_endpoint_struct_t  g_cdcVcom2DicEndpoints[];

extern T_usb_cdc_vcom_struct         vcom1_cbl;
extern T_usb_cdc_vcom_struct         vcom2_cbl;

T_composite_dev                      g_composite;

static usb_status_t USB_DeviceCallback(usb_device_handle device_handle, uint32_t event, void *param);

/* USB device class information */
usb_device_class_config_struct_t g_cdc_acm_config[2] =
{
  {
    USB_DeviceCdcVcomCallback, 0,&g_UsbDeviceCdcVcom1Config,
  },
  {
    USB_DeviceCdcVcom2Callback, 0,&g_UsbDeviceCdcVcom2Config,
  }
};

/* USB device class configuraion information */
usb_device_class_config_list_struct_t g_cdc_acm_config_list =
{
  g_cdc_acm_config, USB_DeviceCallback, 2,
};


#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void USBHS_IRQHandler(void)
{
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();                                               /* Tell uC/OS-III that we are starting an ISR           */
  CPU_CRITICAL_EXIT();

  USB_DeviceEhciIsrFunction(g_usb_dev_handle);

  OSIntExit();
}
#endif


#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void USB0_IRQHandler(void)
{
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  OSIntEnter();                                               /* Tell uC/OS-III that we are starting an ISR           */
  CPU_CRITICAL_EXIT();

  //ITM_EVENT8(1, 3);
  USB_DeviceKhciIsrFunction(g_usb_dev_handle);

  OSIntExit();
}
#endif


#if USB_DEVICE_CONFIG_USE_TASK
/*-----------------------------------------------------------------------------------------------------
 USB task function. 
 This function runs the task for USB device. 
 
 \param handle 
-----------------------------------------------------------------------------------------------------*/
void Task_USB_Device(void *handle)
{
  while (1U)
  {
    #if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
    USB_DeviceEhciTaskFunction(handle);
    #endif
    #if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0)
    USB_DeviceKhciTaskFunction(handle);
    #endif
  }
}
#endif

/*-----------------------------------------------------------------------------------------------------
  USB device callback function. 
  This function handles the usb device specific requests.

 \param handle   The USB device handle.                        
 \param event    The USB device event type.                    
 \param param    The parameter of the device specific request. 
 
 \return usb_status_t A USB error code or kStatus_USB_Success.
-----------------------------------------------------------------------------------------------------*/
static usb_status_t USB_DeviceCallback(usb_device_handle device_handle, uint32_t event, void *param)
{
  usb_status_t error = kStatus_USB_Error;
  uint16_t *temp16 = (uint16_t *)param;
  uint8_t *temp8 = (uint8_t *)param;

  RTT_printf("__________USB_DeviceCallback event=%d\r\n", event);
  switch (event)
  {
  case kUSB_DeviceEventBusReset:
    RTT_printf("kUSB_DeviceEventBusReset\r\n");
    {
      g_composite.attach = 0;
      error = kStatus_USB_Success;
      #if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
      if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_composite.speed))
      {
        USB_DeviceSetSpeed(device_handle, g_composite.speed);
      }
      #endif
    }
    break;
  case kUSB_DeviceEventSetConfiguration:
    RTT_printf("kUSB_DeviceEventSetConfiguration %d\r\n", *temp16);
    if (param)
    {
      g_composite.attach = 1;
      g_composite.current_configuration = *temp8;
      if (USB_COMPOSITE_CONFIGURE_INDEX == (*temp8))
      {
        /* Schedule buffer for receive */
        USB_DeviceCdcAcmRecv(vcom1_cbl.class_handle, CDC_VCOM1_DIC_BULK_OUT_ENDPOINT, vcom1_cbl.rx_pack_cbl[vcom1_cbl.rx_head].buff, g_cdcVcom1DicEndpoints[0].maxPacketSize);
        USB_DeviceCdcAcmRecv(vcom2_cbl.class_handle, CDC_VCOM2_DIC_BULK_OUT_ENDPOINT, vcom2_cbl.rx_pack_cbl[vcom2_cbl.rx_head].buff, g_cdcVcom2DicEndpoints[0].maxPacketSize);
        error = kStatus_USB_Success;
      }
    }
    break;
  case kUSB_DeviceEventSetInterface:
    RTT_printf("kUSB_DeviceEventSetInterface %d\r\n", *temp16);
    if (g_composite.attach)
    {
      uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
      uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);
      if (interface < USB_INTERFACE_COUNT)
      {
        g_composite.current_interface_alternate_setting[interface] = alternateSetting;
      }
    }
    break;
  case kUSB_DeviceEventGetConfiguration:
    RTT_printf("kUSB_DeviceEventGetConfiguration %d\r\n", *temp16);
    if (param)
    {
      *temp8 = g_composite.current_configuration;
      error = kStatus_USB_Success;
    }
    break;
  case kUSB_DeviceEventGetInterface:
    RTT_printf("kUSB_DeviceEventGetInterface %d\r\n", *temp16);
    if (param)
    {
      uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
      if (interface < USB_INTERFACE_COUNT)
      {
        *temp16 = (*temp16 & 0xFF00U) | g_composite.current_interface_alternate_setting[interface];
        error = kStatus_USB_Success;
      }
      else
      {
        error = kStatus_USB_InvalidRequest;
      }
    }
    break;
  case kUSB_DeviceEventGetDeviceDescriptor:
    RTT_printf("kUSB_DeviceEventGetDeviceDescriptor %d\r\n", *temp16);
    if (param)
    {
      error = USB_DeviceGetDeviceDescriptor(device_handle, (usb_device_get_device_descriptor_struct_t *)param);
    }
    break;
  case kUSB_DeviceEventGetConfigurationDescriptor:
    RTT_printf("kUSB_DeviceEventGetConfigurationDescriptor %d\r\n", *temp16);
    if (param)
    {
      error = USB_DeviceGetConfigurationDescriptor(device_handle, (usb_device_get_configuration_descriptor_struct_t *)param);
    }
    break;
  case kUSB_DeviceEventGetStringDescriptor:
    RTT_printf("kUSB_DeviceEventGetStringDescriptor %d\r\n", *temp16);
    if (param)
    {
      error = USB_DeviceGetStringDescriptor(device_handle, (usb_device_get_string_descriptor_struct_t *)param);
    }
    break;
  default:
    break;
  }

  return error;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param p_vcom_cbl 
 \param class_handle 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
static uint32_t Init_vcom_cbl(T_usb_cdc_vcom_struct *p_vcom_cbl, class_handle_t class_handle)
{
  uint32_t i;
  OS_ERR     err;

  p_vcom_cbl->class_handle  = class_handle;
  p_vcom_cbl->rx_head       = 0;
  p_vcom_cbl->rx_tail       = p_vcom_cbl->rx_head;
  p_vcom_cbl->rx_full       = 0;
  for (i = 0; i < IN_BUF_QUANTITY; i++)
  {
    // Подготавдиваем массив управляющих структур для приема пакетов
    p_vcom_cbl->rx_pack_cbl[i].buff = p_vcom_cbl->rx_buf[i];
    p_vcom_cbl->rx_pack_cbl[i].len = 0;
    p_vcom_cbl->rx_pack_cbl[i].pos = 0;
  }

  OSFlagCreate(&p_vcom_cbl->os_flags, "VCOM_flags", (OS_FLAGS)0, &err);
  if (err != OS_ERR_NONE)
  {
    return RESULT_ERROR;
  }
  return RESULT_OK;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
-----------------------------------------------------------------------------------------------------*/
void USB_init_device(void)
{

  if (kStatus_USB_Success != USB_DeviceClassInit(CONTROLLER_ID, &g_cdc_acm_config_list, &g_usb_dev_handle))
  {
    RTT_printf("USB device init failed\r\n");
    return;
  }

  g_composite.attach  = 0;
  g_composite.speed   = USB_SPEED_FULL;

  Init_vcom_cbl(VCOM1_get_cbl(), g_cdc_acm_config_list.config[0].classHandle);
  Init_vcom_cbl(VCOM2_get_cbl(), g_cdc_acm_config_list.config[1].classHandle);

  NVIC_SetPriority((IRQn_Type)USB0_IRQn, USB_DEVICE_PRIO);
  NVIC_EnableIRQ((IRQn_Type)USB0_IRQn);

  USB_DeviceRun(g_usb_dev_handle);


  #if USB_DEVICE_CONFIG_USE_TASK
  if (g_usb_dev_handle)
  {
    OS_ERR   error;
    OSTaskCreate(&task_usbdev_tcb,     /* Task control block handle */
                 "USB device",         /* Task Name */
                 Task_USB_Device,      /* pointer to the task */
                 g_usb_dev_handle,     /* Task input parameter */
                 TASK_USBDEV_PRIO,     /* Task priority */
                 task_usbdev_stack,    /* Task stack base address */
                 0U,                   /* Task stack limit */
                 TASK_USBDEV_STACK_SZ, /* Task stack size */
                 0U,                   /* Mumber of messages can be sent to the task */
                 0U,                   /* Default time_quanta */
                 0U,                   /* Task extension */
                 OS_OPT_TASK_STK_CHK,  /* Task save FP */
                 &error);
    if (error != OS_ERR_NONE)
    {
      RTT_printf("VUSB device create failed!\r\n");
    }
  }
  #endif
}






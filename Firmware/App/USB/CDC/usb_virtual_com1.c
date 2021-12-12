/*
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of Freescale Semiconductor, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "App.h"

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"

#include "usb_device_descriptor.h"
#include "usb_composite_dev.h"
#include "usb_virtual_com1.h"


#if (defined(FSL_FEATURE_SOC_MPU_COUNT) && (FSL_FEATURE_SOC_MPU_COUNT > 0U))
  #include "fsl_mpu.h"
#endif /* FSL_FEATURE_SOC_MPU_COUNT */
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
  #include "usb_phy.h"
#endif
#if defined(FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED) && (FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED > 0U) && defined(USB_DEVICE_CONFIG_KEEP_ALIVE_MODE) && (USB_DEVICE_CONFIG_KEEP_ALIVE_MODE > 0U) && defined(FSL_FEATURE_USB_KHCI_USB_RAM) && (FSL_FEATURE_USB_KHCI_USB_RAM > 0U)
  #include "fsl_smc.h"
#endif



extern T_composite_dev               g_composite;

extern usb_device_endpoint_struct_t  g_cdcVcom1DicEndpoints[];

T_usb_cdc_vcom_struct vcom1_cbl;


/* Line codinig of cdc device */
static uint8_t g_line_coding[LINE_CODING_SIZE] =
{
  /* E.g. 0x00,0xC2,0x01,0x00 : 0x0001C200 is 115200 bits per second */
  (LINE_CODING_DTERATE >> 0U) & 0x000000FFU,
  (LINE_CODING_DTERATE >> 8U) & 0x000000FFU,
  (LINE_CODING_DTERATE >> 16U) & 0x000000FFU,
  (LINE_CODING_DTERATE >> 24U) & 0x000000FFU,
  LINE_CODING_CHARFORMAT,
  LINE_CODING_PARITYTYPE,
  LINE_CODING_DATABITS
};

/* Abstract state of cdc device */
static uint8_t g_abstract_state[COMM_FEATURE_DATA_SIZE] =
{
  (STATUS_ABSTRACT_STATE >> 0U) & 0x00FFU,
  (STATUS_ABSTRACT_STATE >> 8U) & 0x00FFU
};

/* Country code of cdc device */
static uint8_t g_country_code[COMM_FEATURE_DATA_SIZE] =
{
  (COUNTRY_SETTING >> 0U) & 0x00FFU,
  (COUNTRY_SETTING >> 8U) & 0x00FFU
};






/*-----------------------------------------------------------------------------------------------------
  CDC class specific callback function.
  This function handles the CDC class specific requests.

 \param handle The CDC ACM class handle.                     
 \param event  The CDC ACM class event type.                 
 \param param  The parameter of the class specific request.  
 
 \return usb_status_t  A USB error code or kStatus_USB_Success.
-----------------------------------------------------------------------------------------------------*/
usb_status_t USB_DeviceCdcVcomCallback(class_handle_t class_handle, uint32_t event, void *param)
{
  usb_status_t                                       error = kStatus_USB_Error;
  uint32_t                                           len;
  uint16_t                                           *uartBitmap;
  T_usb_cdc_acm_info                                 *cdc_info = &vcom1_cbl.g_usb_cdc_acm_info;
  usb_device_cdc_acm_request_param_struct_t          *cdc_request;
  usb_device_endpoint_callback_message_struct_t      *ep_message;
  OS_ERR                                             err;

  cdc_request = (usb_device_cdc_acm_request_param_struct_t *)param;
  ep_message = (usb_device_endpoint_callback_message_struct_t *)param;
  switch (event)
  {
  case kUSB_DeviceCdcEventSendResponse:
    {
      if ((ep_message->length != 0) && (!(ep_message->length % g_cdcVcom1DicEndpoints[0].maxPacketSize)))
      {
        /* If the last packet is the size of endpoint, then send also zero-ended packet,
         ** meaning that we want to inform the host that we do not have any additional
         ** data, so it can flush the output.
         */
        USB_DeviceCdcAcmSend(class_handle, CDC_VCOM1_DIC_BULK_IN_ENDPOINT, NULL, 0);
      }
      else if ((1 == g_composite.attach) && (1 == vcom1_cbl.start_transactions))
      {
        if ((ep_message->buffer != NULL) || ((ep_message->buffer == NULL) && (ep_message->length == 0)))
        {
          // Здесь сообщаем о том что передача закончена
          //
          //ITM_EVENT8(1, 4);
          OSFlagPost(&vcom1_cbl.os_flags, VCOM_FLAG_TRANSMITED, OS_OPT_POST_FLAG_SET, &err);

          // Подготоавливаем буфер конечной точки для приема
          // Считая что задача здесь всегда готова к приему
          //USB_DeviceCdcAcmRecv(class_handle, USB_CDC_VCOM_BULK_OUT_ENDPOINT, usb_rx_buff, g_cdcVcomDicEndpoints[0].maxPacketSize);

        }
      }
    }
    break;
  case kUSB_DeviceCdcEventRecvResponse:
    {
      if ((g_composite.attach == 1) && (vcom1_cbl.start_transactions == 1))
      {
        uint32_t    headi =  vcom1_cbl.rx_head;;

        // Пакет принят
        vcom1_cbl.rx_pack_cbl[headi].len = ep_message->length;
        vcom1_cbl.rx_pack_cbl[headi].pos = 0;

        //  Перевести указатель на следующий свободный для приема буфер
        headi++;
        if (headi >= IN_BUF_QUANTITY) headi = 0;

        if (headi != vcom1_cbl.rx_tail)
        {
          // Если есть еще свободный буффер в очереди то назначить прием в него
          vcom1_cbl.rx_head = headi;
          if  (USB_DeviceCdcAcmRecv(class_handle, CDC_VCOM1_DIC_BULK_OUT_ENDPOINT, vcom1_cbl.rx_pack_cbl[vcom1_cbl.rx_head].buff, g_cdcVcom1DicEndpoints[0].maxPacketSize) != kStatus_USB_Success) vcom1_cbl.rx_errors++;
        }
        else
        {
          // Буферов свободных для пиема не обнаружено.
          // Сигнализируем флагом о том что не смогли зарегистрировать режим прием для конечной точки
          vcom1_cbl.rx_full = 1;
        }
        OSFlagPost(&vcom1_cbl.os_flags, VCOM_FLAG_RECEIVED, OS_OPT_POST_FLAG_SET, &err);
      }
    }
    break;
  case kUSB_DeviceCdcEventSerialStateNotif:
    ((usb_device_cdc_acm_struct_t *)class_handle)->hasSentState = 0;
    break;
  case kUSB_DeviceCdcEventSendEncapsulatedCommand:
    break;
  case kUSB_DeviceCdcEventGetEncapsulatedResponse:
    break;
  case kUSB_DeviceCdcEventSetCommFeature:
    if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == cdc_request->setupValue)
    {
      if (1 == cdc_request->isSetup)
      {
        *(cdc_request->buffer) = g_abstract_state;
      }
      else
      {
        *(cdc_request->length) = 0;
      }
    }
    else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == cdc_request->setupValue)
    {
      if (1 == cdc_request->isSetup)
      {
        *(cdc_request->buffer) = g_country_code;
      }
      else
      {
        *(cdc_request->length) = 0;
      }
    }
    error = kStatus_USB_Success;
    break;
  case kUSB_DeviceCdcEventGetCommFeature:
    if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == cdc_request->setupValue)
    {
      *(cdc_request->buffer) = g_abstract_state;
      *(cdc_request->length) = COMM_FEATURE_DATA_SIZE;
    }
    else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == cdc_request->setupValue)
    {
      *(cdc_request->buffer) = g_country_code;
      *(cdc_request->length) = COMM_FEATURE_DATA_SIZE;
    }
    error = kStatus_USB_Success;
    break;
  case kUSB_DeviceCdcEventClearCommFeature:
    break;
  case kUSB_DeviceCdcEventGetLineCoding:
    *(cdc_request->buffer) = g_line_coding;
    *(cdc_request->length) = LINE_CODING_SIZE;
    error = kStatus_USB_Success;
    break;
  case kUSB_DeviceCdcEventSetLineCoding:
    {
      if (1 == cdc_request->isSetup)
      {
        *(cdc_request->buffer) = g_line_coding;
      }
      else
      {
        *(cdc_request->length) = 0;
      }
    }
    error = kStatus_USB_Success;
    break;
  case kUSB_DeviceCdcEventSetControlLineState:
    {
      vcom1_cbl.g_usb_cdc_acm_info.dteStatus = cdc_request->setupValue;
      /* activate/deactivate Tx carrier */
      if (cdc_info->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION)
      {
        cdc_info->uartState |= USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
      }
      else
      {
        cdc_info->uartState &= (uint16_t)~USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
      }

      /* activate carrier and DTE */
      if (cdc_info->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE)
      {
        cdc_info->uartState |= USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
      }
      else
      {
        cdc_info->uartState &= (uint16_t)~USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
      }

      /* Indicates to DCE if DTE is present or not */
      cdc_info->dtePresent = (cdc_info->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE) ? true : false;

      /* Initialize the serial state buffer */
      cdc_info->serialStateBuf[0] = NOTIF_REQUEST_TYPE;                /* bmRequestType */
      cdc_info->serialStateBuf[1] = USB_DEVICE_CDC_NOTIF_SERIAL_STATE; /* bNotification */
      cdc_info->serialStateBuf[2] = 0x00;                              /* wValue */
      cdc_info->serialStateBuf[3] = 0x00;
      cdc_info->serialStateBuf[4] = 0x00; /* wIndex */
      cdc_info->serialStateBuf[5] = 0x00;
      cdc_info->serialStateBuf[6] = UART_BITMAP_SIZE; /* wLength */
      cdc_info->serialStateBuf[7] = 0x00;
      /* Notifiy to host the line state */
      cdc_info->serialStateBuf[4] = cdc_request->interfaceIndex;
      /* Lower byte of UART BITMAP */
      uartBitmap = (uint16_t *)&cdc_info->serialStateBuf[NOTIF_PACKET_SIZE + UART_BITMAP_SIZE - 2];
      *uartBitmap = cdc_info->uartState;
      len = (uint32_t)(NOTIF_PACKET_SIZE + UART_BITMAP_SIZE);
      if (0 == ((usb_device_cdc_acm_struct_t *)class_handle)->hasSentState)
      {
        error = USB_DeviceCdcAcmSend(class_handle, CDC_VCOM1_CIC_INTERRUPT_IN_ENDPOINT, cdc_info->serialStateBuf, len);
        if (kStatus_USB_Success != error)
        {
          RTT_printf("kUSB_DeviceCdcEventSetControlLineState error!\r\n");
        }
        ((usb_device_cdc_acm_struct_t *)class_handle)->hasSentState = 1;
      }

      /* Update status */
      if (cdc_info->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION)
      {
        /*  To do: CARRIER_ACTIVATED */
      }
      else
      {
        /* To do: CARRIER_DEACTIVATED */
      }
      if (cdc_info->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE)
      {
        /* DTE_ACTIVATED */
        if (1 == g_composite.attach)
        {
          OS_ERR err;
          vcom1_cbl.start_transactions = 1;
          OSFlagPost(&vcom1_cbl.os_flags, VCOM_FLAG_TRANSMITED, OS_OPT_POST_FLAG_SET, &err);  // Объявляем готовность к передаче
        }
      }
      else
      {
        /* DTE_DEACTIVATED */
        if (1 == g_composite.attach)
        {
          vcom1_cbl.start_transactions = 0;
          OSFlagPost(&vcom1_cbl.os_flags, VCOM_FLAG_TRANSMITED, OS_OPT_POST_FLAG_CLR, &err);  // Объявляем готовность к передаче
        }
      }
    }
    break;
  case kUSB_DeviceCdcEventSendBreak:
    break;
  default:
    break;
  }

  return error;
}





/*-----------------------------------------------------------------------------------------------------
 Ожидаем окончания процедуры отправки данных
 Функци необходима в случае использования единственного буфера данных вызывающей задачей
 
 \param ticks 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t Virtual_com1_wait_sending_end(uint32_t ticks)
{
  OS_ERR   err;
  OSFlagPend(&vcom1_cbl.os_flags, (OS_FLAGS)VCOM_FLAG_TRANSMITED, ticks, OS_OPT_PEND_FLAG_SET_ANY, (CPU_TS *)0U, &err);
  if (err != OS_ERR_NONE)
  {
    return RESULT_ERROR;
  }
  return RESULT_OK;
}
/*------------------------------------------------------------------------------



 \param buff
 \param size
 \param ticks

 \return uint32_t RESULT_OK
 ------------------------------------------------------------------------------*/
uint32_t Virtual_com1_send_data(uint8_t *buff, uint32_t size, uint32_t ticks)
{
  OS_ERR   err;
  uint32_t sz;

  do
  {
    if (ticks != 0)
    {
      OSFlagPend(&vcom1_cbl.os_flags, (OS_FLAGS)VCOM_FLAG_TRANSMITED, ticks, OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME, (CPU_TS *)0U, &err);
      if (err != OS_ERR_NONE)
      {
        vcom1_cbl.tx_errors++;
        return RESULT_ERROR;
      }
    }
    sz = size;
    if (sz > DATA_BUFF_SIZE) sz = DATA_BUFF_SIZE;
    if (kStatus_USB_Success != USB_DeviceCdcAcmSend(vcom1_cbl.class_handle, CDC_VCOM1_DIC_BULK_IN_ENDPOINT, buff, sz))
    {
      vcom1_cbl.tx_errors++;
      return RESULT_ERROR;
    }
    buff += sz;
    size = size - sz;
  }
  while (size > 0);
  return RESULT_OK;

}

/*-----------------------------------------------------------------------------------------------------
  Получить данные из виртуального порта в буфер buff, количество байт данных - size
  В случае невозможности выполнения происходит возврат из функиции без задержки
 
 \param b 
 \param ticks 
 
 \return uint32_t Возвращает RESULT_OK в случае выполнения функции и RESULT_ERROR в случае невыполнения функции
-----------------------------------------------------------------------------------------------------*/
uint32_t Virtual_com1_get_data(uint8_t *b, uint32_t ticks)
{
  int32_t  res;
  OS_ERR   err;
  uint32_t taili = vcom1_cbl.rx_tail;

  // Если индексы буферов равны, то это значит отсутствие принятых пакетов
  if (taili == vcom1_cbl.rx_head)
  {
    if (ticks != 0)
    {
      OSFlagPend(&vcom1_cbl.os_flags, (OS_FLAGS)VCOM_FLAG_RECEIVED, ticks, OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME, (CPU_TS *)0U, &err);
      if (err != OS_ERR_NONE)
      {
        return RESULT_ERROR;
      }
    }
    else return RESULT_ERROR;
  }

  *b = vcom1_cbl.rx_pack_cbl[taili].buff[vcom1_cbl.rx_pack_cbl[taili].pos];
  vcom1_cbl.rx_pack_cbl[taili].pos++;

  // Если позиция достигла конца данных в текущем буфере, то буфер освобождается для приема
  if (vcom1_cbl.rx_pack_cbl[taili].pos >= vcom1_cbl.rx_pack_cbl[taili].len)
  {
    // Смещаем указатель хвоста очереди приемных буфферов
    // Появляется место для движения головы очереди приемных буфферов

    if ((taili + 1) >= IN_BUF_QUANTITY) taili = 0;
    else taili++;
    vcom1_cbl.rx_tail = taili;

    if (vcom1_cbl.rx_full == 1) // Если очередь была заполнена, то посылаем запрос приема , так как в обработчике собыйтий запрос приема был пропущен
    {
      vcom1_cbl.rx_full = 0;

      if ((vcom1_cbl.rx_head + 1) >= IN_BUF_QUANTITY) vcom1_cbl.rx_head = 0;
      else vcom1_cbl.rx_head++;

      res =  USB_DeviceCdcAcmRecv(vcom1_cbl.class_handle,  CDC_VCOM1_DIC_BULK_OUT_ENDPOINT, vcom1_cbl.rx_pack_cbl[vcom1_cbl.rx_head].buff, g_cdcVcom1DicEndpoints[0].maxPacketSize);
      if (res != kStatus_USB_Success) vcom1_cbl.rx_errors++;
    }
  }

  return RESULT_OK;

}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return T_usb_cdc_vcom_struct* 
-----------------------------------------------------------------------------------------------------*/
T_usb_cdc_vcom_struct * VCOM1_get_cbl(void)
{
  return &vcom1_cbl;
}




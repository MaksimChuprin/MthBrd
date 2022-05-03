#include "USB_Task.h"
#include "RTOS.h"
#include "stm32f4xx.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_customhid.h"
#include "usbd_customhid_if.h"
#include "device_struct.h"

#define __DISABLE_USB_GLOBAL      __HAL_PCD_DISABLE(&hpcd)
#define __ENABLE_USB_GLOBAL       __HAL_PCD_ENABLE( &hpcd)

/* macros privat */
static OS_EVENT                   usbEvent;
USBD_HandleTypeDef                USBD_Device;

// externs
extern PCD_HandleTypeDef          hpcd;
extern volatile system_flags_t    system_flags;

/* USB_Stop */
void USB_Stop(void)
{
  USBD_Stop(&USBD_Device);
}

/* USB_Connect */
bool USB_IsConfigured(void)
{
  return (USBD_Device.dev_state == USBD_STATE_CONFIGURED);
}

/* ISR */
void USB_ISR_Handler(void) 
{  
  OS_EnterInterrupt();
    
  OS_EVENT_Set( &usbEvent );
  hpcd.Lock = HAL_LOCKED;
  __DISABLE_USB_GLOBAL;
  
  OS_LeaveInterrupt();
}

/* Task */
void USB_Task(void) 
{
  OS_EVENT_Create ( &usbEvent );
  
  /* Init Device Library */
  USBD_Init( &USBD_Device, &HID_Desc, 0);
  
  /* Add Supported Class */
  USBD_RegisterClass( &USBD_Device, &USBD_CUSTOM_HID);
  
  /* Add Custom HID callbacks */
  USBD_CUSTOM_HID_RegisterInterface( &USBD_Device, &USBD_CustomHID_fops);
  
  /* Start Device Process */
  USBD_Start(&USBD_Device);
  __ENABLE_USB_GLOBAL;
  
  // main USB loop  
  for(U8 i = 0; i < 8;) 
  {   
    OS_EVENT_WaitTimed( &usbEvent, 50);
    if( hpcd.Lock == HAL_LOCKED )
    {
      hpcd.Lock = HAL_UNLOCKED;
      HAL_PCD_IRQHandler( &hpcd);
      __ENABLE_USB_GLOBAL;
    }
    else if( system_flags.start_boot ) i++;
  }
  
  USB_Stop();
    OS_Delay(100);
      NVIC_SystemReset();
}

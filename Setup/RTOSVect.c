#include "defines.h"
#define _NVIC_HFSR       *((volatile unsigned long *)(0xE000ED2C))

/********************************************************************
*
*       Types
*
*********************************************************************
*/
typedef void( *intfunc )( void );
typedef union { intfunc __fun; void * __ptr; } intvec_elem;

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

#ifdef __ICCARM__
  #pragma language=extended
  #if (__VER__ < 500)
    #pragma segment="CSTACK"
    extern void __program_start(void);
  #else
    #pragma section="CSTACK"
    extern void __iar_program_start(void);
  #endif  // #if (__VER__ < 500)
#endif    // #ifdef __ICCARM__

#ifdef __CC_ARM
  extern unsigned int Image$$CSTACK$$ZI$$Limit;
  extern void __main(void);
#endif

static void _DefaultDummyHandler(void) {
  //
  // In case we received a hard fault because 
  // of a breakpoint instruction we return.
  // This may happen with the IAR compiler when using semihosting.
  //
  if (_NVIC_HFSR & 1UL << 31) {
    _NVIC_HFSR |= (1UL << 31);
    return;
  }
  while(1);
}

static void NMIException               (void){_DefaultDummyHandler();};
static void HardFaultException         (void){_DefaultDummyHandler();};
static void MemManageException         (void){_DefaultDummyHandler();};
static void BusFaultException          (void){_DefaultDummyHandler();};
static void UsageFaultException        (void){_DefaultDummyHandler();};
static void DebugMonitor               (void){_DefaultDummyHandler();};

static void SVCHandler                 (void){_DefaultDummyHandler();};
static void WWDG_IRQHandler            (void){_DefaultDummyHandler();};
static void PVD_IRQHandler             (void){_DefaultDummyHandler();};
static void TAMPER_IRQHandler          (void){_DefaultDummyHandler();};
static void RTC_IRQHandler             (void){_DefaultDummyHandler();};
static void FLASH_IRQHandler           (void){_DefaultDummyHandler();};
static void RCC_IRQHandler             (void){_DefaultDummyHandler();};
static void EXTI0_IRQHandler           (void){_DefaultDummyHandler();};
static void EXTI1_IRQHandler           (void){_DefaultDummyHandler();};
static void EXTI2_IRQHandler           (void){_DefaultDummyHandler();};
static void EXTI3_IRQHandler           (void){_DefaultDummyHandler();};
static void EXTI4_IRQHandler           (void){_DefaultDummyHandler();};
static void DMA1Channel1_IRQHandler    (void){_DefaultDummyHandler();};
static void DMA1Channel2_IRQHandler    (void){_DefaultDummyHandler();};
static void DMA1Channel3_IRQHandler    (void){_DefaultDummyHandler();};
static void DMA1Channel4_IRQHandler    (void){_DefaultDummyHandler();};
static void DMA1Channel5_IRQHandler    (void){_DefaultDummyHandler();};
static void DMA1Channel6_IRQHandler    (void){_DefaultDummyHandler();};
static void DMA1Channel7_IRQHandler    (void){_DefaultDummyHandler();};
static void ADC_IRQHandler             (void){_DefaultDummyHandler();};
static void CAN1_TX_IRQHandler         (void){_DefaultDummyHandler();};
static void CAN1_RX0_IRQHandler        (void){_DefaultDummyHandler();};
static void CAN1_RX1_IRQHandler        (void){_DefaultDummyHandler();};
static void CAN1_SCE_IRQHandler        (void){_DefaultDummyHandler();};
static void EXTI9_5_IRQHandler         (void){_DefaultDummyHandler();};
static void TIM1_BRK_IRQHandler        (void){_DefaultDummyHandler();};
static void TIM1_UP_IRQHandler         (void){_DefaultDummyHandler();};
static void TIM1_TRG_COM_IRQHandler    (void){_DefaultDummyHandler();};
static void TIM1_CC_IRQHandler         (void){_DefaultDummyHandler();};
static void TIM2_IRQHandler            (void){_DefaultDummyHandler();};
static void TIM3_IRQHandler            (void){ TIM3_ISR();};
static void TIM4_IRQHandler            (void){_DefaultDummyHandler();};
static void I2C1_EV_IRQHandler         (void){_DefaultDummyHandler();};
static void I2C1_ER_IRQHandler         (void){_DefaultDummyHandler();};
static void I2C2_EV_IRQHandler         (void){_DefaultDummyHandler();};
static void I2C2_ER_IRQHandler         (void){_DefaultDummyHandler();};
static void SPI1_IRQHandler            (void){_DefaultDummyHandler();};
static void SPI2_IRQHandler            (void){_DefaultDummyHandler();};
static void USART1_IRQHandler          (void){ UART1_ISR();};
static void USART2_IRQHandler          (void){ UART2_ISR();};
static void USART3_IRQHandler          (void){_DefaultDummyHandler();};
static void EXTI15_10_IRQHandler       (void){_DefaultDummyHandler();};
static void RTCAlarm_IRQHandler        (void){_DefaultDummyHandler();};
static void USB_OTG_FS_WKUP_IRQHandler (void){_DefaultDummyHandler();};

static void TIM8_BRK_TIM12_IRQHandler  (void){_DefaultDummyHandler();};
static void TIM8_UP_TIM13_IRQHandler   (void){_DefaultDummyHandler();};
static void TIM8_TRG_COM_TIM14_IRQHandler (void){_DefaultDummyHandler();};
static void TIM8_CC_IRQHandler         (void){_DefaultDummyHandler();};
static void DMA1_Stream7_IRQHandler    (void){_DefaultDummyHandler();};
static void FSMC_IRQHandler            (void){_DefaultDummyHandler();};
static void SDIO_IRQHandler            (void){_DefaultDummyHandler();};

static void TIM5_IRQHandler            (void){_DefaultDummyHandler();};
static void SPI3_IRQHandler            (void){_DefaultDummyHandler();};
static void USART4_IRQHandler          (void){_DefaultDummyHandler();};
static void USART5_IRQHandler          (void){_DefaultDummyHandler();};
static void TIM6_IRQHandler            (void){_DefaultDummyHandler();};
static void TIM7_IRQHandler            (void){_DefaultDummyHandler();};
static void DMA2Channel1_IRQHandler    (void){_DefaultDummyHandler();};
static void DMA2Channel2_IRQHandler    (void){_DefaultDummyHandler();};
static void DMA2Channel3_IRQHandler    (void){_DefaultDummyHandler();};
static void DMA2Channel4_IRQHandler    (void){_DefaultDummyHandler();};
static void DMA2Channel5_IRQHandler    (void){_DefaultDummyHandler();};
static void ETH_IRQHandler_loc         (void){ ETH_IRQHandler();};
static void ETH_WKUP_IRQHandler        (void){_DefaultDummyHandler();};
static void CAN2_TX_IRQHandler         (void){_DefaultDummyHandler();};
static void CAN2_RX0_IRQHandler        (void){_DefaultDummyHandler();};
static void CAN2_RX1_IRQHandler        (void){_DefaultDummyHandler();};
static void CAN2_SCE_IRQHandler        (void){_DefaultDummyHandler();};
static void USB_OTG_FS_IRQHandler      (void){ USB_ISR_Handler(); };

static void DMA2_Stream5_IRQHandler    (void){_DefaultDummyHandler();};
static void DMA2_Stream6_IRQHandler    (void){_DefaultDummyHandler();};
static void DMA2_Stream7_IRQHandler    (void){_DefaultDummyHandler();};
static void USART6_IRQHandler          (void){_DefaultDummyHandler();};
static void I2C3_EV_IRQHandler         (void){_DefaultDummyHandler();};
static void I2C3_ER_IRQHandler         (void){_DefaultDummyHandler();};
static void OTG_HS_EP1_OUT_IRQHandler  (void){_DefaultDummyHandler();};
static void OTG_HS_EP1_IN_IRQHandler   (void){_DefaultDummyHandler();};
static void OTG_HS_WKUP_IRQHandler     (void){_DefaultDummyHandler();};
static void OTG_HS_IRQHandler          (void){_DefaultDummyHandler();};
static void DCMI_IRQHandler            (void){_DefaultDummyHandler();};
static void HASH_RNG_IRQHandler        (void){_DefaultDummyHandler();};
static void FPU_IRQHandler             (void){_DefaultDummyHandler();};


#ifdef __ICCARM__
  #if (__VER__ < 500)
    #pragma location = "INTVEC"
  #else
    #pragma location = ".intvec"
  #endif  // #if (__VER__ < 500)
#endif    // #ifdef __ICCARM__
#ifdef __CC_ARM
  #pragma arm section rodata = "INTVEC"
#endif
/* STM32F10x Vector Table entries */
const intvec_elem __vector_table[] =
{
#ifdef __ICCARM__
  { .__ptr = __sfe( "CSTACK" ) },
#if (__VER__ < 500)
  __program_start,
#else
  __iar_program_start,
#endif  // #if (__VER__ >= 500)
#endif  // #ifdef __ICCARM__
#ifdef __CC_ARM
  (intfunc) &Image$$CSTACK$$ZI$$Limit,
  __main,
#endif
  NMIException,
  HardFaultException,
  MemManageException,
  BusFaultException,
  UsageFaultException,
  0,            // Reserved
  0,            // Reserved
  0,            // Reserved
  0,            // Reserved
  SVCHandler,
  DebugMonitor,
  0,            // Reserved
  OS_Exception,
  OS_Systick,
  WWDG_IRQHandler,
  PVD_IRQHandler,
  TAMPER_IRQHandler,
  RTC_IRQHandler,
  FLASH_IRQHandler,
  RCC_IRQHandler,
  EXTI0_IRQHandler,
  EXTI1_IRQHandler,
  EXTI2_IRQHandler,
  EXTI3_IRQHandler,
  EXTI4_IRQHandler,
  DMA1Channel1_IRQHandler,
  DMA1Channel2_IRQHandler,
  DMA1Channel3_IRQHandler,
  DMA1Channel4_IRQHandler,
  DMA1Channel5_IRQHandler,
  DMA1Channel6_IRQHandler,
  DMA1Channel7_IRQHandler,
  ADC_IRQHandler,
  CAN1_TX_IRQHandler,
  CAN1_RX0_IRQHandler,
  CAN1_RX1_IRQHandler,
  CAN1_SCE_IRQHandler,
  EXTI9_5_IRQHandler,
  TIM1_BRK_IRQHandler,
  TIM1_UP_IRQHandler,
  TIM1_TRG_COM_IRQHandler,
  TIM1_CC_IRQHandler,
  TIM2_IRQHandler,
  TIM3_IRQHandler,
  TIM4_IRQHandler,
  I2C1_EV_IRQHandler,
  I2C1_ER_IRQHandler,
  I2C2_EV_IRQHandler,
  I2C2_ER_IRQHandler,
  SPI1_IRQHandler,
  SPI2_IRQHandler,
  USART1_IRQHandler,
  USART2_IRQHandler,
  USART3_IRQHandler,
  EXTI15_10_IRQHandler,
  RTCAlarm_IRQHandler,
  USB_OTG_FS_WKUP_IRQHandler,
  TIM8_BRK_TIM12_IRQHandler, 
  TIM8_UP_TIM13_IRQHandler,            
  TIM8_TRG_COM_TIM14_IRQHandler,           
  TIM8_CC_IRQHandler,            
  DMA1_Stream7_IRQHandler,           
  FSMC_IRQHandler,            
  SDIO_IRQHandler,            
  TIM5_IRQHandler,
  SPI3_IRQHandler,
  USART4_IRQHandler,
  USART5_IRQHandler,
  TIM6_IRQHandler,
  TIM7_IRQHandler,
  DMA2Channel1_IRQHandler,
  DMA2Channel2_IRQHandler,
  DMA2Channel3_IRQHandler,
  DMA2Channel4_IRQHandler,
  DMA2Channel5_IRQHandler,
  ETH_IRQHandler_loc,         // ETH_IRQHandler,
  ETH_WKUP_IRQHandler,
  CAN2_TX_IRQHandler,
  CAN2_RX0_IRQHandler,
  CAN2_RX1_IRQHandler,
  CAN2_SCE_IRQHandler,
  USB_OTG_FS_IRQHandler,             /* USB_OTG_FS_IRQHandler        */
  DMA2_Stream5_IRQHandler,           /* DMA2 Stream 5                */                   
  DMA2_Stream6_IRQHandler,           /* DMA2 Stream 6                */                   
  DMA2_Stream7_IRQHandler,           /* DMA2 Stream 7                */                   
  USART6_IRQHandler,                 /* USART6                       */                    
  I2C3_EV_IRQHandler,                /* I2C3 event                   */                          
  I2C3_ER_IRQHandler,                /* I2C3 error                   */                          
  OTG_HS_EP1_OUT_IRQHandler,         /* USB OTG HS End Point 1 Out   */                   
  OTG_HS_EP1_IN_IRQHandler,          /* USB OTG HS End Point 1 In    */                   
  OTG_HS_WKUP_IRQHandler,            /* USB OTG HS Wakeup through EXTI */                         
  OTG_HS_IRQHandler,                 /* USB OTG HS                   */                   
  DCMI_IRQHandler,                   /* DCMI                         */                   
  0,                                 /* CRYP crypto                  */                   
  HASH_RNG_IRQHandler,               /* Hash and Rng                 */
  FPU_IRQHandler                     /* FPU                          */  
};

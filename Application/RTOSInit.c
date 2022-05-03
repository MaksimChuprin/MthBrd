#include "defines.h"

/*********************************************************************
*
*       Clock frequency settings
*/
#ifndef   OS_FSYS                   /* CPU main clock frequency      */
  #define OS_FSYS 84000000          /* 84000000uL */
#endif

#ifndef   OS_PCLK_TIMER             /* Peripheral clock for timer   */
  #define OS_PCLK_TIMER OS_FSYS     /* May vary from CPU clock      */
#endif                              /* depending on CPU             */

#ifndef   OS_TICK_FREQ
  #define OS_TICK_FREQ (1000)
#endif

#ifndef   OS_USE_VARINTTABLE        /* The interrupt vector table   */
  #define OS_USE_VARINTTABLE (0)    /* may be located in RAM        */
#endif
/****** End of configuration settings *******************************/

/*********************************************************************
*
*       Local defines (sfrs used in RTOSInit.c)
*
**********************************************************************
*/

#define SYS_INT_CTRL_STATE        (*(volatile OS_U32*)(0xE000ED04))
#define SYS_PENDSTSET             26

#define SYSPRI1_ADDR              (0xE000ED18)
#define SYSHND_CTRL_ADDR          (0xE000ED24)  // System Handler Control and State
#define SYSHND_CTRL               (*(volatile OS_U32*) (SYSHND_CTRL_ADDR))

#define NVIC_SYS_HND_CTRL_MEM     (0x00010000)  // Mem manage fault enable
#define NVIC_SYS_HND_CTRL_BUS     (0x00020000)  // Bus fault enable
#define NVIC_SYS_HND_CTRL_USAGE   (0x00040000)  // Usage fault enable

#define NVIC_PRIOBASE_ADDR        (0xE000E400)
#define NVIC_ENABLE_ADDR          (0xE000E100)
#define NVIC_DISABLE_ADDR         (0xE000E180)
#define NVIC_VTOREG_ADDR          (0xE000ED08)

#define NUM_INTERRUPTS            (16+81)

/*****  Interrupt ID numbers ****************************************/
#define ISR_ID_MPU                    (4)                // MPU fault
#define ISR_ID_BUS                    (5)                // Bus fault
#define ISR_ID_USAGE                  (6)                // Usage fault
#define ISR_ID_SYSTICK                (15)               // System Tick

#define OS_ISR_ID_TICK                ISR_ID_SYSTICK     // We use Sys-Timer
#define OS_ISR_TICK_PRIO              (0xFF)             // Lowest priority

/****** OS timer configuration **************************************/
#define OS_TIMER_RELOAD           (OS_PCLK_TIMER / OS_TICK_FREQ - 1)
#if (OS_TIMER_RELOAD >= 0x100000000)
  #error "Systick can not be used, please check configuration"
#endif


/*********************************************************************
*
*       OS_Systick
*
* Function description
*   This is the code that gets called when the processor receives a
*   _SysTick exception. SysTick is used as OS timer tick.
*
* NOTES:
*   (1) It has to be inserted in the interrupt vector table, if RAM
*       vectors are not used. Therefore is is declared public
*/
void OS_Systick(void) {
  OS_EnterNestableInterrupt();
  OS_HandleTick();
  OS_LeaveNestableInterrupt();
}

/*********************************************************************
*
*       OS_InitHW()
*
*       Initialize the hardware (timer) required for the OS to run.
*       May be modified, if an other timer should be used
*/
uint32_t SystemCoreClock = OS_FSYS;

void OS_InitHW(void) {
  
  SCB->VTOR    = 0x8010000;                                                     // новая таблица прерываний
  HAL_DeInit();                                                                 // Reset of all peripherals
  RCC->CR      = RCC_CR_HSION + RCC_CR_HSEON;                                   // Вкл HSI и HSE
  RCC->CFGR    = RCC_CFGR_SW_HSI;                                               // HSI как SYSCLK
  RCC->CSR     = RCC_CSR_LSION;                                                 // Вкл LSI
  
  while (!(RCC->CR & RCC_CR_HSERDY) );                                          // Wait HSE is ready
  HAL_RCC_EnableCSS();
  
  // настройка работы с флеш: буферизациия, кэш, циклы ожидания для 84 МГц @ 3.5V
  __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
  __HAL_FLASH_DATA_CACHE_ENABLE();
  __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_2);
  
  // настройка частоты шин и запуск PLL
  RCC->CFGR |= RCC_CFGR_HPRE_DIV1 + RCC_CFGR_PPRE1_DIV2 + RCC_CFGR_PPRE2_DIV1;  // AHB = SYSCLK = HCLK = 84 MHz, APB1 = HCLK/2 = 42 MHz, APB2 = HCLK = 84 MHz
  __HAL_RCC_PLL_CONFIG( RCC_PLLCFGR_PLLSRC_HSE, 25, 336, 4, 7);                 // inClk = HSE/25; PLLCLK = inClk * 336 / 4 = 84 MHz
  __HAL_RCC_PLL_ENABLE();    
  while (!(RCC->CR & RCC_CR_PLLRDY) );                                               
  __HAL_RCC_SYSCLK_CONFIG(RCC_CFGR_SW_PLL);                                     // select PLL as SYSCLK
  
   //Enable AHB clocks
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();  
   __HAL_RCC_GPIOD_CLK_ENABLE();
   __HAL_RCC_GPIOE_CLK_ENABLE();
   __HAL_RCC_BKPSRAM_CLK_ENABLE();
   __HAL_RCC_CCMDATARAMEN_CLK_ENABLE();
           
  // настройка портов
  GPIO_InitTypeDef  GPIO_InitStructure = {0};   
  
  // порт A
  GPIOA->ODR                   = 0;  
  
  GPIO_InitStructure.Pin       = GPIO_PIN_All;                                  // массовая
  GPIO_InitStructure.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.Pin       = GPIO_PIN_0;                                    // звук PA0
  GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;  
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;  
  GPIO_InitStructure.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.Pin       = GPIO_PIN_4;                                    //  PA4  - analog mode
  GPIO_InitStructure.Mode      = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);   
  
  GPIO_InitStructure.Pin       = GPIO_PIN_5;                                    //  PA5 - SPI1, SCK
  GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_MEDIUM;    
  GPIO_InitStructure.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
     
  GPIO_InitStructure.Pin       = GPIO_PIN_13 | GPIO_PIN_14;                     //  PA13, PA14 - SW-DP
  GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;    
  GPIO_InitStructure.Alternate = GPIO_AF0_SWJ;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
    
  // порт B
  GPIOB->ODR                   = (1<<0) + (1<<1);
  
  GPIO_InitStructure.Pin       = GPIO_PIN_All;                                  // массовая
  GPIO_InitStructure.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;  
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_InitStructure.Pin       = GPIO_PIN_1 | GPIO_PIN_8;                       //  PB1, PB8 - analog
  GPIO_InitStructure.Mode      = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure); 
  
  GPIO_InitStructure.Pin       = GPIO_PIN_2 | GPIO_PIN_10;                      //  PB2, PB10 - input
  GPIO_InitStructure.Mode      = GPIO_MODE_INPUT;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);     
  
  GPIO_InitStructure.Pin       = GPIO_PIN_5;                                    //  PB5 - SPI1 MOSI
  GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_MEDIUM;    
  GPIO_InitStructure.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);   
  
  GPIO_InitStructure.Pin       = GPIO_PIN_6 | GPIO_PIN_7;                       //  PB6, PB7 - UART1
  GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;    
  GPIO_InitStructure.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);  
    
  GPIO_InitStructure.Pin       = GPIO_PIN_14 | GPIO_PIN_15;                     // PB14, PB15 - output
  GPIO_InitStructure.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;  
  GPIO_InitStructure.Alternate = 0;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
  // порт C
  GPIOC->ODR                   = (1<<6) + (1<<7);  
  
  GPIO_InitStructure.Pin       = GPIO_PIN_All;                                  // массовая
  GPIO_InitStructure.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;  
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);  
  
  GPIO_InitStructure.Pin       = GPIO_PIN_2 | GPIO_PIN_3;                       //  PC2, PC3 - analog
  GPIO_InitStructure.Mode      = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);  
  
  GPIO_InitStructure.Pin       = GPIO_PIN_8 | GPIO_PIN_9;                       //  PC8, PC9 - input
  GPIO_InitStructure.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);   
  
  GPIO_InitStructure.Pin       = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;       //  PC10, PC11, PC12 - SPI3
  GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull      = GPIO_PULLUP;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_MEDIUM;    
  GPIO_InitStructure.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);  
  
  // порт D
  GPIOD->ODR                   = (1<<0) + (1<<1) + (1<<7) + (1<<13) + (1<<14);
  
  GPIO_InitStructure.Pin       = GPIO_PIN_All;                                  // массовая
  GPIO_InitStructure.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;  
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure); 
  
  GPIO_InitStructure.Pin       = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_15;         // PD3, PD4, PD15 - output
  GPIO_InitStructure.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;  
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW; 
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  GPIO_InitStructure.Pin       = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;         // PD8...PD10 - input
  GPIO_InitStructure.Pull      = GPIO_NOPULL;
  GPIO_InitStructure.Mode      = GPIO_MODE_INPUT;  
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  GPIO_InitStructure.Pin       = GPIO_PIN_5 | GPIO_PIN_6;                       // PD5, PD6 - UART2
  GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;  
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;   
  GPIO_InitStructure.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);  
    
  // порт E
  GPIOE->ODR= 0x0;  
  GPIO_InitStructure.Pin       = GPIO_PIN_All;                                  // массовая
  GPIO_InitStructure.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull      = GPIO_PULLDOWN;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStructure.Alternate = 0;   
  HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
    
  // вкючение переферийных модулей на APB1 = 42 MHz
  //  PWR
  __HAL_RCC_PWR_CLK_ENABLE(); 
  // PWR->CSR = PWR_CSR_BRE;  
  
  // SPI3 - память конфигурации и SD карта 42/4 = 10.5 MHz
  __HAL_RCC_SPI3_CLK_ENABLE();
  SPI3->CR1=           0;
  SPI3->CR2=           0;   
  SPI3->CR1= SPI_CR1_BR_0 + SPI_CR1_SSM + SPI_CR1_SSI + SPI_CR1_SPE + SPI_CR1_MSTR;  
    
  // TIM2 - звук
  __HAL_RCC_TIM2_CLK_ENABLE();
  TIM2->CCMR1 =   BIT6 + BIT5 + BIT3;                                            // PWM mode 1  канал 1
  TIM2->ARR   =   511;
  TIM2->CR1   =   BIT7;
  TIM2->PSC   =   41999;                                                         // предделитель на ~1 мс
  TIM2->CCER  =   (1<<1) + (1<<0);                                               // вкл 1 инверсный  
    
  // DAC ключен в небуферизированном режиме - 12bit
  __HAL_RCC_DAC_CLK_ENABLE();
  DAC->CR      = (1L<<1) + (1L<<0);
  DAC->DHR12R1 = DAC->DHR12R2= 0;  
       
  // вкючение переферийных модулей на APB2 = 84 MHz  
  // SPI1  - реле, ток, коммутатор, старая клава 84/8 = 10.5 MHz
  __HAL_RCC_SPI1_CLK_ENABLE();
  SPI1->CR1=           0;
  SPI1->CR2=           0;
  SPI1->CR1= SPI_CR1_BR_0 + SPI_CR1_BR_1 + SPI_CR1_SSM + SPI_CR1_SSI + SPI_CR1_SPE + SPI_CR1_MSTR;    
  // Relay off, off all lines 
  shift_byte_spi1(0);
  shift_byte_spi1(0);
  LOAD_RLY;
  LOAD_CMT;
      
  // ADC12 1&2
  __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_ADC2_CLK_ENABLE();
  ADC123_COMMON->CCR = ADC_CLOCK_SYNC_PCLK_DIV8;                                // ADCCLK = 84/8 - 10,5 MHz;
  ADC1->CR2   = ADC2->CR2 = (1<<15) + (7<<12);                                  // ADC1&2 enable SWSTART;
  ADC1->SMPR2 = (3L<<0) + (3L<<3) + (3L<<6) + (3L<<9);                          // ADC1 sample time = 2 us
  ADC2->SMPR2 = (3L<<0) + (3L<<3) + (3L<<6) + (3L<<9);                          // ADC2 sample time = 2 us
  ADC1->JOFR1 = ADC1->JOFR2 = ADC1->JOFR3 = ADC1->JOFR4 = ADC2->JOFR1 = ADC2->JOFR2 = 0;    // ADC1&2 offset = 0  
  ADC1->JSQR  = (3L<<20) + (12L<<0) + (13L<<5) + (14L<<10) + (15L<<15);         // ADC1 set input chns & 4 convs - аналоговые входы преобразователей
  ADC2->JSQR  = (3L<<20) + (8L<<0) + (9L<<5) + (8L<<10) + (9L<<15);             // ADC2 set input chns & 4 convs - тач панель
  ADC1->CR1   = ADC2->CR1 = (1<<8);                                             // ADC1&2 scan mode  
  
  // настройка сторожевого таймера
  IWDG->KR=                0x5555;   // ключ настройки
  IWDG->PR=                5;        // 12 с таймаут
  IWDG->KR=                0x5555;   // ключ настройки
  IWDG->RLR=               0xfff;    // 
  IWDG->KR=                0xaaaa;   // сбросить перед запуском  
  IWDG->KR=                0xcccc;   // start
        
  // Initialize OS timer, clock soure = core clock
  SysTick->LOAD         = OS_TIMER_RELOAD;  
  SysTick->CTRL         = (1 << SysTick_CTRL_ENABLE_Pos) | (1 << SysTick_CTRL_CLKSOURCE_Pos);
  
  // Install Systick Timer Handler and enable timer interrupt
  OS_ARM_InstallISRHandler (OS_ISR_ID_TICK, (OS_ISR_HANDLER*)OS_Systick);
  OS_ARM_ISRSetPrio        (OS_ISR_ID_TICK, OS_ISR_TICK_PRIO);
  OS_ARM_EnableISR         (OS_ISR_ID_TICK);  
}

/*********************************************************************
*
*       Idle loop  (OS_Idle)
*
*       Please note:
*       This is basically the "core" of the idle loop.
*       This core loop can be changed, but:
*       The idle loop does not have a stack of its own, therefore no
*       functionality should be implemented that relies on the stack
*       to be preserved. However, a simple program loop can be programmed
*       (like toggeling an output or incrementing a counter)
*/
void OS_Idle(void) {     // Idle loop: No task is ready to execute
  while (1) {
  }
}

/*********************************************************************
*
*       OS_GetTime_Cycles()
*
*       This routine is required for high
*       resolution time maesurement functions.
*       It returns the system time in timer clock cycles.
*/
OS_U32 OS_GetTime_Cycles(void) {
  unsigned int t_cnt;
  OS_U32 time;
  time  = OS_Time;
  t_cnt = (OS_PCLK_TIMER/1000) - SysTick->VAL;
  if (SYS_INT_CTRL_STATE & (1 << SYS_PENDSTSET)) {    /* Missed a counter interrupt? */
    t_cnt = (OS_PCLK_TIMER/1000) - SysTick->VAL;    /* Adjust result               */
    time++;
  }
  return (OS_PCLK_TIMER/1000) * time + t_cnt;
}

/*********************************************************************
*
*       OS_ConvertCycles2us
*
*       Convert Cycles into micro seconds.
*
*       If your clock frequency is not a multiple of 1 MHz,
*       you may have to modify this routine in order to get proper
*       diagonstics.
*
*       This routine is required for profiling or high resolution time
*       measurement only. It does not affect operation of the OS.
*/
OS_U32 OS_ConvertCycles2us(OS_U32 Cycles) {
  return Cycles/(OS_PCLK_TIMER/1000000);
}



/*********************************************************************
*
*       OS interrupt handler and ISR specific functions
*
**********************************************************************
*/

#if OS_USE_VARINTTABLE
//
// The interrupt vector table may be located anywhere in RAM
//
#ifdef __ICCARM__
#if (__VER__ < 500)
#pragma segment="VTABLE"
#else
#pragma section="VTABLE"
#endif  // #if (__VER__ < 500)
#pragma data_alignment=256
__no_init void (*g_pfnRAMVectors[_NUM_INTERRUPTS])(void) @ "VTABLE";
#endif  // __ICCARM__
#ifdef __CC_ARM
extern unsigned int VTABLE$$Base;
__attribute__((section("VTABLE"), zero_init, aligned(256))) void (*g_pfnRAMVectors[_NUM_INTERRUPTS])(void);
#endif
#define VTBASE_IN_RAM_BIT             (29)
#define _RAM_START_ADDR               (0x20000000)
#endif

/*********************************************************************
*
*       OS_ARM_InstallISRHandler
*/
OS_ISR_HANDLER* OS_ARM_InstallISRHandler (int ISRIndex, OS_ISR_HANDLER* pISRHandler) {
#if OS_USE_VARINTTABLE
  OS_ISR_HANDLER*  pOldHandler;
  OS_U32 ulIdx;

  pOldHandler = NULL;
  //
  // Check whether the RAM vector table has been initialized.
  //
  if ((*(OS_U32*)NVIC_VTOREG_ADDR) != (unsigned long)g_pfnRAMVectors) {
    //
    // Copy the vector table from the beginning of FLASH to the RAM vector table.
    //
    for(ulIdx = 0; ulIdx < NUM_INTERRUPTS; ulIdx++) {
      g_pfnRAMVectors[ulIdx] = (void (*)(void))(*((volatile unsigned long *)(ulIdx * 4)));
    }
    //
    // Program NVIC to point at the RAM vector table.
    //
#ifdef __ICCARM__
    *(OS_U32*)NVIC_VTOREG_ADDR = ((OS_U32)__sfb("VTABLE") - _RAM_START_ADDR) | (1 << VTBASE_IN_RAM_BIT);
#endif
#ifdef __CC_ARM
	*(OS_U32*)NVIC_VTOREG_ADDR = ((OS_U32)&(VTABLE$$Base) - _RAM_START_ADDR) | (1 << VTBASE_IN_RAM_BIT);
#endif
  }
  //
  // Save the interrupt handler.
  //
  pOldHandler = g_pfnRAMVectors[ISRIndex];
  g_pfnRAMVectors[ISRIndex] = pISRHandler;
  return (pOldHandler);
#else
  return (NULL);
#endif
}

/*********************************************************************
*
*       OS_ARM_EnableISR
*/
void OS_ARM_EnableISR(int ISRIndex) {
  OS_DI();
  if (ISRIndex < NUM_INTERRUPTS) {
    if (ISRIndex >= 16) {
      //
      // Enable standard "external" interrupts, starting at index 16
      //
      ISRIndex -= 16;
      *(((OS_U32*) NVIC_ENABLE_ADDR) + (ISRIndex >> 5)) = (1 << (ISRIndex & 0x1F));
    } else if (ISRIndex == ISR_ID_MPU) {
      //
      // Enable the MemManage interrupt.
      //
      SYSHND_CTRL |= NVIC_SYS_HND_CTRL_MEM;
    } else if (ISRIndex == ISR_ID_BUS) {
      //
      // Enable the bus fault interrupt.
      //
      SYSHND_CTRL |= NVIC_SYS_HND_CTRL_BUS;
    } else if (ISRIndex == ISR_ID_USAGE) {
      //
      // Enable the usage fault interrupt.
      //
      SYSHND_CTRL |= NVIC_SYS_HND_CTRL_USAGE;
    } else if (ISRIndex == ISR_ID_SYSTICK) {
      //
      // Enable the System Tick interrupt.
      //
      SysTick->CTRL |= (1 << SysTick_CTRL_TICKINT_Pos);
    }
  }
  OS_RestoreI();
}

/*********************************************************************
*
*       OS_ARM_DisableISR
*/
void OS_ARM_DisableISR(int ISRIndex) {
  OS_DI();
  if (ISRIndex < NUM_INTERRUPTS) {
    if (ISRIndex >= 16) {
      //
      // Disable standard "external" interrupts
      //
      ISRIndex -= 16;
      *(((OS_U32*) NVIC_DISABLE_ADDR) + (ISRIndex >> 5)) = (1 << (ISRIndex & 0x1F));
    } else if (ISRIndex == ISR_ID_MPU) {
      //
      // Disable the MemManage interrupt.
      //
      SYSHND_CTRL &= ~NVIC_SYS_HND_CTRL_MEM;
    } else if (ISRIndex == ISR_ID_BUS) {
      //
      // Disable the bus fault interrupt.
      //
      SYSHND_CTRL &= ~NVIC_SYS_HND_CTRL_BUS;
    } else if (ISRIndex == ISR_ID_USAGE) {
      //
      // Disable the usage fault interrupt.
      //
      SYSHND_CTRL &= ~NVIC_SYS_HND_CTRL_USAGE;
    } else if (ISRIndex == ISR_ID_SYSTICK) {
      //
      // Enable the System Tick interrupt.
      //
      SysTick->CTRL &= ~(1 << SysTick_CTRL_TICKINT_Pos);
    }
  }
  OS_RestoreI();
}

/*********************************************************************
*
*       OS_ARM_ISRSetPrio
*
*   Notes:
*     (1) Some priorities of system handler are reserved
*         0..3 : Priority can not be set
*         7..10: Reserved
*         13   : Reserved
*     (2) System handler use different control register. This affects
*         ISRIndex 0..15
*/
int OS_ARM_ISRSetPrio(int ISRIndex, int Prio) {
  OS_U8* pPrio;
  int    OldPrio;

  OldPrio = 0;
  if (ISRIndex < NUM_INTERRUPTS) {
    if (ISRIndex >= 16) {
      //
      // Handle standard "external" interrupts
      //
      ISRIndex -= 16;                   // Adjust index
      OS_DI();
      pPrio = (OS_U8*)(NVIC_PRIOBASE_ADDR + ISRIndex);
      OldPrio = *pPrio;
      *pPrio = Prio;
      OS_RestoreI();
    } else {
      //
      // Handle System Interrupt controller
      //
      if ((ISRIndex < 4) | ((ISRIndex >= 7) && (ISRIndex <= 10)) | (ISRIndex == 13)) {
        //
        // Reserved ISR channel, do nothing
        //
      } else {
        //
        // Set priority in system interrupt priority control register
        //
        OS_DI();
        pPrio = (OS_U8*)(SYSPRI1_ADDR);
        ISRIndex -= 4;                  // Adjust Index
        OldPrio = pPrio[ISRIndex];
        pPrio[ISRIndex] = Prio;
        OS_RestoreI();
      }
    }
  }
  return OldPrio;
}

/****** Final check of configuration ********************************/

/*****  EOF  ********************************************************/

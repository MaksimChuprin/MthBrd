#ifndef DEFINES_H
  #define DEFINES_H

#include <intrinsics.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal_conf.h"

#include "RTOS.h"
#include "comm.h"
#include "eksis.h"
#include "device_struct.h"

#define MAX_OUTS              16
#define ADC_NUM_AVR           64

// config word
#define CONFIG_MASK           0x00ffffff

#define UART_MASK             0x7
#define S1200                 0
#define S2400                 1
#define S4800                 2
#define S9600                 3
#define S19200                4
#define S38400                5
#define S57600                6
#define S115200               7

#define UNIT_MASK             0xff00

#define PUMP_CONTROL_DEVICE   BIT8
#define MEASURE_MODE_AUTO     BIT9

#define CONTROL_SERVICE       BIT15
#define STATISCTIC_WE         BIT16
#define KEY_BEEP              BIT17
#define ERROR_BEEP            BIT18
#define TRSH_BEEP             BIT19
#define HIDE_LOAD_SCREEN      BIT20
#define HIDE_LOAD_LOGO        BIT21
#define TCP_PRESENT           BIT22
#define INCH_7                BIT23

#define STATISCTIC_OVERFLOW   BIT24
#define FORCE_STATISTIC       BIT25
#define RESTART_STATISTIC     BIT26
#define CLEAR_BACKUP_REG      BIT27
#define LOAD_CONFIG_FLAG      BIT31

// ErrorArray's bits
#define NO_ERR                0
#define CONNECT_ERR           BIT0
#define LOWLIMIT_ERR          BIT1
#define UPLIMIT_ERR           BIT2

#define INTERNAL_ERR          BIT4
#define TYPE_ERR              BIT5
#define COMPLEX_ERR           BIT6
#define ASSIGNED_ERR          BIT7

// param config
#define PARAM_PLACEMENT_MASK   15     // BIT0, BIT1, BIT2, BIT3; сверху вниз слева направо
#define PARAM_ON_MAINSCREEN    BIT4  // 0 - нет, 1 - да
#define PARAM_PRESSURE1_SOURCE BIT5  // давление в точке измерения, 0 - константа, 1 - датчик
#define PARAM_PRESSURE2_SOURCE BIT6  // давление в точке пересчёта, 0 - константа, 1 - датчик
#define PARAM_FLOW_CUT_TYPE    BIT5  // тип сечения для расчёта расхода, 0 - круглое, 1 - прямоугольное

#define TFT_DRIVER_ILI9325    0
#define TFT_DRIVER_ST7789     1

/* звуки */
#define S_MAX                 10
#define REPEAT_SOUND          128
#define OFF_SOUND             0
#define S_SOUND_20            1
#define S_SOUND_100           2
#define S_SOUND_300           3
#define S_SOUND_500           4
#define S_SOUND_1000          5
#define R_SOUND_100_200       6 + REPEAT_SOUND
#define R_SOUND_300_600       7 + REPEAT_SOUND
#define R_SOUND_500_1000      8 + REPEAT_SOUND
#define R_SOUND_1000_2000     9 + REPEAT_SOUND

// SPI-MEM COMMANDS
#define   WRSR                1
#define   WRITE               2
#define   READ                3
#define   WRDI                4 
#define   RDSR                5
#define   WREN                6
#define   WIP_SR              (1<<0)
#define   SPIMEM_PAGE_SIZE    64

// mem addresses
#define ConfigBeginAddr         0x10000000
#define ConfigEndAddr           0x10007fff
#define RamBeginAddr            0x10008000
#define RamEndAddr              0x1000ffff
#define CONFIGSIZE              (RamBeginAddr - ConfigBeginAddr)
#define RAMSIZE                 (RamEndAddr - RamBeginAddr + 1)

#define CONFIG(A)               (ConfigBeginAddr + (A))
#define RAM(A)                  (RamBeginAddr + (A))

#define SD_STATISTIC_OFFSET     0L
#define SD_STATISTIC_SIZE       0x40000000
#define SD_SCREENSHOT_OFFSET    (U32)(SD_STATISTIC_OFFSET  + SD_STATISTIC_SIZE)
#define SCREENSHOT_SIZE         (1048576L)
#define SD_IMAGES_OFFSET        (U32)(SD_SCREENSHOT_OFFSET + SCREENSHOT_SIZE)
#define SD_UPDATECODE_ADR       0x4F000000
#define SD_UPDATECODE_SIZE      0x00FF0000
#define SD_CONFIG_ADR           0x4FFF0000
#define SD_DEFCONFIG_ADR        0x4FFF8000
#define SD_ALLCONFIG_SIZE       0x00010000
#define SD_ALLCONFIG_SECTORS    SD_ALLCONFIG_SIZE / 512

#define LOAD_CMT                { HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET); HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET); }
#define LOAD_RLY                { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); }
#define LOAD_RLY_D              { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); for(U32 i= 0; i < 500; i++); HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); }
#define LOAD_ANG                { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET); }
#define MEM_SEL                 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET)
#define MEM_DES                 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET)
#define IND_SEL                 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET)
#define IND_DES                 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET)
#define IN_KEYB                 HAL_GPIO_ReadPin        (GPIOB, GPIO_PIN_10)

#define PUMP_ON                 PumpStatus = 1
#define PUMP_OFF                PumpStatus = 0
#define CHECK_PUMP              PumpStatus

#define ADC1_CONVERTION         { SET_BIT(ADC1->CR2, ADC_CR2_ADON); SET_BIT( ADC1->CR2, ADC_CR2_JSWSTART); while( !(ADC1->SR & ADC_SR_JEOC) ); CLEAR_BIT(ADC1->CR2, ADC_CR2_ADON); }
#define ADC2_CONVERTION         { SET_BIT(ADC2->CR2, ADC_CR2_ADON); SET_BIT( ADC2->CR2, ADC_CR2_JSWSTART); while( !(ADC2->SR & ADC_SR_JEOC) ); CLEAR_BIT(ADC2->CR2, ADC_CR2_ADON); }

#define CLEAR_IWDG              IWDG->KR = 0xAAAA

#define GPIOx_SET(GPOIx, Bit)   (GPOIx)->BSRR = (1<<(Bit))
#define GPIOx_RESET(GPOIx, Bit) (GPOIx)->BSRR = (1<<((Bit) + 16))

#define OS_SUSPEND_IF_NOT(A)    if(!OS_GetSuspendCnt((A))) OS_Suspend((A))
#define OS_RESUME_IF_NOT(A)     if(OS_GetSuspendCnt((A)))  OS_Resume((A))

#define OS_STOPTIMER_IF_NOT(A)  if(OS_GetTimerStatus((A))) OS_StopTimer((A))
#define OS_STARTTIMER_IF_NOT(A) if(!OS_GetTimerStatus((A))) OS_StartTimer((A))

#define CONFIG_ROM_ADR(A)       (U16)((pU8)(A) - (pU8)ConfigBeginAddr)
#define SAVE_CONFIG(A)          seachSectorsToWrite   ( CONFIG_ROM_ADR(&(A)), sizeof((A)) )
#define LOAD_CONFIG(A)          errors_flags.sd_fail |= read_SDCard ( (pInt8U)&(A), CONFIG_ROM_ADR(&(A)) + SD_CONFIG_ADR, sizeof((A)) )

#include "global_var.h"
#include "global_declaration.h"
#include "SD.h"
#include "utilites.h"
#include "UARTs_task.h"
#include "USB_Task.h"
#include "master.h"
#include "modbus.h"
#include "OS_IP_task.h"
#include "OS_MQTT_task.h"

#ifdef    LED
#include "led.h"
#endif

#ifdef    TFT32
#include "TFT_32.h"
#endif

#ifdef    TFT51
#include "TFT_51.h"
#endif

#ifdef    TFT71
#include "TFT_71.h"
#endif

#endif
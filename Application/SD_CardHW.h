#ifndef __MMC_SD_CARDMODE_X_HW_H__
#define __MMC_SD_CARDMODE_X_HW_H__

#define STUFF_BITS 			        0x00
#define INTERFACE_COND				0x000001AA
#define VER2_OP_COND				0x40FF8000

//Status and Error Codes
#define IN_IDLE_STATE 				0x01
#define SUCCESS 				0x00
#define DATA_ACCEPTED 				0x05
#define CRC_ERROR 				0x0B
#define WRITE_ERROR 				0x0D
#define ERROR					0x01
#define CC_ERROR				0x02
#define CARD_ECC_FAILED				0x04
#define OUT_OF_RANGE				0x08
#define ILLEGAL_COMMAND_IDLE_STATE  		0x05

/*********************************************************************
*
*       Local defines (sfrs)
*
**********************************************************************
*/
#define  MMC_CLK                    72000L

#define _GPIO_PC0_BASE_ADDR         (0x40011000UL)
#define _GPIO_PD0_BASE_ADDR         (0x40011400UL)
#define _GPIO_PE0_BASE_ADDR         (0x40011800UL)
#define _GPIO_PF0_BASE_ADDR         (0x40011C00UL)
#define _RCC_BASE                   (0x40021000UL)
#define _SDIO_BASE                  (0x40018000UL)
#define _DMA2_BASE                  (0x40020400UL)

#define _RCC_APB2RSTR               *(volatile unsigned *)(_RCC_BASE + 0x0C)
#define _RCC_AHBENR                 *(volatile unsigned *)(_RCC_BASE + 0x14)
#define _RCC_APB2ENR                *(volatile unsigned *)(_RCC_BASE + 0x18)

#define _RCC_AHBENR_DMA2EN          (1UL <<  1)
#define _RCC_AHBENR_SDIOEN          (1UL << 10)

#define _SDIO_POWER                 *(volatile unsigned *)(_SDIO_BASE + 0x00)
#define _SDIO_CLKCR                 *(volatile unsigned *)(_SDIO_BASE + 0x04)
#define _SDIO_ARG                   *(volatile unsigned *)(_SDIO_BASE + 0x08)
#define _SDIO_CMD                   *(volatile unsigned *)(_SDIO_BASE + 0x0C)
#define _SDIO_RESPCMD               *(volatile unsigned *)(_SDIO_BASE + 0x10)
#define _SDIO_RESP1                 *(volatile unsigned *)(_SDIO_BASE + 0x14)
#define _SDIO_RESP2                 *(volatile unsigned *)(_SDIO_BASE + 0x18)
#define _SDIO_RESP3                 *(volatile unsigned *)(_SDIO_BASE + 0x1C)
#define _SDIO_RESP4                 *(volatile unsigned *)(_SDIO_BASE + 0x20)
#define _SDIO_DTIMER                *(volatile unsigned *)(_SDIO_BASE + 0x24)
#define _SDIO_DLEN                  *(volatile unsigned *)(_SDIO_BASE + 0x28)
#define _SDIO_DCTRL                 *(volatile unsigned *)(_SDIO_BASE + 0x2C)
#define _SDIO_DCOUNT                *(volatile unsigned *)(_SDIO_BASE + 0x30)
#define _SDIO_STA                   *(volatile unsigned *)(_SDIO_BASE + 0x34)
#define _SDIO_ICR                   *(volatile unsigned *)(_SDIO_BASE + 0x38)
#define _SDIO_MASK                  *(volatile unsigned *)(_SDIO_BASE + 0x3C)
#define _SDIO_FIFOCNT               *(volatile unsigned *)(_SDIO_BASE + 0x48)
#define _SDIO_FIFO                  *(volatile unsigned *)(_SDIO_BASE + 0x80)


#define _DMA_ISR                    *(volatile unsigned *)(_DMA2_BASE + 0x00)
#define _DMA_IFCR                   *(volatile unsigned *)(_DMA2_BASE + 0x04)
#define _DMA_CCR1                   *(volatile unsigned *)(_DMA2_BASE + 0x08)
#define _DMA_CNDTR1                 *(volatile unsigned *)(_DMA2_BASE + 0x0C)
#define _DMA_CPAR1                  *(volatile unsigned *)(_DMA2_BASE + 0x10)
#define _DMA_CMAR1                  *(volatile unsigned *)(_DMA2_BASE + 0x14)
#define _DMA_CCR2                   *(volatile unsigned *)(_DMA2_BASE + 0x1C)
#define _DMA_CNDTR2                 *(volatile unsigned *)(_DMA2_BASE + 0x20)
#define _DMA_CPAR2                  *(volatile unsigned *)(_DMA2_BASE + 0x24)
#define _DMA_CMAR2                  *(volatile unsigned *)(_DMA2_BASE + 0x28)
#define _DMA_CCR3                   *(volatile unsigned *)(_DMA2_BASE + 0x30)
#define _DMA_CNDTR3                 *(volatile unsigned *)(_DMA2_BASE + 0x34)
#define _DMA_CPAR3                  *(volatile unsigned *)(_DMA2_BASE + 0x38)
#define _DMA_CMAR3                  *(volatile unsigned *)(_DMA2_BASE + 0x3C)
#define _DMA_CCR4                   *(volatile unsigned *)(_DMA2_BASE + 0x44)
#define _DMA_CNDTR4                 *(volatile unsigned *)(_DMA2_BASE + 0x48)
#define _DMA_CPAR4                  *(volatile unsigned *)(_DMA2_BASE + 0x4C)
#define _DMA_CMAR4                  *(volatile unsigned *)(_DMA2_BASE + 0x50)
#define _DMA_CCR5                   *(volatile unsigned *)(_DMA2_BASE + 0x58)
#define _DMA_CNDTR5                 *(volatile unsigned *)(_DMA2_BASE + 0x5C)
#define _DMA_CPAR5                  *(volatile unsigned *)(_DMA2_BASE + 0x60)
#define _DMA_CMAR5                  *(volatile unsigned *)(_DMA2_BASE + 0x64)
#define _DMA_CCR6                   *(volatile unsigned *)(_DMA2_BASE + 0x6C)
#define _DMA_CNDTR6                 *(volatile unsigned *)(_DMA2_BASE + 0x70)
#define _DMA_CPAR6                  *(volatile unsigned *)(_DMA2_BASE + 0x74)
#define _DMA_CMAR6                  *(volatile unsigned *)(_DMA2_BASE + 0x78)
#define _DMA_CCR7                   *(volatile unsigned *)(_DMA2_BASE + 0x80)
#define _DMA_CNDTR7                 *(volatile unsigned *)(_DMA2_BASE + 0x84)
#define _DMA_CPAR7                  *(volatile unsigned *)(_DMA2_BASE + 0x88)
#define _DMA_CMAR7                  *(volatile unsigned *)(_DMA2_BASE + 0x8C)

#define _DMA_ISR_GIF3               (1UL  << 8)
#define _DMA_ISR_TCIF3              (1UL  << 9)
#define _DMA_ISR_HTIF3              (1UL  << 10)
#define _DMA_ISR_TEIF3              (1UL  << 11)

#define _DMA_IFCR_CGIF3             (1UL  << 8)
#define _DMA_IFCR_CTCIF3            (1UL  << 9)
#define _DMA_IFCR_CHTIF3            (1UL  << 10)
#define _DMA_IFCR_CTEIF3            (1UL  << 11)

#define _DMA_ISR_GIF4               (1UL  << 12)
#define _DMA_ISR_TCIF4              (1UL  << 13)
#define _DMA_ISR_HTIF4              (1UL  << 14)
#define _DMA_ISR_TEIF4              (1UL  << 15)

#define _DMA_IFCR_CGIF4             (1UL  << 12)
#define _DMA_IFCR_CTCIF4            (1UL  << 13)
#define _DMA_IFCR_CHTIF4            (1UL  << 14)
#define _DMA_IFCR_CTEIF4            (1UL  << 15)

#define _DMA_ISR_GIF5               (1UL  << 16)
#define _DMA_ISR_TCIF5              (1UL  << 17)
#define _DMA_ISR_HTIF5              (1UL  << 18)
#define _DMA_ISR_TEIF5              (1UL  << 19)

#define _DMA_IFCR_CGIF5             (1UL  << 16)
#define _DMA_IFCR_CTCIF5            (1UL  << 17)
#define _DMA_IFCR_CHTIF5            (1UL  << 18)
#define _DMA_IFCR_CTEIF5            (1UL  << 19)


#define _DMA_CCR_EN                 (1UL  <<   0)
#define _DMA_CCR_TCIE               (1UL  <<   1)
#define _DMA_CCR_HTIE               (1UL  <<   2)
#define _DMA_CCR_TEIE               (1UL  <<   3)
#define _DMA_CCR_DIR                (1UL  <<   4)
#define _DMA_CCR_CIRC               (1UL  <<   5)
#define _DMA_CCR_PINC               (1UL  <<   6)
#define _DMA_CCR_MINC               (1UL  <<   7)
#define _DMA_CCR_PSIZE_MASK         (3UL  <<   8)
#define _DMA_CCR_PSIZE_SHIFT        (8)
#define _DMA_CCR_MSIZE_MASK         (3UL  <<  10)
#define _DMA_CCR_MSIZE_SHIFT        (10)
#define _DMA_CCR_PL_MASK            (3UL  <<  12)
#define _DMA_CCR_PL_SHIFT           (12)
#define _DMA_CCR_MEM2MEM            (1UL  <<  14)

#define _SDIO_CP_BIT                (1UL << 11)
#define _SDIO_CP_IDR                *(volatile unsigned int*)(_GPIO_PC0_BASE_ADDR + 0x08)

#define _SYSCTRL_PC_PORT_BIT        (1UL << 4)
#define _SYSCTRL_PD_PORT_BIT        (1UL << 5)

#define _SDIO_PORT_CR               *(volatile unsigned int*)(_GPIO_PC0_BASE_ADDR + 0x04)
#define _SDIO_CMD_PIN_PORT_CR       *(volatile unsigned int*)(_GPIO_PD0_BASE_ADDR + 0x00)

#define PERIPHAL_TO_MEMORY          0
#define MEMORY_TO_PERIPHAL          1

#define _SDIO_CLKCR_WIDBUS_MASK           (0x3UL << 11)
#define _SDIO_CLKCR_WIDBUS_4B             (0x01L << 11)
#define _SDIO_CLKCR_CLKEN                 (1UL   <<  8)

#define _SDIO_CMD_CPSMEN                  (1UL << 10)
#define _SDIO_CMD_CMD_MASK                (0x3FUL)
#define _SDIO_CMD_RESPONSE_FORMAT_MASK    (3UL << 6)
#define _SDIO_CMD_RESPONSE_FORMAT_SHORT   (1UL << 6)
#define _SDIO_CMD_RESPONSE_FORMAT_LONG    (3UL << 6)

#define _SDIO_STA_CCRCFAIL                (1UL << 0)
#define _SDIO_STA_DCRCFAIL                (1UL << 1)
#define _SDIO_STA_CTIMEOUT                (1UL << 2)
#define _SDIO_STA_DTIMEOUT                (1UL << 3)
#define _SDIO_STA_TXUNDERR                (1UL << 4)
#define _SDIO_STA_RXOVERR                 (1UL << 5)
#define _SDIO_STA_CMDREND                 (1UL << 6)
#define _SDIO_STA_CMDSENT                 (1UL << 7)
#define _SDIO_STA_DATAEND                 (1UL << 8)
#define _SDIO_STA_STBITERR                (1UL << 9)
#define _SDIO_STA_DBCKEND                 (1UL <<10)
#define _SDIO_STA_CMDACT                  (1UL <<11)
#define _SDIO_STA_TXACT                   (1UL <<12)
#define _SDIO_STA_RXACT                   (1UL <<13)
#define _SDIO_STA_TXFIFOHE                (1UL <<14)
#define _SDIO_STA_RXFIFOHF                (1UL <<15)
#define _SDIO_STA_TXFIFOF                 (1UL <<16)
#define _SDIO_STA_RXFIFOF                 (1UL <<17)
#define _SDIO_STA_TXFIFOE                 (1UL <<18)
#define _SDIO_STA_RXFIFOE                 (1UL <<19)
#define _SDIO_STA_TXDAVL                  (1UL <<20)
#define _SDIO_STA_RXDAVL                  (1UL <<21)
#define _SDIO_STA_SDIOIT                  (1UL <<22)
#define _SDIO_STA_CEATAEND                (1UL <<23)

#define _SDIO_ICR_CCRCFAIL                (1UL << 0)
#define _SDIO_ICR_DCRCFAIL                (1UL << 1)
#define _SDIO_ICR_CTIMEOUT                (1UL << 2)
#define _SDIO_ICR_DTIMEOUT                (1UL << 3)
#define _SDIO_ICR_TXUNDERR                (1UL << 4)
#define _SDIO_ICR_RXOVERR                 (1UL << 5)
#define _SDIO_ICR_CMDREND                 (1UL << 6)
#define _SDIO_ICR_CMDSENT                 (1UL << 7)
#define _SDIO_ICR_DATAEND                 (1UL << 8)
#define _SDIO_ICR_STBITERR                (1UL << 9)
#define _SDIO_ICR_DBCKEND                 (1UL <<10)
#define _SDIO_ICR_SDIOIT                  (1UL <<22)
#define _SDIO_ICR_CEATAEND                (1UL <<23)

#define _SDIO_DCTRL_DTEN                  (1UL << 0)
#define _SDIO_DCTRL_DTDIR                 (1UL << 1)
#define _SDIO_DCTRL_DTMODE                (1UL << 2)
#define _SDIO_DCTRL_DMAEN                 (1UL << 3)
#define _SDIO_DCTRL_DBLOCKSIZE_SHIFT      (4)
#define _SDIO_DCTRL_DBLOCKSIZE_MASK       (0xFUL << _SDIO_DCTRL_DBLOCKSIZE_SHIFT)
#define _SDIO_DCTRL_RWSTART               (1UL << 8)
#define _SDIO_DCTRL_RWSTOP                (1UL << 9)
#define _SDIO_DCTRL_RWMOD                 (1UL <<10)
#define _SDIO_DCTRL_SDIOEN                (1UL <<11)

/*********************************************************************
*
*        Defines: Error codes
*
**********************************************************************
*/
#define FS_MMC_CARD_NO_ERROR               0
#define FS_MMC_CARD_RESPONSE_TIMEOUT       (1<<0)
#define FS_MMC_CARD_RESPONSE_CRC_ERROR     (1<<1)
#define FS_MMC_CARD_READ_TIMEOUT           (1<<2)
#define FS_MMC_CARD_READ_CRC_ERROR         (1<<3)
#define FS_MMC_CARD_WRITE_CRC_ERROR        (1<<4)
#define FS_MMC_CARD_RESPONSE_GENERIC_ERROR (1<<5)
#define FS_MMC_CARD_READ_GENERIC_ERROR     (1<<6)
#define FS_MMC_CARD_WRITE_GENERIC_ERROR    (1<<7)


#define FS_MMC_RESPONSE_FORMAT_NONE        0
#define FS_MMC_RESPONSE_FORMAT_R1          1
#define FS_MMC_RESPONSE_FORMAT_R2          2
#define FS_MMC_RESPONSE_FORMAT_R3          3
#define FS_MMC_RESPONSE_FORMAT_R6          4  /* Response format R6 is the same as R1 */
#define FS_MMC_RESPONSE_FORMAT_R7          5  /* Response format R7 is the same as R1 */

// response len
#define R1_RESPONSE_LEN                    0x05
#define R2_RESPONSE_LEN                    0x17
#define R3_RESPONSE_LEN                    0x05
#define R6_RESPONSE_LEN                    0x05
#define R7_RESPONSE_LEN                    0x05

#define FS_MMC_CMD_FLAG_DATATRANSFER    (1 <<  0)
#define FS_MMC_CMD_FLAG_WRITETRANSFER   (1 <<  1)
#define FS_MMC_CMD_FLAG_SETBUSY         (1 <<  2)
#define FS_MMC_CMD_FLAG_INITIALIZE      (1 <<  3)
#define FS_MMC_CMD_FLAG_USE_SD4MODE     (1 <<  4)
#define FS_MMC_CMD_FLAG_STOP_TRANS      (1 <<  5)
#define FS_MMC_CMD_FLAG_REPEAT_SAME_SECTOR_DATA (1 <<  6)

/*********************************************************************
*
*             Global function prototypes
*
**********************************************************************
*/

int    FS_MMC_HW_X_IsPresent          (void);
int    FS_MMC_HW_X_ReadData           (void * pBuffer, unsigned NumBytes);
int    FS_MMC_HW_X_SendCmd            (unsigned Cmd, void *pBuffer, unsigned ResponseType, U32 Arg);
void   FS_MMC_HW_X_SetReadDataTimeOut (U32 Value);
U16    FS_MMC_HW_X_SetMaxSpeed        (U16 Freq);
int    FS_MMC_HW_X_WriteData          (const void * pBuffer, unsigned NumBytes);
int    FS_MMC_HW_X_InitHW             (void);
int    read_SDCard                    (void * readBuffer, U32 sdMemAdr, U32 numReadBytes);
int    write_SDCard                   (void * writeBuffer, U32 sdMemAdr, U32 numWriteBytes);

#endif  /* __MMC_SD_CARDMODE_X_HW_H__ */


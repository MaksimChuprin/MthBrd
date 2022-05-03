#define MAX_DATA_LEN_UART2    128
#define U2BUFLEN              (MAX_DATA_LEN_UART2 * 2 + 64)
#define RSPISD_POINTER        ((pInt8U)&Uart2Buffer[144])
#define APB1                  1
#define APB2                  2
#define APB1_FREQ             42000000
#define APB2_FREQ             84000000

#define U2RS485_ON            GPIOx_SET(GPIOD, 4)
#define U2RS485_OFF           GPIOx_RESET(GPIOD, 4)

extern OS_TASK                OS_UARTS;

void                          UART2_ISR(void);
void                          UARTS_Task(void);
void                          TIM3_ISR(void);

U16                           EKSIS_ASCII( void );
U16                           checkEKSIS(U16 rxNum);

U16                           select_speed(U8 speed, U8 apbn);
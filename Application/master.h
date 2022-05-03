#define MAX_INPUT_COUNT           16
#define MAX_ERROR_INPUT           2
#define MAX_DATA_LEN_UART1        64
#define U1BUFLEN                  (MAX_DATA_LEN_UART1 * 2 + 16)
#define U1_MESSAGE_IN             1

#define U1RS485_ON                GPIOx_SET   (GPIOB, 9)
#define U1RS485_OFF               GPIOx_RESET (GPIOB, 9)
#define SELECT_MUX_I            { GPIOx_SET(GPIOC, 6); GPIOx_RESET(GPIOC, 7); }
#define SELECT_MUX_U            { GPIOx_RESET(GPIOC, 6); GPIOx_SET(GPIOC, 7); }
#define SELECT_MUX_DIG          { GPIOx_RESET(GPIOC, 6); GPIOx_RESET(GPIOC, 7); }
#define SELECT_MUX_OFF          { GPIOx_SET(GPIOC, 6); GPIOx_SET(GPIOC, 7); }

#define GET_ID                    0
#define GET_ERR                   1
#define GET_DATA                  2
#define GET_CONFIGWORD            3
#define START_PUMP                4
#define STOP_PUMP                 5
#define STOP_TIMER                6

extern OS_TASK                    OS_MASTER;

void                              UART1_ISR(void);
void                              MASTER_Task(void);
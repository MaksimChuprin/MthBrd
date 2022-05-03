void IP80_Task                    (void);
void IP502_Task                   (void);
void UDP_Task                     (void);
void ETH_IRQHandler               (void);

#include "modbus.h"

#define UDP_PORT                  1337
#define MODBUS_PORT               502
#define WEB_PORT                  80
#define SOCKET_TIMEOUT            1000

#define UDP_DATA_MAXLEN           1024
#define UDP_BUF_LEN               UDP_DATA_MAXLEN + 8
#define WEB_BUF_LEN               1024
#define TCP_MODBUS_LEN            MAX_REG_COUNT * 2 + MODBUS_TCP_HEADER_LEN + MODBUS_CRC_LEN + MODBUS_RTU_HEADER_LEN

SOCKTYPE                          ListenAtTcpAddr       (U16 port);
SOCKTYPE                          OpenAtUdpAddr         (U16 port);
U16                               EKSIS_TCP             (pU8 buffer);
void                              proceedWEB(SOCKTYPE pOutput, pU8 tcp_buffer);

int                               _IP_WEBS_SendString   (SOCKTYPE pOutput, pU8 str);
int                               send_sock_cyclone     (SOCKTYPE pOutput, pU8 str);

void WS_IndexPage                 (SOCKTYPE pOutput, pU8 buffer);
void WS_GetStatistic              (SOCKTYPE pOutput, pU8 buffer);
void WS_ResetStatistic            (SOCKTYPE pOutput);

void WS_SendCSS                   (SOCKTYPE pOutput);
void WS_SendHeader                (SOCKTYPE pOutput, pU8 buffer);
void WS_SendChannels              (SOCKTYPE pOutput, pU8 buffer);
void WS_SendChannel               (SOCKTYPE pOutput, Int8U display_channel_index, pU8 buffer);
void WS_SendParameter             (SOCKTYPE pOutput, Int8U display_channel_index, Int8U dispay_param_index, pU8 buffer);
void WS_SendControl               (SOCKTYPE pOutput, pU8 buffer);
void WS_SendFooter                (SOCKTYPE pOutput, pU8 buffer);
void SendStatisticHeaderForWS     (SOCKTYPE pOutput, pU8 buffer);
void SendStatisticRecordForWS     (int record_index, SOCKTYPE pOutput, pU8 buffer);
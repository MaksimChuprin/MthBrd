#include "defines.h"
#include "IP_init.h"
#include <string.h>
#include "core/net.h"
#include "core/bsd_socket.h"
#include "core/socket.h"

#pragma data_alignment = 4
static U8             web_buffer[WEB_BUF_LEN];
    
// web port 80
void IP80_Task(void) 
{ 
  static SOCKTYPE     socket80, socket80_w;
  int                 addrlen, result;
  struct  sockaddr    saddr80;
  timeval             timeout;
       
  // open server's sockets          
  do 
  {
    socket80 = ListenAtTcpAddr(WEB_PORT);
    OS_Delay(100);
  } while(socket80 == SOCKET_ERROR); 
    
  // prosess command socket
  for(;;)
  {    
    // wait for link 
    while (IP_IFaceIsReady(0) == 0)  
    {
      OS_Delay(100);
    }
    
    // look for incoming connection
    addrlen                 = sizeof(saddr80);
    socket80_w              = accept(socket80, &saddr80, &addrlen);        
    if(socket80_w == SOCKET_ERROR)      continue;             
    
    timeout.tv_sec  = 5; 
    timeout.tv_usec = 0;
    setsockopt(socket80_w, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));  // Set socket timeout 5 sec
            
    // web port
    for(int i = 0, r = 0; ; i++)
    {
      if( i == 0 )
      {
        result = recv(socket80_w, (char*)web_buffer, sizeof(web_buffer), 0);
        if( result == sizeof(web_buffer) )  
        {
          timeout.tv_sec  = 0; 
          timeout.tv_usec = 200000;
          setsockopt(socket80_w, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));  // Set socket timeout 0.2 sec
        }
        else break;
      }
      else   
      {
        r = recv(socket80_w, (char*)&web_buffer[32], sizeof(web_buffer) - 32, 0); // dummy read
        if( ( r == -1) || ( r == 0 ) || ( r < sizeof(web_buffer) - 32 ) ) break;
      }
    }
                            
    if( result > 0 )  
    {
      proceedWEB( socket80_w, web_buffer );
    } 
    
    shutdown( socket80_w, SOCKET_SD_BOTH); 
      closesocket(socket80_w);
  }
}

#pragma data_alignment = 4
static U8           tcp_buffer[TCP_MODBUS_LEN]; 

// modbus port 502
void IP502_Task(void) 
{  
  static SOCKTYPE     socket502, socket502_w;
  int                 addrlen, result;
  struct  sockaddr    saddr502;
  timeval             timeout = { 5, 0 };
    
       
  // open server's sockets
  do 
  {
    socket502 = ListenAtTcpAddr(MODBUS_PORT);
    OS_Delay(100);
  } while(socket502 == SOCKET_ERROR); 
    
  // prosess command socket
  for(;;)
  {    
    // wait for link 
    while (IP_IFaceIsReady(0) == 0)
    {
      OS_Delay(100);
    }
    
    // look for incoming connection
    addrlen                 = sizeof(saddr502);
    socket502_w             = accept(socket502, &saddr502, &addrlen); 
    if(socket502_w == SOCKET_ERROR)      continue;             
    
    timeout.tv_sec  = 5; 
    timeout.tv_usec = 0;       
    setsockopt(socket502_w, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));  // Set socket timeout 5 s         
    
    // command port 502
    for(;;)
    {
      result = recv(socket502_w, (char*)tcp_buffer, sizeof(tcp_buffer), 0);
      
      if( result > 0 )
      {
        // modbus protocol
        result = MODBUS_TCP(tcp_buffer);
        if(result) 
        {   
          send(socket502_w, tcp_buffer, result, 0);
        }
        else break;
      }
      else break;
    }
    
    shutdown(socket502_w, SOCKET_SD_BOTH);
      closesocket(socket502_w);
  }
}

#pragma data_alignment = 4
static U8                 udp_buffer[UDP_BUF_LEN];

// UDP Socket
void UDP_Task(void) 
{
  SOCKTYPE  sock;
  int       Len;
  timeval   timeout;
  struct    sockaddr_in     TargetAddr;  
  
  // open UDP port
  while( 1 )
  {
    sock = OpenAtUdpAddr(UDP_PORT);
    if( sock != SOCKET_ERROR ) break;
    OS_Delay(100);
  }

  TargetAddr.sin_family       = AF_INET;
  TargetAddr.sin_port         = htons(MODBUS_PORT);
  TargetAddr.sin_addr.s_addr  = htonl(INADDR_ANY);

  for(;;)
  {
    // wait for link 
    while (IP_IFaceIsReady(0) == 0)
    {
      OS_Delay(100);
    }
    
    timeout.tv_sec  = 1; 
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));  // Set socket timeout   
    
    Len = sizeof(TargetAddr);
    Len = recvfrom(sock, (char*)udp_buffer, sizeof(udp_buffer), 0, (struct sockaddr*)&TargetAddr, &Len);
    if (Len > 0) 
    {
      // prosess eksis  protocol         
      Len = EKSIS_TCP(udp_buffer);
      sendto(sock, (char*)udp_buffer, Len, 0, (struct sockaddr*)&TargetAddr, sizeof(TargetAddr));
      if(system_flags.start_boot) 
      {
        OS_Delay(50);
        NVIC_SystemReset();
      }
    }
  }
}

// open socket in server mode
static SOCKTYPE ListenAtTcpAddr(U16 port) 
{
  SOCKTYPE            sock;
  struct sockaddr_in  addr;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock != SOCKET_ERROR) 
  {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      =   AF_INET;
    addr.sin_port        =   htons(port);
    addr.sin_addr.s_addr =   htonl(INADDR_ANY);
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(sock, 1);
  }
  return sock;
}

// open UDP socket
static SOCKTYPE OpenAtUdpAddr(U16 port) 
{
  SOCKTYPE            sock;
  struct sockaddr_in  addr;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock != SOCKET_ERROR) 
  {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      =   AF_INET;
    addr.sin_port        =   htons(port);
    addr.sin_addr.s_addr =   htonl(INADDR_ANY);
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  }
  return sock;
}

int send_sock_cyclone(SOCKTYPE pOutput, pU8 str)
{
  int     len    = strlen(str);
  int     result = 0;
  int     pointerBuf;
  
  for(pointerBuf = 0;len > 0; )
  {
    result      = send(pOutput, &str[pointerBuf], len, 0);
    if( result == -1)         break;  //шота пошло не так
    
    pointerBuf += result;
    len        -= result;
  }
  return result;
}

int _IP_WEBS_SendString(SOCKTYPE pOutput, pU8 str)
{
  int     result;
  
  result = send_sock_cyclone(pOutput, str);
  if(result >= 0) return send_sock_cyclone(pOutput, "\r\n");
  return result;
}

// web string maker
/*
int _IP_WEBS_SendString(SOCKTYPE pOutput, pU8 str)
{
  static  U16   pointerBuf;
  int     len = strlen(str);
  int     result = len;
  
  memcpy(&web_buffer[pointerBuf], str, len);
  pointerBuf                   += len;
  web_buffer[pointerBuf++]     = 0xd;
  web_buffer[pointerBuf++]     = 0xa;

//  if( (pointerBuf >= WEB_BUF_LEN / 2) || system_flags.LastWEBSERstring) // порог отсылки - половина буфера
  {
    for(;pointerBuf;)
    {
      result      = send(pOutput, web_buffer, pointerBuf, 0);
      if( result == -1) 
      { 
        pointerBuf = 0; 
        break; 
      } //шота пошло не так
      pointerBuf -= result;
      if(pointerBuf) memcpy(web_buffer, &web_buffer[result], pointerBuf);
    }
    system_flags.LastWEBSERstring = 0;
  }
 
  return result;
}
*/


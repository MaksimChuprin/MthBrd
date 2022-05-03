#include "defines.h"
#include "IP_init.h"
#include <string.h>
#include "core/net.h"
#include "core/bsd_socket.h"
#include "core/socket.h"
#include "sntp/sntp_client.h"
#include "mqtt/mqtt_client.h"
#include "mqtt/mqtt_client_packet.h"
#include "mqtt/mqtt_client_transport.h"
#include "mqtt/mqtt_client_misc.h"
#include "debug.h"

#define T_ERROR           (1<<0)
#define H_ERROR           (1<<1)
#define P_ERROR           (1<<2)
#define NOHUM_ERROR       (1<<6)
#define ISPRESS_ERROR     (1<<7)

static MqttClientContext dev_mqtt_context;
static char              topic_buff   [32];
static char              message_buff [512];

static void              NTPService      ( void );
static void              setTime         ( U32 timeUTC );

void MQTT_Task(void) 
{    
  IpAddr  ipaddr         = { 4, 0 };
  U32     ntpLastTimeTry =        0;
  U32     ttime, pingtime, infotime, len;  
  U8      ntpTry         = NTPConfig.ntpuse;
    
  for(system_flags.mqtt_ini = 1; ; OS_Delay(1000))
  {    
    system_flags.mqtt_on = 0;
    
    /* синхронизаци€ времени */
    if( ntpTry ) 
    {       
      NTPService();
      ntpTry         = 0;
      ntpLastTimeTry = OS_GetTime32();
      continue;
    }    
    ntpTry = NTPConfig.ntpuse && (NTPConfig.NTPSyncReq) && ((ntpLastTimeTry + 3600000) < OS_GetTime32());
    /* синхронизаци€ времени */
    
    if( !MQTTConfig.mqtt_use ) continue;
          
    if( system_flags.mqtt_ini )
    {
      system_flags.mqtt_ini = 0;      
      infotime              = 0;
      
      mqttClientInit            ( &dev_mqtt_context);
      mqttClientBindToInterface ( &dev_mqtt_context, &netInterface[0]);
      mqttClientSetIdentifier   ( &dev_mqtt_context, SerialNumber);
      mqttClientSetAuthInfo     ( &dev_mqtt_context, MQTTConfig.UserName, MQTTConfig.UserPass);
    }
    
    // set protocol and check interface
    if( !IP_IFaceIsReady(0) )                                                                                           continue;        
    if( getHostByName(&netInterface[0], MQTTConfig.SeverHostName, &ipaddr, HOST_TYPE_IPV4 | HOST_NAME_RESOLVER_DNS) )   continue;
    mqttClientSetTimeout          ( &dev_mqtt_context, 20000);
    mqttClientSetTransportProtocol( &dev_mqtt_context, MQTT_TRANSPORT_PROTOCOL_TCP);
                  
    // connect to broker
    if ( mqttClientConnect( &dev_mqtt_context, &ipaddr, MQTTConfig.ServerPort, true) != NO_ERROR ) continue;
    
    system_flags.mqtt_on = 1;
    
    for(;;OS_Delay(250))
    {
      //  Time Need Sync
      ntpTry = NTPConfig.ntpuse && (NTPConfig.NTPSyncReq) && ((ntpLastTimeTry + 3600000) < OS_GetTime32());
      if( ntpTry ) break; 
      
      // changes?
      if( system_flags.mqtt_ini ) break;
      
      // publish info-block
      ttime = OS_GetTime32();
      if( MQTTConfig.info_interval || !infotime )  
      {
        if( ((infotime + MQTTConfig.info_interval*1000) < ttime ) || !infotime )
        {
          sprintf( topic_buff, "%s/%s/info", MQTTConfig.TopicBase, SerialNumber );
          len = 0;
          message_buff[len++] = '{';
          len += sprintf( &message_buff[len], "   \"timeStamp\": %u ", Time);
          len += sprintf( &message_buff[len], " , \"deviceClass\": 71 ");
          len += sprintf( &message_buff[len], " , \"deviceName\": \"\xD0\xA0\xD0\x9C-2-L\" ");
          len += sprintf( &message_buff[len], " , \"firmwareVersion\": \"%s\" ", Version);
                    
          message_buff[len++] = '}';
          if( mqttClientPublish( &dev_mqtt_context, topic_buff, message_buff, len, MQTTConfig.mqtt_qos, true, NULL) != NO_ERROR ) break;  // break if anyerrors
          
          // сбросить врем€ "неактивности" и infotime
          dev_mqtt_context.pingTimestamp = infotime = OS_GetTime32();
        }
      }      
      
      // keepalive ping
      ttime = OS_GetTime32();
      if( MQTTConfig.ping_interval )  
      {
        if( (dev_mqtt_context.pingTimestamp + MQTTConfig.ping_interval*1000) < ttime )
        {
          if( mqttClientPing( &dev_mqtt_context, &pingtime) != NO_ERROR ) break; // break if anyerrors
        }
      }
                                        
      sprintf( topic_buff, "%s/%s/data/", MQTTConfig.TopicBase, SerialNumber );
   
      if( mqttClientPublish( &dev_mqtt_context, topic_buff, message_buff, len, MQTTConfig.mqtt_qos, true, NULL) != NO_ERROR ) break; // break if anyerrors

      // сбросить врем€ "неактивности" 
      dev_mqtt_context.pingTimestamp = OS_GetTime32(); 
    }      
    
    mqttClientClose(&dev_mqtt_context); // закрыть сокет при ошибке    
  }
}

/*
static U32 messageFor7M4( void )
{
  U32 len = 0;
    
  message_buff[len++] = '{';
  
  len += sprintf( &message_buff[len], " , \"deviceClass\": 91 " );
  len += sprintf( &message_buff[len], " , \"deviceName\": \"\xD0\x98\xD0\x92\xD0\xA2\xD0\x9C-7\xD0\x9C\x34\" ");
  len += sprintf( &message_buff[len], " , \"channels\": ");
    message_buff[len++] = '[';
      message_buff[len++] = '{';
  
  len += sprintf( &message_buff[len], "   \"parameters\": ");
        message_buff[len++] = '[';
        
          message_buff[len++] = '{';          
  len += sprintf( &message_buff[len], "   \"value\": %.1f", SavedList[0].Tempr / 10.);
  len += sprintf( &message_buff[len], " , \"name\": \"\xD0\xA2\xD0\xB5\xD0\xBC\xD0\xBF\xD0\xB5\xD1\x80\xD0\xB0\xD1\x82\xD1\x83\xD1\x80\xD0\xB0\" ");
  len += sprintf( &message_buff[len], " , \"units\": \"\xC2\xB0\x43\" ");  
  if( SavedList[0].Errors & T_ERROR ) len += sprintf( &message_buff[len], ", \"error\": [\"\"] " );
          message_buff[len++] = '}';
  
  if( SavedList[0].Errors & NOHUM_ERROR ) ;
  else
  {
          message_buff[len++] = ',';
          message_buff[len++] = '{';
    len += sprintf( &message_buff[len], "   \"value\": %.1f", SavedList[0].Humidy / 10.);
    len += sprintf( &message_buff[len], " , \"name\": \"\xD0\x92\xD0\xBB\xD0\xB0\xD0\xB6\xD0\xBD\xD0\xBE\xD1\x81\xD1\x82\xD1\x8C\" ");
    len += sprintf( &message_buff[len], " , \"units\": \"%\" ");      
    if( SavedList[0].Errors & H_ERROR ) len += sprintf( &message_buff[len], " , \"error\": [\"\"] " );
          message_buff[len++] = '}';
  }
  
//  if( SavedList[0].Errors & ISPRESS_ERROR )
  {
          message_buff[len++] = ',';
          message_buff[len++] = '{';
    len += sprintf( &message_buff[len], "   \"value\": %u", SavedList[0].Pressure);
    len += sprintf( &message_buff[len], " , \"name\": \"\xD0\x94\xD0\xB0\xD0\xB2\xD0\xBB\xD0\xB5\xD0\xBD\xD0\xB8\xD0\xB5\" ");
    len += sprintf( &message_buff[len], " , \"units\": \"\xD0\xBC\xD0\xBC.\xD1\x80\xD1\x82.\xD1\x81\xD1\x82.\" ");      
    if( SavedList[0].Errors & P_ERROR ) len += sprintf( &message_buff[len], " , \"error\": [\"\"] " );
          message_buff[len++] = '}';
  }  
  
          message_buff[len++] = ',';
          message_buff[len++] = '{';
  len += sprintf( &message_buff[len], "   \"value\": %u", SavedList[0].PowLev);
  len += sprintf( &message_buff[len], " , \"name\": \"\xD0\x97\xD0\xB0\xD1\x80\xD1\x8F\xD0\xB4\" ");
  len += sprintf( &message_buff[len], " , \"units\": \"%\" ");        
          message_buff[len++] = '}';

          message_buff[len++] = ',';
          message_buff[len++] = '{';
  len += sprintf( &message_buff[len], "   \"value\": %d", SavedList[0].RSSI);
  len += sprintf( &message_buff[len], " , \"name\": \"\xD0\xA1\xD0\xB8\xD0\xB3\xD0\xBD\xD0\xB0\xD0\xBB\" ");
  len += sprintf( &message_buff[len], " , \"units\": \"\xD0\xB4\xD0\x91\xD0\xBC\" ");        
          message_buff[len++] = '}';  
  
        message_buff[len++] = ']';  // parameters array
      message_buff[len++] = '}';
    message_buff[len++] = ']';  // channels array
    
  message_buff[len++] = '}';
  
  return len;
}
*/

static void NTPService( void)
{
  IpAddr        ipaddr    = { 4, 0 };
  NtpTimestamp  timestamp = { 0, 0 };
  
  // канал ethernet
  if( IP_IFaceIsReady(0) )
  {
    if( !getHostByName(&netInterface[0], NTPConfig.NTPSeverName, &ipaddr, HOST_TYPE_IPV4 | HOST_NAME_RESOLVER_DNS) ) 
    {
      if( !sntpClientGetTimestamp(&netInterface[0], &ipaddr, &timestamp) )  
      {
        setTime( timestamp.seconds - 0x83AA7E80 ); // 0x83AA7E80 = 70 лет от 01-01-1900 до 01-01-1970
        return;
      }      
    }
  }
}

static void setTime ( U32 timeUTC )
{
  U32 timeshift = abs(3600 * NTPConfig.TimeZone);
        
  OS_Use( &SemaRAM);
    if( NTPConfig.TimeZone < 0 )  Time = timeUTC - timeshift;
    else                          Time = timeUTC + timeshift;
    
    Time -= EKSIS_TIME_SHIFT;  // преобразование Time к Ёксис-времени
    RTC_SetCounter( Time );                                
    set_bkp_reg( ERR_BASE, get_bkp_reg(ERR_BASE) & ~(1L<<1));
    
    NTPConfig.NTPSyncReq = errors_flags.timeinvalid = 0; 
  OS_Unuse( &SemaRAM);
}

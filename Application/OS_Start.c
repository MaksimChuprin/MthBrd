#include "defines.h"
#include "IP_init.h"

const U8 st1[]= "       �������� �������";
const U8 st2[]= "���� EEPROM       ... ";
const U8 st3[]= "���� SD �����     ... ";
const U8 st4[]= "���� LSE          ... ";
const U8 st5[]= "���� RTC          ... ";
const U8 st6[]= "���� ������������ ... ";
const U8 st7[]= " ��������� ��������� ���������!";
const U8 st8[]= "�������";
const U8 st9[]= "������!";
const U8 st10[]= "   ���� ����������� ������";
const U8 st11[]= "���������� ������ ����������!";
const U8 st12[]= "��������� �������� ��������� RTC";
const U8 st13[]= "     ��������� ������������!    ";
const U8 st14[]= " ��� �������� ������������ ... ";
const U8 st15[]= "������� ��� ��� ������� ��������";
const U8 st16[]= "���� Ethernet    ... ";

OS_TASK                           TP80;                           // Task-control-block
OS_TASK                           TP502;
OS_TASK                           TCP_CMD; 
OS_TASK                           OS_TFT;
OS_TASK                           OS_USB;
OS_TASK                           OS_SUPERVISER;
OS_TASK                           OS_MASTER;
OS_TASK                           OS_UARTS; 
OS_TASK                           OS_MQTT;

/* Semafores */
OS_RSEMA                          SemaRAM;
OS_RSEMA                          SemaSPI1;
OS_RSEMA                          SemaSPI3;
OS_RSEMA                          SemaTCP;
OS_RSEMA                          SemaTFT;

/* mail boxes */
OS_MAILBOX                        TouchPad_MB;
OS_MAILBOX                        Key_MB;

/* ������� */
OS_TIMER                          OneSecTimer;
OS_TIMER                          SDTimer;
OS_TIMER                          PumpTimer;
OS_TIMER                          TFTTimer;
OS_TIMER                          SDConfWrite;

       OS_STACKPTR U32            StackTFT[384];                  // Define the stack of the OS_TFT
static OS_STACKPTR U32            IP80Stack[384];                 // Define the stack of the IP80_Task 
static OS_STACKPTR U32            IP502Stack[192];                // Define the stack of the IP502_Task
static OS_STACKPTR U32            TCPCMDStack[192];               // Define the stack of the CMD_Task
static OS_STACKPTR U32            StackUSB[128];                  // Define the stack of the USB_Task
static OS_STACKPTR U32            StackSUPERVISER[128];
static OS_STACKPTR U32            StackMASTER[128];
static OS_STACKPTR U32            StackUARTS[128]; 
static OS_STACKPTR U32            StackMQTT[256];

int main(void) {
  OS_IncDI();                      /* Initially disable interrupts  */
  OS_InitKern();                   /* initialize OS                 */
  
  OS_CREATERSEMA(&SemaRAM);         // create RAM semafor
  OS_CREATERSEMA(&SemaSPI1);        // create SPI1 semafor
  OS_CREATERSEMA(&SemaSPI3);        // create SPI3 semafor
  OS_CREATERSEMA(&SemaTCP);
  
  OS_InitHW();                     /* initialize Hardware for OS    */
  
  OS_CREATEMB( &TouchPad_MB, 4, 1, &MBTouchPadKeyBuffer);
  OS_CREATEMB( &Key_MB, 1, 1, &MBKeyBuffer);
  
  OS_CreateTimer( &OneSecTimer,  oneSecISR, 1000);
  OS_CreateTimer( &SDTimer,     SDTimerISR, 100);
  OS_CreateTimer( &PumpTimer,      PumpISR, 1000);
  OS_CreateTimer( &TFTTimer,        TFTISR, 60000);
  OS_CreateTimer( &SDConfWrite,  SDConfISR, 500);
  
  OS_CREATETASK(&OS_SUPERVISER, 0, Superviser_Task, 50, StackSUPERVISER); 
     
  /* Start multitasking */
  OS_Start();
  return 0;
}

void  Superviser_Task(void)
{
  U32   startTime = OS_GetTime32();
    
  // start
  set_sound(S_SOUND_500);
      
  /* �������� ������ ������������, �������� ������������, �������� ������������ */
  LoadAndCheckConfig( );
      
  /* ������ ������������ */
  OS_CREATETASK(&OS_UARTS,      0, UARTS_Task,      50, StackUARTS); 
  OS_CREATETASK(&OS_USB,        0, USB_Task,        50, StackUSB);  
  
  /* start Ethernet */
  if(ConfigWord & TCP_PRESENT) 
  {
    errors_flags.eth_error = OS_Eth_init(0);
    if( !errors_flags.eth_error )
    {
      OS_CREATETASK( &TP80,    0, IP80_Task , 50, IP80Stack);                                      // Start the port80_Task
      OS_CREATETASK( &TP502,   0, IP502_Task, 50, IP502Stack);                                     // Start the port502_Task    
      OS_CREATETASK( &TCP_CMD, 0, UDP_Task  , 50, TCPCMDStack);                                    // Start the port502_Task
      OS_CREATETASK( &OS_MQTT, 0, MQTT_Task , 50, StackMQTT);
    }
  }
  
#ifndef LED
  /* ������������� TFT */
  errors_flags.tft_fail  =  TFT_init( );
  
  if(errors_flags.tft_fail)
  {
    // ������ ���� �� ������ ������� ����������
    set_sound(R_SOUND_1000_2000);        
    for(;;CLEAR_IWDG) OS_Delay(1000); 
  }
          
  /* ������������ ����� ��������� */  
  {   
    char        st[64];
    // ����
    TFT_String.fColor   =   WHITE;
    TFT_String.bColor   =   BLACK;
    TFT_String.Y        =   0;
    // �����
#ifdef TFT32
    TFT_String.fontType =  &Tahoma19B; 
#else
    TFT_String.fontType =  &Tahoma34;
#endif    
    if( system_flags.code_updated ) { TFT_WriteStringT((pU8)st15); OS_Delay(1000); }    
    TFT_WriteStringT((pU8)st1); TFT_WriteStringT(""); OS_Delay(250); 
      sprintf(st,"%s%s", st3, (errors_flags.config_memory_fail || errors_flags.sd_fail || errors_flags.sd_not_present) ? st9 : st8); TFT_WriteStringT((pU8)st); OS_Delay(250); 
        sprintf(st,"%s%s", st4, errors_flags.LSEfail ? st9 : st8); TFT_WriteStringT((pU8)st); OS_Delay(250); 
          sprintf(st,"%s%s", st5, errors_flags.timeinvalid ? st9 : st8); TFT_WriteStringT((pU8)st); OS_Delay(250); 
            if(ConfigWord & TCP_PRESENT) { sprintf(st,"%s%s", st16, errors_flags.eth_error ? st9 : st8); TFT_WriteStringT((pU8)st); OS_Delay(250); }
              sprintf(st,"%s%s", st6, errors_flags.config_fail ? st9 : st8); TFT_WriteStringT((pU8)st); OS_Delay(250); 
                OS_Delay(1000);
  }
    
  /* ����������� ������ */
  if( errors_flags.config_memory_fail || errors_flags.sd_fail || errors_flags.sd_not_present || errors_flags.LSEfail )
  {
    set_sound(S_SOUND_500);  
    TFT_WriteStringT(""); 
      TFT_WriteStringT((pU8)st10); 
        TFT_WriteStringT((pU8)st11); 
    
    for(;;CLEAR_IWDG) OS_Delay(1000);
  }
     
  /* ��� ������������  */
  if(errors_flags.config_fail)
  {
    set_sound(S_SOUND_500);
    TFT_WriteStringT("");
      TFT_WriteStringT((pU8)st13);
        
    while((ConfigWord & CONFIG_MASK) != (~InvConfigWord & CONFIG_MASK)) 
    {
      OS_Delay(1000);
      CLEAR_IWDG;
    }
    
    U8 result = 0;
    for(U32 i = 0; i < SD_ALLCONFIG_SECTORS; i++, CLEAR_IWDG) result |= saveSDConfDelayed(2);
      
    if( !result ) 
    { 
      if ( !ConfigDeal() )       set_sound(S_SOUND_500);
    }
    
    OS_Delay(1000);
    NVIC_SystemReset();     
  }
  
  /* ����� �������� */
  if(errors_flags.timeinvalid)
  {
    TFT_WriteStringT("");
    TFT_WriteStringT((pU8)st12);
    set_sound(S_SOUND_500);
    for(U32 i = 0; i < 3; i++, CLEAR_IWDG) OS_Delay(1000);
  }
  
  OS_CREATETASK(&OS_TFT, "TFT Task", TFT_Task, 50, StackTFT);
#else
  led_init ();
#endif
  
 /* ��������� ������ */
  OS_CREATETASK( &OS_MASTER, "MASTER Task", MASTER_Task, 50, StackMASTER);  
  OS_STARTTIMER_IF_NOT( &OneSecTimer);
  Time = RTC_GetCounter();
  TimeUpCounter += (OS_GetTime32() - startTime) / 1000;
    
  for(U32 superviserWaitTime = 50; ; CLEAR_IWDG)
  {
    /*  */
    OS_Delay(superviserWaitTime);
    U32 startLoopTime = OS_GetTime32();
        
#ifndef LED
    /* ���� ��� ������ */
    scan_touch();
    /* AUX keys */
    scan_key();        
#endif                
    /*  ���������� ���� � ��� */
    set_relay_ang();                
    
    /* ����� �������� ���������� � ���������� */
    char message = OS_ClearEvents(NULL);
    {
      // 1 sec event
      if(message & (1<<0))
      {       
        // ���������
        ControlManager();
        // �������� ������������
        ConfigDeal();
        // ����
        SoundDeal();
        // ����� ������
        set_bkp_reg( TIMEUP_COUNTER, ++TimeUpCounter);
        // ������� �������
        Time++;
        // ������ ���������� �������        
        if( ++NTPConfig.updateCounter > NTPConfig.periodUpdateTime ) 
        {
          NTPConfig.NTPSyncReq    = 1;
          NTPConfig.updateCounter = 0;
        }
      }
      // ������ ����� ����������
      if(message & (1<<1)) StatisticManager();      
    }
    
    /* ���������� ������ ������������ */
    if( !OS_GetTimerStatus(&SDConfWrite) )
    {
      OS_Use( &SemaRAM);      
        errors_flags.config_memory_fail |= saveSDConfDelayed(2);
      OS_Unuse( &SemaRAM);
    }
    
    /* ���� ������� ������������ "�� ����" - ���������� */
    if( OS_GetSuspendCnt(&OS_MASTER) )
      if ( !ifAnyLeft2Write() ) 
      {
        OS_Delay(1000);
        NVIC_SystemReset();
      }    
      
    /*        50 ��          */
    startLoopTime = OS_GetTime32() - startLoopTime; 
    if(startLoopTime < 50)  superviserWaitTime = 50 - startLoopTime; 
    else                    superviserWaitTime = 0;
  }
}

/* ���������� */
void  StatisticManager(void)
{
  // sync Time
  Time = RTC_GetCounter();
  
  if (errors_flags.sd_fail) return;
  
  OS_Use( &SemaRAM);
  {
     errors_flags.sd_fail |= write_SDCard( (pU8)MainArray, WriteStatAddr, 0x600);
     WriteStatAddr        += 0x600;
     if(WriteStatAddr >= (SD_STATISTIC_SIZE + SD_STATISTIC_OFFSET))
     {
        WriteStatAddr     = SD_STATISTIC_OFFSET;
        ConfigWord       |= STATISCTIC_OVERFLOW;
        SAVE_CONFIG (ConfigWord);
     }
     set_bkp_reg(STATADR_REG, WriteStatAddr);
  }
  OS_Unuse( &SemaRAM);
}

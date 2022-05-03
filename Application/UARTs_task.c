#include "defines.h"

// global

// local static
U8                            Uart2Buffer[U2BUFLEN];
U16                           BytesToTransfer;
static U16                    Rx2Counter;

// установка скорости UARTов
U16   select_speed(U8 speed, U8 apbn)
{
 U32    tim3;
 U16    br;
 F32    time_out;
  
 switch(apbn)
 {          
  case APB1:            
            switch(speed)
            {
            case S1200:   time_out  = 0.06;
                          br        = APB1_FREQ / 1200;
                          break;
                          
            case S2400:   time_out  = 0.045;
                          br        = APB1_FREQ / 2400;
                          break;
                          
            case S4800:   time_out  = 0.03;
                          br        = APB1_FREQ / 4800;
                          break;                          
                          
            case S9600:   time_out  = 0.02;
                          br        = APB1_FREQ / 9600;
                          break;
                          
            case S19200:  time_out  = 0.02;
                          br        = APB1_FREQ / 19200;
                          break;
                          
            case S38400:  time_out  = 0.01;
                          br        = APB1_FREQ / 38400;
                          break;
                          
            case S57600:  time_out  = 0.005;
                          br        = APB1_FREQ / 57600;
                          break;
                          
            case S115200: time_out  = 0.005;
                          br        = APB1_FREQ / 115200;
                          break;
                          
            default:      time_out  = 0.02;
                          br        = APB1_FREQ / 9600;
                          break;
            }
            tim3        =   time_out * APB1_FREQ;
            TIM3->ARR   =   TIM3->CCR1 = (U16)tim3; 
            TIM3->PSC   =   (U16)(tim3 >> 16);
            break;
          
  case APB2: switch(speed)
            {                         
            case S2400:   br        = APB2_FREQ / 2400;
                          break;
                          
            case S4800:   br        = APB2_FREQ / 4800;
                          break;                          
                          
            case S9600:   br        = APB2_FREQ / 9600;
                          break;
                          
            case S19200:  br        = APB2_FREQ / 19200;
                          break;
                          
            case S38400:  br        = APB2_FREQ / 38400;
                          break;
                          
            case S57600:  br        = APB2_FREQ / 57600;
                          break;
                          
            case S115200: br        = APB2_FREQ / 115200;
                          break;
                          
            default:      br        = APB2_FREQ / 9600;
                          break;                                        
            }
            break;
 }
 
 return br;
}

// isr TIM3
#pragma optimize = none
void  TIM3_ISR(void)
{
  OS_EnterInterrupt();
  
  TIM3->SR           &=  ~BIT1;
  TIM3->CR1          &=  ~BIT0;
  CLEAR_BIT(USART2->CR1, USART_CR1_RE);
  OS_SignalEvent( BIT0, &OS_UARTS);
  
  OS_LeaveInterrupt();
}

// isr uart2
#pragma optimize = none
void UART2_ISR(void) 
{
  static U16    txCounter;
  U8            inByte;
  U32           uart_status;
  
  OS_EnterInterrupt();
  
  uart_status  = USART2->SR;
  inByte       = USART2->DR;  
  
  if(uart_status & (0x0f)) // FE NE ORE - errors
  {
    Rx2Counter     =      0;
    TIM3->CR1     &=  ~BIT0;
    TIM3->SR      &=  ~BIT1;
  }
  else
  if(uart_status & (1<<5))  // RXNE
  {
    if(Rx2Counter == U2BUFLEN)   
    {
      Rx2Counter   =  0; 
    }

    Uart2Buffer[Rx2Counter++] = inByte;     
    //запустить или обнулить таймер
    TIM3->EGR      |=   BIT0;
    TIM3->CR1      |=   BIT0;
  }
  
  if( (USART2->CR1 & USART_CR1_TCIE) && (uart_status & (1<<6))) // TC
  {    
    if((txCounter == BytesToTransfer) || (txCounter == U2BUFLEN))
    {
      txCounter =           0;
      U2RS485_OFF;
      CLEAR_BIT(USART2->CR1, USART_CR1_TCIE);
      SET_BIT  (USART2->CR1, USART_CR1_RE);

      if(system_flags.reiniuart2) USART2->BRR = select_speed( ConfigWord & UART_MASK, APB1);
      if(system_flags.start_boot) NVIC_SystemReset();      
    } 
    else USART2->DR = Uart2Buffer[txCounter++];
  }
  
  OS_LeaveInterruptNoSwitch();
}

void UARTS_Task(void) 
{
  U16     rxnum;
  
  // инициализаци€ UART
  __HAL_RCC_USART2_CLK_ENABLE();
  __HAL_RCC_TIM3_CLK_ENABLE();
  USART2->BRR = select_speed(ConfigWord & UART_MASK, APB1);
  USART2->CR1= USART2->CR2 = USART2->CR3= USART2->GTPR= 0;
  SET_BIT(USART2->CR1, USART_CR1_UE | USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE);

  // инициализаци€ TIM3  
  TIM3->CR1   = 0;                   // 
  TIM3->CCMR1 = BIT4;                // CC1 
  TIM3->DIER  = BIT1;                // CC1IE
  
  // инициализаци€ в VIC   
  OS_ARM_InstallISRHandler(USART2_IRQn + 16, (OS_ISR_HANDLER*)UART2_ISR);
  OS_ARM_ISRSetPrio(USART2_IRQn + 16, 0xFE);
  OS_ARM_EnableISR(USART2_IRQn + 16); 
  
  OS_ARM_InstallISRHandler(TIM3_IRQn + 16, (OS_ISR_HANDLER*)TIM3_ISR);
  OS_ARM_ISRSetPrio(TIM3_IRQn + 16, 0xFE);
  OS_ARM_EnableISR(TIM3_IRQn + 16);
    
  for(;;)
  {
    OS_WaitSingleEvent(BIT0);
    
    if(!Rx2Counter)
    {
      SET_BIT  (USART2->CR1, USART_CR1_RE);
      continue;
    }
    
    rxnum       = checkEKSIS(Rx2Counter);
    Rx2Counter  = 0;  

    if( rxnum )
    {      
      if ( check_sum8(Uart2Buffer, rxnum - 2) == hex_to_char(rxnum - 2, Uart2Buffer) )  BytesToTransfer =  EKSIS_ASCII( );
      else                                                                              BytesToTransfer =  MODBUS_RTU (Uart2Buffer, NetAddress);
    }
    else                                                                                BytesToTransfer =  MODBUS_RTU (Uart2Buffer, NetAddress);
  
    if( !BytesToTransfer )
    {
      memset(Uart2Buffer, 0, U2BUFLEN );
      SET_BIT  (USART2->CR1, USART_CR1_RE);
    }
    else 
    {        
      U2RS485_ON;
      SET_BIT  (USART2->CR1, USART_CR1_TCIE);
    }       
  }
}

U16 checkEKSIS(U16 rxNum)
{
  if( (Uart2Buffer[0] != '$') || (rxNum < 10) ) return 0;
  
  for(U16 i = 1; i < rxNum; i++)
  {
    if( Uart2Buffer[i] == 0x0d )                                 return ( i > 8 ? i : 0);
    if( (Uart2Buffer[i] < 0x21) || (Uart2Buffer[i] > 0x7A) )     return 0;
  }
  return 0;
}

// ....
U16 EKSIS_ASCII( void )
{
  U32     hiaddr, addr, command, i, tx_num;
  U8      chksum;
  
  addr = hex_to_int(1, Uart2Buffer);
  if( (addr != NetAddress) && (addr != 0xffff)  )  return 0;
  
  command         =  Uart2Buffer[5] * 256 + Uart2Buffer[6];
  addr            =  hex_to_int(7, Uart2Buffer);  
  hiaddr          =  hex_to_int(13, Uart2Buffer);
  tx_num          =  hex_to_char(11, Uart2Buffer);  
  Uart2Buffer[0]  =  '!';
    
  switch(command)
  {      
     case 'BS':   OS_Use( &SemaRAM);
                    system_flags.start_boot= 1;
                      OS_Unuse( &SemaRAM);
                  set_mark_forbooter(0);
                  tx_num = 7;
                  goto final;
          
     case 'WI':   if((tx_num > MAX_DATA_LEN_UART2) || ((addr + tx_num) >  SD_ALLCONFIG_SIZE))   goto fail;
                  OS_Use( &SemaRAM);
                    hexbuffer_2_binbuffer(Uart2Buffer,(pInt8U)(ConfigBeginAddr + addr), tx_num, 13);
                    if( (addr == CONFIG_ROM_ADR(&ConfigWord)) ||  (addr == (CONFIG_ROM_ADR(&ConfigWord)) + 3) )
                    {
                      system_flags.reiniuart2 = 1;                      
                      
                      if(ConfigWord & RESTART_STATISTIC)
                      {
                        ConfigWord    &= ~(RESTART_STATISTIC + STATISCTIC_OVERFLOW);
                        WriteStatAddr  =  SD_STATISTIC_OFFSET;
                        set_bkp_reg (STATADR_REG, WriteStatAddr);
                      }
                      
                      if(ConfigWord & CLEAR_BACKUP_REG)
                      {
                        ConfigWord    &= ~(CLEAR_BACKUP_REG);
                        set_bkp_reg( RESET_COUNTER,     0);
                        set_bkp_reg( POWERUP_COUNTER,   0);
                        set_bkp_reg( TIMEUP_COUNTER,    0);
                        set_bkp_reg( USBUPDATE_COUNTER, 0);
                        set_bkp_reg( SDUPDATE_COUNTER,  0);
                        set_bkp_reg( LASTUPDATE_RESULT, 0);
                        set_bkp_reg( SDFAIL_COUNT, 0);
                        set_bkp_reg( SDCFGFAIL_COUNT, 0);
                      }
                    }                    
                    seachSectorsToWrite ( addr, tx_num);                       
                  OS_Unuse( &SemaRAM);
                  tx_num = 7; 
                  goto final;
                  
     case 'WR':   if( (tx_num > MAX_DATA_LEN_UART2) || ((addr+tx_num) > RAMSIZE) ) goto fail;
                  OS_Use( &SemaRAM);
                    hexbuffer_2_binbuffer(Uart2Buffer,(pInt8U)(RamBeginAddr + addr), tx_num, 13);
                    if( (RamBeginAddr + addr) == (U32)&Time )  // установить RTC
                    {
                      RTC_SetCounter (Time); // приходит Ёксис-врем€
                      errors_flags.timeinvalid = 0;
                      set_bkp_reg( ERR_BASE, get_bkp_reg(ERR_BASE) & ~(1L<<1));
                    }
                  OS_Unuse( &SemaRAM);
                  tx_num = 7;
                  goto final;

     case 'RR':   if( (tx_num > MAX_DATA_LEN_UART2) || ((addr + tx_num) >  RAMSIZE) ) goto fail;
                  OS_Use( &SemaRAM);
                    binbuffer_2_hexbuffer(Uart2Buffer,(pInt8U)(RamBeginAddr + addr), tx_num, 7);
                  OS_Unuse( &SemaRAM);
                  tx_num = 7 + tx_num * 2;
                  goto final;
                  
     case 'RI':   if((tx_num > MAX_DATA_LEN_UART2) || ((addr + tx_num) >  SD_ALLCONFIG_SIZE)) goto fail;
                  OS_Use( &SemaRAM);                                      
                    binbuffer_2_hexbuffer(Uart2Buffer, (pU8)(ConfigBeginAddr + addr), tx_num, 7); 
                  OS_Unuse( &SemaRAM);
                  tx_num=  7 + tx_num * 2;
                  goto final;
                  
     case 'RF':   if(tx_num > MAX_DATA_LEN_UART2) goto fail;                  
                  addr  += (hiaddr << 16);
                  errors_flags.sd_fail |= read_SDCard( RSPISD_POINTER, addr, tx_num);
                  if( errors_flags.sd_fail ) goto fail;
                  
                  binbuffer_2_hexbuffer(Uart2Buffer, RSPISD_POINTER, tx_num, 7); 
                  tx_num= tx_num*2 + 7; 
                  goto final;
                  
    case 'CR':    tx_num = 7;
                  binbuffer_2_hexbuffer(Uart2Buffer, (pU8)&IDWordLo, sizeof(IDWordLo), tx_num);                  
                  tx_num += sizeof(IDWordLo) * 2;
                  binbuffer_2_hexbuffer(Uart2Buffer, (pU8)&IDWordHi, sizeof(IDWordHi), tx_num);                  
                  tx_num += sizeof(IDWordHi) * 2;
                  goto final;                  

     case 'IR':   tx_num= 7;
                  for(i=0;i<8;i++,tx_num++)             Uart2Buffer[tx_num]= SerialNumber[i];
                  Uart2Buffer[tx_num++]= ' ';
                  for(i=0;Version[i];i++,tx_num++)      Uart2Buffer[tx_num]= Version[i];
                  Uart2Buffer[tx_num++]= ' ';
                  for(i=0;DeviceName[i];i++,tx_num++)   Uart2Buffer[tx_num]= DeviceName[i];
                  Uart2Buffer[tx_num++]= ' ';
                  goto final;                  
                  
    case 'TS':    tx_num= 7;
                  OS_Use( &SemaRAM);
                    system_flags.set_retrans= 1;
                      OS_Unuse( &SemaRAM);
                  SetComInputLine(hex_to_char(7,Uart2Buffer)); 
                  goto final;
                  
    case 'TC':    tx_num= 7;
                  OS_Use( &SemaRAM);
                    system_flags.set_retrans= 0;
                      OS_Unuse( &SemaRAM);
                  goto final;                  
                  
    case 'DW':    OS_Use( &SemaRAM);
                    system_flags.writedefaultmem= 1;
                      OS_Unuse( &SemaRAM);
                  tx_num= 7;
                  goto final;

    case 'DR':    OS_Use( &SemaRAM);
                    system_flags.readdefaultmem= 1;
                      OS_Unuse( &SemaRAM);
                  tx_num= 7;
                  goto final;
                  
    case 'RS':    OS_Use( &SemaRAM);
                  system_flags.redraw_gui = 1;
                  OS_Unuse( &SemaRAM);
                  tx_num = 7;
                  goto final;
                  
    case 'CT':    OS_Use( &SemaRAM);
                    system_flags.ScreenCalib = 1;
                      OS_Unuse( &SemaRAM);
                  tx_num = 7;
                  goto final;                  
                  
    default:
fail:             Uart2Buffer[0]  =   '?';
                  tx_num =     7;
final:
                  chksum = check_sum8(Uart2Buffer, tx_num);
                  binbuffer_2_hexbuffer(Uart2Buffer, &chksum, 1, tx_num);
                  tx_num               += 2;
                  Uart2Buffer[tx_num++] = 0x0d;                                        
    }
  return tx_num;
}

#include "defines.h"
#include "IP_init.h"

#define TCP_ASK                   0x00
#define TCP_CMDERR                0x01
#define TCP_NAK                   0xFF
#define PAYLOADANS_LEN            4

#define TCP_WDATA_POINTER         &buffer[8]
#define TCP_RDATA_POINTER         &buffer[4]
#define ASK_TCPANS                buffer[0]
#define TCP_DATA_NUM              *((pU16)&buffer[2])

// EKSIS - forever
U16 EKSIS_TCP(pU8 buffer)
{
  U8            command;
  U16           bytenum;
  U32           addr;  
  int           response;
  
  addr          = *((pU32)&buffer[0]);
  bytenum       = *((pU16)&buffer[4]);
  command       = *((pU16)&buffer[6]);
    
  switch(command)
  {
  // WR 
  case WR_USB:  if( (bytenum > UDP_DATA_MAXLEN) || ((addr + bytenum) > RAMSIZE) ) goto fail;
  
                  OS_Use( &SemaRAM);
                    binbuffer_2_binbuffer( TCP_WDATA_POINTER, (pU8)(RamBeginAddr + addr), bytenum);
                    if( (RamBeginAddr + addr) == (U32)&Time )  // установить RTC
                    {
                      RTC_SetCounter (Time);    // приходит Ёксис-врем€
                      errors_flags.timeinvalid = 0;
                      set_bkp_reg( ERR_BASE, get_bkp_reg(ERR_BASE) & ~(1L<<1));
                    }
                  OS_Unuse( &SemaRAM);
                  ASK_TCPANS   =  TCP_ASK;
                  TCP_DATA_NUM =  PAYLOADANS_LEN;
                  break;

  // WI 
  case WI_USB:  if( (bytenum > UDP_DATA_MAXLEN) || ((addr + bytenum) > SD_ALLCONFIG_SIZE) )  goto fail;
  
                OS_Use( &SemaRAM);
                  binbuffer_2_binbuffer( TCP_WDATA_POINTER, (pInt8U)(ConfigBeginAddr + addr), bytenum);
                  if( (addr == CONFIG_ROM_ADR(&ConfigWord)) || (addr == (CONFIG_ROM_ADR(&ConfigWord)) + 3) )
                  {
                    // немедленно перенастраиваем UART-ы
                    USART2->BRR = select_speed( ConfigWord & UART_MASK, APB1);
                          
                    if(ConfigWord & RESTART_STATISTIC)
                    {
                      ConfigWord    &= ~(RESTART_STATISTIC + STATISCTIC_OVERFLOW);
                      WriteStatAddr  = SD_STATISTIC_OFFSET;
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
                  seachSectorsToWrite ( addr, bytenum); 
                  response     = 0;
                OS_Unuse( &SemaRAM);

                ASK_TCPANS   = response ? TCP_CMDERR : TCP_ASK;
                TCP_DATA_NUM = PAYLOADANS_LEN;
                break;
                
  // WF                  
  case WF_USB:  if(bytenum > UDP_DATA_MAXLEN) goto fail;                
                
                response     = write_SDCard( TCP_WDATA_POINTER, addr, bytenum);
                errors_flags.sd_fail |= response;
                ASK_TCPANS   = response ? TCP_CMDERR : TCP_ASK;
                TCP_DATA_NUM = PAYLOADANS_LEN;
                break;
                
  // RR
  case RR_USB:  if( (bytenum > UDP_DATA_MAXLEN) || ((addr + bytenum) > RAMSIZE) )  goto fail;
  
                OS_Use( &SemaRAM);
                  binbuffer_2_binbuffer( (pInt8U)(RamBeginAddr + addr), TCP_RDATA_POINTER, bytenum);  
                OS_Unuse( &SemaRAM);
                
                ASK_TCPANS   = TCP_ASK;                
                TCP_DATA_NUM = PAYLOADANS_LEN + bytenum;
                break;
  // RF
  case RF_USB:  if(bytenum > UDP_DATA_MAXLEN) goto fail;
                
                response     = read_SDCard( TCP_RDATA_POINTER, addr, bytenum); 
                errors_flags.sd_fail |= response;
                ASK_TCPANS   = response ? TCP_CMDERR : TCP_ASK;
                TCP_DATA_NUM = PAYLOADANS_LEN + bytenum;
                break;
  // RI         
  case RI_USB:  if( (bytenum > UDP_DATA_MAXLEN) || ((addr + bytenum) > SD_ALLCONFIG_SIZE) )  goto fail;                
  
                OS_Use( &SemaRAM);
                 memcpy( (void *)TCP_RDATA_POINTER, (void *)(ConfigBeginAddr + addr), bytenum);
                OS_Unuse( &SemaRAM);
                
                ASK_TCPANS   = TCP_ASK;
                TCP_DATA_NUM = PAYLOADANS_LEN + bytenum;
                break;   

  // IR
  case IR_USB:  bytenum = 8; 
                addr = htonl(IP_GetIPAddr(0));               
                strcpy( (void *) &buffer[bytenum], (void *)SerialNumber); bytenum += strlen(SerialNumber) + 1;
                strcpy( (void *) &buffer[bytenum], (void *)Version);      bytenum += strlen(Version) + 1;
                strcpy( (void *) &buffer[bytenum], (void *)DeviceName);   bytenum += strlen(DeviceName) + 1;
                memcpy((void*)   &buffer[bytenum], (void*)&addr, 4);      bytenum += 4;
                *(pU16)TCP_RDATA_POINTER = bytenum - 8;
                ASK_TCPANS   = TCP_ASK;
                TCP_DATA_NUM = PAYLOADANS_LEN + bytenum;
                break;
                
  // CR
  case CR_USB:  bytenum = 8; 
                memcpy( (pU8)&buffer[bytenum], (pU8)&IDWordLo, sizeof(IDWordLo) ); bytenum += sizeof(IDWordLo);
                memcpy( (pU8)&buffer[bytenum], (pU8)&IDWordHi, sizeof(IDWordHi) ); bytenum += sizeof(IDWordHi);
                *(pU16)TCP_RDATA_POINTER = bytenum - 8;
                ASK_TCPANS   = TCP_ASK;
                TCP_DATA_NUM = PAYLOADANS_LEN + bytenum;
                break;                
                                        
  // CT
  case CT_USB: OS_Use( &SemaRAM);
                system_flags.ScreenCalib = 1;
               OS_Unuse( &SemaRAM);
               
               ASK_TCPANS = TCP_ASK;
               TCP_DATA_NUM = PAYLOADANS_LEN;
               break;       
               
  // ѕерерисовать экран
  case RS_USB: OS_Use( &SemaRAM);
                system_flags.redraw_gui = 1;
               OS_Unuse( &SemaRAM);
               
               ASK_TCPANS   = TCP_ASK;
               TCP_DATA_NUM = PAYLOADANS_LEN;
               break;    
             
  //копировать текущую конфигурацию в конф. по умолчанию
  case DW_USB: OS_Use( &SemaRAM);
                system_flags.writedefaultmem= 1;
               OS_Unuse( &SemaRAM);

               ASK_TCPANS   = TCP_ASK;
               TCP_DATA_NUM = PAYLOADANS_LEN;               
               break;
      
  // копировать конф. по умолчанию в текущую конфигурацию                
  case DR_USB: ASK_TCPANS = TCP_ASK;
               TCP_DATA_NUM = PAYLOADANS_LEN;
               
               OS_Use( &SemaRAM);
                system_flags.readdefaultmem= 1;
               OS_Unuse( &SemaRAM);
               break;     
               
  /* сн€ть скриншот */
  case SS_USB: ASK_TCPANS = TCP_ASK;
               TCP_DATA_NUM = PAYLOADANS_LEN;
                  
               OS_Use( &SemaRAM);
                system_flags.ScreenShoot = 1;
               OS_Unuse( &SemaRAM);
               break;  
                  
 case BS_USB:  ASK_TCPANS = TCP_ASK;
               TCP_DATA_NUM = PAYLOADANS_LEN;
                  
               set_mark_forbooter(2); 
               OS_Use( &SemaRAM);
                system_flags.start_boot = 1;
               OS_Unuse( &SemaRAM);
               break;                  
             
    fail:
    default:     ASK_TCPANS   = TCP_NAK;
                 TCP_DATA_NUM = PAYLOADANS_LEN;
  }
  
  return TCP_DATA_NUM;
}

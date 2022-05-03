#include "defines.h"
#include "IP_init.h"
#include "date_time.h"

#define WDATA_POINTER           &buf[6]
#define RDATA_POINTER           &buf[2]
#define ASK_USBANS              buf[0]
#define BYTENUM_USB             buf[1]

#define PUT_CHK_NUM_ASK(A)      { U16 i; ASK_USBANS = ASK_USBEXCH; for (BYTENUM_USB = (A), i = 0, buf[2+(A)] = 0xff; i < (A)+2; i++) buf[(A)+2] += buf[i]; }
#define PUT_CHK_NUM_NAK(A)      { U16 i; ASK_USBANS = NAK_USBEXCH; for (BYTENUM_USB = (A), i = 0, buf[2+(A)] = 0xff; i < (A)+2; i++) buf[(A)+2] += buf[i]; }

static RTC_HandleTypeDef        hrtc;
static RTC_TimeTypeDef          sTime;
static RTC_DateTypeDef          sDate;
static DateTime                 ssDate;

static U8                       sdwrite_buffer[512];
static U8                       SectorsToWrite[SD_ALLCONFIG_SECTORS];

void HAL_Delay(__IO uint32_t Delay)
{
  OS_Delay(Delay);
}

U32 EncodeDate(U8 s, U8 mi, U8 h, U8 d, U8 m, U16 y)
{
  ssDate.seconds = s;
  ssDate.minutes = mi;
  ssDate.hours   = h;
  ssDate.day     = d;
  ssDate.month   = m;
  ssDate.year    = y;
  return convertDateToUnixTime( &ssDate) - EKSIS_TIME_SHIFT;
}

void ParseDate(U32 t, int* s, int* mi, int* h, int* d, int* m, int* y)
{
  convertUnixTimeToDate( t + EKSIS_TIME_SHIFT, &ssDate) ;
  *s  = ssDate.seconds;
  *mi = ssDate.minutes;
  *h  = ssDate.hours;
  *d  = ssDate.day;
  *m  = ssDate.month;
  *y  = ssDate.year;
}

/* байт по SPI1 передать/принять */
#pragma optimize=none
U8  shift_byte_spi1(U8 byte)
{
  SPI1->DR= byte; 
  while  ( !(SPI1->SR & SPI_SR_RXNE) ); 
  return SPI1->DR; 
}

/* байт по SPI3 передать/принять */
#pragma optimize=none
U8  shift_byte_spi3(U8 byte)
{
  SPI3->DR = byte; 
  while  ( !(SPI3->SR & SPI_SR_RXNE) ); 
  return SPI3->DR; 
}

// ################################################# SPI1 -  управление платой реле, тока, коммутатром ###################################################
/*  установить коммутатор */
void  SetComInputLine(U8 adr)
{ 
  OS_Use( &SemaSPI1 );
  shift_byte_spi1(adr);
  LOAD_CMT;
  OS_Unuse( &SemaSPI1 );
}

/*  установить аналоговый выход */
void  SetCurrentDac(U16 Dac1, U8 currentChanel)
{   
  OS_Use(&SemaSPI1);
  if(currentChanel < 8) currentChanel |= 0x10;  // разруливаем между двумя платами
    
  // выкл все каналы
  shift_byte_spi1( 0x18 );
  LOAD_ANG;
  
  // грузим ЦАП
  DAC->DHR12R1 = Dac1; 
  
  // включаем текущий канал
  shift_byte_spi1( currentChanel );
  LOAD_ANG; 
  OS_Unuse(&SemaSPI1);
}

/*  установить реле */
#pragma optimize=none
void  SetRelay(U16 relayword)
{ 
  OS_Use(&SemaSPI1);
  shift_byte_spi1(relayword >> 8);
  shift_byte_spi1(relayword);
  LOAD_RLY_D;  
  OS_Unuse(&SemaSPI1);
}

/* обновление состояния реле и аналогового выхода */
void  set_relay_ang(void)
{
  static U8   currentAnCh;
  
  SetCurrentDac(AnalogArray[currentAnCh], currentAnCh);
  SetRelay(RelayOuts);
  if(++currentAnCh == 16) currentAnCh= 0;  
}
// ################################################# SPI1 -  управление платой реле, тока, коммутатром ###################################################
#pragma optimize=none
void LoadAndCheckConfig(void)
{
  /*  check for BOR Level 3   */
  if( FLASH->OPTCR & (BIT3+BIT2) )
  {
    HAL_FLASH_OB_Unlock();
      *(__IO uint8_t *)OPTCR_BYTE0_ADDRESS &= ~(BIT3+BIT2); 
        HAL_FLASH_OB_Launch();
    HAL_FLASH_OB_Lock();
  }
    
  /* очистка и инициализация */
  memset((void *)ConfigBeginAddr, 0, CONFIGSIZE); 
  memset((void *)RamBeginAddr, 0, RAMSIZE);
  memset((void *)ErrorArray, ASSIGNED_ERR, sizeof(ErrorArray));
  hrtc.Instance = RTC;
    
  // запуск кварца 32768 если не включен
  if( !(RCC->BDCR & RCC_BDCR_LSEON) )
  {    
    SET_BIT(PWR->CR, PWR_CR_DBP);

    SET_BIT(RCC->BDCR, RCC_BDCR_BDRST); OS_Delay(10); CLEAR_BIT(RCC->BDCR, RCC_BDCR_BDRST);   // Resets the entire Backup domain
    
    SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);
    OS_Delay(LSE_STARTUP_TIMEOUT);
    
    if( RCC->BDCR & RCC_BDCR_LSERDY )
    {
      SET_BIT(RCC->BDCR, RCC_BDCR_RTCSEL_0);                                    // LSE
    }
    else
    {
      CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSEON);
      SET_BIT  (RCC->BDCR, RCC_BDCR_RTCSEL_1);                                  // LSI
    }        
    
    SET_BIT(RCC->BDCR, RCC_BDCR_RTCEN);                                         // разрешить  RTC        
    CLEAR_BIT(PWR->CR, PWR_CR_DBP);     
    
    set_bkp_reg( ERR_BASE,     (1<<1)); 
    set_bkp_reg( RESET_COUNTER,     0);
    set_bkp_reg( POWERUP_COUNTER,   0);
    set_bkp_reg( TIMEUP_COUNTER,    0);
    set_bkp_reg( USBUPDATE_COUNTER, 0);
    set_bkp_reg( SDUPDATE_COUNTER,  0);
    set_bkp_reg( LASTUPDATE_RESULT, 0);
    set_bkp_reg( SDFAIL_COUNT,      0);
    set_bkp_reg( SDCFGFAIL_COUNT,   0);    
    set_bkp_reg( STATADR_REG, SD_STATISTIC_OFFSET);
    
    RTC_SetCounter(JAN2021 - EKSIS_TIME_SHIFT);    
  }
  
  errors_flags.LSEfail            = (RCC->BDCR & RCC_BDCR_RTCSEL_1) ? 1 : 0;
  errors_flags.timeinvalid        = get_bkp_reg(ERR_BASE) & (1<<1) ;
  
  ResetCounter     = get_bkp_reg(RESET_COUNTER) + 1; set_bkp_reg(RESET_COUNTER,  ResetCounter);
  TimeUpCounter    = get_bkp_reg(TIMEUP_COUNTER);
  LastUpdateResult = get_bkp_reg(LASTUPDATE_RESULT);
  SDFailCounter    = get_bkp_reg(SDFAIL_COUNT);
  SDCfgFailCounter = get_bkp_reg(SDCFGFAIL_COUNT);
  
  switch( get_bkp_reg(BOOTMARK_BASE) )
  {
  case 0x12345678:      UsbUpdateCounter = get_bkp_reg(USBUPDATE_COUNTER) + 1; set_bkp_reg(USBUPDATE_COUNTER,  UsbUpdateCounter);                        
                        if( !LastUpdateResult ) system_flags.code_updated = 1;
                        break;
                        
  case 0x87654321:      SDUpdateCounter = get_bkp_reg(SDUPDATE_COUNTER) + 1; set_bkp_reg(SDUPDATE_COUNTER,  SDUpdateCounter);
                        if( !LastUpdateResult ) system_flags.code_updated = 1;                        
                        break;  
                        
  default:              ;
  }
  
  set_mark_forbooter(0);
    
  /* проверка наличия SD карты и перевод её в SPI режим */
  errors_flags.sd_not_present = sd_check( ); 
  if(!errors_flags.sd_not_present) errors_flags.sd_fail = sd_initialization( );
  /* проверка памяти конфигурации, загрузка конфигурации, проверка конфигурации */
  if( !errors_flags.sd_not_present && !errors_flags.sd_fail )
  {
    errors_flags.config_memory_fail = read_SDCard((pInt8U)ConfigBeginAddr, SD_CONFIG_ADR, CONFIGSIZE);
  
    if( !errors_flags.config_memory_fail )
    {
      if((ConfigWord & CONFIG_MASK) != (~InvConfigWord & CONFIG_MASK)) 
      {
        errors_flags.sd_fail |= read_SDCard((pInt8U)ConfigBeginAddr, SD_DEFCONFIG_ADR, CONFIGSIZE);
        
        if((ConfigWord & CONFIG_MASK) != (~InvConfigWord & CONFIG_MASK)) 
        { 
          errors_flags.config_fail=  1; 
          errors_flags.load_default= 0; 
        }
        else 
        {          
          for(U32 i = 0; i < CONFIGSIZE / (2 * 0x200); i++, CLEAR_IWDG) errors_flags.config_memory_fail |= write_SDCard( (pInt8U)(ConfigBeginAddr + i * (2 * 0x200)), SD_CONFIG_ADR + i * (2 * 0x200), (2 * 0x200)); // записываем в текущую  конфигурацию
          errors_flags.load_default = 1;
        }
      }
      WriteStatAddr = get_bkp_reg(STATADR_REG);
    }    
  }
  else errors_flags.config_memory_fail = 1;
    
  /* загрузка аварийного минимума */
  if(errors_flags.config_memory_fail || errors_flags.config_fail) 
  {    
    ConfigWord       =  S115200;
    NetAddress       =  1;
    Uref             =  2.5;
    
    memcpy(SerialNumber, "10000000", 9);
    memcpy(DeviceName,   "Блок управления", 16);
    
    TFT_Calib[0][0]     =    -47;       // (0.183 * x - 47);
    TFT_Calib[0][1]     =  0.183;    
    TFT_Calib[1][0]     =    -44;       // (0.193 * y - 44);
    TFT_Calib[1][1]     =  0.193;
    
    TCPIPConfig.UseDHCP =  0;
    TCPIPConfig.IP      = ((U32)192 << 24) + (168L << 16) + (1L << 8)   + 241L;   // 192.168.1.241
    TCPIPConfig.Gate    = ((U32)192 << 24) + (168L << 16) + (1L << 8)   + 1L;     // 192.168.1.1
    TCPIPConfig.Mask    = ((U32)255 << 24) + (255L << 16) + (255L << 8) + 0L;     // 255.255.255.0
    Brightness          = 75;
  }
  else
  {  
    DeviceName[31] = SerialNumber[8] = 0;
  }
  
  for(U8 i = 0; i < MAX_OUTS; i++) ProgramStatus[i].data = OutConfig[i].ProgramStatus;
  
  // hardware конфигурация
  IDWordLo = IDWordHi;
}

void RTC_SetCounter (U32 newtime)
{
  convertUnixTimeToDate(newtime + EKSIS_TIME_SHIFT, &ssDate);
  sTime.Seconds = ssDate.seconds;
  sTime.Minutes = ssDate.minutes;
  sTime.Hours   = ssDate.hours;
  sDate.WeekDay = ssDate.dayOfWeek;
  sDate.Date    = ssDate.day;
  sDate.Month   = ssDate.month;
  sDate.Year    = ssDate.year - 2000;
     
  SET_BIT(PWR->CR, PWR_CR_DBP);    
  HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BIN);  
  HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BIN);      
  CLEAR_BIT(PWR->CR, PWR_CR_DBP);
}

Int32U RTC_GetCounter(void)
{
  SET_BIT(PWR->CR, PWR_CR_DBP);
  HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN);
  CLEAR_BIT(PWR->CR, PWR_CR_DBP);
  
  ssDate.seconds = sTime.Seconds;
  ssDate.minutes = sTime.Minutes;
  ssDate.hours   = sTime.Hours;
  ssDate.dayOfWeek = sDate.WeekDay;
  ssDate.day     = sDate.Date;
  ssDate.month   = sDate.Month;
  ssDate.year    = sDate.Year + 2000;
    
  return convertDateToUnixTime( &ssDate) - EKSIS_TIME_SHIFT;
}

U8 ConfigDeal(void)
{    
    /* операции с кофигурацией по умолчанию - текущую конфу сделать дефолтной */
    if(system_flags.writedefaultmem)
    {     
      OS_Use( &SemaRAM);
        for(U32 i = 0; i < CONFIGSIZE / (2 * 0x200); i++, CLEAR_IWDG) errors_flags.config_memory_fail |= write_SDCard( (pInt8U)(ConfigBeginAddr + i * (2 * 0x200)), SD_DEFCONFIG_ADR + i * (2 * 0x200), (2 * 0x200));
          system_flags.writedefaultmem = 0;
      OS_Unuse( &SemaRAM);
    }
    
    /* загрузить дефолтную конфу */
    if(system_flags.readdefaultmem)
    {
      OS_Use( &SemaRAM);
        errors_flags.config_memory_fail |= read_SDCard( (pInt8U)(ConfigBeginAddr), SD_DEFCONFIG_ADR, CONFIGSIZE);                // грузим в ОЗУ дефолтную конфигурацию
          for(U32 i = 0; i < CONFIGSIZE / (2 * 0x200); i++, CLEAR_IWDG) errors_flags.config_memory_fail |= write_SDCard( (pInt8U)(ConfigBeginAddr + i * (2 * 0x200)), SD_CONFIG_ADR + i * (2 * 0x200), (2 * 0x200)); // записываем в текущую  конфигурацию
              NVIC_SystemReset();                                                                       // перезапуск системы
    }
        
    /* проверить ошибки конфигурации "на ходу" */
    if( errors_flags.config_memory_fail )
    {
      set_bkp_reg( SDCFGFAIL_COUNT, ++SDCfgFailCounter);
      NVIC_SystemReset();         // перезапуск системы
    }
    
    if( errors_flags.sd_fail )
    {
      set_bkp_reg( SDFAIL_COUNT, ++SDFailCounter);
      errors_flags.sd_fail = 0;
    }
    
    if( (ConfigWord & CONFIG_MASK) != (~InvConfigWord & CONFIG_MASK) )     NVIC_SystemReset();         // перезапуск системы
    
    return errors_flags.config_memory_fail == 0;
}

void  SoundDeal(void)
{
    if (ConfigWord & ERROR_BEEP)
    {
      Int8U error = ErrorsPresent();
      
      if (error == ERRORS_PRESENT)
      {
        if (EBeep != ERRORS_PRESENT)
        {
          EBeep = ERRORS_PRESENT;
          set_sound(R_SOUND_1000_2000);
          BeepControl = 1;
        }
      }
      else if (error == ERRORS_NOT_PRESENT)
        EBeep = ERRORS_NOT_PRESENT;
    }
    else EBeep = ERRORS_NOT_PRESENT;

    if ((ConfigWord & TRSH_BEEP) && (EBeep == ERRORS_NOT_PRESENT))
    {
      Int8U violation = ThresholdsViolated();
      
      if (violation & ALARM_VIOLATION)
      {
        if (TBeep != ALARM_VIOLATION)
        {
          TBeep = ALARM_VIOLATION;
          set_sound(R_SOUND_300_600);
          BeepControl = 1;
        }
      }
      else if (violation & ATTENTION_VIOLATION)
      {
        if (TBeep != ATTENTION_VIOLATION)
        {
          TBeep = ATTENTION_VIOLATION;
          set_sound(R_SOUND_500_1000);
          BeepControl = 1;
        }
      }
      else if (violation == NO_VIOLATION)
        TBeep = NO_VIOLATION;
    }
    else TBeep = NO_VIOLATION;
    
    if (BeepControl)
      if (!EBeep && !TBeep)
      {
        set_sound(OFF_SOUND);
        BeepControl = 0;
       }
}


#ifndef LED
static const U8    Key[2] = { BACK_BUTTON, SWITCH_BUTTON };
// кнопки на шильде
void scan_key(void)
{  
 static U8  keyTimer[2];
 
  if( !(GPIOC->IDR & (1<<9)) )
  {
    if(++keyTimer[0] >= 4)
    {
      OS_PutMailCond1( &Key_MB, &Key[0]);      
      keyTimer[0] = 0;
      OS_RetriggerTimer( &TFTTimer);
    }
  }
  else keyTimer[0] = 0;
  
  if( !(GPIOC->IDR & (1<<8)) )
  {
    if(++keyTimer[1] >= 4)
    {
      OS_PutMailCond1( &Key_MB, &Key[1]);
      keyTimer[1] = 0;
      OS_RetriggerTimer( &TFTTimer);
    }
  }
  else keyTimer[1] = 0;  
}

/* сканирование тач-панели */
void scan_touch(void)
{
  static U32    keyPosition  =   0xffffffff;
  static U8     keyTimer, xy = 1, needCalib;
  static S32    xs, ys;
  S32           x, y;
  
  ADC2_CONVERTION;
  if(xy)
  {
    // Y-axis
    if(ADC2->JDR2 < 0xC00) keyPosition  = (keyPosition & 0xffff) + ((U32)ADC2->JDR2 << 16);
    else                   keyPosition |= 0xffff0000;
  }
  else
  {
     // X-axis
    if(ADC2->JDR1 < 0xC00) keyPosition  = (keyPosition & 0xffff0000) + (U32)ADC2->JDR1;
    else                   keyPosition |= 0xffff;
  } 
    
  if( ((keyPosition & 0xffff) != 0xffff) && ((keyPosition >> 16) != 0xffff) )
  {
    #ifdef TFT32
      x = keyPosition & 0xffff;
      y = keyPosition >> 16;
    #endif
        
    #ifdef TFT51
      y = keyPosition & 0xffff;
      x = keyPosition >> 16;
    #endif
        
    #ifdef TFT71
      x = keyPosition & 0xffff;
      y = keyPosition >> 16;        
    #endif
    
    if(!system_flags.touch_calib)
    {
      x= (int)(TFT_Calib[0][1] * x + TFT_Calib[0][0]); 
      if(x < 0)                  x = 0;
      else if(x > RESOLUTION_X)  x = RESOLUTION_X; 
      
      y= (int)(TFT_Calib[1][1] * y + TFT_Calib[1][0]);
      if(y < 0)                  y = 0;
      else if(y > RESOLUTION_Y)  y = RESOLUTION_Y;
    }
      
    if(keyTimer++ == 0) 
    {
      xs = x;
      ys = y;
    }
    else  if(keyTimer != 2)
    {
      if( (abs(xs - x) < 60) && (abs(ys - y) < 60))
      {
        xs = (xs + x)/2;
        ys = (ys + y)/2;
      }
      else keyTimer = 0;
    }
    else
    {
      keyPosition= ((U32)ys << 16) + xs;
      if (ys == 0 && xs == 0) 
      {
        needCalib++;
        if(needCalib > 5)
        {
          needCalib = 0;
          OS_Use( &SemaRAM);
          system_flags.ScreenCalib = 1;
          OS_Unuse( &SemaRAM);
        }
      }
       
      OS_RetriggerTimer( &TFTTimer);
      OS_PutMailCond( &TouchPad_MB, &keyPosition);
      keyPosition    = 0xffffffff;
      keyTimer       =  0;
    }
  }
  else keyTimer = 0;               
    
  /* X <-> Y ось */
  if(xy)  SCAN_PANEL_XA;
  else    SCAN_PANEL_YA;
  
  xy ^= 1;  
}
#endif

/* звуки природы ... */
void set_sound(U8 soundtype)
{
  if (soundtype != OFF_SOUND)
    if ((LastSound & BIT7) && !(soundtype & BIT7))
      return;

  TIM2->CR1 &=  ~BIT0;
  TIM2->ARR  =  SoundArr[soundtype & 0x7f][0];
  TIM2->CCR1 =  SoundArr[soundtype & 0x7f][1];
  TIM2->EGR  =  BIT0;
  TIM2->CCER =  BIT0;                           // OUTput Enable  - для первого раза  
  
  if(soundtype & BIT7)   TIM2->CR1 &= ~BIT3;    // пик-пик-пик-...
  else                   TIM2->CR1 |=  BIT3;    // пик.
  
  TIM2->CR1 |=  BIT0;
  
  LastSound = soundtype;
}

void check_stat_timer()
{
  if(StatTimer >= StatisticPeriod) 
  {
    OS_SignalEvent( 2, &OS_SUPERVISER);  // записать точку статистики немедленно, WriteStatAddr запишется в StatisticManager()
    StatTimer = 0;
  }
}

/* программный таймер на 1 секунду */
void oneSecISR(void)
{
  OS_RetriggerTimer( &OneSecTimer);
  
  OS_SignalEvent( (1<<0) , &OS_SUPERVISER);
  // таймер статистики
  StatTimer++;
  
  if (DownCount) DownCount--;
  
  if (ConfigWord & PUMP_CONTROL_DEVICE)
  {
    if ((ConfigWord & MEASURE_MODE_AUTO) && (CHECK_PUMP))
    {
      check_stat_timer();
    }
  }
  else
  {
    check_stat_timer();
  }
}

void PumpISR(void)
{
  PUMP_OFF;
  OS_SignalEvent ( (1<<1), &OS_SUPERVISER);  // записать точку статистики немедленно, WriteStatAddr запишется в StatisticManager()
  set_sound      (S_SOUND_300);
}

void TFTISR(void)
{
#ifndef LED
  OS_SignalEvent    ( 1, &OS_TFT);  
  OS_RetriggerTimer ( &TFTTimer );
#endif
}

// hex conversions
Int8U  hex_to_char(Int8U pos,pInt8U pBuffer)
{
     Int8U  a,b;
     
     a= pBuffer[pos] - 48;
     if(a>9) a-= 7;
           
     b= pBuffer[pos+1] - 48;
     if(b>9) b-= 7;
           
     return((a<<4) | b);
}

Int16U   hex_to_int(Int8U pos,pInt8U pBuffer)
{
     return ((hex_to_char(pos,pBuffer)<<8) + hex_to_char(pos+2,pBuffer));
}

Int16U   char_to_hex(Int8U cnum)
{
      Int8U  a,b;
      
      a= (cnum & 0x0f);
      b= (cnum & 0xf0)>>4;
      
      if(a>9) a+= 55;
          else a+= 48;
          
      if(b>9) b+= 55;
          else b+= 48;
          
      return(a | (b<<8));
}

// check sum 
Int8U check_sum8(pInt8U pBuffer, Int16U len)
{     
  Int8U crc= 0;
  
  for(Int16U i= 0; i<len; i++) crc+= pBuffer[i];
  return crc;
}

void  hexbuffer_2_binbuffer(pInt8U hexBuffer,pInt8U binBuffer,Int16U nBytes,Int16U Pointer)
{
  for(Int16U i=0;i<nBytes;i++,Pointer+= 2) binBuffer[i]= hex_to_char(Pointer,hexBuffer);
}

void  binbuffer_2_hexbuffer(pInt8U hexBuffer,pInt8U binBuffer,Int16U nBytes,Int16U Pointer)
{
  for(Int16U i=0,codedbyte;i<nBytes;i++) 
  {
    codedbyte=  char_to_hex(binBuffer[i]);
    hexBuffer[Pointer++]= codedbyte>>8;
    hexBuffer[Pointer++]= codedbyte;
  }
}

void  binbuffer_2_binbuffer(pInt8U pBufferSource,pInt8U pBufferDestination,Int16U nBytes)
{
  for(Int16U i=0;i<nBytes;i++) pBufferDestination[i]= pBufferSource[i];
}

Int32U check_sum32(pInt32U pBuffer,Int32U Size)
{
  Int32U    crc,i;
  
  for(i = 0, crc = 0, Size /= 4; i < Size; i++)  crc+= pBuffer[i];
  return crc;
}

Int8U GetDisplayChannelsCount()
{
  Int8U count = 0;
  
  for (int i = 0; i < 16; i++)
    if (DisplayConfig[i].DisplayParam[0] != 255)
      count++;
  else
    break;
  
  return count;
}

Int8U GetDisplayChannelParamsCount(Int8U display_config)
{
  Int8U count = 0;
  
  for (int i = 0; i < 6; i++)
    if (DisplayConfig[display_config].DisplayParam[i] != 255)
      count++;
  else
    break;
    
   return count;
}

Int8U GetDisplayChannelChosenParamsCount(Int8U display_config)
{
  Int8U count = 0;
  
  for (int i = 0; i < 6; i++)
    if (DisplayConfig[display_config].DisplayParam[i] != 255)
    {
      if (DisplayConfig[display_config].ParamConfig[i] & PARAM_ON_MAINSCREEN) count++;
    }
    else
    {
      break;
    }
    
   return count;
}

void GetChosenParams(Int8U display_config, int* first, int* second)
{
  *first = -1;
  *second = -1;
  
  for (int i = 0; i < 6; i++)
    if (DisplayConfig[display_config].DisplayParam[i] != 255)
      if (DisplayConfig[display_config].ParamConfig[i] & PARAM_ON_MAINSCREEN)
        if (*first == -1)
          *first = i;
        else if (*second == -1)
          *second = i;
}

void GetChosenParamsMask(Int8U display_config, int* mask)
{
  *mask = 0;
  
  for (int i = 0; i < 6; i++)
  {
    if (DisplayConfig[display_config].DisplayParam[i] != 255)
    {
      if (DisplayConfig[display_config].ParamConfig[i] & PARAM_ON_MAINSCREEN)
      {
        *mask |= (1 << i);
      }
    }
  }
}

Int8U GetRectParam(Int8U display_config, Int8U rect_index)
{
  for (int i = 0; i < 6; i++)
    if ((DisplayConfig[display_config].ParamConfig[i] & PARAM_PLACEMENT_MASK) == rect_index)
      return i;
  
  return 255;
}

Int8U GetParamRect(Int8U display_config, Int8U param_index)
{
  for (int i = 0; i < 12; i++)
    if ((DisplayConfig[display_config].ParamConfig[param_index] & PARAM_PLACEMENT_MASK) == i)
      return i;
  
  return 255;
}

Int8U GetParametersCount()
{
  Int8U parameters_count = 0;
  
  Int8U channels_count = GetDisplayChannelsCount();
  for (int i = 0; i < channels_count; i++)
    parameters_count += GetDisplayChannelParamsCount(i);

  return parameters_count;
}

void GetMeasureString(Int8U display_channel_index, Int8U dispay_param_index, char* display, Int16U* color)
{
  char value[20];
  char units[20];
  
  Int8U param_index = DisplayConfig[display_channel_index].DisplayParam[dispay_param_index];
  
  if (ErrorArray[param_index])
  {
    sprintf(display, "---"/*"ОШИБКА"*/);
    *color = RED;
  }
  else
  {
//    Int8U pt = MeasureAttribute[param_index].ParamType;
    Int8U ut = MeasureAttribute[param_index].UnitType;
    
    sprintf(value, "%.*f", MeasureAttribute[param_index].DecPoint, MainArray[param_index]);
    sprintf(units, "%s", UNIT_TYPES[ut]);
    sprintf(display, "%s %s", value, units);

    *color = ((DisplayConfig[display_channel_index].TreshConfig[dispay_param_index*2].Status == 1) || (DisplayConfig[display_channel_index].TreshConfig[dispay_param_index*2+1].Status == 1)) ? RED : DEEPBLUE/*BROWN*/;
  }
}

void GetMeasureStringNoUnits(Int8U display_channel_index, Int8U dispay_param_index, char* display, Int16U* color)
{
  Int8U param_index = DisplayConfig[display_channel_index].DisplayParam[dispay_param_index];
  
  if (ErrorArray[param_index])
  {
    sprintf(display, "---"/*"ОШИБКА"*/);
    *color = RED;
  }
  else
  {
    sprintf(display, "%.*f", MeasureAttribute[param_index].DecPoint, MainArray[param_index]);
    *color = ((DisplayConfig[display_channel_index].TreshConfig[dispay_param_index*2].Status == 1) || (DisplayConfig[display_channel_index].TreshConfig[dispay_param_index*2+1].Status == 1)) ? RED : DEEPBLUE;
  }
}

void GetParamMinMax(Int8U display_channel_index, Int8U dispay_param_index, float* min, float* max)
{
  Int8U param_index = DisplayConfig[display_channel_index].DisplayParam[dispay_param_index];
  
  *min = MeasureAttribute[param_index].LowLimit;
  *max = MeasureAttribute[param_index].UpLimit;
}

void GetParamString(Int8U display_channel_index, Int8U dispay_param_index, char* display)
{
  char param_string[25];
  GetParamTypeAndUnits(display_channel_index, dispay_param_index, param_string);
  
  sprintf(display, "[К%d]%s", display_channel_index+1, param_string);
}

void GetParamStringByMeasureAttribute(Int8U measure_attribute, char* display)
{
  sprintf(display, "---"/*"ОШИБКА"*/);
  for (int i = 0; i < GetDisplayChannelsCount(); i++)
  {
    for (int j = 0; j < GetDisplayChannelParamsCount(i); j++)
    {
      if (DisplayConfig[i].DisplayParam[j] == measure_attribute)
      {
        GetParamString(i, j, display);
        return;
      }
    }
  }
}

void GetParamTypeAndUnits(Int8U display_channel_index, Int8U dispay_param_index, char* display)
{
  char param[20];
  char units[20];
  
  Int8U param_index = DisplayConfig[display_channel_index].DisplayParam[dispay_param_index];
  
  Int8U pt = MeasureAttribute[param_index].ParamType;
  Int8U ut = MeasureAttribute[param_index].UnitType;
    
  sprintf(param, "%s", PARAM_TYPES[pt]);
  sprintf(units, "%s", UNIT_TYPES[ut]);
  sprintf(display, "%s,%s", param, units);
}

void GetParamNameAndUnits(Int8U display_channel_index, Int8U dispay_param_index, char* display)
{
  char param[30];
  char units[20];
  
  Int8U param_index = DisplayConfig[display_channel_index].DisplayParam[dispay_param_index];
  
  Int8U pt = MeasureAttribute[param_index].ParamType;
  Int8U ut = MeasureAttribute[param_index].UnitType;
    
  sprintf(param, "%s", PARAM_NAMES[pt]);
  sprintf(units, "%s", UNIT_TYPES[ut]);
  sprintf(display, "%s, %s", param, units);
}

void GetParamName(Int8U display_channel_index, Int8U dispay_param_index, char* display)
{
  Int8U param_index = DisplayConfig[display_channel_index].DisplayParam[dispay_param_index];
  
  Int8U pt = MeasureAttribute[param_index].ParamType;
  sprintf(display, "%s", PARAM_NAMES[pt]);
}

void GetParamType(Int8U display_channel_index, Int8U dispay_param_index, char* display)
{
  Int8U param_index = DisplayConfig[display_channel_index].DisplayParam[dispay_param_index];
  
  Int8U pt = MeasureAttribute[param_index].ParamType;
  sprintf(display, "%s", PARAM_TYPES[pt]);
}

void GetParamUnits(Int8U display_channel_index, Int8U dispay_param_index, char* display)
{
  Int8U param_index = DisplayConfig[display_channel_index].DisplayParam[dispay_param_index];
  
  Int8U ut = MeasureAttribute[param_index].UnitType;
  sprintf(display, "%s", UNIT_TYPES[ut]);
}

void GetParamValueForWS(Int8U display_channel_index, Int8U dispay_param_index, char* display, Int8U* red)
{
  Int8U param_index = DisplayConfig[display_channel_index].DisplayParam[dispay_param_index];
  
  if (ErrorArray[param_index])
  {
    sprintf(display, "---");
    *red = 1;
  }
  else
  {
    sprintf(display, "%.*f", MeasureAttribute[param_index].DecPoint, MainArray[param_index]);
    *red = ((DisplayConfig[display_channel_index].TreshConfig[dispay_param_index*2].Status == 1) || (DisplayConfig[display_channel_index].TreshConfig[dispay_param_index*2+1].Status == 1)) ? 1 : 0;
  }
}

void GetParamUnitsForWS(Int8U display_channel_index, Int8U dispay_param_index, char* display)
{
  Int8U param_index = DisplayConfig[display_channel_index].DisplayParam[dispay_param_index];
  
  Int8U ut = MeasureAttribute[param_index].UnitType;
  sprintf(display, "%s", UNIT_TYPES[ut]);

  strdel_(display, '^');
  strdel_(display, 'v');
}

Int8U GetOutChannelsCount()
{
  Int8U count = 0;
  
  for (int i = 0; i < 16; i++)
    if (OutConfig[i].ChanelType != NO_CONTROL)
      count++;
  else
    break;
  
  return count;
}

Int8U GetControlChannelMeasureAttributeIndex(Int8U input)
{
  Int8U   dispConfNum, paramNumd;
  
  dispConfNum=  input >> 3;
  paramNumd=    input & 7;
  
  return DisplayConfig[dispConfNum].DisplayParam[paramNumd];
}

Int8U GetControlChannelDecimal(Int8U input)
{
  return MeasureAttribute[GetControlChannelMeasureAttributeIndex(input)].DecPoint;
}

Int8U GetControlChannelUnitType(Int8U input)
{
  return MeasureAttribute[GetControlChannelMeasureAttributeIndex(input)].UnitType;
}

void GetControlChannelInputMeasureString(Int8U input, char* display, Int16U* color)
{
  Int8U   dispConfNum, paramNumd;
  
  dispConfNum=  input >> 3;
  paramNumd=    input & 7;
  
  GetMeasureString(dispConfNum, paramNumd, display, color);
}

void GetControlChannelInputMeasureStringNoUnits(Int8U input, char* display, Int16U* color)
{
  Int8U   dispConfNum, paramNumd;
  
  dispConfNum=  input >> 3;
  paramNumd=    input & 7;
  
  GetMeasureStringNoUnits(dispConfNum, paramNumd, display, color);
}

void GetControlChannelInputParamString(Int8U input, char* param)
{
  Int8U   dispConfNum, paramNumd;
  
  dispConfNum=  input >> 3;
  paramNumd=    input & 7;
  
  char param_string[20];
  GetParamTypeAndUnits(dispConfNum, paramNumd, param_string);
  
  sprintf(param, "[К%d]%s", dispConfNum+1, param_string);
}

void GetControlChannelInputParamTypeAndUnits(Int8U input, char* type_and_units)
{
  Int8U   dispConfNum, paramNumd;
  
  dispConfNum=  input >> 3;
  paramNumd=    input & 7;
  
  GetParamTypeAndUnits(dispConfNum, paramNumd, type_and_units);
}

void GetControlChannelInputThreshold1String(Int8U input, char* threshold1)
{
  Int8U   dispConfNum, paramNumd;
  
  dispConfNum=  input >> 3;
  paramNumd=    input & 7;
  
  Int8U index = DisplayConfig[dispConfNum].DisplayParam[paramNumd];
  Int8U decimal = MeasureAttribute[index].DecPoint;
  Flo32 threshold = DisplayConfig[dispConfNum].Treshold[paramNumd*2];
  
  if (DisplayConfig[dispConfNum].TreshConfig[paramNumd*2].Up) sprintf(threshold1, "(В) %.*f", decimal, threshold);
  else                                                        sprintf(threshold1, "(Н) %.*f", decimal, threshold);
}

void GetControlChannelInputThreshold2String(Int8U input, char* threshold2)
{
  Int8U   dispConfNum, paramNumd;
  
  dispConfNum=  input >> 3;
  paramNumd=    input & 7;
  
  Int8U index = DisplayConfig[dispConfNum].DisplayParam[paramNumd];
  Int8U decimal = MeasureAttribute[index].DecPoint;
  Flo32 threshold = DisplayConfig[dispConfNum].Treshold[paramNumd*2+1];
  
  if (DisplayConfig[dispConfNum].TreshConfig[paramNumd*2+1].Up) sprintf(threshold2, "(В) %.*f", decimal, threshold);
  else                                                          sprintf(threshold2, "(Н) %.*f", decimal, threshold);
}

void GetControlChannelInputThresholdsString(Int8U input, char* thresholds)
{
  Int8U   dispConfNum, paramNumd;
  
  dispConfNum=  input >> 3;
  paramNumd=    input & 7;
  
  Int8U index = DisplayConfig[dispConfNum].DisplayParam[paramNumd];
  Int8U decimal = MeasureAttribute[index].DecPoint;
  
  Flo32 threshold1 = DisplayConfig[dispConfNum].Treshold[paramNumd*2];
  Flo32 threshold2 = DisplayConfig[dispConfNum].Treshold[paramNumd*2+1];
  sprintf(thresholds, "%.*f|%.*f", decimal, threshold1, decimal, threshold2);
}

void GetControlChannelInputParamMinMax(Int8U input, float* min, float* max)
{
  Int8U param_index = GetControlChannelMeasureAttributeIndex(input);
  
  *min = MeasureAttribute[param_index].LowLimit;
  *max = MeasureAttribute[param_index].UpLimit;
}

void IncrementDisplayParam(Int8U* display_channel, Int8U* display_param)
{
  Int8U channels_count = GetDisplayChannelsCount();
  Int8U params_count = GetDisplayChannelParamsCount(*display_channel);

  if (channels_count == 0) return;

  do
  {
    if (*display_channel >= channels_count)
    {
      *display_channel = 0;
      *display_param = 0;
      params_count = GetDisplayChannelParamsCount(*display_channel);
      continue;
    }
    
    if (*display_param >= params_count)
    {
      *display_channel = *display_channel+1;
      *display_param = 0;
      params_count = GetDisplayChannelParamsCount(*display_channel);
      continue;
    }
    
    *display_param = *display_param+1;
  } while ((*display_param >= params_count) || (*display_channel >= channels_count));
}

void DecrementDisplayParam(Int8U* display_channel, Int8U* display_param)
{
  Int8U channels_count = GetDisplayChannelsCount();
  Int8U params_count = GetDisplayChannelParamsCount(*display_channel);

  if (channels_count == 0) return;

  do
  {
    if (*display_channel >= channels_count)
    {
      *display_channel = channels_count-1;
      params_count = GetDisplayChannelParamsCount(*display_channel);
      *display_param = params_count-1;
      continue;
    }
    
    if (*display_param >= params_count)
    {
      *display_channel = *display_channel-1;
      params_count = GetDisplayChannelParamsCount(*display_channel);
      *display_param = params_count-1;
      continue;
    }
    
    *display_param = *display_param-1;
  } while ((*display_param >= params_count) || (*display_channel >= channels_count));
}

void IncrementControlChannelInputParam(Int8U* input)
{
  Int8U   dispConfNum, paramNumd;
  Int8U   channels_count, params_count;
  
  dispConfNum=  *input >> 3;
  paramNumd=    *input & 7;
  
  channels_count = GetDisplayChannelsCount();
  params_count = GetDisplayChannelParamsCount(dispConfNum);
  
  if (channels_count == 0) return;

  do
  {
    if (dispConfNum >= channels_count)
    {
      dispConfNum = 0;
      paramNumd = 0;
      params_count = GetDisplayChannelParamsCount(dispConfNum);
      continue;
    }
    
    if (paramNumd >= params_count)
    {
      dispConfNum++;
      paramNumd = 0;
      params_count = GetDisplayChannelParamsCount(dispConfNum);
      continue;
    }
    
    paramNumd++;
  } while ((paramNumd >= params_count) || (dispConfNum >= channels_count));
  
  *input = (dispConfNum << 3) | paramNumd;
}

void IncrementControlChannelHesterParam(Int8U* input, Int8U unit_type)
{
  Int8U   dispConfNum, paramNumd;
  Int8U   channels_count, params_count;
  
  dispConfNum=  *input >> 3;
  paramNumd=    *input & 7;
  
  channels_count = GetDisplayChannelsCount();
  params_count = GetDisplayChannelParamsCount(dispConfNum);
  
  if (channels_count == 0) return;

  do
  {
    do
    {
      if (dispConfNum >= channels_count)
      {
        dispConfNum = 0;
        paramNumd = 0;
        params_count = GetDisplayChannelParamsCount(dispConfNum);
        continue;
      }
      
      if (paramNumd >= params_count)
      {
        dispConfNum++;
        paramNumd = 0;
        params_count = GetDisplayChannelParamsCount(dispConfNum);
        continue;
      }
      
      paramNumd++;
    } while ((paramNumd >= params_count) || (dispConfNum >= channels_count));
    
    *input = (dispConfNum << 3) | paramNumd;
  } while (GetControlChannelUnitType(*input) != unit_type);
}

void IncrementPressureParam(Int8U* measure_attribute)
{
  Int8U channels_count = GetDisplayChannelsCount();
  Int8U params_count;
  Int8U display_channel, display_param;

  GetDisplayConfigAndParam(*measure_attribute, &display_channel, &display_param);  
  do
  {
    params_count = GetDisplayChannelParamsCount(display_channel);
  
    if (display_channel >= channels_count)
    {
      display_channel = 0;
      display_param = 0;
      continue;
    }
    
    if (display_param >= params_count)
    {
      display_channel = display_channel+1;
      display_param = 0;
      continue;
    }
    
    display_param = display_param+1;
  } while ((display_param >= params_count) || (display_channel >= channels_count) || (MeasureAttribute[DisplayConfig[display_channel].DisplayParam[display_param]].ParamType != 3));
  
  *measure_attribute = DisplayConfig[display_channel].DisplayParam[display_param];
}

void ParseIPAddress(Int32U ip, char* display)
{
  sprintf(display, "%u.%u.%u.%u", (ip >> 24), (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
}

void PackIPAddress(Int32U* ip, char* display)
{
  int b1, b2, b3, b4;
  sscanf(display, "%d.%d.%d.%d", &b1, &b2, &b3, &b4);
  
  *ip = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
}

void HTMLColor(Int16U color, char* html_color)
{
  //#define RGB565(r, g, b)   (b >> 3) | ((g >> 2) << 5) | ((r >> 3) << 11)
  Int8U b = color & 0x1F;
  Int8U g = (color >> 5) & 0x3F;
  Int8U r = (color >> 11) & 0x1F;
  
  sprintf(html_color, "#%.2x%.2x%.2x", r, g, b);
}

U16 GetParameterValueForMODBUSHoldReg(U16 parameter_number, Int8U * buffer)
{      
  return 0;
}

Int16U GetParameterValueForMODBUS(U16 parameter_number, Int8U * buffer)
{
  U16  chanel_num       = parameter_number / 32;
  U16  r_num            = parameter_number % 32;
  U8   p_num            = r_num / 2 & 7;
  U16  parameter_index  = DisplayConfig[chanel_num].DisplayParam[p_num];
  U8   value[4];
  U8   error            = ErrorArray[parameter_index];
  
  *(F32*)value = MainArray[parameter_index];
  
  if( r_num < 16 )
  {
    buffer[0] = (r_num & 1) ? value[3] : value[1];
    buffer[1] = (r_num & 1) ? value[2] : value[0];
    return 0;
  }
  else
  {
    buffer[0] = buffer[1] = error;
    return 0;
  }
}

Int16U SetParameterValueForMODBUS(U16 register_adr, U16 preset_value) {return 1;}

Int8U ThresholdsViolated()
{
  Int8U result = NO_VIOLATION;
  
  for (int i = 0; i < GetDisplayChannelsCount(); i++)
  {
    for (int j = 0; j < GetDisplayChannelParamsCount(i); j++)
    {
      if (DisplayConfig[i].TreshConfig[j*2].Status)
      {
        if (DisplayConfig[i].TreshConfig[j*2].Weight)
          result |= ALARM_VIOLATION;
        else
          result |= ATTENTION_VIOLATION;
      }
      if (DisplayConfig[i].TreshConfig[j*2+1].Status)
      {
        if (DisplayConfig[i].TreshConfig[j*2+1].Weight)
          result |= ALARM_VIOLATION;
        else
          result |= ATTENTION_VIOLATION;
      }
    }
  }
  
  return result;
}

Int8U ErrorsPresent()
{
  for (int i = 0; i < GetDisplayChannelsCount(); i++)
  {
    for (int j = 0; j < GetDisplayChannelParamsCount(i); j++)
    {
      if (ErrorArray[DisplayConfig[i].DisplayParam[j]])
      {
        return ERRORS_PRESENT;
      }
    }
  }
  
  return ERRORS_NOT_PRESENT;
}

Int8U HumidityCalculationPossible(Int8U display_config)
{
  for (int i = 0; /*(i < 2) &&*/ (i < GetDisplayChannelParamsCount(display_config)); i++)
  {
    //если присутствует влажность[°Cтр]
    if ((MeasureAttribute[DisplayConfig[display_config].DisplayParam[i]].ParamType == 2) && (MeasureAttribute[DisplayConfig[display_config].DisplayParam[i]].UnitType == 2)) return 1;
  }

  return 0;
}

Int8U OxygenCalculationPossible(Int8U display_config)
{
  for (int i = 0; /*(i < 2) &&*/ (i < GetDisplayChannelParamsCount(display_config)); i++)
  {
    //если присутствует кислород [об. %]
    if ((MeasureAttribute[DisplayConfig[display_config].DisplayParam[i]].ParamType == 4) && (MeasureAttribute[DisplayConfig[display_config].DisplayParam[i]].UnitType == 8)) return 1;
  }

  return 0;
}

Int8U PressureCalculationPossible(Int8U display_config)
{
  //если P
  for (int i = 0; /*(i < 2) &&*/ (i < GetDisplayChannelParamsCount(display_config)); i++)
    if (MeasureAttribute[DisplayConfig[display_config].DisplayParam[i]].ParamType == 3) return 1;
  return 0;
  /*if (MeasureAttribute[DisplayConfig[display_config].DisplayParam[0]].ParamType == 3)
    return 1;
  else
    return 0;*/
}

Int8U FlowCalculationPossible(Int8U display_config)
{
  //если P
  for (int i = 0; (i < GetDisplayChannelParamsCount(display_config)); i++)
    if (MeasureAttribute[DisplayConfig[display_config].DisplayParam[i]].ParamType == 14) return 1;
  return 0;
}

// тут хранятся константы для пересчёта влажности по давлению
Int8U GetDewPointParamIndex(Int8U display_config)
{
  Int8U measure_attribute;
  for (int i = 0; i < GetDisplayChannelParamsCount(display_config); i++)
  {
     measure_attribute = DisplayConfig[display_config].DisplayParam[i];
     if ((MeasureAttribute[measure_attribute].ParamType == 2) && (MeasureAttribute[measure_attribute].UnitType == 2)) return i;
  }
  
  return 255;
}

Int8U GetFlowParamIndex(Int8U display_config)
{
  Int8U measure_attribute;
  for (int i = 0; i < GetDisplayChannelParamsCount(display_config); i++)
  {
     measure_attribute = DisplayConfig[display_config].DisplayParam[i];
     if (MeasureAttribute[measure_attribute].ParamType == 14) return i;
  }
  
  return 255;
}

// тут хранятся константы для пересчёта влажности по давлению
Int8U GetOxygenParamIndex(Int8U display_config)
{
  Int8U measure_attribute;
  for (int i = 0; i < GetDisplayChannelParamsCount(display_config); i++)
  {
     measure_attribute = DisplayConfig[display_config].DisplayParam[i];
     if ((MeasureAttribute[measure_attribute].ParamType == 4) && (MeasureAttribute[measure_attribute].UnitType == 8)) return i;
  }
  
  return 255;
}

void SetPressure1Source(Int8U display_config, bool transducer)
{
  Int8U measure_attribute;
  for (int i = 0; i < GetDisplayChannelParamsCount(display_config); i++)
  {
    measure_attribute = DisplayConfig[display_config].DisplayParam[i];
    if (MeasureAttribute[measure_attribute].ParamType == 2)
    {
      if (transducer)
        DisplayConfig[display_config].ParamConfig[i] |= PARAM_PRESSURE1_SOURCE;
      else
        DisplayConfig[display_config].ParamConfig[i] &= ~PARAM_PRESSURE1_SOURCE;
    }
  }
}

void SetPressure2Source(Int8U display_config, bool transducer)
{
  Int8U measure_attribute;
  for (int i = 0; i < GetDisplayChannelParamsCount(display_config); i++)
  {
    measure_attribute = DisplayConfig[display_config].DisplayParam[i];
    if (MeasureAttribute[measure_attribute].ParamType == 2)
    {
      if (transducer)
        DisplayConfig[display_config].ParamConfig[i] |= PARAM_PRESSURE2_SOURCE;
      else
        DisplayConfig[display_config].ParamConfig[i] &= ~PARAM_PRESSURE2_SOURCE;
    }
  }
}

Int8U PressureTransducerExists()
{
  for (int i = 0; i < 256; i ++)
  {
    //если P
    if (MeasureAttribute[i].ParamType == 3) return 1;
  }
  
  return 0;
}

bool IsChannelExists(int channel_index)
{
  return DisplayConfig[channel_index].DisplayParam[0] != 255;
}

void GetDisplayConfigAndParam(Int8U measure_attribute, Int8U* display_config, Int8U* display_param)
{
  *display_config = 0;
  *display_param = 0;
  
  for (int i = 0; i < GetDisplayChannelsCount(); i++)
  {
    for (int j = 0; j < GetDisplayChannelParamsCount(i); j++)
    {
      if (DisplayConfig[i].DisplayParam[j] == measure_attribute)
      {
        *display_config = i;
        *display_param = j;
        return;
      }
    }
  }
}

Int8U GetMainScreenMaximumParameters()
{
  int channels_count = GetDisplayChannelsCount();
  if (channels_count > 4)
  {
    return 2;
  }
  else if (channels_count > 2)
  {
    return 4;
  }
  else
  {
    return 8;
  }
}

Int8U GetMainScreenParametersCount(int channel_index)
{
  Int8U result = 0;
  
  for (int i = 0; i < 6; i++)
  {
    if (DisplayConfig[channel_index].DisplayParam[i] != 255)
    {
      if (DisplayConfig[channel_index].ParamConfig[i] & PARAM_ON_MAINSCREEN)
      {
        result++;
      }
    }
  }
  return result;
}

Int8U GetActiveBitIndex(Int8U mask, Int8U bit_order)
{
  int cnt = 0;
  for (int i = 0; i < 8; i++)
  {
    if (mask & (1 << i))
    {
      if (cnt == bit_order)
      {
        return i;
      }
      else
      {
        cnt++;
      }
    }
  }
  return 0;
}

/*  обмен Ексис USB */
/* check  SUM */
static bool check_CHKSUM(pU8 buf)
{
  Int8U             bytenum, command, i, check_sum;
  bool              result;
  
  command   = buf[4];
  bytenum   = buf[5];
          
  switch(command)
  {
    case WF_USB:
    case WR_USB:      
    case WI_USB:          
                    for (i = 0, check_sum = 0xff; i < (6 + bytenum); i++) check_sum += buf[i];
                    result = (check_sum != buf[6 + bytenum]) ? 1 : 0;
                    break;
                    
    case RR_USB:
    case RF_USB:      
    case RI_USB:
    case IR_USB:      
    case CR_USB:
    case CT_USB:
    case DW_USB:
    case DR_USB:
    case RS_USB:
    case SS_USB:         
                    for (i = 0, check_sum = 0xff; i < 6; i++) check_sum += buf[i];
                    result = (check_sum != buf[6]) ? 1 : 0;
                    break;
                    
    case BS_USB:    result = 0;
                    break;
      
    default:  result = 1;
  }
  
  return   result;
}

#pragma optimize=none
U32 get_bkp_reg(U8 reg_num)
{  
  if( reg_num > 19 ) return 0;
  return *(pU32)((U32)&RTC->BKP0R + reg_num * sizeof(U32));
}

#pragma optimize=none
void set_bkp_reg(U8 reg_num, U32 data)
{ 
  if( reg_num > 19 ) return;
  
  PWR->CR |=  PWR_CR_DBP;
    *(pU32)((U32)&RTC->BKP0R + reg_num * sizeof(U32)) = data;
  PWR->CR &= ~PWR_CR_DBP;    
}

/* метки для загрузчика */
void  set_mark_forbooter(U8 n)
{
  switch(n)
  {
    case 1: set_bkp_reg(BOOTMARK_BASE, 0x12345678); return;
    case 2: set_bkp_reg(BOOTMARK_BASE, 0x87654321); return;
  }
  set_bkp_reg(BOOTMARK_BASE, 0);
}

void usb_exchange(pU8 buf)
{
  U32 addr      =  *((pU32)buf);
  U8  command   =  buf[4];
  U8  bytenum   =  buf[5];
  U8  response;
          
  if( (bytenum > USB_MAX_DATA_LEN) || check_CHKSUM( buf )) goto fail;
      
  switch(command)
  {
    case WR_USB:  if( ((addr + bytenum) > RAMSIZE) ) goto fail;
                  OS_Use( &SemaRAM);
                    memcpy( (void *)(RamBeginAddr + addr), (void *)WDATA_POINTER, bytenum);
                    if( (RamBeginAddr + addr) == (U32)&Time )  // установить RTC
                    {
                      RTC_SetCounter (Time);  // приходит Эксис-время
                      errors_flags.timeinvalid = 0;
                      set_bkp_reg( ERR_BASE, get_bkp_reg(ERR_BASE) & ~(1L<<1));
                    }
                  OS_Unuse( &SemaRAM);
                  
                  PUT_CHK_NUM_ASK(0); 
                  break;    

    case WI_USB:  if( ((addr + bytenum) > SD_ALLCONFIG_SIZE) ) goto fail;
                  OS_Use( &SemaRAM);
                   OS_SUSPEND_IF_NOT( &OS_MASTER );
#ifndef LED   
                    OS_SUSPEND_IF_NOT( &OS_TFT );
#endif                   
                    memcpy( (void *)(ConfigBeginAddr + addr), (void *)WDATA_POINTER, bytenum);
                    if( (addr == CONFIG_ROM_ADR(&ConfigWord)) || (addr == (CONFIG_ROM_ADR(&ConfigWord)) + 3) )
                    {
                      USART2->BRR = select_speed( ConfigWord & UART_MASK, APB1);
                      
                      if(ConfigWord & RESTART_STATISTIC)
                      {
                        ConfigWord    &= ~(RESTART_STATISTIC + STATISCTIC_OVERFLOW);
                        WriteStatAddr  =   SD_STATISTIC_OFFSET;
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
                  OS_Unuse( &SemaRAM);
                                    
                  PUT_CHK_NUM_ASK(0);
                  break;
                  
    case WF_USB:  if( (bytenum != USB_MAX_DATA_LEN) || (addr & 0x7f) ) goto fail;
                  memcpy( (void *)&sdwrite_buffer[addr & 0x1ff], (void *)WDATA_POINTER, USB_MAX_DATA_LEN);
                  
                  if( (addr & 0x1ff) == 0x180 ) 
                  {
                    response = write_SDCard( sdwrite_buffer, addr & 0xfffffe00, 512);
                    errors_flags.sd_fail |= response;
                  } else response = 0;
                  
                  if( response ) goto fail;                  
                  PUT_CHK_NUM_ASK(0);
                  break;                  

    case RI_USB:  if( (addr + bytenum) > SD_ALLCONFIG_SIZE )  goto fail;
                  OS_Use( &SemaRAM);
                    memcpy( (void *)RDATA_POINTER, (void *)(ConfigBeginAddr + addr), bytenum);
                  OS_Unuse( &SemaRAM);
                  
                  PUT_CHK_NUM_ASK(bytenum);
                  break;  
                  
    case RR_USB:  if( (addr + bytenum) > RAMSIZE )  goto fail;
                  OS_Use( &SemaRAM);
                    memcpy( (void *)RDATA_POINTER, (void *)(RamBeginAddr + addr), bytenum);
                  OS_Unuse( &SemaRAM);
                  
                  PUT_CHK_NUM_ASK(bytenum);
                  break;
    
    case RF_USB:  response = read_SDCard( RDATA_POINTER, addr, bytenum);                  
                  errors_flags.sd_fail |= response;
                  if( response ) goto fail;                  
                  PUT_CHK_NUM_ASK(bytenum);
                  break;                  
    
    case IR_USB:  bytenum = 0;
                  strcpy( (void *) &buf[bytenum+2], (void *)SerialNumber);              bytenum += strlen(SerialNumber) + 1;
                  strcpy( (void *) &buf[bytenum+2], (void *)Version);                   bytenum += strlen(Version) + 1;
                  strcpy( (void *) &buf[bytenum+2], (void *)DeviceName);                bytenum += strlen(DeviceName) + 1;
                  bytenum += sprintf(&buf[bytenum+2], "VID=%x PID=%x", USBD_VID, USBD_PID) + 1;
                  
                  PUT_CHK_NUM_ASK(bytenum);
                  break;
                  
    case CR_USB:  bytenum = 0;
                  memcpy( (pU8)&buf[bytenum+2], (pU8)&IDWordLo, sizeof(IDWordLo) ); bytenum += sizeof(IDWordLo);
                  memcpy( (pU8)&buf[bytenum+2], (pU8)&IDWordHi, sizeof(IDWordHi) ); bytenum += sizeof(IDWordHi);                  

                  PUT_CHK_NUM_ASK(bytenum);
                  break;                  
                       
     case CT_USB: OS_Use( &SemaRAM);
                    system_flags.ScreenCalib = 1;
                  OS_Unuse( &SemaRAM);
                      
                  PUT_CHK_NUM_ASK(0);  
                  break;
                  
     case RS_USB: OS_Use( &SemaRAM);
                    system_flags.redraw_gui = 1;
                  OS_Unuse( &SemaRAM);
                      
                  PUT_CHK_NUM_ASK(0);   
                  break;
       
     /* копировать текущую конфигурацию в конф. по умолчанию */
     case DW_USB: OS_Use( &SemaRAM);
                    system_flags.writedefaultmem = 1;
                  OS_Unuse( &SemaRAM);
                  
                  PUT_CHK_NUM_ASK(0);
                  break;

     /* копировать конф. по умолчанию в текущую конфигурацию */                  
     case DR_USB: OS_Use( &SemaRAM);
                    system_flags.readdefaultmem = 1;
                  OS_Unuse( &SemaRAM);
                  
                  PUT_CHK_NUM_ASK(0);
                  break;  
                  
     /* снять скриншот */
     case SS_USB: OS_Use( &SemaRAM);
                    system_flags.ScreenShoot = 1;
                  OS_Unuse( &SemaRAM);
                  
                  PUT_CHK_NUM_ASK(0);       
                  break;                  
     
     case BS_USB: PUT_CHK_NUM_ASK(0);
                  set_mark_forbooter(1); 
                  OS_Use( &SemaRAM);
                    system_flags.start_boot = 1;
                  OS_Unuse( &SemaRAM);
                  break;
                  
     fail:
     default:     PUT_CHK_NUM_NAK(0);
    }    
}

void seachSectorsToWrite( U32 addr, U32 bytenum)
{
  U16 firstSector = addr >> 9;
  U16 numSectors  = ((addr & 0x1ff) + bytenum) / 0x200;
  
  if( numSectors ) numSectors += ( ( ((addr & 0x1ff) + bytenum) % 0x200) ? 1 : 0 );
  else             numSectors  = 1;
  
  for(U16 i = 0; i < numSectors; i++)  SectorsToWrite[firstSector + i] = 1;
  
  OS_RetriggerTimer   ( &SDConfWrite);
    OS_STARTTIMER_IF_NOT( &SDConfWrite);    
}

U8 saveSDConfDelayed(U16 maxsectwrite)
{
  for(U16 i = 0, k = 0; i < SD_ALLCONFIG_SECTORS; i++)
  {
    if( SectorsToWrite[i] ) 
    {
      if( write_SDCard( (pU8)(ConfigBeginAddr + i * 0x200), SD_CONFIG_ADR + i * 0x200, 0x200) ) return 1;
      SectorsToWrite[i] = 0;
      k++;
    }
    if( k > maxsectwrite ) break;
  }
    
  return 0;
}

U8 ifAnyLeft2Write(void)
{
  for(U16 i = 0, k = 0; i < SD_ALLCONFIG_SECTORS; i++)
    if( SectorsToWrite[i] ) return 1;
  
  return 0;
}

void SDConfISR(void)
{
 OS_STOPTIMER_IF_NOT ( &SDConfWrite); 
}

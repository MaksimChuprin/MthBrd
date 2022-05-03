#include "led.h"
/*
const  U8   OffVersionIVTM[] = "r1.11";
const  U8   OffVersionIVG[]  = "r1.11";
const  U8   OffVersionPKG[]  = "r1.15";
const  U8   OffVersionPKU[]  = "r1.15";
const  U8   OffVersionTGS[]  = "r1.11";
const  U8   OffVersionTTM[]  = "r1.15";
const  U8   OffVersionMAG[]  = "r1.00";
*/

const  char *  const OffVersion[]={"r1.11", "r1.11", "r1.15", "r1.15", "r1.15", "r1.15", "r1.00"};

static Int8U param_ind_and_leds_exist=1;
static Int8U control_leds_exist=1;
static Int8U temp_ind_width=4;
//��� ��� ���� ������������ � 12 �����, ��� ��������� - � 6
static Int8U menu_pos=6;
static Int8U device_type=0;
static Int8U HeatTimer=0;

#define IVTM 0
#define IVG 1
#define PKG_O2 2
#define PKG_CO 3
#define PKU 4
#define TTM 5
#define MAG 6

const struct Device Devices[]={
{"����-7-8",      1, 0, 4},
{"����-7-16",     1, 0, 4},
{"����-7-4",      1, 1, 3},
{"����-7-2",      1, 1, 3},
{"����-7-1-�-2�", 0, 0, 4},
{"����-7-1",      1, 1, 4},//����-7-1-�-4�-2�
{"���-1-1-�-2�",  0, 0, 4},
{"���-1-1",       1, 1, 4},
{"���-1-2",       1, 1, 3},
{"���-1-4",       1, 0, 4},
{"���-1-8",       1, 0, 4},
{"���-1-16",      1, 0, 4},
{"���-4-2",       1, 1, 3},
};

const char* PARAM_NAMES[] = {"����������", "�����������", "���������", "��������", "��������", "�����. �����.", "���. �����.", "�������", "�����", "����� �����", "�������� ������", "����������", "����. ����������", "�����������", "������", "���������� O2", "���-�� ������ (>0.3 ���)", "���-�� ������ (>0.5 ���)", "���-�� ������ (>5.0 ���)", "����� ��������", "������", "�����������", "������� �������", "��������� ��������� ������", "���", "������������������", "������", "����� �����", "����� ����", "������", "�������", "��� �������", "", "���������", "������������� ���������", "���������� ����������", "���������������� ��������", "������� ������"};
const char* PARAM_TYPES[] = {"?", "T", "H", "P", "Ov2", "COv2", "CO", "Hv2", "CHv4", "NO", "V", "U", "� ���.", "P", "F", "Ov2", "D>0.3", "D>0.5", "D>5.0", "W", "NHv3", "Hv2S", "S", "F", "���", "R", "D", "NOv2", "SOv2", "N", "W", "�������", "", "���������", "E�", "pH", "", "B"};
const char* UNIT_TYPES[]  = {"?", "%", "�C��", "ppm", "�/�^3", "��/�^3", "�C", "��.��.��.", "��.%", "���", "�/�", "�", "���/��^2", "%", "�^3/�", "��.", "��.", "��.", "���", "*", "��", "��/�", "��/�^2", "�C��", "�C��", "�C��", "�^3/�", "�C/��", "���", "��", "�", "", "��", "���", "���"};

static int start_param_index=1;

bool is_any_pressure_chnl ();

// ������� i-�� ������ �� �����
void strdel(char *st, int i)
{
  for (int j = i; j < strlen(st) - 1 ; j++) st[j] = st[j+1];
  st[strlen(st)-1] = 0;
}

void strdel_(char *st, char c)
{
  for (int i = 0; i < strlen(st) - 1 ; i++)
    if (st[i] == c)
      strdel(st, i);
}

void init_panel ()
{
  start_param_index=1;
  
  if (strstr ((char const *)DeviceName, "����")==(char const *)DeviceName) device_type=IVTM;
  else if (strstr ((char const *)DeviceName, "���")==(char const *)DeviceName) device_type=IVG;
  else if (strstr ((char const *)DeviceName, "���")==(char const *)DeviceName) 
  {
      if ((strstr ((char const *)DeviceName, "CO")!=0) || (strstr ((char const *)DeviceName, "��")!=0)) device_type=PKG_CO; else device_type=PKG_O2;
      if (!is_any_pressure_chnl ()) start_param_index=0;//!!!!!!!!!!!!!
  }
  else if (strstr ((char const *)DeviceName, "���")==(char const *)DeviceName) 
  {
    HeatTimer=60;
    device_type=PKU;
    if (!is_any_pressure_chnl ()) start_param_index=0;//!!!!!!!!!!!!!
  }  
  else if (strstr ((char const *)DeviceName, "���")==(char const *)DeviceName)
  {
    HeatTimer=10;
    device_type=TTM;
  }
  else if (strstr ((char const *)DeviceName, "���")==(char const *)DeviceName) 
  {
    HeatTimer=60;
    device_type=MAG;
    menu_pos=12;
    start_param_index=0;
    temp_ind_width=4;
  }
  
  //�������� ������� > 6
  if (GetOutChannelsCount()>6)
  {
    param_ind_and_leds_exist=1;
    control_leds_exist=0;
    temp_ind_width=4;
  }
  else
  {
    //���� ������������ �����
    if (GetDisplayChannelsCount()==1)
    {
      param_ind_and_leds_exist=1;
      control_leds_exist=1;
      temp_ind_width=4;
    }
    else
    {
      param_ind_and_leds_exist=1;
      control_leds_exist=1;
      temp_ind_width=3;
    }
  }
  
  //�-2�
  if (strstr ((char const *)DeviceName, "�-2�")!=0)
  {
    param_ind_and_leds_exist=0;
    control_leds_exist=0;
    temp_ind_width=4;
  }
  
  for (int i=0; i<sizeof (Devices)/sizeof (struct Device); i++)
  {
    if (strstr ((char const *)DeviceName, Devices[i].DeviceName)==(char const *)DeviceName)
    {    
      param_ind_and_leds_exist=Devices[i].param_ind_and_leds_exist;
      control_leds_exist=Devices[i].control_leds_exist;
      temp_ind_width=Devices[i].temp_ind_width;
      break;
    }
  }
}

OS_MAILBOX key_mb;
#define MAIL_SIZE 3
#define MAIL_COUNT 5
//#pragma data_alignment = 8
char mail_storage[MAIL_SIZE*MAIL_COUNT];

Int8U char_convert_table[128]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0x7F, 0x39, 0x3F, 0x79, 0x71, 0x00, 0x76, 0x30, 0x0E, 0x00, 0x38, 0x00, 0x37, 0x3F, 0x73, 0x00, 0x31, 0x6D, 0x78, 0x3E, 0x00, 0x00, 0x00, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x77, 0x7C, 0x58, 0x5E, 0x79, 0x71, 0x6F, 0x74, 0x10, 0x00, 0x00, 0x30, 0x00, 0x54, 0x5C, 0x73, 0x00, 0x50, 0x6D, 0x78, 0x1C, 0x00, 0x00, 0x00, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
inline Int8U char_convert(Int8U c)
{
  return char_convert_table[c];                                      
}

void  write_spi_byte(Int8U Byte)
{
  shift_byte_spi1(Byte);
}

void  write_spi_word(Int16U Word)
{
  shift_byte_spi1(Word>>8);
  shift_byte_spi1(Word);
}

static Int32U                            FlashMaskDisplay;

static Int8U                             DisplayBuffer[DISPLAY_LEN+4]="012345678901ABCEd";
OS_TIMER LEDTimer;
OS_TIMER LEDTimer_task;
Int16U KeyCode;

//��������� ����� ���������
static Int8S current_channel_index=0;
//�������� ��� ����
static Int8S current_param_index=1;
//��������� ����� ����������
static Int8S current_out_channel=0;

static int ProgramModeSelect=0;
static int next_programm_count=0;

//���������� �������������� ������� � �����������
OS_RSEMA use_display;
static volatile int led_start=1;//��������� ��� ������ ���������

//����� �� ����� � ������ � �����������
void led_keyboard ()
{
  static Int8U  key_flag=0;
  static Int8U  FlashFlag=0;
  static int timer1s=0;
  char mail[MAIL_SIZE];
  static Int16U     KeyPress, KeySave, KeyTimer, LedFlashClock;
  static Int8U      DisplPosition;
  Int16U            mask;
  
  timer1s++;
  if (timer1s>=1000)
  {
    timer1s=0;
    if (HeatTimer) HeatTimer--; 
    if (OS_Request(&use_display))//��������� � ������� ������
    {   
      mail[0]=0;
      mail[1]=0;
      mail[2]=1;//������ 1 �������
      OS_PutMailCond (&key_mb, &mail);
      OS_Unuse (&use_display);
    }
  }
  mail[2]=0;
  if(LedFlashClock)  LedFlashClock--;
  else            
  {
    LedFlashClock=500;
    FlashFlag^=  1;
  }
  
  // key
  if(IN_KEYB) KeyPress|= (1<<DisplPosition);
  
  if(++DisplPosition == 10)
  {
    DisplPosition= 0;
    
    if (led_start==0)
    
    {
      // if key pressed
    if(KeyPress)
    {
      if(KeyPress == KeySave) 
      {
        if(KeyTimer > CONSTPRESS_KEYTIME) 
        {
          mask= CONSTPRESS_MASK;
        }
        else  if(KeyTimer == LONGPRESS_KEYTIME) 
        {
          KeyTimer++;
          mask= LONGPRESS_MASK;
          }
        else 
        {
          KeyTimer++;
          mask= 0;
        }
        
        if(!key_flag && mask)
        {            
          if(KeySave & 0x03C0)
          {
            KeySave= 0;    //errors_flags.keyb_error= 1;
            KeyTimer= 0;
          }
          else
          {
            key_flag=      1;           
            KeyCode=               KeySave | mask;
            
            mail[0]=KeySave | mask;
            mail[1]=(KeySave | mask)>>8;
            mail[2]=0;//������
            OS_PutMailCond (&key_mb, &mail);
            
            key_flag=0;           
            KeyCode=0;
        
          }
        }
      }
      else 
      {
        KeySave=  KeyPress;
        KeyTimer= 0;
      }      
      // clear KeyPress
      KeyPress= 0;
    }
    // if key up
    else 
    {
      if(KeySave)
      {         
        if(KeyTimer > LONGPRESS_KEYTIME)         mask= 0;//LONGPRESS_MASK;
        else if(KeyTimer > SHORTPRESS_KEYTIME)   mask= SHORTPRESS_MASK;
        else mask=  0;

        if(!key_flag && mask)
        {
          if(KeySave & 0x3C0)
          {
            KeySave=   0;
            KeyTimer=  0;
          }
          else
          {
            key_flag=      1;
            KeyCode=               KeySave | mask;
                    
        
        mail[0]=KeyCode;
        mail[1]=KeyCode>>8;
        mail[2]=0;//������
        OS_PutMailCond (&key_mb, &mail);
                    key_flag=0;           
            KeyCode=0;
        
          }
        }
      }   
      KeySave=  0;
      KeyTimer= 0;
    }
  }  
  }
  
  // write to led-display
  OS_Use(&SemaSPI1);
  CSON_IND;
  write_spi_byte(DisplPosition);
  
  Int8U   s1, s2;
  Int16U  screen;
  
  s1= FlashFlag && (FlashMaskDisplay & (1 << DisplPosition));
  s2= FlashFlag && (FlashMaskDisplay & (1 << (DisplPosition + 10)));
  screen= (s1 ? 0 : DisplayBuffer[DisplPosition]) + (s2 ? 0 : (DisplayBuffer[DisplPosition + 10]<<8));
  
  write_spi_word(screen);
  CSOFF_IND;
  OS_Unuse(&SemaSPI1);
  OS_RetriggerTimer (&LEDTimer);
}

void int_to_str (int i, char *str)
{
  if (i<-999) i=-999;
  else if (i>9999) i=9999;
  sprintf (str, "%4d", i);
}

void float_to_str (float f, int dec_point, char *str)
{
  if (dec_point==0) int_to_str ((int) f, str);
  else 
  {
    if (f>9999.0) f=9999.0;
    else if (f<-999.0) f=999;
    sprintf (str, "%#5.*f", dec_point, f);
  }
}

Int8S value_to_str (int display_channel_index, int dispay_param_index, char *str/*, int *color*/)
{
  Flo32 value;
  Int8U param_index = DisplayConfig[display_channel_index].DisplayParam[dispay_param_index];
  if (dispay_param_index>=GetDisplayChannelParamsCount(display_channel_index)) { sprintf (str, "    "); return 1; }
  if (ErrorArray[param_index]==0)
  {
    value=MainArray[param_index];
    
    //���� ��� � ��� ������ ��� ��������
    if ((MeasureAttribute[DisplayConfig[current_channel_index].DisplayParam[dispay_param_index]].ParamType==10) &&
          (device_type==TTM))
    {
      if (value>9999) sprintf (str, "9999");
      else if (value>999) float_to_str (value, 0, str);
      else if (value<10) float_to_str (value, 2, str);
      else float_to_str (value, 1, str);
    }
    //���� ��� ��� ��� � ��� ������������ ��2
    else if ((MeasureAttribute[DisplayConfig[current_channel_index].DisplayParam[dispay_param_index]].ParamType==5) &&
          ((device_type==PKU)||(device_type==MAG)))
    {
      //��.%
      if (MeasureAttribute[DisplayConfig[current_channel_index].DisplayParam[dispay_param_index]].UnitType==8)
      {
        if (value<1) float_to_str (value, 2, str);
        else float_to_str (value, 1, str);
      }
      //�/�^3
      else// if (MeasureAttribute[DisplayConfig[current_channel_index].DisplayParam[dispay_param_index]].UnitType==4)
      {
        if (value<10) float_to_str (value, 1, str);
        else float_to_str (value, 0, str);
      }        
    }    
    //���� ��� �������� � �� ���, �� ��������� � ��.�.��.
    else if ((MeasureAttribute[DisplayConfig[current_channel_index].DisplayParam[dispay_param_index]].ParamType==3)&&
             (device_type!=IVG))
    {
      value*=760;
      if (temp_ind_width==3)
      {
        if (value<=999) float_to_str (value, 0, str);
        else sprintf (str, "9999");
      }
      else
      {
        if (value<=9999) float_to_str (value, 0, str);
        else sprintf (str, "9999");
      }
    }
    else
    {
      if (value<=9999) float_to_str (value, MeasureAttribute[param_index].DecPoint, str);
      else sprintf (str, "9999");
    }
    return 1;
  }
  else
    /*for (int i=0; i<8; i++)
    if (ErrorArray[param_index] & (1<<i)) 
  { 
    sprintf (str, "E-0%1d", i+1); 
    return 0; 
}*/
  {
    if ((temp_ind_width==4)||(device_type==MAG)) sprintf (str, "E-%02X", ErrorArray[param_index]);
    else sprintf (str, " E%02X", ErrorArray[param_index]);
    return 0;
  }
}

//������� 4 ������� � ��������� �����
void print_parametr (const char *s, int disp_point)
{
  for (int i=0, j=disp_point; j<disp_point+4; i++)
  {
    if (s[i]=='.') DisplayBuffer[j-1]|=0x80;
    else DisplayBuffer[j++]=char_convert(s[i]);
  }
}

Int8U  float_print(Flo32 f_var,Int8U dec_p, int disp_point)
{
  char str[6];
  if((f_var > 9999.4) || (f_var < -999.4)) 
  { 
    sprintf(str, "----"); 
    print_parametr (str, disp_point);
    return 0; 
  }
  
  if(dec_p > 3) dec_p= 3;
  
  switch(dec_p)
  {
    case 3: if((f_var < 9.9994) && (f_var > -0.9994)) break;
            dec_p--;
            
    case 2: if((f_var < 99.994) && (f_var > -9.994))  break;
            dec_p--;
            
    case 1: if((f_var < 999.94) && (f_var > -99.94))  break;
            dec_p--;
  }
  sprintf(str, "%#5.*f",dec_p,f_var);
  print_parametr (str, disp_point);
  return dec_p;
}

void print_ctrl_status (int disp_point)
//��������� "��������" � ������ "������"
{
  char str[6];
  int param_index;
  int dec_point=1;
  //float hest;
  
  if (OutConfig[current_out_channel].DefParamIsIndex)
  {
    int display_channel_index=  ((Int8U)OutConfig[current_out_channel].DefParam) >> 3;
    int dispay_param_index=    ((Int8U)OutConfig[current_out_channel].DefParam) & 7;
    value_to_str (display_channel_index, dispay_param_index, str);
  }
  else
  {
    param_index = GetControlChannelMeasureAttributeIndex(OutConfig[current_out_channel].Input);
    dec_point=MeasureAttribute[param_index].DecPoint;
  }  
  
  if (OutConfig[current_out_channel].ChanelType == RELAY_TYPE)
  {
    switch (OutConfig[current_out_channel].CntrAttr)
    {
    case OFF_CONTROL:if ((RelayOuts >> current_out_channel) & 1) print_parametr ("hAnd", disp_point); else print_parametr (" oFF", disp_point);
    break;
    case LOGIC_CONTROL:print_parametr ("Logc", disp_point);
    break;
    case HEST_CONTROL://hest=OutConfig[current_out_channel].Hesteresis;
      
      if(ProgramStatus[current_out_channel].ControlFail)
            {
              print_parametr("Err ", disp_point);
              break;
            }
      
      
    if (OutConfig[current_out_channel].ProgUse)
    {
      
      if (ProgramModeSelect!=0)
      {
        //��������� � �������� ����� ������, ������� �� ��������� ������� �����
        switch (TempProgStatus)
        {
        case STOP_PROGRAM:  print_parametr("Stop", disp_point);
        break;
        
        case PAUSE_PROGRAM: print_parametr("Paus", disp_point);
        break;
        
        case WORK_PROGRAM:  print_parametr("Cont", disp_point);
                                    break;
                                  
                case START_PROGRAM: print_parametr("Star", disp_point);
                                    break;
        } 
      }
      else
      {
        //��������� ��������
        switch (ProgramStatus[current_out_channel].ProgStatus)
        {
        case STOP_PROGRAM:  print_parametr("Stop", disp_point);
        break;
        
        case PAUSE_PROGRAM: print_parametr("Paus", disp_point);
        break;
        
        case START_PROGRAM:
        case WORK_PROGRAM:  
          /*if (OutConfig[current_out_channel].DefParamIsIndex) print_parametr (str, disp_point);
          else float_print (OutConfig[current_out_channel].DefParam, dec_point, disp_point);*/
          float_print (MainArray[CONTROL_BASE + current_out_channel], dec_point, disp_point);
          break;
        }
      }
    }
    else
    {
      //���������� ��� ��������� ����������. �������� ��� ����� ���������
      if (OutConfig[current_out_channel].DefParamIsIndex) print_parametr (str, 12);
      else float_print (OutConfig[current_out_channel].DefParam, dec_point, 12);
      //float_print (hest, dec_point, disp_point);
    }
    }
  }
  else if (OutConfig[current_out_channel].ChanelType > RELAY_TYPE)
  {
    switch (OutConfig[current_out_channel].CntrAttr)
    {
    case OFF_CONTROL:if (OutCurrents[current_out_channel]>4) print_parametr ("hAnd", disp_point); else print_parametr (" oFF", disp_point);
    break;
    case LOGIC_CONTROL:print_parametr ("Logc", disp_point);
    break;
    case LINE_CONTROL:print_parametr ("LinE", disp_point);
    break;
    }
  }
}

Int8S key(Int8U wait)
{ 
  char mail[MAIL_SIZE];
  
  if (OS_GetMailTimed (&key_mb, mail, 30000)==1) return(NO_KEY);
  int key=mail[1]; key<<=8; key|=mail[0];
  switch(key)
  {
    case KEY_3 + SHORTPRESS_MASK:   
    case KEY_3 + LONGPRESS_MASK:    if(ConfigWord & KEY_BEEP) set_sound(S_SOUND_20);
    case KEY_3 + CONSTPRESS_MASK:   return(1);
    
    case KEY_4 + LONGPRESS_MASK:
    case KEY_4 + SHORTPRESS_MASK:   if(ConfigWord & KEY_BEEP) set_sound(S_SOUND_20);
    case KEY_4 + CONSTPRESS_MASK:   return(-1);
    
    case KEY_5 + SHORTPRESS_MASK:   if(ConfigWord & KEY_BEEP) set_sound(S_SOUND_20); 
    return(ENTR);
    
    case KEY_6 + SHORTPRESS_MASK:   if(ConfigWord & KEY_BEEP) set_sound(S_SOUND_20); 
    return(ABORT);
  }
  return FAIL_KEY;
}

Int8S positionSet(Int16S min, Int16S max, Int16S *pos)
{
  Int8S       k;
  
  k= key(30);
  if((k != NO_KEY) && (k != FAIL_KEY) && (k != ABORT))
  {
    (*pos)+= k;
    if(*pos > max)          *pos= min;
    else if(*pos < min)     *pos= max;
  }
  return k;
}

Int8U show_list (const char * const menu[], int count, Int8S* position, int disp_pos)
{
  Int8S     k;
  Int16S temp_pos;
  
  if ((*position)>=count) position=0;
  
  for(;;)
  {
    print_parametr (menu[*position], disp_pos);
    temp_pos=*position;
    k=positionSet(0, count-1, &temp_pos);
    *position=temp_pos;
    if((k == NO_KEY) || (k == ENTR) || (k == ABORT) || (k==FAIL_KEY)) return k;
  }
}

Int16S set_int(Int16S i, Int16S down_range, Int16S up_range, Int8U dispPoint)
{
  Int8S   k;
  Int16S  save= i;
  char buf[6];

  if ((down_range==0) && (up_range==0)) { down_range=-999; up_range=9999; }
  for(;;)
  {
    sprintf(buf, "%4d",i);
    print_parametr (buf, dispPoint);
    
    k =key(30);
    if((k == NO_KEY) || (k == ENTR))  return i;
    if(k == ABORT)                    return save;
      
    if(k != FAIL_KEY)
    {
      i= i + k;
      if (i < down_range)  i= up_range;
      if (i > up_range)    i= down_range;
    }
  }
}

F32 set_float (F32 f, F32 down_range, F32 up_range, int dec_point)
{
  Int8S   k;
  Flo32   mul, save= f;

  if ((fabs(down_range)<0.1) && (fabs (up_range)<0.1)) { down_range=-999.0; up_range=9999.0; }
  if (up_range>9999) up_range=9999;
  if (down_range<-999) down_range=-999;
  if (f>up_range) f=up_range;
  if (f<down_range) f=down_range;
  
  for(;;)
  {
    switch(float_print(f,dec_point, menu_pos))
    {
      case 0: mul= 1;
              break;
              
      case 1: mul= 0.1;
              break;
              
      case 2: mul= 0.01;
              break;
      
      case 3: mul= 0.001;
              break;
    }
    
    k= key(30);
    if((k == NO_KEY) || (k == ENTR))  return f;
    if(k == ABORT)                    return save;
    
    else if(k != FAIL_KEY)
    {
      f+= k*mul;
      if (f < down_range)  f= up_range;
      if (f > up_range)    f= down_range;
    }
  }
}

const char* const menu_net_spd[]={"1200", "2400", "4800", "9600", "19.20", "38.40", "57.60", "115.2"};
const char* const menu_ip[]={"Adr ", "nEt ", "9Ate", "DHCP"};
const char* const menu_ip_addr[]={"Adr1", "Adr2", "Adr3", "Adr4"};
const char* const menu_ip_net[]={"nEt1", "nEt2", "nEt3", "nEt4"};
const char* const menu_ip_host[]={"9At1", "9At2", "9At3", "9At4"};
const char* const menu_onoff[]={" off", " on "};
const char* const menu_beep[]={"botn", "err ", "trsh"};
const char* const menu_yesno[]={" no ", " yes" };
const char* const menu_trs_lo_up[]={"Lo", "uP"};

const char* const menu_outA_line[]={"0-5 ", "0-20", "4-20"};

const char* const menu_p1_p2[]={"p1", "p2"};
const char* const menu_p[]={"ConS", "parm"};

const char* const menu_p_chnl[]={"type", "out", "lo", "hi"};
const char* const menu_p_type[]={"abS", "eces"};

static Int8U j=0;

//���� ������ �������� ����� ���� �������
Int16S GetParamByMeasureAttribute(Int8U measure_attribute)
{
  for (int i = 0; i < GetDisplayChannelsCount(); i++)
  {
    for (int j = 0; j < GetDisplayChannelParamsCount(i); j++)
    {
      if (DisplayConfig[i].DisplayParam[j] == measure_attribute)
      {
        return i;
      }
    }
  }
  return -1;
}

Int8S first_pressure_chnl ()
{
  for (int foo=0; foo<GetDisplayChannelsCount(); foo++)
  {
    for (int foo1=0; foo1<GetDisplayChannelParamsCount(foo); foo1++)
      if (MeasureAttribute[DisplayConfig[foo].DisplayParam[foo1]].ParamType==3) return foo;
  }
  return 0;
}

//��������� ����� ��������
Int8S inc_pressure_chnl (Int8S chnl)
{
  //�� ������� ������ - ���� ����� ����, �� �������� � ���� ��
  for (int foo=0; foo<GetDisplayChannelsCount()+1; foo++)
  {
    chnl++;
    if (chnl>GetDisplayChannelsCount()) chnl=0;
    //��������� ��� ������������ ���������
    for (int i=0; i<GetDisplayChannelParamsCount(chnl); i++)
      //���� �� ����� ���������� ���������� ��������
      if (MeasureAttribute[DisplayConfig[chnl].DisplayParam[i]].ParamType==3) return chnl;
  }  
  return 0;
}

//���������� ����� ��������
Int8S dec_pressure_chnl (Int8S chnl)
{
  //�� ������� ������ - ���� ����� ����, �� �������� � ���� ��
  for (int foo=0; foo<GetDisplayChannelsCount()+1; foo++)
  {
    chnl--;
    if (chnl<0) chnl=GetDisplayChannelsCount()-1;
    //��������� ��� ������������ ���������
    for (int i=0; i<GetDisplayChannelParamsCount(chnl); i++)
      //���� �� ����� ���������� ���������� ��������
      if (MeasureAttribute[DisplayConfig[chnl].DisplayParam[i]].ParamType==3) return chnl;
  }  
  return 0;
}

//���������� ����� � ��� �� ����� UNIT_TYPES (������� defparam ����� ��� �����������)
//IncrementControlChannelHesterParam ��������� � utilities.c
void DecrementControlChannelHesterParam(Int8U* input, Int8U unit_type)
{
  Int8S   dispConfNum, paramNumd;
  Int8S   channels_count, params_count;
  
  dispConfNum=  *input >> 3;
  paramNumd=    *input & 7;
  
  channels_count = GetDisplayChannelsCount();
  params_count = GetDisplayChannelParamsCount(dispConfNum);
  
  if (channels_count == 0) return;

  do
  {
    paramNumd--;
    if (paramNumd<0)
    {
      dispConfNum--;
      if (dispConfNum<0) dispConfNum=channels_count-1;
      params_count = GetDisplayChannelParamsCount(dispConfNum);
      paramNumd=params_count-1;
    }
    *input = (dispConfNum << 3) | paramNumd;
  } while (GetControlChannelUnitType(*input) != unit_type);
}

//���������, ��� ������� ����� - ��� ����� ��������
bool is_pressure_chnl ()
{
    for (int i=0; i<GetDisplayChannelParamsCount(current_channel_index); i++)
      //���� �� ����� ���������� ���������� ��������
      if (MeasureAttribute[DisplayConfig[current_channel_index].DisplayParam[i]].ParamType==3) return true;
    return false;
}

//���������, ���� �� ���� ���� ����� � �������� ��������
bool is_any_pressure_chnl ()
{
  for (int j=0; j<GetDisplayChannelsCount (); j++)
    for (int i=0; i<GetDisplayChannelParamsCount(current_channel_index); i++)
      //���� �� ����� ���������� ���������� ��������
      if (MeasureAttribute[DisplayConfig[current_channel_index].DisplayParam[i]].ParamType==3) return true;
  return false;    
}

//��������� ������� ��������
void pressure_sensor ()
{
  Int8S chnl=-1;
  Int8S p_chnl=0;
  Int8S p_out, p_type;
  
  if (!is_pressure_chnl ()) return;
  chnl=current_channel_index;
  for (;;)
  {
    j=show_list (menu_p_chnl, sizeof (menu_p_chnl)/sizeof (char*), &p_chnl, 2);
    if((j == ABORT) || (j == NO_KEY)) break;
    switch (p_chnl)
    {
    case 0://��� ������� �������� (��� ��� ���)
      p_type=(Int8U)DisplayConfig[chnl].Constant[0];
      if (p_type>1) p_type=1;
      FlashMaskDisplay=15<<6;
      j=show_list (menu_p_type, sizeof (menu_p_type)/sizeof (char*), &p_type, 6);
      FlashMaskDisplay=0;
      if((j == ABORT) || (j == NO_KEY)) break;
      DisplayConfig[chnl].Constant[0]=p_type;
      OS_Use(&SemaRAM);
      SAVE_CONFIG(DisplayConfig[chnl]);
      OS_Unuse(&SemaRAM);
      break;
    case 1://����� �������, ��
      p_out=(Int8U)DisplayConfig[chnl].Constant[1];
      if (p_out>2) p_out=1;
      FlashMaskDisplay=15<<6;
      j=show_list (menu_outA_line, sizeof (menu_outA_line)/sizeof (char*), &p_out, 6);
      FlashMaskDisplay=0;
      if((j == ABORT) || (j == NO_KEY)) break;          
      DisplayConfig[chnl].Constant[1]=p_out;
      OS_Use(&SemaRAM);
      SAVE_CONFIG(DisplayConfig[chnl]);
      OS_Unuse(&SemaRAM);
      break;
    case 2://���. ��������, ���
      FlashMaskDisplay=15<<6;
      DisplayConfig[chnl].Constant[2]=set_int ((Int16S) DisplayConfig[chnl].Constant[2], 0, 9999, 6);
      FlashMaskDisplay=0;
      OS_Use(&SemaRAM);
      SAVE_CONFIG(DisplayConfig[chnl]);
      OS_Unuse(&SemaRAM);
      break;
    case 3://����. ��������, ���
      FlashMaskDisplay=15<<6;
      DisplayConfig[chnl].Constant[3]=set_int ((Int16S) DisplayConfig[chnl].Constant[3], 0, 9999, 6);
      FlashMaskDisplay=0;
      OS_Use(&SemaRAM);
      SAVE_CONFIG(DisplayConfig[chnl]);
      OS_Unuse(&SemaRAM);
      break;
    }
    print_parametr ("    ", 6);
  }
}

//chnl - PARAM_PRESSURE1_SOURCE ��� PARAM_PRESSURE2_SOURCE
void SetPressureSource(Int8U display_config, bool transducer, int ParamType, int chnl)
{
  Int8U measure_attribute;
  for (int i = 0; i < GetDisplayChannelParamsCount(display_config); i++)
  {
    measure_attribute = DisplayConfig[display_config].DisplayParam[i];
    if (MeasureAttribute[measure_attribute].ParamType == ParamType)
    {
      if (transducer)
        DisplayConfig[display_config].ParamConfig[i] |= chnl;
      else
        DisplayConfig[display_config].ParamConfig[i] &= ~chnl;
    }
  }
}

void IncrementPressureParam1(Int8U* measure_attribute)
{
  Int8U channels_count = GetDisplayChannelsCount();
  Int8U params_count;
  Int8U display_channel, display_param;
  int count=0;
  
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
    count++;
    if (count==GetDisplayChannelsCount()+1) break;
  } while ((display_param >= params_count) || (display_channel >= channels_count) || (MeasureAttribute[DisplayConfig[display_channel].DisplayParam[display_param]].ParamType != 3));
  
  *measure_attribute = DisplayConfig[display_channel].DisplayParam[display_param];
}

//��������� ��������� ������������ � ������ ��������
void select_concentration_sensors ()
{
  Int8S p=0;
  char str[5];
  Int16S key_code;
  Int8U measure_attribute;
  Int16S p_chnl;
  Int8U index =255;
  F32 cons;
  int consts_index[2];
  
  for (int i = 0; i < GetDisplayChannelParamsCount(current_channel_index); i++)
  {
    measure_attribute = DisplayConfig[current_channel_index].DisplayParam[i];
    //��������� � ���
    if (((MeasureAttribute[measure_attribute].ParamType == 4)||(MeasureAttribute[measure_attribute].ParamType == 5)) && 
        (MeasureAttribute[measure_attribute].UnitType == 8))
    {
      index=i;
      break;
    }
  }
  
  //�������, ��� ����� ��������� ��� ��������  
  measure_attribute = DisplayConfig[current_channel_index].DisplayParam[index];
  Int8U calc_config_index = measure_attribute - CALC_BASE;
  consts_index[0] = CalcConfig[calc_config_index].InputConstantParamNum[0];
  consts_index[1] = CalcConfig[calc_config_index].InputConstantParamNum[1];
  
  //���� � ������������ ��� �������� ��������, �� ����� ������ ������ ��������� ��� ���������
  if (!is_any_pressure_chnl()) p=0;
  else 
  {
    //����� ��������� ���� ������ (��������� ��� ��������)
    j=show_list (menu_p, sizeof (menu_p)/sizeof (char*), &p, 2);
    if((j == ABORT) || (j == NO_KEY)) return;
  }

  //������� ���������, � �� ����� ��� ��������
  if ((p==0)&&(DisplayConfig[current_channel_index].ParamConfig[index] & PARAM_PRESSURE1_SOURCE))
  {
    DisplayConfig[consts_index[0]/12].Constant[consts_index[0]%12] = 1;
  }
  //������ ��������, � �� ����� ���� ���������
  else if ((p==1)&&(!(DisplayConfig[current_channel_index].ParamConfig[index] & PARAM_PRESSURE1_SOURCE)))
  {
    measure_attribute = 0;
    IncrementPressureParam1(&measure_attribute);
    DisplayConfig[consts_index[0]/12].Constant[consts_index[0]%12]=measure_attribute;
  }
  
  switch (p)
  {
  case 0://���������
    FlashMaskDisplay=15<<6;  
    //cons=set_float (DisplayConfig[consts_index[0]/12].Constant[consts_index[0]%12], 0, 1000, DisplayConfig[consts_index[0]/12].ConstConfig[consts_index[0]%12].DecPoint);
    cons=DisplayConfig[consts_index[0]/12].Constant[consts_index[0]%12]*760;
    cons=set_float (cons, 0, 9999, 0);      
    cons/=760;
    OS_Use(&SemaRAM);
    FlashMaskDisplay=0;
    print_parametr ("    ", 6);
    DisplayConfig[consts_index[0]/12].Constant[consts_index[0]%12]=cons;
    if (device_type==PKG_O2) SetPressureSource(current_channel_index, false, 4, PARAM_PRESSURE1_SOURCE);
    else if (device_type==PKU) SetPressureSource(current_channel_index, false, 5, PARAM_PRESSURE1_SOURCE);
    SAVE_CONFIG (DisplayConfig[current_channel_index]);
    OS_Unuse(&SemaRAM);
    break;
  case 1://������
    FlashMaskDisplay=15<<6;      
    p_chnl=GetParamByMeasureAttribute((Int8U)DisplayConfig[consts_index[0]/12].Constant[consts_index[0]%12]);
    if (p_chnl==-1)
    {
      p_chnl=0;
      p_chnl=inc_pressure_chnl (p_chnl);
    }
    for (;;)
    {
      sprintf (str, "ch%2d", p_chnl+1);
      print_parametr (str, 6);
      key_code= key(30);
      if((key_code != NO_KEY) && (key_code != FAIL_KEY) && (key_code != ABORT))
      {
        if (key_code==1) p_chnl=inc_pressure_chnl (p_chnl);
        else if (key_code==-1) p_chnl=dec_pressure_chnl (p_chnl);
      }
      if((key_code == NO_KEY) || (key_code == ENTR) || (key_code == ABORT) || (key_code==FAIL_KEY)) {  break; }
    }
    if (key_code!=ENTR) 
    { 
      OS_Use(&SemaRAM);
      LOAD_CONFIG (DisplayConfig[current_channel_index]); 
      OS_Unuse(&SemaRAM);
      FlashMaskDisplay=0;
      print_parametr ("    ", 6);
      return;
    }
    measure_attribute=DisplayConfig[p_chnl].DisplayParam[0]; 
    for (int i=0; i<GetDisplayChannelParamsCount(p_chnl); i++) 
      if (MeasureAttribute[DisplayConfig[p_chnl].DisplayParam[i]].ParamType==3) 
      { 
        measure_attribute=DisplayConfig[p_chnl].DisplayParam[i]; 
        break; 
      }    
    OS_Use(&SemaRAM);
    DisplayConfig[consts_index[0]/12].Constant[consts_index[0]%12]=measure_attribute;
    
    if (device_type==PKG_O2) SetPressureSource(current_channel_index, true, 4, PARAM_PRESSURE1_SOURCE);
    else if (device_type==PKU) SetPressureSource(current_channel_index, true, 5, PARAM_PRESSURE1_SOURCE);
    
    SAVE_CONFIG (DisplayConfig[current_channel_index]);
    OS_Unuse(&SemaRAM);
    FlashMaskDisplay=0;
    print_parametr ("    ", 6);
    break;
  }
}

//��������� ��������� ��������� � ������ ��������
void select_pressure_sensors ()
{
  Int8S p1_p2=0;
  Int8S p=0;
  char str[5];
  Int16S key_code;
  Int8U measure_attribute;
  Int8S p_chnl;
  Int8U index =255;
  F32 cons;
  int consts_index[2];
  
  //��������� ��� ��������� ��������� �� ��������
  for (int i = 0; i < GetDisplayChannelParamsCount(current_channel_index); i++)
  {
     measure_attribute = DisplayConfig[current_channel_index].DisplayParam[i];
     //��������� � ���
     if ((MeasureAttribute[measure_attribute].ParamType == 2) && (MeasureAttribute[measure_attribute].UnitType == 2))
     {
       index=i;
       break;
     }
  }

  //�������, ��� ����� ��������� ��� ��������  
  measure_attribute = DisplayConfig[current_channel_index].DisplayParam[index];
  Int8U calc_config_index = measure_attribute - CALC_BASE;
  consts_index[0] = CalcConfig[calc_config_index].InputConstantParamNum[0];
  consts_index[1] = CalcConfig[calc_config_index].InputConstantParamNum[1];    
  
  for (;;)
  {
    //�������� � ����� ��������� ��� � ����� ���������    
    j=show_list (menu_p1_p2, sizeof (menu_p1_p2)/sizeof (char*), &p1_p2, 2);
    if((j == ABORT) || (j == NO_KEY)) break;
    if (p1_p2==0) 
    {
      if (DisplayConfig[current_channel_index].ParamConfig[index] & PARAM_PRESSURE1_SOURCE) p=1; else p=0;
    }
    else
    {
      if (DisplayConfig[current_channel_index].ParamConfig[index] & PARAM_PRESSURE2_SOURCE) p=1; else p=0;
    }
    
    //���� � ������������ ��� �������� ��������, �� ����� ������ ������ ��������� ��� ���������
    if (!is_any_pressure_chnl()) p=0;
    else 
    {
      //����� ��������� ���� ������ (��������� ��� ��������)
      j=show_list (menu_p, sizeof (menu_p)/sizeof (char*), &p, 2);
      if((j == ABORT) || (j == NO_KEY)) break;
    }
       
    //���� ��������� �������� �� �������� ��� ��������, �� ��������
    if (p1_p2==0)
    {
      //������� �������� P1
      //������� ���������, � �� ����� ��� ��������
      if ((p==0)&&(DisplayConfig[current_channel_index].ParamConfig[index] & PARAM_PRESSURE1_SOURCE))
      {
        DisplayConfig[consts_index[0]/12].Constant[consts_index[0]%12] = 1;
        SetPressure1Source(current_channel_index, false);
      }
      //������ ��������, � �� ����� ���� ���������
      else if ((p==1)&&(!(DisplayConfig[current_channel_index].ParamConfig[index] & PARAM_PRESSURE1_SOURCE)))
      {
        measure_attribute = 0;
        IncrementPressureParam1(&measure_attribute);
        DisplayConfig[consts_index[0]/12].Constant[consts_index[0]%12]=measure_attribute;
        SetPressure1Source(current_channel_index, true);
      }
    }
    else
    {
      //������� �������� P2
      if ((p==0)&&(DisplayConfig[current_channel_index].ParamConfig[index] & PARAM_PRESSURE2_SOURCE))
      {
        DisplayConfig[consts_index[1]/12].Constant[consts_index[1]%12]=1;
        SetPressure2Source(current_channel_index, false);
      }
      else if ((p==1)&&(!(DisplayConfig[current_channel_index].ParamConfig[index] & PARAM_PRESSURE2_SOURCE)))
      {
        measure_attribute = 0;
        IncrementPressureParam1(&measure_attribute);
        DisplayConfig[consts_index[1]/12].Constant[consts_index[1]%12]=measure_attribute;
        SetPressure2Source(current_channel_index, true);
      }
    }
    
    switch (p)
    {
    case 0://���������
      FlashMaskDisplay=15<<6;  
      cons=set_float (DisplayConfig[consts_index[p1_p2]/12].Constant[consts_index[p1_p2]%12], 0, 1000, DisplayConfig[consts_index[p1_p2]/12].ConstConfig[consts_index[p1_p2]%12].DecPoint);      
      /*cons=DisplayConfig[consts_index[p1_p2]/12].Constant[consts_index[p1_p2]%12]*760;
      cons=set_float (cons, 0, 9999, 0);      
      cons/=760;*/
      OS_Use(&SemaRAM);
      DisplayConfig[consts_index[p1_p2]/12].Constant[consts_index[p1_p2]%12]=cons;
      if (p1_p2==0) SetPressure1Source (current_channel_index, false); else SetPressure2Source (current_channel_index, false);
      SAVE_CONFIG (DisplayConfig[current_channel_index]);
      OS_Unuse(&SemaRAM);
      FlashMaskDisplay=0;
      print_parametr ("    ", 6);
      break;
    case 1://������
      FlashMaskDisplay=15<<6;      
      p_chnl=GetParamByMeasureAttribute((Int8U)DisplayConfig[consts_index[p1_p2]/12].Constant[consts_index[p1_p2]%12]);
      for (;;)
      {
        sprintf (str, "ch%2d", p_chnl+1);
        print_parametr (str, 6);
        key_code= key(30);
        if((key_code != NO_KEY) && (key_code != FAIL_KEY) && (key_code != ABORT))
        {
          if (key_code==1) p_chnl=inc_pressure_chnl (p_chnl);
          else if (key_code==-1) p_chnl=dec_pressure_chnl (p_chnl);
        }
        if((key_code == NO_KEY) || (key_code == ENTR) || (key_code == ABORT) || (key_code==FAIL_KEY)) { LOAD_CONFIG (DisplayConfig[current_channel_index]); break; }
      }
      
      measure_attribute=DisplayConfig[p_chnl].DisplayParam[0]; 
      for (int i=0; i<GetDisplayChannelParamsCount(p_chnl); i++) 
        if (MeasureAttribute[DisplayConfig[p_chnl].DisplayParam[i]].ParamType==3) 
        { 
          measure_attribute=DisplayConfig[p_chnl].DisplayParam[i]; 
          break; 
        }    
      OS_Use(&SemaRAM);
      DisplayConfig[consts_index[p1_p2]/12].Constant[consts_index[p1_p2]%12]=measure_attribute;
      if (p1_p2==0) SetPressure1Source (current_channel_index, true); else SetPressure2Source (current_channel_index, true);
      SAVE_CONFIG (DisplayConfig[current_channel_index]);
      OS_Unuse(&SemaRAM);
      FlashMaskDisplay=0;
      print_parametr ("    ", 6);
      break;
    }
  }
}

//���� 4 ����� ip ������
void get_ip_address (const char * const s[], Int32U *ip)
{
  int foo;
  unsigned char *ptr=(unsigned char *)ip;
  ptr+=3;
  for (int i=0; i<4; i++)
  {
    foo=*ptr;
    print_parametr (s[i], 2);
    foo=set_int(foo,0,255,menu_pos);
    *ptr=foo;
    ptr--;
  }
}

//����� RS485
void rs485_address ()
{
  Int16U net_address;
  OS_Use(&SemaRAM);
  net_address=NetAddress;
  OS_Unuse(&SemaRAM);
  
  FlashMaskDisplay=15<<menu_pos;
  net_address=set_int(net_address,1,9999,menu_pos);
  FlashMaskDisplay=0;
  
  OS_Use(&SemaRAM);
  NetAddress=net_address;
  SAVE_CONFIG(ConfigWord);
  SAVE_CONFIG(InvConfigWord);
  SAVE_CONFIG(NetAddress);
  OS_Unuse(&SemaRAM);
}

//�������� RS485
void rs485_speed ()
{
  Int8S speed=0;
  
  speed=ConfigWord & UART_MASK;
  
  FlashMaskDisplay=15<<menu_pos;
  j=show_list (menu_net_spd, sizeof (menu_net_spd)/sizeof (char*), &speed, menu_pos);
  FlashMaskDisplay=0;
  if((j == ABORT) || (j == NO_KEY)) return;
  
  OS_Use(&SemaRAM);
  ConfigWord = ConfigWord & ~(Int32U)UART_MASK;
  ConfigWord |= speed;
  InvConfigWord = ~ConfigWord;    
  SAVE_CONFIG(ConfigWord);
  SAVE_CONFIG(InvConfigWord);
  SAVE_CONFIG(NetAddress);
  USART2_BRR =  select_speed(ConfigWord & UART_MASK, APB1);
  OS_Unuse(&SemaRAM);  
}

//���������/���������� ������
void beeps ()
{
  Int8S beep=0;
  Int8S beep_on_off=0;
  j=show_list (menu_beep, sizeof (menu_beep)/sizeof (char*), &beep, 2);
  if((j == ABORT) || (j == NO_KEY)) return;
  FlashMaskDisplay=15<<menu_pos;
  j=show_list (menu_onoff, sizeof (menu_onoff)/sizeof (char*), &beep_on_off, menu_pos);
  FlashMaskDisplay=0;
  if((j == ABORT) || (j == NO_KEY)) return;
  
  OS_Use(&SemaRAM);
  ConfigWord=    (ConfigWord & ~(1 << (beep + 17))) | ( beep_on_off << (beep + 17));
  InvConfigWord= ~ConfigWord;
  SAVE_CONFIG(ConfigWord);
  SAVE_CONFIG(InvConfigWord);
  SAVE_CONFIG(NetAddress);
  OS_Unuse(&SemaRAM);
}

const U8 mag_leds[28]={ 8, 8, 8, 8, 0, 
                        1, 3, 8, 2, 8,
                        8, 8, 8, 8, 8,
                        8, 8, 8, 8, 8,
                        5, 4, 8, 8, 8,
                        8, 8, 8};

//������
void tresholds ()
{
  Int8S trs=0;
  Int8S trs_lo_up=0;
  Int8S key_code;
  Int8U param_index = 0;
  F32 min=0;
  F32 max=0;
  Int8U dec_point=0;
  char str[6];
  int count_tr=GetDisplayChannelParamsCount(current_channel_index)*2;//���������� ���� ������� ��� ������ ������  
  
  trs=0;  
  for (;;)
  {
    for (;;)
    {
      //���������� ��� ������
      if (device_type!=MAG)
      {
        if (trs>=2*start_param_index) DisplayBuffer[18]=1<<((trs/2)-start_param_index); else DisplayBuffer[18]=0;
      }
      else
      {
        U8 param_type=MeasureAttribute[DisplayConfig[current_channel_index].DisplayParam[trs/2]].ParamType;
        if (param_type<28) DisplayBuffer[18]=1<<(mag_leds[param_type]); else DisplayBuffer[18]=0;
      }
      sprintf (str, "%d.%d%s", (trs/2)+1, (trs&1)+1, (DisplayConfig[current_channel_index].TreshConfig[trs].Up==1)?"uP":"Lo");
      print_parametr (str, 2);
      param_index = DisplayConfig[current_channel_index].DisplayParam[trs>>1];
      float_to_str (DisplayConfig[current_channel_index].Treshold[trs], MeasureAttribute[param_index].DecPoint, str);
      print_parametr (str, menu_pos);
      key_code= key(30);
      if((key_code != NO_KEY) && (key_code != FAIL_KEY) && (key_code != ABORT))
      {
        trs+= key_code;
        if(trs >= count_tr)          trs= 0;
        else if(trs < 0)     trs= count_tr-1;
      }
      if((key_code == NO_KEY) || (key_code == ENTR) || (key_code == ABORT) || (key_code==FAIL_KEY)) break;
    }
    if((key_code == ABORT) || (key_code == NO_KEY)) { DisplayBuffer[18]=0; return; }
    
    //������� ��� ������ �����
    trs_lo_up=DisplayConfig[current_channel_index].TreshConfig[trs].Up;
    FlashMaskDisplay=3<<4;
    for (;;)
    {
      DisplayBuffer[4]=char_convert(menu_trs_lo_up[trs_lo_up][0]);
      DisplayBuffer[5]=char_convert(menu_trs_lo_up[trs_lo_up][1]);
      key_code= key(30);
      if((key_code != NO_KEY) && (key_code != FAIL_KEY) && (key_code != ABORT))
      {
        trs_lo_up+= key_code;
        if(trs_lo_up >= 2)          trs_lo_up= 0;
        else if(trs_lo_up < 0)     trs_lo_up= 1;
      }
      if((key_code == NO_KEY) || (key_code == ENTR) || (key_code == ABORT) || (key_code==FAIL_KEY)) break;
    }
    FlashMaskDisplay=0;
    if((key_code == ABORT) || (key_code == NO_KEY)) { DisplayBuffer[18]=0; return; }
    DisplayConfig[current_channel_index].TreshConfig[trs].Up=trs_lo_up;
    
    //��������� �������� ������
    FlashMaskDisplay=15<<menu_pos;
    param_index = DisplayConfig[current_channel_index].DisplayParam[trs>>1];
    min=MeasureAttribute[param_index].LowLimit;
    max=MeasureAttribute[param_index].UpLimit;
    dec_point=MeasureAttribute[param_index].DecPoint;
    DisplayConfig[current_channel_index].Treshold[trs]=set_float (DisplayConfig[current_channel_index].Treshold[trs], min, max, dec_point);
    DisplayConfig[current_channel_index].TreshConfig[trs].Weight=1;
    FlashMaskDisplay=0;
    
    OS_Use(&SemaRAM);
    SAVE_CONFIG(DisplayConfig[current_channel_index].TreshConfig[trs]);
    SAVE_CONFIG(DisplayConfig[current_channel_index].Treshold[trs]);
    OS_Unuse(&SemaRAM);
  }
}

//����� ��������
void defaults ()
{
  Int8S yes_no=0;
  j=show_list(menu_yesno, sizeof (menu_yesno)/sizeof (char *), &yes_no, menu_pos);
  if((j == ABORT) || (j == NO_KEY) || (yes_no == 0)) return;
  
  OS_Use( &SemaRAM);
  system_flags.readdefaultmem = 1;
  OS_Unuse( &SemaRAM);
}

//������ ������ ����������
void stat_period ()
{
  Int32U stat;
  
  OS_Use(&SemaRAM);
  stat=StatisticPeriod;
  OS_Unuse(&SemaRAM);
  
  FlashMaskDisplay=15<<6;
  stat=set_int(stat, 30, 6000, menu_pos);
  FlashMaskDisplay=0;
  
  OS_Use(&SemaRAM);
  StatisticPeriod=stat;
  SAVE_CONFIG(StatisticPeriod);
  OS_Unuse(&SemaRAM);
}

//��������� ip ������, �����, ����� � DHCP
void net_settings ()
{
  Int8S ip=0;
  Int8S ip_on_off=0;
  for (;;)
  {
    FlashMaskDisplay=15<<menu_pos;
    print_parametr ("    ", menu_pos);
    j=show_list(menu_ip, sizeof (menu_ip)/sizeof (char *), &ip, 2);
    if ((j == ABORT) || (j == NO_KEY)) 
    { 
      FlashMaskDisplay=0;
      OS_Use(&SemaRAM);
      SAVE_CONFIG(TCPIPConfig);
      OS_Unuse(&SemaRAM);
      Initialize_TCPIP_Config(0);
      break; 
    }
    switch (ip)
    {
    case 0://ip addr
      get_ip_address (menu_ip_addr, &TCPIPConfig.IP);
      break;
    case 1://network mask
      get_ip_address (menu_ip_net, &TCPIPConfig.Mask);
      break;
    case 2://gateway
      get_ip_address (menu_ip_host, &TCPIPConfig.Gate);    
      break;
    case 3://dhcp
      ip_on_off=TCPIPConfig.UseDHCP;
      j=show_list(menu_onoff, sizeof (menu_onoff)/sizeof (char *), &ip_on_off, menu_pos);
      if((j == ABORT) || (j == NO_KEY)) { break; }
      TCPIPConfig.UseDHCP=ip_on_off;
      break;
    }
  }
}

//WebServer �������
void webserver_acivity ()
{
  Int8S web=0;
  web=!WebServerConfig.Disabled;
  FlashMaskDisplay=15<<6;
  j=show_list(menu_onoff, sizeof (menu_onoff)/sizeof (char *), &web, menu_pos);
  FlashMaskDisplay=0;
  if((j == ABORT) || (j == NO_KEY)) return;
  OS_Use(&SemaRAM);
  WebServerConfig.Disabled=!web;
  SAVE_CONFIG(WebServerConfig);
  OS_Unuse(&SemaRAM);
}

//������ ������ ���������� WebServer
void webserver_stat ()
{
  Int8S web_stat=0;
  web_stat=WebServerConfig.HideStatisticReset;
  FlashMaskDisplay=15<<6;
  j=show_list(menu_onoff, sizeof (menu_onoff)/sizeof (char *), &web_stat, menu_pos);
  FlashMaskDisplay=0;
  if((j == ABORT) || (j == NO_KEY)) return;
  OS_Use(&SemaRAM);
  WebServerConfig.HideStatisticReset=web_stat;
  SAVE_CONFIG(WebServerConfig);
  OS_Unuse(&SemaRAM);
}

//���� �������� ��� ���
void airduct ()
{
  int d;
  Int8U measure_attribute;
  int consts_index;
  int index;
  
  for (int i = 0; i < GetDisplayChannelParamsCount(current_channel_index); i++)
  {
    measure_attribute = DisplayConfig[current_channel_index].DisplayParam[i];
    //������ �^3/�
    if (((MeasureAttribute[measure_attribute].ParamType == 10)&&(MeasureAttribute[measure_attribute].UnitType == 14)))
    {
      index=i;
      break;
    }
  }
  
  //�������, ��� ����� ��������� ��� ��������� �������� � �����
  measure_attribute = DisplayConfig[current_channel_index].DisplayParam[index];
  consts_index = CalcConfig[measure_attribute - CALC_BASE].InputConstantParamNum[0];
  d=DisplayConfig[consts_index/12].Constant[consts_index%12];
  
  FlashMaskDisplay=15<menu_pos;
  d=set_int (d, 1, 9999, menu_pos);
  FlashMaskDisplay=0;
  
  DisplayConfig[consts_index/12].Constant[consts_index%12]=d;
  
  OS_Use(&SemaRAM);
  SAVE_CONFIG(DisplayConfig[current_channel_index]);
  OS_Unuse(&SemaRAM);  
}

//��� �������� �������� (����, ��� CO � ���)
const char* const menu_main_ivtm[]=    {"Adr ", "rS  ", "beep", "trs ", "defL", "Per ", "iP  ", "ser ", "dis "};
void (*menu_main_func_ivtm[]) ()={rs485_address, rs485_speed, beeps, tresholds, defaults, stat_period, net_settings, webserver_acivity, webserver_stat};

//������������� ��� ������� �������� �� ���� ����� (���)
const char* const menu_main_ivg[]={"Adr ", "rS  ", "beep", "trs ", "defL", "Per ", "cons", "sens", "iP  ", "ser ", "dis "};
void (*menu_main_ivg_func[]) ()={rs485_address, rs485_speed, beeps, tresholds, defaults, stat_period, select_pressure_sensors, pressure_sensor, net_settings, webserver_acivity, webserver_stat};

//������������� ���� ������ �������� �� ���� ����� (��� �2 � ���)
const char* const menu_main_pkg[]={"Adr ", "rS  ", "beep", "trs ", "defL", "Per ", "cons", "sens", "iP  ", "ser ", "dis "};
void (*menu_main_pkg_func[]) ()={rs485_address, rs485_speed, beeps, tresholds, defaults, stat_period, select_concentration_sensors, pressure_sensor, net_settings, webserver_acivity, webserver_stat};

//��� ��� ���� ������� �����������
const char* const menu_main_ttm[]=    {"Adr ", "rS  ", "beep", "trs ", "defL", "Per ", "cons", "iP  ", "ser ", "dis "};
void (*menu_main_func_ttm[]) ()={rs485_address, rs485_speed, beeps, tresholds, defaults, stat_period, airduct, net_settings, webserver_acivity, webserver_stat};

//��������� �������
void ProgManager1 ()
{
  //��������� ������ ����
  Int8S position=0;
  
  OS_Use (&use_display); 
  DisplayBuffer[18]=0;
  DisplayBuffer[10]=0;
  DisplayBuffer[11]=0;
  print_parametr ("    ", 12);
  for (;;)
  {
    DisplayBuffer[16]=0;
    DisplayBuffer[17]=0;
    FlashMaskDisplay=0;
    print_parametr ("    ", menu_pos);
                
    if ((device_type==PKG_O2)||(device_type==PKU))
    {
      if(ConfigWord & TCP_PRESENT) j=show_list (menu_main_pkg, sizeof (menu_main_pkg)/sizeof (char*), &position, 2);
      else j=show_list (menu_main_pkg, sizeof (menu_main_pkg)/sizeof (char*)-3, &position, 2);
      if((j == ABORT) || (j == NO_KEY)) { FlashMaskDisplay=0; print_ctrl_status (12);  OS_Unuse (&use_display); return; }
      if (j==FAIL_KEY) continue;
      menu_main_pkg_func[position]();
    }
    else if (device_type==IVG)
    {
      if(ConfigWord & TCP_PRESENT) j=show_list (menu_main_ivg, sizeof (menu_main_ivg)/sizeof (char*), &position, 2);
      else j=show_list (menu_main_ivg, sizeof (menu_main_ivg)/sizeof (char*)-3, &position, 2);
      if((j == ABORT) || (j == NO_KEY)) { FlashMaskDisplay=0; print_ctrl_status (12);  OS_Unuse (&use_display); return; }
      if (j==FAIL_KEY) continue;
      menu_main_ivg_func[position](); 
    }
    else if ((device_type==IVTM)||(device_type==PKG_CO)||(device_type==MAG))
    {
      if(ConfigWord & TCP_PRESENT) j=show_list (menu_main_ivtm, sizeof (menu_main_ivtm)/sizeof (char*), &position, 2);
      else j=show_list (menu_main_ivtm, sizeof (menu_main_ivtm)/sizeof (char*)-3, &position, 2);        
      if((j == ABORT) || (j == NO_KEY)) { FlashMaskDisplay=0; print_ctrl_status (12);  OS_Unuse (&use_display); return; }
      if (j==FAIL_KEY) continue;
      menu_main_func_ivtm[position]();
    }
    else if (device_type==TTM)
    {
      if(ConfigWord & TCP_PRESENT) j=show_list (menu_main_ttm, sizeof (menu_main_ttm)/sizeof (char*), &position, 2);
      else j=show_list (menu_main_ttm, sizeof (menu_main_ttm)/sizeof (char*)-3, &position, 2);        
      if((j == ABORT) || (j == NO_KEY)) { FlashMaskDisplay=0; print_ctrl_status (12);  OS_Unuse (&use_display); return; }
      if (j==FAIL_KEY) continue;
      menu_main_func_ttm[position]();
    }
  }
}

Int8U GetRelayCount()
{
  Int8U count = 0;
  
  for (int i = 0; i < 16; i++)
    if (OutConfig[i].ChanelType == RELAY_TYPE)
      count++;
    else
      break;
    
  return count;
}

const char* const menu_out_main[]={"c_ch", "tYpe", "Prog"};
const char* const menu_outP_type[]={"off ", "L_ct", "h_ct"};
const char* const menu_outA_type[]={"off ", "L_ct", "Lin "};
const char* const menu_prog[]={"defP", "Puse", "PStr", "PStp", "PEnd", "PSEt"};
const char* const menu_prog_End[]={"stop", "Last", "rest", "defp" };
const char* const menu_defparam[]={"ConS", "par "};
//const char* const menu_logic[]={"Lo", "uP", "Err", "in"};
const char* const menu_logic[]={"Lo", "uP", "Err"};

//��������� ������� �������������
void ProgManager2 ()
{
//��������� ������ ����
  Int8S position=0;
  Int8S position2=0;
  Int8S position3=0;
  Int8S current=0;
  Int8S prog=0;
  Int8S def_param;
  Int8S logic;
  Int8S prog_use=0;
  Int8S end_prog=0;

  Int16U count=GetDisplayChannelParamsCount(current_channel_index);
  Int16U all_count=count*GetDisplayChannelsCount();
  char str[6];
  Int8S key_code;
  
  float min, max;
  Int8U dec_point;
  volatile Int8U param_index;
  int chnl;
  int param;
  
  Int8U input=0;

  OS_Use (&use_display);
  DisplayBuffer[18]=0;
  sprintf (str, "%2d", current_out_channel+1);
  DisplayBuffer[0]=char_convert(str[0]); 
  DisplayBuffer[1]=char_convert(str[1]);
  DisplayBuffer[10]=0; DisplayBuffer[11]=0;
  for (;;)
  {
    DisplayBuffer[16]=0;
    DisplayBuffer[17]=0;
    FlashMaskDisplay=0;
    print_parametr ("    ", menu_pos);
    print_parametr ("    ", 12);
    if (GetRelayCount()>0) j=show_list (menu_out_main, sizeof (menu_out_main)/sizeof (char*), &position, 2);
    else j=show_list (menu_out_main, sizeof (menu_out_main)/sizeof (char*)-1, &position, 2);
    if((j == ABORT) || (j == NO_KEY)) { print_ctrl_status (12); OS_Unuse (&use_display); return; }
    if (j==FAIL_KEY) continue;
    switch (position)
    {
    case 0://����� �������� ���������    
      chnl=OutConfig[current_out_channel].Input>>3;
      param=OutConfig[current_out_channel].Input&7;
      FlashMaskDisplay=15<<menu_pos;
      for (;;)
      {
        if (device_type!=MAG)
        {
          if (param>=start_param_index) DisplayBuffer[18]=1<<(param-start_param_index); else DisplayBuffer[18]=0;
        }
        else
        {
          U8 param_type=MeasureAttribute[DisplayConfig[chnl].DisplayParam[param]].ParamType;
          if (param_type<28) DisplayBuffer[18]=1<<(mag_leds[param_type]); else DisplayBuffer[18]=0;
        }
        
        sprintf (str, "%2d.%2d", chnl+1, param+1);
        print_parametr (str, menu_pos);
      
        key_code= key(30);
        if((key_code != NO_KEY) && (key_code != FAIL_KEY) && (key_code != ABORT))
        {
          if (key_code==1)
          {
            param++;
            if (param>=GetDisplayChannelParamsCount(chnl)) { param=0; chnl++; if (chnl>=GetDisplayChannelsCount()) chnl=0; }
          }
          else if (key_code==-1)
          {
            param--;
            if (param<0) { chnl--; if (chnl<0) chnl=GetDisplayChannelsCount ()-1; param=GetDisplayChannelParamsCount(chnl)-1; }
          }
        }
        if((key_code == NO_KEY) || (key_code == ENTR) || (key_code == ABORT) || (key_code==FAIL_KEY)) break;
      }    
      FlashMaskDisplay=0;
      if((key_code == ABORT) || (key_code == NO_KEY)) break;
      OutConfig[current_out_channel].Input=(param&7)+(chnl<<3);
      OS_Use(&SemaRAM);
      SAVE_CONFIG(OutConfig[current_out_channel]);
      OS_Unuse(&SemaRAM);
      
      //��� �������� ������ ����� ���������
      if ((OutConfig[current_out_channel].ChanelType>= ANALOG_TYPE_05) && (OutConfig[current_out_channel].ChanelType <= ANALOG_TYPE_420))
      {
        switch (OutConfig[current_out_channel].ChanelType)
        {
        case ANALOG_TYPE_05: current=0; break;
        case ANALOG_TYPE_020: current=1; break;      
        case ANALOG_TYPE_420: current=2; break;
        }
        FlashMaskDisplay=15<<menu_pos;
        j=show_list (menu_outA_line, sizeof (menu_outA_line)/sizeof (char*), &current, menu_pos);
        FlashMaskDisplay=0;
        if ((j == ABORT) || (j == NO_KEY)) break;
        switch (current)
        {
        case 0:OutConfig[current_out_channel].ChanelType=ANALOG_TYPE_05; break;
        case 1:OutConfig[current_out_channel].ChanelType=ANALOG_TYPE_020; break;      
        case 2:OutConfig[current_out_channel].ChanelType=ANALOG_TYPE_420; break;
        }
        OS_Use(&SemaRAM);
        SAVE_CONFIG(OutConfig[current_out_channel]);
        OS_Unuse(&SemaRAM);
      }
      //��� ���� ������ ��������
      else if (OutConfig[current_out_channel].ChanelType == RELAY_TYPE)
      {
        print_parametr ("in  ", 2);
        current=OutConfig[current_out_channel].ControlType;
        FlashMaskDisplay=15<<menu_pos;
        j=show_list (menu_onoff, sizeof (menu_onoff)/sizeof (char*), &current, menu_pos);
        FlashMaskDisplay=0;
        if ((j == ABORT) || (j == NO_KEY)) break;
        OutConfig[current_out_channel].ControlType=current;
        OS_Use(&SemaRAM);
        SAVE_CONFIG(OutConfig[current_out_channel]);
        OS_Unuse(&SemaRAM);
      }
      
     break;
    case 1://������ ������
      switch (OutConfig[current_out_channel].CntrAttr)
      {
      case OFF_CONTROL:position2=0; break;
      case LOGIC_CONTROL:position2=1; break;
      case HEST_CONTROL:position2=2; break;
      }
      if (OutConfig[current_out_channel].ChanelType==RELAY_TYPE)//��� ����
      {        
        j=show_list (menu_outP_type, sizeof (menu_outP_type)/sizeof (char*), &position2, 2);
        if ((j == ABORT) || (j == NO_KEY)) break;
        switch (position2)
        {
        case 0://������ �������� ��� ����
          OutConfig[current_out_channel].CntrAttr=OFF_CONTROL;
          OS_Use(&SemaRAM);
          SAVE_CONFIG(OutConfig[current_out_channel]);
          OS_Unuse(&SemaRAM);
          break;
        case 1://���������� ������������ ��� ����
          logic=0;
          FlashMaskDisplay=15<<menu_pos;
          for (;;)
          {
            print_parametr ("    ", menu_pos);
            j=show_list (menu_logic, sizeof (menu_logic)/sizeof (char*), &logic, 2);
            if ((j == ABORT) || (j == NO_KEY)) break;
            position3=0;
            switch (logic)
            {
            case 0: if (OutConfig[current_out_channel].LogicUse&LOGIC_TR1) position3=1; break;
            case 1: if (OutConfig[current_out_channel].LogicUse&LOGIC_TR2) position3=1; break;
            case 2: if (OutConfig[current_out_channel].LogicUse&LOGIC_ERR) position3=1; break;
            case 3: if (OutConfig[current_out_channel].ControlType) position3=1; break;
            }
            
            j=show_list (menu_onoff, sizeof (menu_onoff)/sizeof (char*), &position3, menu_pos);
            if ((j == ABORT) || (j == NO_KEY)) break;
            
            switch (logic)
            {
            case 0: if (position3==1) OutConfig[current_out_channel].LogicUse|=LOGIC_TR1;
            else OutConfig[current_out_channel].LogicUse&=~LOGIC_TR1; break;
            case 1: if (position3==1) OutConfig[current_out_channel].LogicUse|=LOGIC_TR2;
            else OutConfig[current_out_channel].LogicUse&=~LOGIC_TR2; break;
            case 2: if (position3==1) OutConfig[current_out_channel].LogicUse|=LOGIC_ERR;
            else OutConfig[current_out_channel].LogicUse&=~LOGIC_ERR; break;
            case 3: if (position3==1) OutConfig[current_out_channel].ControlType=1;
            else OutConfig[current_out_channel].ControlType=0; break;
            }
            
            OutConfig[current_out_channel].CntrAttr=LOGIC_CONTROL;
            OS_Use(&SemaRAM);
            SAVE_CONFIG(OutConfig[current_out_channel]);
            OS_Unuse(&SemaRAM);
          }
          FlashMaskDisplay=0;          
          break;
        case 2://���������� ��� ����
          OutConfig[current_out_channel].CntrAttr=HEST_CONTROL;
          FlashMaskDisplay=15<<menu_pos;
          OutConfig[current_out_channel].Hesteresis= set_float(OutConfig[current_out_channel].Hesteresis,0,9999,/*MeasureAttribute[OutConfig[current_out_channel].Input].DecPoint*/1);
          FlashMaskDisplay=0;
          OS_Use(&SemaRAM);
          SAVE_CONFIG(OutConfig[current_out_channel]);
          OS_Unuse(&SemaRAM);
          break;
        }
      }
      else//��� �������� ������
      {
        current=0;
        switch (OutConfig[current_out_channel].CntrAttr)
        {
        case OFF_CONTROL: current=0; break;
        case LOGIC_CONTROL: current=1; break;
        case LINE_CONTROL: current=2; break;
        }
        j=show_list (menu_outA_type, sizeof (menu_outA_type)/sizeof (char*), &current, 2);
        if ((j == ABORT) || (j == NO_KEY)) break;
        switch (current)
        {
        case 0://������ �������� ��� �������� ������
          OutConfig[current_out_channel].CntrAttr=OFF_CONTROL;
          OS_Use(&SemaRAM);
          SAVE_CONFIG(OutConfig[current_out_channel]);
          OS_Unuse(&SemaRAM);
          break;
        case 1://���������� ������������ ��� �������� ������
          logic=0;
          FlashMaskDisplay=15<<menu_pos;
          for (;;)
          {
            print_parametr ("    ", menu_pos);
            j=show_list (menu_logic, sizeof (menu_logic)/sizeof (char*), &logic, 2);
            if ((j == ABORT) || (j == NO_KEY)) break;
            position3=0;
            switch (logic)
            {
            case 0: if (OutConfig[current_out_channel].LogicUse&LOGIC_TR1) position3=1; break;
            case 1: if (OutConfig[current_out_channel].LogicUse&LOGIC_TR2) position3=1; break;
            case 2: if (OutConfig[current_out_channel].LogicUse&LOGIC_ERR) position3=1; break;
            case 3: if (OutConfig[current_out_channel].ControlType) position3=1; break;
            }
            
            j=show_list (menu_onoff, sizeof (menu_onoff)/sizeof (char*), &position3, menu_pos);
            if ((j == ABORT) || (j == NO_KEY)) break;
            
            switch (logic)
            {
            case 0: if (position3==1) OutConfig[current_out_channel].LogicUse|=LOGIC_TR1;
            else OutConfig[current_out_channel].LogicUse&=~LOGIC_TR1; break;
            case 1: if (position3==1) OutConfig[current_out_channel].LogicUse|=LOGIC_TR2;
            else OutConfig[current_out_channel].LogicUse&=~LOGIC_TR2; break;
            case 2: if (position3==1) OutConfig[current_out_channel].LogicUse|=LOGIC_ERR;
            else OutConfig[current_out_channel].LogicUse&=~LOGIC_ERR; break;
            case 3: if (position3==1) OutConfig[current_out_channel].ControlType=1;
            else OutConfig[current_out_channel].ControlType=0; break;
            }
            
            OutConfig[current_out_channel].CntrAttr=LOGIC_CONTROL;
            OS_Use(&SemaRAM);
            SAVE_CONFIG(OutConfig[current_out_channel]);
            OS_Unuse(&SemaRAM);
          }
          FlashMaskDisplay=0;   
          break;
        case 2://�������� ����� ��� �������� ������
          OutConfig[current_out_channel].CntrAttr=LINE_CONTROL;
          param_index = GetControlChannelMeasureAttributeIndex(OutConfig[current_out_channel].Input);
          min = MeasureAttribute[param_index].LowLimit;
          max = MeasureAttribute[param_index].UpLimit;
          dec_point=MeasureAttribute[param_index].DecPoint;
          FlashMaskDisplay=15<<menu_pos;
          print_parametr ("Lo P", 2);
          AnalogConfig[current_out_channel][0]=set_float (AnalogConfig[current_out_channel][0], min, max, dec_point);
          print_parametr ("Hi P", 2);        
          AnalogConfig[current_out_channel][1]=set_float (AnalogConfig[current_out_channel][1], min, max, dec_point);
          
          OS_Use(&SemaRAM);
          SAVE_CONFIG(OutConfig[current_out_channel]);
          SAVE_CONFIG(AnalogConfig[current_out_channel]);
          OS_Unuse(&SemaRAM);
          
          FlashMaskDisplay=0;
          break;
        }
      }
      break;
     case 2://��������� ��������� ����������
      prog=0;
      for (;;)
      {
        print_parametr ("    ", menu_pos);
      j=show_list (menu_prog, sizeof (menu_prog)/sizeof (char*), &prog, 2);
      if ((j == ABORT) || (j == NO_KEY)) break;
      switch (prog)
      {
      case 0://���������� �������� ������������� (����� ��� ���������)
        if (OutConfig[current_out_channel].DefParamIsIndex) def_param=1; else def_param=0;
        //���� ����� ��������� ������ ����, �� �������� ������������� - ������ ���������
        if (GetDisplayChannelsCount()==1) def_param=0;
        else 
        { 
          FlashMaskDisplay=15<<menu_pos;
          j=show_list (menu_defparam, sizeof (menu_defparam)/sizeof (char*), &def_param, menu_pos);
          FlashMaskDisplay=0;
        }
        if (def_param)
        {
          //����� ���������          
          FlashMaskDisplay=15<<menu_pos;
          Int8U unit_type = GetControlChannelUnitType(OutConfig[current_out_channel].Input);
          //input=OutConfig[current_out_channel].Input;
          //��������, ��� ����� � DefParam          
          input=OutConfig[current_out_channel].DefParam;
          //���� ���������� ����� ������
          if (((input>>3)<GetDisplayChannelsCount()) &&
              //���� ���������� ����� ���������
              ((input&7)<GetDisplayChannelParamsCount(input>>3)) &&
                //���� ������� ��������� ��������� � ������� ����������
                (MeasureAttribute[GetControlChannelMeasureAttributeIndex(input)].UnitType==
                 MeasureAttribute[GetControlChannelMeasureAttributeIndex(OutConfig[current_out_channel].Input)].UnitType) &&
                  //���� ��� ��������� ��������� � ������� ����������
                  (MeasureAttribute[GetControlChannelMeasureAttributeIndex(input)].ParamType==
                 MeasureAttribute[GetControlChannelMeasureAttributeIndex(OutConfig[current_out_channel].Input)].ParamType))
                    input=OutConfig[current_out_channel].DefParam;
          //���� ��� �� �� ���, ���������� ������� ��������
          else input=OutConfig[current_out_channel].Input;

          for (;;)
          {
            //����� ����� ���������, �� �������� ���� �� ����, ��� � � �����
            //���������� ������� ��������            
            sprintf (str, "% 3d.%d", (input>>3)+1, (input&7)+1);
            print_parametr (str, menu_pos);
            key_code= key(30);
            if((key_code != NO_KEY) && (key_code != FAIL_KEY) && (key_code != ABORT))
            {
              if (key_code==1) IncrementControlChannelHesterParam (&input, unit_type);
              else if (key_code==-1) DecrementControlChannelHesterParam (&input, unit_type);
            }
            if((key_code == NO_KEY) || (key_code == ENTR) || (key_code == ABORT) || (key_code==FAIL_KEY)) break;
          }    
          FlashMaskDisplay=0;
          if((key_code == ABORT) || (key_code == NO_KEY)) break;
          OutConfig[current_out_channel].DefParamIsIndex=1;
          OutConfig[current_out_channel].DefParam=input;//(position3<<3)+(OutConfig[current_out_channel].Input&7);
          OS_Use(&SemaRAM);
          SAVE_CONFIG(OutConfig[current_out_channel]);
          OS_Unuse(&SemaRAM);
        }
        else
        {
          //���������
          param_index=DisplayConfig[OutConfig[current_out_channel].Input>>3].DisplayParam[OutConfig[current_out_channel].Input&7];
//          param_index = GetControlChannelMeasureAttributeIndex(OutConfig[current_out_channel].Input);
          min = MeasureAttribute[param_index].LowLimit;
          max = MeasureAttribute[param_index].UpLimit;
          dec_point=MeasureAttribute[param_index].DecPoint;
          FlashMaskDisplay=15<<menu_pos;
          OutConfig[current_out_channel].DefParam=set_float(OutConfig[current_out_channel].DefParam, min, max, dec_point);
          FlashMaskDisplay=0;
          OutConfig[current_out_channel].DefParamIsIndex=0;
          OS_Use(&SemaRAM);
          SAVE_CONFIG(OutConfig[current_out_channel]);
          OS_Unuse(&SemaRAM);
        }
        break;
      case 1:
        //��������� ������������? 
        prog_use= OutConfig[current_out_channel].ProgUse;
        j=show_list (menu_onoff, sizeof (menu_onoff)/sizeof (char*), &prog_use, menu_pos);
        if ((j == ABORT) || (j == NO_KEY)) break;
        OutConfig[current_out_channel].ProgUse=prog_use;
        OS_Use(&SemaRAM);
        SAVE_CONFIG(OutConfig[current_out_channel]);
        OS_Unuse(&SemaRAM);
        break;
      case 2:
        //��������� ���
        FlashMaskDisplay=15<<menu_pos;
        OutConfig[current_out_channel].StartStep=set_int(OutConfig[current_out_channel].StartStep + 1,1,512,menu_pos)-1;
        OS_Use(&SemaRAM);
        SAVE_CONFIG(OutConfig[current_out_channel]);
        OS_Unuse(&SemaRAM);
        FlashMaskDisplay=0;
        break;
      case 3:
        //�������� ���
        FlashMaskDisplay=15<<menu_pos;
        OutConfig[current_out_channel].StopStep=set_int(OutConfig[current_out_channel].StopStep + 1,OutConfig[current_out_channel].StartStep + 1,512,menu_pos)-1;
        OS_Use(&SemaRAM);
        SAVE_CONFIG(OutConfig[current_out_channel]);
        OS_Unuse(&SemaRAM);
        FlashMaskDisplay=0;
        break;
      case 4:
        //��� ������ �� ���������� ���������
        FlashMaskDisplay=15<<menu_pos;
        end_prog=OutConfig[current_out_channel].EndProgCond;
        j=show_list (menu_prog_End, sizeof (menu_prog_End)/sizeof (char*), &end_prog, menu_pos);
        if ((j == ABORT) || (j == NO_KEY)) { FlashMaskDisplay=0; break; }
        OutConfig[current_out_channel].EndProgCond=end_prog;
        OS_Use(&SemaRAM);
        SAVE_CONFIG(OutConfig[current_out_channel]);
        OS_Unuse(&SemaRAM);
        FlashMaskDisplay=0;
        break;
      case 5:
        //�������������� ���������
        FlashMaskDisplay=15<<menu_pos;
        for(Int16S p= OutConfig[current_out_channel].StartStep;;)
        {
          print_parametr ("Step", 2);
          sprintf (str, "%4d",p+1);
          print_parametr (str, menu_pos);
          key_code= positionSet(OutConfig[current_out_channel].StartStep,OutConfig[current_out_channel].StopStep,&p);
          if((key_code == NO_KEY) || (key_code == ABORT)) break;
          if(key_code == ENTR)
          {
            
            param_index = GetControlChannelMeasureAttributeIndex(OutConfig[current_out_channel].Input);
            min = MeasureAttribute[param_index].LowLimit;
            max = MeasureAttribute[param_index].UpLimit;
            dec_point=MeasureAttribute[param_index].DecPoint;
            
            print_parametr("par ", 2);
            Program[p].Param= set_float(Program[p].Param, min, max, dec_point);
            
            print_parametr("SetL", 2);
            Program[p].SetlTime= set_int(Program[p].SetlTime,0,9999,menu_pos);
            
            print_parametr("HoLd", 2);
            Program[p].HoldTime= set_int(Program[p].HoldTime,0,9999,menu_pos);
            
    /*        OS_Use(&SemaRAM);
            SAVE_CONFIG(Program[p]);
            OS_Unuse(&SemaRAM);*/
          }
        }
        FlashMaskDisplay=0;
        
        OS_Use(&SemaRAM);
        SAVE_CONFIG(OutConfig[current_out_channel]);
        write_spimem(CONFIG_ROM_ADR(Program), (pInt8U)Program, 512*sizeof(Program_t));
        OS_Unuse(&SemaRAM);
                
        break;
      }
      }
      break;
    }
  }
}

//����������� ��������� ��� � �������
void led_timer_task ()
{
  char temp[6], humid[6], channel[3], out[3];
  
  if (HeatTimer)
  {
    print_parametr ("HEAT", 2);
    sprintf (temp, "%4d", HeatTimer);
    print_parametr (temp, menu_pos);
    return;
  }
  sprintf (channel, "%2d", current_channel_index+1);
  sprintf (out, "%2d", current_out_channel+1);
  
  U8 type=MeasureAttribute[DisplayConfig[current_channel_index].DisplayParam[0]].ParamType;
  U8 param_type=MeasureAttribute[DisplayConfig[current_channel_index].DisplayParam[current_param_index]].ParamType;
  //���� ��� ��� ���, �� � ������ ������ ��� ������� ��������
  if (((device_type==PKG_O2)||(device_type==PKU)||(device_type==PKG_CO))&&(type!=3))
  {
    sprintf (temp, "    ");
  }
  //���� ��� �����������
  else if (type==0)
  {
    sprintf (temp, "    ");
  }
  else
  {
    if (value_to_str (current_channel_index, 0, temp)) FlashMaskDisplay&=~(15<<2); 
    else
    {
      if ((temp_ind_width==4)||(device_type==MAG)) FlashMaskDisplay|=15<<2;//����������� ��� �������� � ������ ����������
      else FlashMaskDisplay|=7<<3;
    }
  }
  if (value_to_str (current_channel_index, current_param_index, humid)) FlashMaskDisplay&=~(15<<6); else FlashMaskDisplay|=15<<6;//��������� ��� ������������ �� ������ ����������

  if (current_param_index>=GetDisplayChannelParamsCount(current_channel_index)) DisplayBuffer[18]=0;
  else if (GetDisplayChannelParamsCount(current_channel_index)==1) DisplayBuffer[18]=0;
  else 
  {
    if (device_type==MAG)
    {
      if (param_type<28) DisplayBuffer[18]=1<<(mag_leds[param_type]); else DisplayBuffer[18]=0;
    }
    else DisplayBuffer[18]=1<<(current_param_index-start_param_index);//������� ��������� ��� ������������
  }

  for (int i=0; i<2; i++)//������� ����� ���������
  {
    DisplayBuffer[i]=char_convert(channel[i]);
    DisplayBuffer[i+10]=char_convert(out[i]);
  }
  
  if (device_type!=MAG) 
  {
    print_parametr (temp, 2);
    print_parametr (humid, 6);
  }
  else print_parametr (humid, 2);
  
  if ((temp_ind_width==3)&&(device_type!=MAG))
  {
    DisplayBuffer[2]=char_convert(channel[1]);
  }
  //���������� ��������� ������� ����������
  DisplayBuffer[16]=0;
  DisplayBuffer[17]=0;
  int count=GetOutChannelsCount();
  if (count > 16) count = 16;
  
  for (int i = 0; i < count; i++)
  {
    if (OutConfig[i].ChanelType == RELAY_TYPE)
      if ((RelayOuts >> i) & 1) DisplayBuffer[16+(i>>3)]|=1<<(i&7);
  }
  //�������� ��������� ���������� ����� (������ ��� � ���-�� ������� >1)
  if ((device_type==MAG) && (GetDisplayChannelsCount()>1)) DisplayBuffer[16]|=(1<<(current_channel_index+4));
  //��������� ���������� ����� ����������
  if (control_leds_exist) 
    DisplayBuffer[17] = 1<<current_out_channel;
  
  //��������� "��������"
  print_ctrl_status (12);
}

#pragma data_alignment = 8 
//static OS_STACKPTR U32            StackMENU[256];
OS_TASK                           OS_MENU;

//���� ��� ���������� "��������", �� 1 ������� ������� ���������� � ������������ ���������� 
void print_out_ch ()
{
  FlashMaskDisplay=0;
  DisplayBuffer[2]=char_convert_table['c'];
  DisplayBuffer[3]=char_convert_table['h'];
  DisplayBuffer[4]=char_convert_table['0'];
  DisplayBuffer[5]=char_convert_table['0'+current_out_channel+1];
  print_ctrl_status (6);
}

//�������� ����
//������������ ������� ������
void led_menu_task (int code)
{
  int i;
  if(ConfigWord & KEY_BEEP) set_sound(S_SOUND_20);
  switch (code)
  {
  case KEY_2 + SHORTPRESS_MASK:
  case KEY_3 + SHORTPRESS_MASK://������ ���������� ++
    //���� ����� ������ ��������� ��������� � ������ ����� ���������� �����������
    if (temp_ind_width==3)
    {
      current_channel_index++;
      if (current_channel_index>=GetDisplayChannelsCount ()) current_channel_index=0;
    }
    else
    {
      current_out_channel++;
      if (current_out_channel>=GetOutChannelsCount()) current_out_channel=0;
      if (control_leds_exist) DisplayBuffer[17] = 1<<current_out_channel;
      TempProgStatus=ProgramStatus[current_out_channel].ProgStatus;
      if (param_ind_and_leds_exist==0)
      {
       print_out_ch ();
       OS_Delay (1500);
      }
    }
    break;
    
  case KEY_4 + SHORTPRESS_MASK://������ ���������� --
    if (temp_ind_width==3)
    {
      current_out_channel++;
      if (current_out_channel>=GetOutChannelsCount()) current_out_channel=0;      
    }
    else
    {
      current_out_channel--;
      if (current_out_channel<0) current_out_channel=GetOutChannelsCount()-1;
    }
    if (control_leds_exist) DisplayBuffer[17] = 1<<current_out_channel;
    TempProgStatus=ProgramStatus[current_out_channel].ProgStatus;
    if (param_ind_and_leds_exist==0)
    {
      print_out_ch ();
      OS_Delay (1500);
    }
    break;
    
  case KEY_1 + SHORTPRESS_MASK://������ ���������
    current_channel_index++;
    if (current_channel_index>=GetDisplayChannelsCount ()) current_channel_index=0;
    break;
    
  case KEY_6 + SHORTPRESS_MASK://����� ������ ���������
    current_param_index++;
    if (current_param_index>=GetDisplayChannelParamsCount (current_channel_index)) current_param_index=start_param_index;
    break;
    
  case KEY_2 + LONGPRESS_MASK:
  case KEY_5 + LONGPRESS_MASK:
    ProgManager2();//��������� ������� ����������
    break;
    
  case KEY_1 + LONGPRESS_MASK:  
  case KEY_6 + LONGPRESS_MASK:
    ProgManager1();//��������� �������
    break;    
    
  case KEY_3 + LONGPRESS_MASK:
  case KEY_4 + LONGPRESS_MASK://������ ���������� ����
    if(OutConfig[current_out_channel].CntrAttr!=OFF_CONTROL) break;
    ManualControl^= (1<<current_out_channel);
    if (param_ind_and_leds_exist==0)
    {
      OS_Delay (1000);
      print_out_ch ();
      OS_Delay (1000);
    }
    break;
    
  case KEY_5 + SHORTPRESS_MASK:
    i=OutConfig[current_out_channel].ProgUse && (OutConfig[current_out_channel].CntrAttr > 1) 
      && (ProgramStatus[current_out_channel].ProgStatus != START_PROGRAM);
    if(i)
    {
      if (ProgramModeSelect==0)
      {
        ProgramModeSelect=1;
        TempProgStatus= ProgramStatus[current_out_channel].ProgStatus;
      }
              switch(ProgramStatus[current_out_channel].ProgStatus)
              {
                case STOP_PROGRAM:  switch(TempProgStatus)
                                    {
                                    case WORK_PROGRAM:
                                    case PAUSE_PROGRAM:
                                      case STOP_PROGRAM:  TempProgStatus= START_PROGRAM;
                                                          break;
                                                          
                                      case START_PROGRAM: TempProgStatus= STOP_PROGRAM;
                                                          break;
                                    }
                                    break;
                                                                                                            
                case WORK_PROGRAM: 
                case PAUSE_PROGRAM: switch(TempProgStatus)
                                    {
                                      case STOP_PROGRAM:  TempProgStatus= START_PROGRAM;
                                                          break;
                                                          
                                      case START_PROGRAM: TempProgStatus= WORK_PROGRAM;
                                                          break;
                                                          
                                      case WORK_PROGRAM:  TempProgStatus= PAUSE_PROGRAM;
                                                          break; 
                                                          
                                      case PAUSE_PROGRAM: TempProgStatus= STOP_PROGRAM;
                                                          break;                    
                                    }
                                    break;
                  
              }
      next_programm_count=3;
      ProgramModeSelect=1;
    }
  }
}

//�������� ������ LED
//������������ ������ � ��������� ���������� �� �������
void common_led_task ()
{
  char mail[MAIL_SIZE];
  led_timer_task ();
  if (device_type!=MAG) print_ctrl_status (12);
  else if (!HeatTimer) print_ctrl_status (12);
  for (;;)
  {
    OS_GetMail (&key_mb, mail);
    int code=mail[1]; code<<=8; code|=mail[0];
    if (mail[2]==0) led_menu_task (code);//���� ������ ������
    else 
    {
      if (next_programm_count) 
      {
        next_programm_count--; 
        if (next_programm_count==0)
        {
          ProgramModeSelect=0;
          if (ProgramStatus[current_out_channel].ProgStatus!=TempProgStatus)
          {
            set_sound (S_SOUND_500);
            ProgramStatus[current_out_channel].ProgStatus=TempProgStatus;
          }
        }
      }
    }
    led_timer_task ();//��������� ���������� �� ������� ��� ������
  }
}

extern OS_STACKPTR U32            StackTFT[256+64+64];

//�������������
//��������� ������
//����� �������� ����������
//������� ������ ���������
//��������� �������� ������
void led_init ()
{
  int i;
  init_panel ();
  current_param_index=start_param_index;
  for (i=0; i<DISPLAY_LEN+4; i++) DisplayBuffer[i]=char_convert(' ');
  DisplayBuffer[16] = 0;
  if (control_leds_exist) DisplayBuffer[17] = 1<<current_out_channel;  else DisplayBuffer[17] = 0;
  DisplayBuffer[18] = 0;
  CSOFF_IND;
  GPIOD_CRH= 0x23333444;
  OS_CREATEMB (&key_mb, MAIL_SIZE, MAIL_COUNT, mail_storage);//���� ������� ������
  OS_CREATETIMER( &LEDTimer, led_keyboard, 1);//������������ ���������� � �����������
  OS_CREATERSEMA(&use_display);//���������� ������ ������� ����, �� ��������� ���������� 1 ��� � �������
  
  print_parametr ((const char *) OffVersion[device_type], 2);
  OS_Use (&use_display);
  led_start=1;
  
  if( errors_flags.config_memory_fail /*|| errors_flags.touch_error*/ || errors_flags.sd_fail || errors_flags.sd_not_present)
  {
    print_parametr ("crit", 2);
    print_parametr (" err", 6);
    set_sound(S_SOUND_500);  
    for(;;CLEAR_IWDG) OS_Delay(1000);
  }
     
  /* ����� ������������ "���!" */
  if(errors_flags.config_fail)
  {
    print_parametr ("  no", 2);
    print_parametr ("conf", 6);
    set_sound(S_SOUND_500);
    
    while((ConfigWord & CONFIG_MASK) != (~InvConfigWord & CONFIG_MASK)) 
    {
      OS_Delay(1000); 
      CLEAR_IWDG;
    }
    
    print_parametr ("    ", 2);
    print_parametr (" rst", 6);
    
    set_sound(S_SOUND_500);
    OS_Delay(1000); 
    HW_RESET;     
  }
  
  /* ������ ��������� */
  if(errors_flags.timeinvalid)
  {
    print_parametr ("test", 2);
    print_parametr ("01  ", 6);
    set_sound(S_SOUND_500);
    for(U32 i = 0; i < 3; i++, CLEAR_IWDG) OS_Delay(1000);
  }
  else set_sound(S_SOUND_300);
    
  if (!HeatTimer) OS_Delay (3000);
  
  OS_Unuse (&use_display);
  led_start=0;
  
  TempProgStatus=ProgramStatus[current_out_channel].ProgStatus;
  //OS_CREATETASK(&OS_MENU, 0, common_led_task, 50, StackMENU);
  OS_CREATETASK(&OS_MENU, 0, common_led_task, 50, StackTFT);
}
 
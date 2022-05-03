#ifndef DEVICE_STRUCT_H
  #define DEVICE_STRUCT_H

typedef int SOCKTYPE;

// structures, union ect
typedef struct {  
  Int32U  reiniuart1:         1;
  Int32U  reiniuart2:         1;
  Int32U  start_boot:         1;

  Int32U  writedefaultmem:    1;
  Int32U  readdefaultmem:     1;  
  Int32U  code_updated:       1;
  
  Int32U  test_mode:          1;
  Int32U  set_retrans:        1;
  Int32U  ErrorFlag:          1;  
  Int32U  SDHC:               1;
  Int32U  touch_calib:        1;
  
  Int32U  redraw_gui:         1;
  Int32U  ScreenShoot:        1;
  Int32U  ScreenCalib:        1;
  Int32U  tftdriver:          1;
  
  Int32U  mqtt_ini:           1;
  Int32U  mqtt_on:            1;
  
} system_flags_t;

typedef struct 
{ 
  Int32U  LSEfail:            1;
  Int32U  timeinvalid:        1;
  Int32U  config_fail:        1;
  Int32U  config_memory_fail: 1;
  Int32U  load_default:       1;
  Int32U  sd_fail:            1;
  Int32U  sd_not_present:     1;
  Int32U  tft_fail:           1;
  Int32U  touch_error:        1;
  Int32U  tresh_error:        1;
  Int32U  chanl_error:        1;  
  Int32U  eth_error:          1;
} errors_flags_t;

typedef struct
{
  const U8 * data;
  U16 width;
  U16 height;
} tImage;

typedef struct
{
  long int code;
  const tImage *image;
} tChar;

typedef struct
{
  int length;
  const tChar *chars;
} tFont;

typedef struct
{
  Int16U        fColor;
  Int16U        bColor;  
  Int16U        Y; 
  const tFont   *fontType;
} TFT_String_st;

// 16 байт
typedef struct  
{
  U32           ErrorMask;      // маска ошибок
  U16           DataAdr;        // адрес данных при чтении из преобразователя
  U16           ErrorAdr;       // адрес ошибок при чтении из преобразователя
  U16           Id;             // id - преобразователя
  U8            InpNum;         // количество вычитываемых параметров
  U8            ChanelAttr;     // атрибуты канала
  U8            ComAdr;         // адрес порта коммутатора
  U8            NetAdr;         // адрес цифрового преобразователя
  U8            StartAdr;       // адрес в MainArray первого вычитываемого F32
  U8            dummy;          // выравнивание структуры на 16 байт
} InpConfig_t;


// ChanelAttr
#define INPUT_OFF         0
#define INPUT_DIGIT_1200  1
#define INPUT_DIGIT_2400  2
#define INPUT_DIGIT_4800  3
#define INPUT_DIGIT_9600  4
#define INPUT_DIGIT_19200 5
#define INPUT_DIGIT_38400 6
#define INPUT_I           7
#define INPUT_U           8

// 16 байт
typedef struct
{
  U16   FuncNum;
  U8    InputMeasureParamNum[5];
  U8    InputConstantParamNum[5];
  U32   ErrorAllowed;
} CalcConfig_t;

// FuncNum 
#define CALC_OFF                  0
#define CALC_AVR                  0x80
#define CALC_SUM                  0x81
#define CALC_SUB                  0x82
#define CALC_POLY                 0x83

// 16 байт
typedef struct
{
  F32     LowLimit;
  F32     UpLimit;
  U32     ErrLow:     8;    // % от LowLimit
  U32     ErrUp:      8;    // % от UpLimit
  U32     Reserv1:    8;  
  U32     DecPoint:   7;
  U32     WriteStat:  1;
  U32     ParamType:  8;
  U32     UnitType:   8;
  U32     Reserv2:    16;
} MeasureAtt_t;

/* структуры для DisplConfig_t  ----------------------------- */
typedef union
{
  Int8U data;
  struct 
  {
    Int8U Up:         1;
    Int8U Weight:     1;
    Int8U ShowStat:   2;
    Int8U Status:     1;
    Int8U :           3;
  };
} TreshConfig_t;

// size = 12
typedef struct
{
  Int8U DecPoint:   2;
  Int8U ShowStat:   2;
  Int8U :           4;
  U8         ParamType;
  U8         UnitType;
  U8         Name[9];
} ConstConfig_t;
/* структуры для DisplConfig_t  ----------------------------- */

#define DO_NOT_SHOW   0
#define ONLY_SHOW     1
#define SHOW_AN_EDIT  2

// структура отображения - ??? байт
typedef struct 
{
  F32             Treshold[12];    //48
  F32             Constant[12];    //48
  TreshConfig_t   TreshConfig[12]; //12
  ConstConfig_t   ConstConfig[12]; //144
  U8              DisplayParam[6]; //6
  U8              ParamConfig[6];  //6
} DisplConfig_t;

typedef struct
{
  Flo32   DefParam;
  Flo32   Hesteresis;
  
  Int32U  StartStep:          10;
  Int32U  StopStep:           10;
  Int32U  ProgUse:            1;
  Int32U  EndProgCond:        2;
  Int32U  ChanelNum:          4;
  Int32U  LogicUse:           3;
  Int32U  DefParamIsIndex:    1;
  Int32U  :                   1;
      
  Int16U  ProgramStatus;
  
  Int8U   Input;             // Input param
  
  Int8U   CntrAttr:    3;    // OFF,logic,Hes,PID
  Int8U   ChanelType:  3;    // NO, relay, analog - type 0-5,0-20,4-20
  Int8U   ControlType: 2;    // inv
} OutConfig_t;

#define LOGIC_TR1   BIT0
#define LOGIC_TR2   BIT1
#define LOGIC_ERR   BIT2

typedef struct
{
  Flo32   Pk;
  Flo32   Ik;
  Flo32   Dk;
  Int32U  Tpwm;
} PidConfig_t;

#define NO_CONTROL        0
#define RELAY_TYPE        1
#define ANALOG_TYPE_05    2
#define ANALOG_TYPE_020   3
#define ANALOG_TYPE_420   4

#define OFF_CONTROL       0
#define LOGIC_CONTROL     1
#define HEST_CONTROL      2
#define PID_CONTROL       3

#define LINE_CONTROL      2

#define STOP_PROGRAM      0
#define START_PROGRAM     1
#define WORK_PROGRAM      2
#define PAUSE_PROGRAM     3

#define ENDP_STOP         0
#define ENDP_KEEP_LAST    1
#define ENDP_RESTART      2
#define ENDP_LOADDEF      3

#define ANALOG_RANGE_MASK (BIT15+BIT14)
#define IOUT_OFF          0
#define IOUT_0_5          BIT14
#define IOUT_0_20         BIT15
#define IOUT_4_20         (BIT15+BIT14)

typedef struct
{
  Flo32       Param;
  Int16U      SetlTime;
  Int16U      HoldTime;
} Program_t;

typedef union
{
  Int16U    data;
  struct 
  {
    Int16U  CurrentStep: 10;
    Int16U  ProgStatus:  2;
    Int16U  SetPass:     1;
    Int16U  ControlFail: 1;
    Int16U  ProgFinished:1;
  };
} ProgramStatus_t;

typedef struct
{
  Int32U IP;
  Int32U Mask;
  Int32U Gate;
  Int8U  MAC[3];
  Int8U  UseDHCP:  1;
  Int8U  ConSpeed: 2; // 0 - auto, 1 - 100, 2 - 10
  Int8U  ConFHDuplex: 1; // 0 - full, 1 - half
  
} TCPIP_CONFIG;

typedef struct
{
  Int16U AutoRefreshPeriod;
  Int16U Dummy1;
  Int16U Disabled: 1;
  Int16U HideStatisticReset: 1;
  Int16U : 14;
  Int16U Dummy2;
} WEB_SERVER_CONFIG;

#pragma pack(4)
typedef struct              // 96 bytes
{
  char   UserName[16];      // 16
  char   UserPass[16];      // 16
  char   TopicBase[16];     // 16
  char   SeverHostName[32]; // 32         
  U16    ServerPort;        // 2
  U16    mqtt_use:          1;
  U16    mqtt_interface:    2;  // 0 - Eth, 1 - GSM, 2 - WiFi
  U16    mqtt_buff_policy:  2;  // not used
  U16    mqtt_qos:          2;  // 0, 1, 2
  U16    ping_interval;     // 2
  U16    info_interval;     // 2
  U8     Reserv[8];         // 8
} MQTT_CONFIG;
#pragma pack()

#pragma pack(4)
typedef struct              // 48 bytes
{
  Int8U  NTPSeverName[32];  // 32
  S16    TimeZone;          // 2
  U16    ntpuse:       1;   // 2
  U16    NTPSyncReq:   1;
  U32    periodUpdateTime;  // 4
  U32    updateCounter;     // 4  
  U8     Reserv[4];         // 4
} NTP_CONFIG;
#pragma pack()

#endif
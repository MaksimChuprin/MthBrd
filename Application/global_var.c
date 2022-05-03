#include "defines.h"

// Appl
volatile system_flags_t           system_flags;
volatile Int16U                   ProgModeTimer;

F32                               StepShift[MAX_OUTS];
F32                               OutCurrents[MAX_OUTS];
U32                               OS_JLINKMEM_BufferSize = 128;
U32                               MBTouchPadKeyBuffer[4];
U32                               StatTimer;
U16                               DownCount;
U16                               ProgramTimer[MAX_OUTS];
U16                               AnalogArray[MAX_OUTS];
U8                                ErrorCount[MAX_INPUT_COUNT];
U8                                CommandStatus[MAX_INPUT_COUNT];
U8                                TempProgStatus;
U8                                MBKeyBuffer[4];
TFT_String_st                     TFT_String;
char                              HostName[32];
Int8U                             EBeep, TBeep, BeepControl;
Int8U                             LastSound;
Int8U                             PumpStatus;

// CONFIG segment 0x0000 .... 0x7FFF
#define A0_CONF                   0
__root __no_init InpConfig_t      InputConfig[16] @ CONFIG(A0_CONF);        
#define A1_CONF                   sizeof(InputConfig) + A0_CONF
__root __no_init CalcConfig_t     CalcConfig[128] @ CONFIG(A1_CONF);        
#define A2_CONF                   sizeof(CalcConfig) + A1_CONF
__root __no_init MeasureAtt_t     MeasureAttribute[256] @ CONFIG(A2_CONF);  
#define A3_CONF                   sizeof(MeasureAttribute) + A2_CONF
__root __no_init DisplConfig_t    DisplayConfig[16] @ CONFIG(A3_CONF);
#define A4_CONF                   sizeof(DisplayConfig) + A3_CONF
__root __no_init OutConfig_t      OutConfig[16] @ CONFIG(A4_CONF);          
#define A5_CONF                   sizeof(OutConfig) + A4_CONF
__root __no_init Flo32            AnalogConfig[16][2] @ CONFIG(A5_CONF);
#define A6_CONF                   sizeof(AnalogConfig) + A5_CONF
__root __no_init PidConfig_t      PidConfig[16] @ CONFIG(A6_CONF);
#define A7_CONF                   sizeof(PidConfig) + A6_CONF
__root __no_init Program_t        Program[512] @ CONFIG(A7_CONF);


// config part    0x3F00 .... 0x3FFF
#define A0_CONF_C                  0x3F00
__root __no_init Int32U            WriteStatAddr @ CONFIG(A0_CONF_C);
#define A1_CONF_C                  A0_CONF_C + sizeof(WriteStatAddr)
__root __no_init Int32U            StatisticPeriod @ CONFIG(A1_CONF_C);
#define A2_CONF_C                  A1_CONF_C + sizeof(StatisticPeriod)
__root __no_init Flo32             Uref @ CONFIG(A2_CONF_C);
#define A3_CONF_C                  A2_CONF_C + sizeof(Uref)
__root __no_init Int16U            NetAddress @ CONFIG(A3_CONF_C);
#define A4_CONF_C                  A3_CONF_C + sizeof(NetAddress)
__root __no_init Int16U            MinInputLoopTime @ CONFIG(A4_CONF_C);
#define A5_CONF_C                  A4_CONF_C + sizeof(MinInputLoopTime)
__root __no_init Int8U             SerialNumber[12] @ CONFIG(A5_CONF_C);
#define A6_CONF_C                  A5_CONF_C + sizeof(SerialNumber)
__root __no_init Int32U            ConfigWord @ CONFIG(A6_CONF_C);
#define A7_CONF_C                  A6_CONF_C + sizeof(ConfigWord)
__root __no_init Int32U            InvConfigWord @ CONFIG(A7_CONF_C);
#define A8_CONF_C                  A7_CONF_C + sizeof(InvConfigWord)
__root __no_init F32               TFT_Calib[2][2] @ CONFIG(A8_CONF_C);
#define A9_CONF_C                  A8_CONF_C + sizeof(TFT_Calib)
__root __no_init TCPIP_CONFIG      TCPIPConfig @ CONFIG(A9_CONF_C);
#define A10_CONF_C                 A9_CONF_C + sizeof(TCPIP_CONFIG)
__root __no_init Int8U             DeviceName[32] @ CONFIG(A10_CONF_C);
#define A11_CONF_C                 A10_CONF_C + sizeof(DeviceName)
__root __no_init Int16U            Brightness @ CONFIG(A11_CONF_C);
#define A12_CONF_C                 A11_CONF_C + sizeof(Brightness)
__root __no_init WEB_SERVER_CONFIG WebServerConfig @ CONFIG(A12_CONF_C);
#define A13_CONF_C                 A12_CONF_C + sizeof(WebServerConfig)
__root __no_init Int16U            PumpTime @ CONFIG(A13_CONF_C);
#define A14_CONF_C                 A13_CONF_C + sizeof(PumpTime)
__root __no_init Int32U            IDWordLo @ CONFIG(A14_CONF_C);
#define A15_CONF_C                 A14_CONF_C + sizeof(IDWordLo)
__root __no_init MQTT_CONFIG       MQTTConfig @ CONFIG(A15_CONF_C);
#define A16_CONF_C                 A15_CONF_C + sizeof(MQTTConfig)
__root __no_init NTP_CONFIG        NTPConfig @ CONFIG(A16_CONF_C);


// RAM segment 0x0000 .... 0x7FFF
#define A0_RAM                    0x0000
__root __no_init Flo32            MainArray[272] @ RAM(A0_RAM);   // 128 - измерения, 127 - пересчет, 16 - управление
#define A1_RAM                    A0_RAM + sizeof(MainArray)
__root __no_init Int16U           RelayOuts @ RAM(A1_RAM);  // 0x440
#define A2_RAM                    A1_RAM + sizeof(RelayOuts)
__root __no_init Int16U           ManualControl @ RAM(A2_RAM); // 0x442
#define A3_RAM                    A2_RAM + sizeof(ManualControl)
__root __no_init ProgramStatus_t  ProgramStatus[16] @ RAM(A3_RAM); // 0x444
#define A4_RAM                    A3_RAM + sizeof(ProgramStatus)
__root __no_init Int8U            ErrorArray[256] @ RAM(A4_RAM); // 0x464
#define A5_RAM                    A4_RAM + sizeof(ErrorArray)
__root __no_init U32              Time @ RAM(A5_RAM); // 0x564  Эксис-время
#define A6_RAM                    A5_RAM + sizeof(Time)
__root __no_init volatile errors_flags_t  errors_flags @ RAM(A6_RAM); // 0x568
#define A7_RAM                    A6_RAM + sizeof(errors_flags_t)
__root __no_init U32              ResetCounter @ RAM(A7_RAM); // 0x56c
#define A8_RAM                    A7_RAM + sizeof(ResetCounter)
__root __no_init U32              PowerUpCounter @ RAM(A8_RAM); // 0x570
#define A9_RAM                    A8_RAM + sizeof(PowerUpCounter)
__root __no_init U32              TimeUpCounter @ RAM(A9_RAM); // 0x574
#define A10_RAM                   A9_RAM + sizeof(TimeUpCounter)
__root __no_init U32              UsbUpdateCounter @ RAM(A10_RAM); // 0x578
#define A11_RAM                   A10_RAM + sizeof(UsbUpdateCounter)
__root __no_init U32              SDUpdateCounter @ RAM(A11_RAM); // 0x57c
#define A12_RAM                   A11_RAM + sizeof(SDUpdateCounter)
__root __no_init U32              LastUpdateResult @ RAM(A12_RAM); // 0x580
#define A13_RAM                   A12_RAM + sizeof(LastUpdateResult)
__root __no_init U32              SDFailCounter; // 0x584
#define A14_RAM                   A13_RAM + sizeof(LastUpdateResult)
__root __no_init U32              SDCfgFailCounter; // 0x588

// constants
const U32                         IDWordHi = _UNIX_TIME + _MQTT_PRESENT + _NTP_PRESENT;
const char                        ID[]= "Блок управления TFT", Version[]= "r3.01";
//  12/02/21 3.00 - STM32F407
//  20/12/21 3.01 - перезапуск при записи конфигурации на ходу

const U16 SoundArr[S_MAX][2] = {
1, 2,
2, 1,
101, 1,
301, 1,
501, 1,
1001, 1,
200,  100,
600, 300,
1000, 500,
2000, 1000,
};

const F32 E_TAB[]= {
6.106359361,
5.621913509,
5.172714271,
4.756444744,
4.370922676,
4.014093781,
3.684025324,
3.378899958,
3.097009813,
2.836750838,
2.596617372,
2.375196944,
2.171165305,
1.983281669,
1.810384169,
1.651385507,
1.505268814,
1.371083691,
1.247942432,
1.135016429,
1.031532749,
0.936770871,
0.850059584,
0.770774042,
0.698332957,
0.632195945,
0.571861002,
0.516862109,
0.466766967,
0.421174853,
0.379714586,
0.342042608,
0.307841171,
0.276816624,
0.248697798,
0.223234477,
0.200195972,
0.179369761,
0.160560224,
0.143587446,
0.128286096,
0.114504374,
0.102103021,
0.090954399,
0.080941624,
0.071957751,
0.063905025,
0.056694166,
0.050243716,
0.044479422,
0.039333663,
0.034744918,
0.03065727,
0.027019947,
0.023786896,
0.020916383,
0.018370633,
0.016115488,
0.014120092,
0.012356603,
0.010799927,
0.009427472,
0.008218921,
0.007156022,
0.0062224,
0.005403381,
0.004685829,
0.004058,
0.003509408,
0.003030703,
0.002613554,
0.002250554,
0.001935119,
0.001661411,
0.001424256,
0.001219076,
0.001041826,
0.000888937,
0.000757265,
0.000644044,
0.000546844,
0.000463532,
0.000392241,
0.00033134,
0.000279401,
0.000235182,
0.000197602,
0.00016572,
0.000138721,
0.0001159,
9.66459E-05,
8.04317E-05,
6.6804E-05,
5.53725E-05,
4.58024E-05,
3.78068E-05,
3.11403E-05,
2.55937E-05,
2.09886E-05,
1.71735E-05,
1.40198E-05,
1.00198E-05
};

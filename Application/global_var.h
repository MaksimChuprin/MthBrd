#define MEASURE_BASE                0
#define CALC_BASE                   128
#define CONTROL_BASE                256

extern OS_STACKPTR U32              StackSUPERVISER[128], StackTFT[384];
extern OS_TASK                      OS_SUPERVISER, TP80; 
extern OS_RSEMA                     SemaRAM, SemaSPI1, SemaSPI3, SemaTFT, SemaTCP;
extern OS_MAILBOX                   TouchPad_MB, Key_MB;
extern OS_TIMER                     OneSecTimer, SDTimer, PumpTimer, TFTTimer, SDConfWrite;

// global var
extern volatile system_flags_t      system_flags;
extern volatile errors_flags_t      errors_flags;

extern Flo32                        StepShift[MAX_OUTS], TFT_Calib[2][2];
extern U32                          MBTouchPadKeyBuffer[4], StatTimer;

extern Int16U                       RelayOuts,AnalogArray[MAX_OUTS],ConfigSaveCounter,ConfigSaveTime,KeyCode;
extern Flo32                        OutCurrents[MAX_OUTS];
extern Int16U                       InputLoopTimer, MinInputLoopTime, ADC_Filter[4], ManualControl, StaisticTimer;
extern Int16U                       ProgramTimer[MAX_OUTS];

extern Int8U                        ErrorCount[], ErrorArray[256], MBSoundBuffer[], MBOneSecBuffer[], CommandStatus[], MBKeyBuffer[4];
extern Int8U                        MeasureDisplayPointer, ControlDisplayPointer, DisplayPoint, TempProgStatus, NeedCalib, PumpStatus;

extern TFT_String_st                TFT_String;

extern char                         HostName[];

extern Int8U                        EBeep, TBeep, BeepControl;
extern Int8U                        LastSound;

// ram+rom
extern ProgramStatus_t              ProgramStatus[];

// CONFIG segment
extern Flo32                        Uref, AnalogConfig[16][2];
extern Int32U                       ConfigWord, InvConfigWord, WriteStatAddr, StatisticPeriod, IDWordLo;
extern Int16U                       NetAddress, MinInputLoopTime, PageAddress;
extern Int8U                        SerialNumber[], DeviceName[];
extern Int16U                       Brightness, PumpTime, DownCount;
extern TCPIP_CONFIG                 TCPIPConfig;
extern WEB_SERVER_CONFIG            WebServerConfig;
extern NTP_CONFIG                   NTPConfig;
extern MQTT_CONFIG                  MQTTConfig;

extern InpConfig_t                  InputConfig[];
extern MeasureAtt_t                 MeasureAttribute[];
extern CalcConfig_t                 CalcConfig[];
extern DisplConfig_t                DisplayConfig[];
extern OutConfig_t                  OutConfig[];
extern PidConfig_t                  PidConfig[];
extern Program_t                    Program[];

// RAM segment
extern Flo32                        MainArray[];
extern Flo32                        StepShift[];
extern Int32U                       Time, ResetCounter, PowerUpCounter, TimeUpCounter, UsbUpdateCounter, SDUpdateCounter, LastUpdateResult, SDCfgFailCounter, SDFailCounter;

// consts
extern const char                   ID[], Version[];
extern const Int8U                  ShortSound_50, ShortSound_100, LongSound_500, LongSound_1000;
extern const U16                    SoundArr[S_MAX][2];
extern const Flo32                  E_TAB[];
extern const U32                    IDWordHi;
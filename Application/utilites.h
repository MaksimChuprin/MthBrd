#include <stdbool.h>

#define JAN2021                     1609459200
#define EKSIS_TIME_SHIFT            946684800

#define SCAN_PANEL_XA               MODIFY_REG(GPIOB->MODER, 0x0003030F, 0x00010307)
#define SCAN_PANEL_YA               MODIFY_REG(GPIOB->MODER, 0x0003030F, 0x0003010D)

#define NO_VIOLATION                0
#define ATTENTION_VIOLATION         1
#define ALARM_VIOLATION             2

#define ERRORS_NOT_PRESENT          0
#define ERRORS_PRESENT              1

#define NOT_PRESSED                 0
#define BACK_BUTTON                 1
#define SWITCH_BUTTON               2

#define BOOTMARK_BASE               0
#define ERR_BASE                    1
#define RESET_COUNTER               2
#define POWERUP_COUNTER             3
#define TIMEUP_COUNTER              4
#define USBUPDATE_COUNTER           5
#define SDUPDATE_COUNTER            6
#define LASTUPDATE_RESULT           7
#define STATADR_REG                 8
#define SDFAIL_COUNT                9
#define SDCFGFAIL_COUNT             10

void  SetComInputLine               (U8 adr);
void  SetCurrentDac                 (U16 Dac1, U8 CCh);
void  SetRelay                      (U16 relayword);
void  decode_date                   (Int32U as, pU8 sdata, Int8U plusm, Int8U type);
void  scan_touch                    (void);
void  set_relay_ang                 (void);
void  set_sound                     (U8 soundtype);
void  oneSecISR                     (void);
void  PumpISR                       (void);
void  TFT_Load                      (pU16, U32 NumBytes);

void  check_stat_timer              (void);

void  scan_key                      (void);
void  oneSecISR                     (void);
void  TFTISR                        (void);
void  StatisticManager              (void);
void  decode_date                   (Int32U as, pU8 sdata, Int8U plusm, Int8U type);
void  shift_word                    (U16 word);

void  eecons_read                   (Int16U adr);
void  start_write_GRAM_TFT          (void);
void  stop_write_GRAM_TFT           (void);

U8    shift_byte_spi1               (U8 byte);
U8    shift_byte_spi3               (U8 byte);
U16   read_control_TFT              (U8 adr);

U8    saveSDConfDelayed             ( U16 maxsectwrite);
void  seachSectorsToWrite           ( U32 addr, U32 bytenum);
void  SDConfISR                     (void);

Int8U GetDisplayChannelsCount();
Int8U GetDisplayChannelParamsCount(Int8U display_config);
Int8U GetDisplayChannelChosenParamsCount(Int8U display_config);
void  GetChosenParams(Int8U display_config, int* first, int* second);
void  GetChosenParamsMask(Int8U display_config, int* mask);
Int8U GetRectParam(Int8U display_config, Int8U rect_index);
Int8U GetParamRect(Int8U display_config, Int8U param_index);
Int8U GetParametersCount();
void  GetMeasureString(Int8U display_channel_index, Int8U dispay_param_index, char* display, Int16U* color);
void  GetMeasureStringNoUnits(Int8U display_channel_index, Int8U dispay_param_index, char* display, Int16U* color);
void  GetParamMinMax(Int8U display_channel_index, Int8U dispay_param_index, float* min, float* max);
void  GetParamString(Int8U display_channel_index, Int8U dispay_param_index, char* display);
void  GetParamStringByMeasureAttribute(Int8U measure_attribute, char* display);
void  GetParamTypeAndUnits(Int8U display_channel_index, Int8U dispay_param_index, char* display);
void  GetParamNameAndUnits(Int8U display_channel_index, Int8U dispay_param_index, char* display);
void  GetParamName(Int8U display_channel_index, Int8U dispay_param_index, char* display);
void  GetParamType(Int8U display_channel_index, Int8U dispay_param_index, char* display);
void  GetParamUnits(Int8U display_channel_index, Int8U dispay_param_index, char* display);
void  GetParamValueForWS(Int8U display_channel_index, Int8U dispay_param_index, char* display, Int8U* red);
void  GetParamUnitsForWS(Int8U display_channel_index, Int8U dispay_param_index, char* display);

Int8U GetOutChannelsCount();

Int8U GetControlChannelMeasureAttributeIndex(Int8U input);
Int8U GetControlChannelDecimal(Int8U input);
Int8U GetControlChannelUnitType(Int8U input);
void  GetControlChannelInputMeasureString(Int8U input, char* display, Int16U* color);
void  GetControlChannelInputMeasureStringNoUnits(Int8U input, char* display, Int16U* color);
void  GetControlChannelInputParamString(Int8U input, char* param);
void  GetControlChannelInputParamTypeAndUnits(Int8U input, char* type_and_units);
void  GetControlChannelInputThreshold1String(Int8U input, char* threshold1);
void  GetControlChannelInputThreshold2String(Int8U input, char* threshold2);
void  GetControlChannelInputThresholdsString(Int8U input, char* thresholds);
void  GetControlChannelInputParamMinMax(Int8U input, float* min, float* max);

void IncrementDisplayParam(Int8U* display_channel, Int8U* display_param);
void DecrementDisplayParam(Int8U* display_channel, Int8U* display_param);
void IncrementControlChannelInputParam(Int8U* input);
void IncrementControlChannelHesterParam(Int8U* input, Int8U unit_type);
void IncrementPressureParam(Int8U* measure_attribute);

void ParseIPAddress(Int32U ip, char* display);
void PackIPAddress(Int32U* ip, char* display);

void HTMLColor(Int16U color, char* html_color);

Int16U GetParameterValueForMODBUS   (U16 parameter_number, Int8U* buffer);
Int16U GetErrorValueForMODBUS       (Int8U parameter_number, Int8U* buffer);

void GetStatisticHeaderForWS(                  char* out);
void GetStatisticRecordForWS(int record_index, char* out);

Int8U ThresholdsViolated();
Int8U ErrorsPresent();

Int8U HumidityCalculationPossible(Int8U display_config);
Int8U OxygenCalculationPossible  (Int8U display_config);
Int8U PressureCalculationPossible(Int8U display_config);
Int8U FlowCalculationPossible    (Int8U display_config);

Int8U GetDewPointParamIndex(Int8U display_config);
Int8U GetFlowParamIndex    (Int8U display_config);
Int8U GetOxygenParamIndex  (Int8U display_config);
void  SetPressure1Source(Int8U display_config, bool transducer);
void  SetPressure2Source(Int8U display_config, bool transducer);

Int8U PressureTransducerExists();
bool  IsChannelExists(int channel_index);

void  GetDisplayConfigAndParam(Int8U measure_attribute, Int8U* display_config, Int8U* display_param);
Int8U GetMainScreenMaximumParameters();
Int8U GetMainScreenParametersCount(int channel_index);
Int8U GetActiveBitIndex(Int8U mask, Int8U bit_order);

bool get_mark_forbooter   (void);
void usb_exchange         (pU8 buf);
void set_mark_forbooter   (U8 n);
void SoundDeal            (void);
U8   ConfigDeal           (void);
void LoadAndCheckConfig   (void);
void set_bkp_reg          (U8 adr, U32 data);
U32  get_bkp_reg          (U8 adr);
U32  RTC_GetCounter       (void);
void RTC_SetCounter       (U32 newtime);
void ParseDate            (U32 t, int* s, int* mi, int* h, int* d, int* m, int* y);
U32  EncodeDate           (U8 s, U8 mi, U8 h, U8 d, U8 m, U16 y);
U8   ifAnyLeft2Write      (void);
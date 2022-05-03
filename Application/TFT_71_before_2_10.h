#include "stdbool.h"

#define MULTIPLIER_N            35
#define DIVIDER_M               2
#define RESOLUTION_X            800
#define RESOLUTION_Y            480
#define LCDC_FPR                235928
#define H_Sync_total            950
#define H_Sync_to_DE            150  
#define H_Sync_Pluse_Wide       50
#define V_Sync_total            500
#define V_Sync_to_DE            20
#define V_Sync_Pluse_Wide       10

#define SD_STATISTIC_OFFSET     0L
#define SD_STATISTIC_SIZE       0x40000000
#define SD_SCREENSHOT_OFFSET    (U32)(SD_STATISTIC_OFFSET  + SD_STATISTIC_SIZE)
#define SCREENSHOT_SIZE         (1048576L)
#define SD_IMAGES_OFFSET        (U32)(SD_SCREENSHOT_OFFSET + SCREENSHOT_SIZE)

// Структура точки
typedef struct
{
  Int16U X, Y;
} POINT;

// Структура прямоугольника
typedef struct
{
  Int16U sX, eX;
  Int16U sY, eY;
  Int16U color;
} RECT;

// Цветовая схема
typedef struct
{
  Int16U first_head;
  Int16U first_body;
  Int16U first_text;
  Int16U second_head;
  Int16U second_body;
  Int16U second_text;
  Int16U third_head;
  Int16U third_body;
  Int16U third_text;
  
  Int8U use_default;
  Int8U reserved;
} COLOR_SCHEME;

enum CONTROL_SCREEN {csDetail, csBrief};

// Конвертирует цвет из RGB888 в RGB565
#define RGB565(r, g, b)   (b >> 3) | ((g >> 2) << 5) | ((r >> 3) << 11)

// Базовые цвета
#define RED                   0xF800
#define ORANGE                0xF9E0
#define YELLOW                0xFE00
#define GREEN                 0x07E0
#define DARKGREEN             RGB565(60, 160, 40)
#define LIGHTGREEN            0xDFFB
#define BLUE                  0x19FF
#define DEEPBLUE              RGB565(40, 40, 160)//0x001F
#define PURPLE                0xF86F
#define WHITE                 0xFFFF
#define BLACK                 0x0000
#define LIGHTRED              RGB565(255, 100, 100)
#define BROWN                 RGB565(200, 145, 75)

// Параметры шрифтов
#define TAHOMA22_HEIGHT        22
#define TAHOMA34_HEIGHT        34
#define TAHOMA87_HEIGHT        87

#define BACKGROUND_COLOR         RGB565(242, 242, 242)
#define CALB_RECTS_COLOR         RGB565(151, 227, 143)
#define TEMPLATE_COLOR           RGB565(163, 73, 164)

// Константы окна сообщения
#define MESSAGEBOX_OK                     0
#define MESSAGEBOX_YESNO                  1
#define MESSAGEBOX_SX                     150
#define MESSAGEBOX_EX                     649
#define MESSAGEBOX_SY                     100
#define MESSAGEBOX_EY                     379
#define MESSAGEBOX_CENTER                 (MESSAGEBOX_EX+MESSAGEBOX_SX)/2
#define MESSAGEBOX_COLOR                  RGB565(250, 228, 163)
#define MESSAGEBOX_HEAD_COLOR             RGB565(230, 208, 143)
#define MESSAGEBOX_CAPTION_SX             MESSAGEBOX_SX+10
#define MESSAGEBOX_CAPTION_SY             MESSAGEBOX_SY
#define MESSAGEBOX_LINES_SX               MESSAGEBOX_SX+15
#define MESSAGEBOX_LINE1_SY               MESSAGEBOX_CAPTION_SY+45
#define MESSAGEBOX_LINE2_SY               MESSAGEBOX_LINE1_SY+TAHOMA34_HEIGHT+5
#define MESSAGEBOX_LINE3_SY               MESSAGEBOX_LINE2_SY+TAHOMA34_HEIGHT+5
#define MESSAGEBOX_LBUTTON_SX             MESSAGEBOX_CENTER-200-35
#define MESSAGEBOX_LBUTTON_EX             MESSAGEBOX_LBUTTON_SX+200-1
#define MESSAGEBOX_RBUTTON_SX             MESSAGEBOX_CENTER+35
#define MESSAGEBOX_RBUTTON_EX             MESSAGEBOX_RBUTTON_SX+200-1
#define MESSAGEBOX_CBUTTON_SX             MESSAGEBOX_CENTER-100
#define MESSAGEBOX_CBUTTON_EX             MESSAGEBOX_CENTER+100-1
#define MESSAGEBOX_BUTTONS_SY             MESSAGEBOX_EY-100
#define MESSAGEBOX_BUTTONS_EY             MESSAGEBOX_BUTTONS_SY+100+1

#define ODD_BLUE                       RGB565(133, 215, 242)
#define EVEN_BLUE                      RGB565(153, 235, 255)
#define ODD_GREEN                      RGB565(151, 227, 143)
#define EVEN_GREEN                     RGB565(171, 247, 163)
#define ODD_GRAY                       RGB565(202, 202, 202)
#define EVEN_GRAY                      RGB565(222, 222, 222)
#define EVEN_YELLOW                    RGB565(215, 235, 133)

#define SHIFTED_ODD_GREEN              ODD_GREEN+4
#define SHIFTED_EVEN_GREEN             EVEN_GREEN+7
#define SHIFTED_EVEN_YELLOW            EVEN_YELLOW-1075

#define KEY_COLOR                   RGB565(250, 250, 250)
#define KEY_ROW_1_SX                0
#define KEY_ROW_1_EX                99
#define KEY_ROW_2_SX                100
#define KEY_ROW_2_EX                199
#define KEY_ROW_3_SX                200
#define KEY_ROW_3_EX                299
#define KEY_ROW_4_SX                300
#define KEY_ROW_4_EX                399
#define KEY_ROW_5_SX                400
#define KEY_ROW_5_EX                499
#define KEY_ROW_6_SX                500
#define KEY_ROW_6_EX                599
#define KEY_ROW_7_SX                600
#define KEY_ROW_7_EX                699
#define KEY_ROW_8_SX                700
#define KEY_ROW_8_EX                799

/* 51 */
#define UPPERMOST_LINE_SY           0
#define UPPERMOST_LINE_EY           95
#define UPPER_LINE_SY               96
#define UPPER_LINE_EY               191
#define MIDDLE_LINE_SY              192
#define MIDDLE_LINE_EY              287
#define LOWER_LINE_SY               288
#define LOWER_LINE_EY               383
#define LOWERMOST_LINE_SY           384
#define LOWERMOST_LINE_EY           479

#define UPPERMOST_TOP_LINE_SY       0
#define UPPERMOST_TOP_LINE_EY       47
#define UPPERMOST_BOTTOM_LINE_SY    48
#define UPPERMOST_BOTTOM_LINE_EY    95
#define UPPER_TOP_LINE_SY           96
#define UPPER_TOP_LINE_EY           143
#define UPPER_BOTTOM_LINE_SY        144
#define UPPER_BOTTOM_LINE_EY        191
#define MIDDLE_TOP_LINE_SY          192
#define MIDDLE_TOP_LINE_EY          239
#define MIDDLE_BOTTOM_LINE_SY       240
#define MIDDLE_BOTTOM_LINE_EY       287
#define LOWER_TOP_LINE_SY           288
#define LOWER_TOP_LINE_EY           335
#define LOWER_BOTTOM_LINE_SY        336
#define LOWER_BOTTOM_LINE_EY        383
#define LOWERMOST_TOP_LINE_SY       384
#define LOWERMOST_TOP_LINE_EY       431
#define LOWERMOST_BOTTOM_LINE_SY    432
#define LOWERMOST_BOTTOM_LINE_EY    479

#define LEFTHALF_SX                 0
#define LEFTHALF_EX                 399
#define RIGHTHALF_SX                400
#define RIGHTHALF_EX                799

#define LEFTTHIRD_SX                0
#define LEFTTHIRD_EX                265
#define CENTRALTHIRD_SX             267
#define CENTRALTHIRD_EX             532
#define RIGHTTHIRD_SX               534
#define RIGHTTHIRD_EX               799

#define LEFTMOSTFOURTH_SX           0
#define LEFTMOSTFOURTH_EX           199
#define LEFTFOURTH_SX               200
#define LEFTFOURTH_EX               399
#define RIGHTFOURTH_SX              400
#define RIGHTFOURTH_EX              599
#define RIGHTMOSTFOURTH_SX          600
#define RIGHTMOSTFOURTH_EX          799

#define LEFT_COLUMN_SX              0
#define LEFT_COLUMN_EX              99
#define RIGHT_COLUMN_SX             700
#define RIGHT_COLUMN_EX             799

#define LEFT_PARAMS_SX              100
#define LEFT_PARAMS_EX              399
#define RIGHT_PARAMS_SX             400
#define RIGHT_PARAMS_EX             699

#define LEFT_PARAMS_LEFTHALF_SX     100
#define LEFT_PARAMS_LEFTHALF_EX     249
#define LEFT_PARAMS_RIGHTHALF_SX    250
#define LEFT_PARAMS_RIGHTHALF_EX    399
#define RIGHT_PARAMS_LEFTHALF_SX    400
#define RIGHT_PARAMS_LEFTHALF_EX    549
#define RIGHT_PARAMS_RIGHTHALF_SX   550
#define RIGHT_PARAMS_RIGHTHALF_EX   699

// Константы изображений
#define MAX_IMAGES               128                                                   // Максимальное количество картинок
#define IMAGE_INFO_ADDR          (U32)SD_IMAGES_OFFSET                                 // Адрес информации о картинках
#define IMAGE_INFO_SIZE          16                                                    // Размер информации об одной картинке
#define IMAGE_INFO_HEADER_SIZE   (U32)((U32)IMAGE_INFO_ADDR + (U32)((U32)IMAGE_INFO_SIZE*(U32)MAX_IMAGES))

#define NO_IMAGE                 255
#define EKSIS_LOGO_200x80        0
#define OK_60x60                 1
#define CANCEL_60x60             2
#define ULCORNER_34x34           3
#define URCORNER_34x34           4
#define DLCORNER_34x34           5
#define DRCORNER_34x34           6
#define SETTINGS_60x60           7
#define MAGNIFIER_60x60          8
#define RELAY_34x34              9
#define CURRENT_34x34            10
#define SWITCH_60x60             11
#define CURRENT_ON_60x60         12
#define RELAY_UP_60x60           13
#define DELETE_30x30             14
#define RELAY_DOWN_60x60         15
#define CURRENT_OFF_60x60        16
#define PLAY_60x60               17
#define STOP_60x60               18
#define PAUSE_60x60              19
#define MAGNIFIER_34x34          20
#define BACK_60x60               21
#define BACKWARD_60x60           22
#define FORWARD_60x60            23
#define PLAY_GRAYED_60x60        24
#define STOP_GRAYED_60x60        25
#define PAUSE_GRAYED_60x60       26
#define SETTINGS_CHANNEL_60x60   27
#define SETTINGS_RELAY_60x60     28
#define SETTINGS_CURRENT_60x60   29
#define PARAMETER_34x34          30
#define SETTINGS_PARAM_60x60     31
#define INFO_PARAM_60x60         32
#define INFO_MENU_60x60          33
#define CONNECTION_MENU_60x60    34
#define STATISTIC_MENU_60x60     35
#define OTHER_MENU_60x60         36
#define PROBE_60x60              37

// Прототипы; коментарии в TFT.c
void TFT_SetArea(pInt16U sX, pInt16U eX, pInt16U sY, pInt16U eY);
void TFT_WritePixelsAt(Int16U sX, Int16U eX, Int16U sY, Int16U eY, pInt16U pPixels);
void TFT_FillScreen(Int16U Color);
void TFT_FillRect(Int16U sX, Int16U eX, Int16U sY, Int16U eY, Int16U Color);
void TFT_FillRect_(RECT rect, Int16U color);
void TFT_FillRect_Offset(RECT rect, Int16U color, Int8S x_offset, Int8S y_offset);
void TFT_FillRect_Shift(RECT rect, Int16U color, Int8S x_shift, Int8S y_shift);
void TFT_DrawRect(RECT r);
void TFT_DrawRectWithColor(RECT r, Int16U color);
void TFT_GrayoutRect(Int16U sX, Int16U eX, Int16U sY, Int16U eY, Int16U color);
void TFT_GrayoutRect_(RECT rect, Int16U color);
void TFT_UnderlineRect_Offset(RECT rect, Int16U color, Int8U line, Int8S x_offset, Int8S y_offset);
void TFT_DrawRoundedRect(Int16U sX, Int16U eX, Int16U sY, Int16U eY, Int16U head_color, Int16U body_color, Int16U back_color);
void TFT_DrawRoundedRectEx(Int16U sX, Int16U eX, Int16U sY, Int16U eY, Int16U head_color, Int16U body_color, Int16U back_color, Int8U rects_mask);
void TFT_DrawRoundedRect_(RECT rect, Int16U head_color, Int16U body_color, Int16U back_color);
void TFT_DrawRoundedRectEx_(RECT rect, Int16U head_color, Int16U body_color, Int16U back_color, Int8U rects_mask);
void TFT_DrawRoundedRect_TwoColored(RECT rect, Int16U top_color, Int16U bottom_color, Int16U back_color);
void TFT_DrawRoundedRect_Offset(RECT rect, Int16U head_color, Int16U body_color, Int16U back_color, Int8S x_offset, Int8S y_offset);
void TFT_DrawRoundedRect_OffsetWithImageAndCaption(RECT rect, Int16U head_color, Int16U body_color, Int16U back_color, Int8S x_offset, Int8S y_offset, U32 image_index, char* caption);
void TFT_DrawRoundedRectEx_Offset(RECT rect, Int16U head_color, Int16U body_color, Int16U back_color, Int8S x_offset, Int8S y_offset, Int8U rects_mask);
void TFT_DrawBorder(Int16U sX, Int16U eX, Int16U sY, Int16U eY, Int16U width, Int16U color);
void TFT_DrawBorder_(RECT rect, Int16U width, Int16U color);
void TFT_DrawBorder_WithOffset(RECT rect, Int16U width, Int16U color, Int8S x_offset, Int8S y_offset);
void TFT_WriteChar(const tImage* ch, Int16U FontColor, Int16U BackColor, Int16U sX, Int16U sY);
void TFT_WriteString(char* st, Int16U FontColor, Int16U BackColor, Int16U sX, Int16U sY, const tFont* Font);
void TFT_WriteStringInRectLeft(char* st, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
void TFT_WriteStringInRectCenter(char* st, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
void TFT_WriteStringInRectRight(char* st, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
void TFT_WriteStringT(pU8 st);
int  TFT_StringWidth(char* st, const tFont* Font);
int  TFT_StringHeight(char* st, const tFont* Font);
void TFT_WaitForCoords(Int16U* X, Int16U* Y);
bool TFT_Touched(Int16U* X, Int16U* Y);
bool TFT_CoordsInRect(Int16U X, Int16U Y, RECT r);
void TFT_DrawImage(Int32U* data_addr, Int16U sX, Int16U sY, Int16U width, Int16U height);
void TFT_DrawImageSwapColor(Int32U* data_addr, Int16U sX, Int16U sY, Int16U width, Int16U height, Int16U color_to_swap, Int16U color);
void TFT_DrawImageSwapTwoColors(Int32U* data_addr, Int16U sX, Int16U sY, Int16U width, Int16U height, Int16U color1_to_swap, Int16U color1, Int16U color2_to_swap, Int16U color2);
void TFT_DrawImageSwapThreeColors(Int32U* data_addr, Int16U sX, Int16U sY, Int16U width, Int16U height, Int16U color1_to_swap, Int16U color1, Int16U color2_to_swap, Int16U color2, Int16U color3_to_swap, Int16U color3);
void TFT_DrawMessageBox(char* caption, char* line1, char* line2, char* line3, Int8U type, Int16U background_color);
void TFT_DrawKeyboard(char* topic, char* title, bool is_unsigned, bool is_float, bool is_ip);
void TFT_WriteStringInRectCenter(char* st, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
void TFT_WriteStringInRectCenterInTwoLines(char* line1, char* line2, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
void TFT_WriteStringInRectLeft(char* st, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
void TFT_WriteStringInRectLeftInTwoLines(char* line1, char* line2, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
void TFT_WriteStringInRectRight(char* st, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);

void GUI_DrawImage(U32 image_index, Int16U sX, Int16U sY);
void GUI_DrawImageSwapColor(U32 image_index, Int16U sX, Int16U sY, Int16U color_to_swap, Int16U color);
void GUI_DrawImageSwapTwoColors(U32 image_index, Int16U sX, Int16U sY, Int16U color1_to_swap, Int16U color1, Int16U color2_to_swap, Int16U color2);
void GUI_DrawImageSwapThreeColors(U32 image_index, Int16U sX, Int16U sY, Int16U color1_to_swap, Int16U color1, Int16U color2_to_swap, Int16U color2, Int16U color3_to_swap, Int16U color3);
void GUI_DrawImageInRect(U32 image_index, RECT rect);
void GUI_DrawImageInRectCenterSwapColor(U32 image_index, RECT rect, Int16U color_to_swap, Int16U color);
void GUI_DrawImageInRectCenterSwapColor_Shift(U32 image_index, RECT rect, Int16U color_to_swap, Int16U color, Int8S x_shift, Int8S y_shift);
void GUI_DrawTextAndImageInRectCenter(char* st, Int16U FontColor, Int16U BackColor, const tFont* font, U32 image_index, RECT rect, Int16U color_to_swap, Int16U color);
bool GUI_MessageBox(char* caption, char* line1, char* line2, char* line3, Int8U timeout, Int8U type, Int16U background_color);
void strdel_(char *st, char c);
bool GUI_KeyboardChar(char* value, char* topic, char* title, bool is_unsigned, bool is_float, bool is_ip);
bool GUI_KeyboardInt(int* value, char* topic, char* title, bool is_unsigned, int min, int max);
bool GUI_KeyboardFloat(float* value, char* topic, char* title, bool is_unsigned, float min, float max, int decimal);
bool GUI_KeyboardIP(Int32U* value, char* topic, char* title);

void GUI_ShowNotification();

void GUI_SetGUI(void (*draw_function)(void), void (*update_function)(void), void (*touch_function)(void));
void GUI_CalibTouch();

void GUI_LogoScreen();

void GUI_DrawHeader(Int8U main_button, bool main_active, Int8U left_button, bool left_active, Int8U right_button, bool right_active, char* topic, char* title);
void GUI_DrawHeaderWide(Int8U main_button, bool main_active, char* topic, char* title);
void GUI_DrawHeaderMainButton(Int8U main_button, bool main_active);
void GUI_DrawHeaderLButton(Int8U left_button, bool left_active);
void GUI_DrawHeaderRButton(Int8U right_button, bool right_active);
void GUI_DrawHeaderLRButtons(Int8U left_button, bool left_active, Int8U right_button, bool right_active);
void GUI_UpdateHeader();
void GUI_UpdateHeaderWide();
void GUI_UpdateHeaderText(char* topic, char* title);
void GUI_UpdateHeaderWideText(char* topic, char* title);
void GUI_DrawTime();
void GUI_DrawStatusIcons();

void GUI_DrawMeasuresScreen();
void GUI_UpdateMeasuresScreen();
void GUI_TouchMeasuresScreen();
void GUI_DrawMeasuresAll();
void GUI_UpdateMeasuresAll();
void GUI_TouchMeasuresAll(Int16U X, Int16U Y);
void GUI_DrawMeasuresChannel();
void GUI_UpdateMeasuresChannel();
void GUI_TouchMeasuresChannel(Int16U X, Int16U Y);
void GUI_DrawMeasuresChannelOne();
void GUI_UpdateMeasuresChannelOne();
void GUI_TouchMeasuresChannelOne(Int16U X, Int16U Y);

void GUI_DrawPumpControlPanel();
void GUI_UpdatePumpControlPanel();
void GUI_TouchPumpControlPanel(Int16U X, Int16U Y);

void GUI_DrawChannelSettingsRootScreen();
void GUI_UpdateChannelSettingsRootScreen();
void GUI_TouchChannelSettingsRootScreen();
void GUI_DrawChannelSettingsDisplayScreen();
void GUI_UpdateChannelSettingsDisplayScreen();
void GUI_ManualUpdateChannelSettingsDisplayScreen();
void GUI_TouchChannelSettingsDisplayScreen();
void GUI_DrawChannelSettingsCalculateHumidityScreen();
void GUI_UpdateChannelSettingsCalculateHumidityScreen();
void GUI_ManualUpdateChannelSettingsCalculateHumidityScreen();
void GUI_TouchChannelSettingsCalculateHumidityScreen();
void GUI_DrawChannelSettingsCalculatePressureScreen();
void GUI_UpdateChannelSettingsCalculatePressureScreen();
void GUI_ManualUpdateChannelSettingsCalculatePressureScreen();
void GUI_TouchChannelSettingsCalculatePressureScreen();

void GUI_DrawParamRootScreen();
void GUI_UpdateParamRootScreen();
void GUI_TouchParamRootScreen();

void GUI_DrawParamThreshAndConstScreen();
void GUI_UpdateParamThreshAndConstScreen();
void GUI_TouchParamThreshAndConstScreen();

void GUI_DrawParamStateScreen();
void GUI_UpdateParamStateScreen();
void GUI_TouchParamStateScreen();

void GUI_DrawThresholdConfigScreen();
void GUI_UpdateThresholdConfigScreen();
void GUI_TouchThresholdConfigScreen();

void GUI_DrawControlsScreen();
void GUI_UpdateControlsScreen();
void GUI_TouchControlsScreen();
void GUI_DrawDetailControls();
void GUI_UpdateDetailControls();
void GUI_DrawBriefControls();
void GUI_UpdateBriefControls();

void GUI_DrawControlScreen();
void GUI_UpdateControlScreen();
void GUI_TouchControlScreen();

void GUI_DrawControlRelayManualScreen();
void GUI_UpdateControlRelayManualScreen();
void GUI_TouchControlRelayManualScreen(Int16U X, Int16U Y);
void GUI_TouchControlCurrentManualScreen(Int16U X, Int16U Y);
void GUI_DrawControlRelayLogicScreen();
void GUI_UpdateControlRelayLogicScreen();
void GUI_TouchControlRelayLogicScreen(Int16U X, Int16U Y);
void GUI_DrawControlRelayHesterScreen();
void GUI_UpdateControlRelayHesterScreen();
void GUI_TouchControlRelayHesterScreen(Int16U X, Int16U Y);
void GUI_DrawControlRelayHesterNoProgrammScreen();
void GUI_UpdateControlRelayHesterNoProgrammScreen();
void GUI_TouchControlRelayHesterNoProgrammScreen(Int16U X, Int16U Y);
void GUI_DrawControlRelayHesterProgrammScreen();
void GUI_UpdateControlRelayHesterProgrammScreen();
void GUI_TouchControlRelayHesterProgrammScreen(Int16U X, Int16U Y);

void GUI_DrawControlCurrentManualScreen();
void GUI_UpdateControlCurrentManualScreen();
void GUI_TouchControlCurrentManualScreen(Int16U X, Int16U Y);
void GUI_DrawControlCurrentLogicScreen();
void GUI_UpdateControlCurrentLogicScreen();
void GUI_TouchControlCurrentLogicScreen(Int16U X, Int16U Y);
void GUI_DrawControlCurrentLineoutScreen();
void GUI_UpdateControlCurrentLineoutScreen();
void GUI_TouchControlCurrentLineoutScreen(Int16U X, Int16U Y);

void GUI_DrawRelayConfigurationCommonScreen();
void GUI_UpdateRelayConfigurationCommonScreen();
void GUI_ManualUpdateRelayConfigurationCommonScreen();
void GUI_TouchRelayConfigurationCommonScreen();
void GUI_DrawRelayConfigurationLogicScreen();
void GUI_UpdateRelayConfigurationLogicScreen();
void GUI_ManualUpdateRelayConfigurationLogicScreen();
void GUI_TouchRelayConfigurationLogicScreen();
void GUI_DrawRelayConfigurationHesterScreen();
void GUI_UpdateRelayConfigurationHesterScreen();
void GUI_ManualUpdateRelayConfigurationHesterScreen();
void GUI_TouchRelayConfigurationHesterScreen();
void GUI_DrawRelayConfigurationHesterParamScreen();
void GUI_UpdateRelayConfigurationHesterParamScreen();
void GUI_ManualUpdateRelayConfigurationHesterParamScreen();
void GUI_TouchRelayConfigurationHesterParamScreen();
void GUI_DrawRelayConfigurationProgrammScreen();
void GUI_UpdateRelayConfigurationProgrammScreen();
void GUI_ManualUpdateRelayConfigurationProgrammScreen();
void GUI_TouchRelayConfigurationProgrammScreen();

void GUI_DrawProgrammStepScreen();
void GUI_UpdateProgrammStepScreen();
void GUI_ManualUpdateProgrammStepScreen();
void GUI_TouchProgrammStepScreen();

void GUI_DrawCurrentConfigurationCommonScreen();
void GUI_UpdateCurrentConfigurationCommonScreen();
void GUI_ManualUpdateCurrentConfigurationCommonScreen();
void GUI_TouchCurrentConfigurationCommonScreen();
void GUI_DrawCurrentConfigurationLogicScreen();
void GUI_UpdateCurrentConfigurationLogicScreen();
void GUI_ManualUpdateCurrentConfigurationLogicScreen();
void GUI_TouchCurrentConfigurationLogicScreen();
void GUI_DrawCurrentConfigurationLineoutScreen();
void GUI_UpdateCurrentConfigurationLineoutScreen();
void GUI_ManualUpdateCurrentConfigurationLineoutScreen();
void GUI_TouchCurrentConfigurationLineoutScreen();

void GUI_DrawSettingsRootScreen();
void GUI_UpdateSettingsRootScreen();
void GUI_TouchSettingsRootScreen();

void GUI_DrawSettingsInfoScreen();
void GUI_UpdateSettingsInfoScreen();
void GUI_TouchSettingsInfoScreen();

void GUI_DrawSettingsConnectionScreen();
void GUI_UpdateSettingsConnectionScreen();
void GUI_TouchSettingsConnectionScreen();
void GUI_DrawSettingsRSScreen();
void GUI_UpdateSettingsRSScreen();
void GUI_TouchSettingsRSScreen();
void GUI_DrawSettingsTCPScreen();
void GUI_UpdateSettingsTCPScreen();
void GUI_TouchSettingsTCPScreen();

void GUI_DrawSettingsStatisticScreen();
void GUI_UpdateSettingsStatisticScreen();
void GUI_TouchSettingsStatisticScreen();

void GUI_DrawSettingsOtherScreen1();
void GUI_UpdateSettingsOtherScreen1();
void GUI_TouchSettingsOtherScreen1();
void GUI_DrawSettingsOtherScreen2();
void GUI_UpdateSettingsOtherScreen2();
void GUI_TouchSettingsOtherScreen2();

void GUI_DrawSettingsDateScreen();
void GUI_UpdateSettingsDateScreen();
void GUI_TouchSettingsDateScreen();
void GUI_DrawSettingsTimeScreen();
void GUI_UpdateSettingsTimeScreen();
void GUI_TouchSettingsTimeScreen();

void GUI_DrawSettingsSoundsScreen();
void GUI_UpdateSettingsSoundsScreen();
void GUI_TouchSettingsSoundsScreen();

void GUI_DrawSettingsWebServerScreen();
void GUI_UpdateSettingsWebServerScreen();
void GUI_TouchSettingsWebServerScreen();

void  load_SD_TFT                   (U32 PixNum, U32 SDaddress);
void  write_control_TFT             (U8 adr, U16 data);
void  write_GRAM_TFT                (U32 pixnum, pU16 buffer);
void  fill_GRAM_TFT                 (U32 bytenum, U16 content);
void  start_read_GRAM_TFT           (void);

void  stop_read_GRAM_TFT            (void);
void  save_TFT_SD                   (U32 SDaddress, U32 TFTaddress, U32 PixNum);
void  ScreenShoot                   (U32 SDaddress);
void  TFT_Read                      (pU16 sourceMemory, U32 TFTaddress, U32 NumRead);
void  IncTFTAdr                     (U32 * TFTaddress, U32 Inc);

void TFT_Task();

extern OS_TASK              OS_TFT;

extern const tFont Tahoma22;
extern const tFont Tahoma34;
extern const tFont Tahoma87;

extern COLOR_SCHEME tft_color_scheme;

extern const char* PARAM_NAMES[];
extern const char* PARAM_TYPES[];
extern const char* UNIT_TYPES[];

#define RS_TFT_LOW              GPIOD_BRR=  (1<<2)
#define RS_TFT_HIGH             GPIOD_BSRR= (1<<2)
#define CS_TFT_LOW              GPIOD_BRR=  (1<<7)
#define CS_TFT_HIGH             GPIOD_BSRR= (1<<7)
#define RD_TFT_LOW              { GPIOD_BRR=  (1<<0); __no_operation(); __no_operation(); __no_operation(); __no_operation(); __no_operation(); __no_operation(); }
#define RD_TFT_HIGH             { GPIOD_BSRR= (1<<0); __no_operation(); __no_operation(); __no_operation(); __no_operation(); __no_operation(); __no_operation(); }
#define WR_TFT_LOW              { GPIOD_BRR=  (1<<1);   }
#define WR_TFT_HIGH             { GPIOD_BSRR= (1<<1);   }
#define RES_TFT_LOW             GPIOD_BRR=  (1<<3)
#define RES_TFT_HIGH            GPIOD_BSRR= (1<<3)
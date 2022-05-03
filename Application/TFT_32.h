#include "stdbool.h"

#define MULTIPLIER_N            35
#define DIVIDER_M               2
#define RESOLUTION_X            320
#define RESOLUTION_Y            240
#define LCDC_FPR                235928
#define H_Sync_total            950
#define H_Sync_to_DE            150  
#define H_Sync_Pluse_Wide       50
#define V_Sync_total            500
#define V_Sync_to_DE            20
#define V_Sync_Pluse_Wide       10

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
#define TAHOMA10B_HEIGHT         10
#define TAHOMA19B_HEIGHT         19
#define TAHOMA43_HEIGHT          43

#define BACKGROUND_COLOR         RGB565(242, 242, 242)
#define CALB_RECTS_COLOR         RGB565(151, 227, 143)
#define TEMPLATE_COLOR           RGB565(163, 73, 164)

// Константы окна сообщения
#define MESSAGEBOX_OK                     0
#define MESSAGEBOX_YESNO                  1
#define MESSAGEBOX_SX                     20
#define MESSAGEBOX_EX                     299
#define MESSAGEBOX_SY                     40
#define MESSAGEBOX_EY                     199
#define MESSAGEBOX_COLOR                  RGB565(250, 228, 163)
#define MESSAGEBOX_HEAD_COLOR             RGB565(230, 208, 143)
#define MESSAGEBOX_CAPTION_SX             MESSAGEBOX_SX+10
#define MESSAGEBOX_CAPTION_SY             MESSAGEBOX_SY
#define MESSAGEBOX_LINES_SX               MESSAGEBOX_SX+5
#define MESSAGEBOX_LINE1_SY               MESSAGEBOX_SY+35
#define MESSAGEBOX_LINE2_SY               MESSAGEBOX_LINE1_SY+TAHOMA19B_HEIGHT
#define MESSAGEBOX_LINE3_SY               MESSAGEBOX_LINE2_SY+TAHOMA19B_HEIGHT
#define MESSAGEBOX_LBUTTON_SX             MESSAGEBOX_SX+10+20
#define MESSAGEBOX_LBUTTON_EX             MESSAGEBOX_LBUTTON_SX+90-1
#define MESSAGEBOX_RBUTTON_SX             MESSAGEBOX_EX-90-20+1
#define MESSAGEBOX_RBUTTON_EX             MESSAGEBOX_RBUTTON_SX+90-1
#define MESSAGEBOX_CBUTTON_SX             115
#define MESSAGEBOX_CBUTTON_EX             MESSAGEBOX_CBUTTON_SX+90-1
#define MESSAGEBOX_BUTTONS_SY             MESSAGEBOX_SY+120
#define MESSAGEBOX_BUTTONS_EY             MESSAGEBOX_BUTTONS_SY+40-1

// Константы заголовка
#define HEADER_SX                TOPIC_SX
#define HEADER_EX                RESOLUTION_X-1
#define HEADER_SY                0
#define HEADER_EY                TITLES_EY
#define HEADER_COLOR             WHITE
#define HEADER_ICONS_SY          0
#define HEADER_ICONS_HEIGHT      20

// Константы тела главного экрана
#define BODY_SX                  0
#define BODY_EX                  RESOLUTION_X-1
#define BODY_SY                  HEADER_EY+1
#define BODY_EY                  282
#define BODY_COLOR               BACKGROUND_COLOR

#define BODY_RECTS_SX            BODY_SX
#define BODY_RECTS_EX            BODY_EX

#define BODY_ICONS_SX            10
#define BODY_CAPTIONS_SX         30
#define BODY_ERROR_ICON_SX       210

#define BODY_BIG_PARAMS_UNIT_SX          120
#define BODY_BIG_PARAMS_VALUE_EX         110
#define BODY_SMALL_PARAMS_UNIT_SX        170
#define BODY_SMALL_PARAMS_VALUE_EX       160
#define BODY_SMALL_CAPTIONS_SX           15

#define ODD_BLUE                       RGB565(133, 215, 242)
#define EVEN_BLUE                      RGB565(153, 235, 255)
#define ODD_GREEN                      RGB565(151, 227, 143)
#define EVEN_GREEN                     RGB565(171, 247, 163)
#define ODD_GRAY                       RGB565(202, 202, 202)
#define EVEN_GRAY                      RGB565(222, 222, 222)

#define EVEN_YELLOW                    RGB565(215, 235, 133)

#define KEY_COLOR                   RGB565(250, 250, 250)

#define KEY_LLBUTTONS_SX            0
#define KEY_LLBUTTONS_EX            59
#define KEY_LBUTTONS_SX             60
#define KEY_LBUTTONS_EX             119
#define KEY_RBUTTONS_SX             120
#define KEY_RBUTTONS_EX             179
#define KEY_RRBUTTONS_SX            180
#define KEY_RRBUTTONS_EX            239
#define KEY_UUBUTTONS_SY            80
#define KEY_UUBUTTONS_EY            139
#define KEY_UBUTTONS_SY             140
#define KEY_UBUTTONS_EY             199
#define KEY_DBUTTONS_SY             200
#define KEY_DBUTTONS_EY             259
#define KEY_DDBUTTONS_SY            260
#define KEY_DDBUTTONS_EY            319

#define KEY_LLBUTTONS_TEXT_SX       KEY_LLBUTTONS_SX+25
#define KEY_LBUTTONS_TEXT_SX        KEY_LBUTTONS_SX+25
#define KEY_RBUTTONS_TEXT_SX        KEY_RBUTTONS_SX+25
#define KEY_RRBUTTONS_TEXT_SX       KEY_RRBUTTONS_SX+25
#define KEY_UUBUTTONS_TEXT_SY       KEY_UUBUTTONS_SY+20
#define KEY_UBUTTONS_TEXT_SY        KEY_UBUTTONS_SY+20
#define KEY_DBUTTONS_TEXT_SY        KEY_DBUTTONS_SY+20
#define KEY_DDBUTTONS_TEXT_SY       KEY_DDBUTTONS_SY+20

#define KEY_VALUE_SY                35

#define CHANNELS_SX                 0
#define CHANNELS_EX                 79
#define CHANNEL0_SY                 0
#define CHANNEL0_EY                 47
#define CHANNEL1_SY                 48
#define CHANNEL1_EY                 95
#define CHANNEL2_SY                 96
#define CHANNEL2_EY                 143
#define CHANNEL3_SY                 144
#define CHANNEL3_EY                 191
#define CHANNEL4_SY                 192
#define CHANNEL4_EY                 239

#define TOPIC_SX                    80
#define TOPIC_EX                    269
#define TIME_SX                     270
#define TIME_EX                     319
#define CAPTIONS_SY                 0
#define CAPTIONS_EY                 23

#define LTITLE_SX                   80
#define LTITLE_EX                   199
#define RTITLE_SX                   200
#define RTITLE_EX                   319
#define TITLES_SY                   24
#define TITLES_EY                   47

#define LEFT_PARAMS_SX              LTITLE_SX
#define LEFT_PARAMS_EX              LTITLE_EX
#define CENTER_PARAMS_SX            140
#define CENTER_PARAMS_EX            259
#define RIGHT_PARAMS_SX             RTITLE_SX
#define RIGHT_PARAMS_EX             RTITLE_EX
#define UPPERMOST_PARAMS_SY         CHANNEL1_SY
#define UPPERMOST_PARAMS_EY         CHANNEL1_EY
#define UPPER_PARAMS_SY             CHANNEL2_SY
#define UPPER_PARAMS_EY             CHANNEL2_EY
#define LOWER_PARAMS_SY             CHANNEL3_SY
#define LOWER_PARAMS_EY             CHANNEL3_EY
#define LOWERMOST_PARAMS_SY         CHANNEL4_SY
#define LOWERMOST_PARAMS_EY         CHANNEL4_EY

#define UPPERMOST_UPPER_PARAMS_SY   48
#define UPPERMOST_UPPER_PARAMS_EY   71
#define UPPERMOST_LOWER_PARAMS_SY   72
#define UPPERMOST_LOWER_PARAMS_EY   95
#define UPPER_UPPER_PARAMS_SY       96
#define UPPER_UPPER_PARAMS_EY       119
#define UPPER_LOWER_PARAMS_SY       120
#define UPPER_LOWER_PARAMS_EY       143
#define LOWER_UPPER_PARAMS_SY       144
#define LOWER_UPPER_PARAMS_EY       167
#define LOWER_LOWER_PARAMS_SY       168
#define LOWER_LOWER_PARAMS_EY       191
#define LOWERMOST_UPPER_PARAMS_SY   192
#define LOWERMOST_UPPER_PARAMS_EY   215
#define LOWERMOST_LOWER_PARAMS_SY   216
#define LOWERMOST_LOWER_PARAMS_EY   239

#define LEFTHALF_PARAMS_SX          0
#define LEFTHALF_PARAMS_EX          159
#define RIGHTHALF_PARAMS_SX         160
#define RIGHTHALF_PARAMS_EX         319

#define UPPER_CONTROLS_SY           48
#define UPPER_CONTROLS_EY           143
#define LOWER_CONTROLS_SY           144
#define LOWER_CONTROLS_EY           239
#define LEFT_CONTROLS_SX            0
#define LEFT_CONTROLS_EX            105
#define CENTRAL_CONTROLS_SX         107
#define CENTRAL_CONTROLS_EX         212
#define RIGHT_CONTROLS_SX           214
#define RIGHT_CONTROLS_EX           319

#define UPPER_CONTROLS_TOP_SY        UPPER_CONTROLS_SY+20
#define UPPER_CONTROLS_TOP_EY        UPPER_CONTROLS_TOP_SY+19
#define UPPER_CONTROLS_MIDDLE_SY     UPPER_CONTROLS_TOP_EY+5
#define UPPER_CONTROLS_MIDDLE_EY     UPPER_CONTROLS_MIDDLE_SY+19
#define UPPER_CONTROLS_BOTTOM_SY     UPPER_CONTROLS_MIDDLE_EY+5
#define UPPER_CONTROLS_BOTTOM_EY     UPPER_CONTROLS_BOTTOM_SY+19
#define LOWER_CONTROLS_TOP_SY        LOWER_CONTROLS_SY+20
#define LOWER_CONTROLS_TOP_EY        LOWER_CONTROLS_TOP_SY+19
#define LOWER_CONTROLS_MIDDLE_SY     LOWER_CONTROLS_TOP_EY+5
#define LOWER_CONTROLS_MIDDLE_EY     LOWER_CONTROLS_MIDDLE_SY+19
#define LOWER_CONTROLS_BOTTOM_SY     LOWER_CONTROLS_MIDDLE_EY+5
#define LOWER_CONTROLS_BOTTOM_EY     LOWER_CONTROLS_BOTTOM_SY+19

#define BOTTOM_BUTTONS_SY           196
#define BOTTOM_BUTTONS_EY           RESOLUTION_Y-1
#define BOTTOM_SMALLBUTTONS_SY      220
#define BOTTOM_SMALLBUTTONS_EY      RESOLUTION_Y-1

#define LEFTMOST_KEYS_SX            0
#define LEFTMOST_KEYS_EX            79
#define LEFT_KEYS_SX                80
#define LEFT_KEYS_EX                159
#define RIGHT_KEYS_SX               160
#define RIGHT_KEYS_EX               239
#define RIGHTMOST_KEYS_SX           240
#define RIGHTMOST_KEYS_EX           319

#define UPPERMOST_KEYS_SY           48
#define UPPERMOST_KEYS_EY           95
#define UPPER_KEYS_SY               96
#define UPPER_KEYS_EY               143
#define LOWER_KEYS_SY               144
#define LOWER_KEYS_EY               191
#define LOWERMOST_KEYS_SY           192
#define LOWERMOST_KEYS_EY           239

// Константы изображений
#define MAX_IMAGES               128                                                   // Максимальное количество картинок
#define IMAGE_INFO_ADDR          (U32)SD_IMAGES_OFFSET                                      // Адрес информации о картинках
#define IMAGE_INFO_SIZE          16                                                    // Размер информации об одной картинке
#define IMAGE_INFO_HEADER_SIZE   (U32)((U32)IMAGE_INFO_ADDR + (U32)((U32)IMAGE_INFO_SIZE*(U32)MAX_IMAGES))

#define NO_IMAGE                 255
#define EKSIS_LOGO_320x240       0
#define OK_30x30                 1
#define CANCEL_30x30             2
#define ULCORNER_19x19           3
#define URCORNER_19x19           4
#define DLCORNER_19x19           5
#define DRCORNER_19x19           6
#define SETTINGS_30x30           7
#define MAGNIFIER_30x30          8
#define RELAY_19x19              9
#define CURRENT_19x19            10
#define SWITCH_30x30             11
#define CURRENT_ON_30x30         12
#define RELAY_UP_30x30           13
#define DELETE_30x30             14
#define RELAY_DOWN_30x30         15
#define CURRENT_OFF_30x30        16
#define PLAY_30x30               17
#define STOP_30x30               18
#define PAUSE_30x30              19
#define MAGNIFIER_19x19          20
#define BACK_30x30               21
#define INFO_MENU_30x30          22
#define CONNECTION_MENU_30x30    23
#define STATISTIC_MENU_30x30     24
#define OTHER_MENU_30x30         25
#define SETTINGS_CHANNEL_30x30   26
#define SETTINGS_RELAY_30x30     27
#define SETTINGS_CURRENT_30x30   28
#define PARAMETER_19x19          29
#define SETTINGS_PARAM_30x30     30
#define INFO_PARAM_30x30         31
#define PROBE_30x30              32

// Прототипы; коментарии в TFT.c
void TFT_SetCoords(Int16U X, Int16U Y);
void TFT_SetArea(Int16U sX, Int16U eX, Int16U sY, Int16U eY);
void TFT_WritePixel(Int16U X, Int16U Y, Int16U Pixel);
void TFT_WritePixels(Int16U X, Int16U Y, pInt16U pPixels, Int16U Count);
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
const tImage* TFT_GetCharacterImage(char code, const tFont* Font);
RECT TFT_WriteString(char* st, Int16U FontColor, Int16U BackColor, Int16U sX, Int16U sY, const tFont* Font);
RECT TFT_WriteStringInRectLeft(char* st, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
RECT TFT_WriteStringInRectCenter(char* st, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
RECT TFT_WriteStringInRectRight(char* st, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
void TFT_WriteStringT(pU8 st);
int  TFT_StringWidth(char* st, const tFont* Font);
int  TFT_StringHeight(char* st, const tFont* Font);
void TFT_WaitForCoords(Int16U* X, Int16U* Y);
bool TFT_Touched(Int16U* X, Int16U* Y);
Int8U TFT_Pressed();
bool TFT_CoordsInRect(Int16U X, Int16U Y, RECT r);
void TFT_DrawImage(Int32U* data_addr, Int16U sX, Int16U sY, Int16U width, Int16U height);
void TFT_DrawImageSwapColor(Int32U* data_addr, Int16U sX, Int16U sY, Int16U width, Int16U height, Int16U color_to_swap, Int16U color);
void TFT_DrawImageSwapTwoColors(Int32U* data_addr, Int16U sX, Int16U sY, Int16U width, Int16U height, Int16U color1_to_swap, Int16U color1, Int16U color2_to_swap, Int16U color2);
void TFT_DrawImageSwapThreeColors(Int32U* data_addr, Int16U sX, Int16U sY, Int16U width, Int16U height, Int16U color1_to_swap, Int16U color1, Int16U color2_to_swap, Int16U color2, Int16U color3_to_swap, Int16U color3);
void TFT_DrawMessageBox(char* caption, char* line1, char* line2, char* line3, Int8U type, Int16U background_color);
void TFT_DrawKeyboard(char* topic, char* title, bool is_unsigned, bool is_float, bool is_ip);
RECT TFT_WriteStringInRectCenter(char* st, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
RECT TFT_WriteStringInRectCenterInTwoLines(char* line1, char* line2, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
RECT TFT_WriteStringInRectLeft(char* st, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
RECT TFT_WriteStringInRectLeftInTwoLines(char* line1, char* line2, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);
RECT TFT_WriteStringInRectRight(char* st, Int16U FontColor, Int16U BackColor, RECT rect, Int8S x_offset, Int8S y_offset, const tFont* Font);

void GUI_DrawImage(U32 image_index, Int16U sX, Int16U sY);
void GUI_DrawImageSwapColor(U32 image_index, Int16U sX, Int16U sY, Int16U color_to_swap, Int16U color);
void GUI_DrawImageSwapTwoColors(U32 image_index, Int16U sX, Int16U sY, Int16U color1_to_swap, Int16U color1, Int16U color2_to_swap, Int16U color2);
void GUI_DrawImageSwapThreeColors(U32 image_index, Int16U sX, Int16U sY, Int16U color1_to_swap, Int16U color1, Int16U color2_to_swap, Int16U color2, Int16U color3_to_swap, Int16U color3);
void GUI_DrawImageInRect(U32 image_index, RECT rect);
void GUI_DrawImageInRectCenter(U32 image_index, RECT rect);
void GUI_DrawImageInRectCenterSwapColor(U32 image_index, RECT rect, Int16U color_to_swap, Int16U color);
void GUI_DrawImageInRectCenterSwapColor_Shift(U32 image_index, RECT rect, Int16U color_to_swap, Int16U color, Int8S x_shift, Int8S y_shift);
void GUI_DrawTextAndImageInRectCenter(char* st, Int16U FontColor, Int16U BackColor, const tFont* font, U32 image_index, RECT rect, Int16U color_to_swap, Int16U color);
void GUI_FillImageRect(U32 image_index, Int16U sX, Int16U sY, Int16U color);
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

void GUI_DrawHeader(Int8U button_image, Int16U back_color, char* topic, char* title);
void GUI_DrawHeaderGrayoutButton(Int8U button_image, Int16U back_color, char* topic, char* title);
void GUI_UpdateHeader();
void GUI_UpdateHeaderImage(Int8U button_image, Int16U back_color);
void GUI_UpdateHeaderText(char* topic, char* title);
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
void GUI_TouchChannelSettingsDisplayScreen();
void GUI_DrawChannelSettingsCalculateHumidityScreen();
void GUI_UpdateChannelSettingsCalculateHumidityScreen();
void GUI_TouchChannelSettingsCalculateHumidityScreen();
void GUI_DrawChannelSettingsCalculateOxygenScreen();
void GUI_UpdateChannelSettingsCalculateOxygenScreen();
void GUI_TouchChannelSettingsCalculateOxygenScreen();
void GUI_DrawChannelSettingsCalculatePressureScreen();
void GUI_UpdateChannelSettingsCalculatePressureScreen();
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
void GUI_TouchRelayConfigurationCommonScreen();
void GUI_DrawRelayConfigurationLogic1Screen();
void GUI_UpdateRelayConfigurationLogic1Screen();
void GUI_TouchRelayConfigurationLogic1Screen();
void GUI_DrawRelayConfigurationLogic2Screen();
void GUI_UpdateRelayConfigurationLogic2Screen();
void GUI_TouchRelayConfigurationLogic2Screen();
void GUI_DrawRelayConfigurationHesterScreen();
void GUI_UpdateRelayConfigurationHesterScreen();
void GUI_TouchRelayConfigurationHesterScreen();
void GUI_DrawRelayConfigurationHesterParamScreen();
void GUI_UpdateRelayConfigurationHesterParamScreen();
void GUI_TouchRelayConfigurationHesterParamScreen();
void GUI_DrawRelayConfigurationProgrammCommonScreen();
void GUI_UpdateRelayConfigurationProgrammCommonScreen();
void GUI_TouchRelayConfigurationProgrammCommonScreen();
void GUI_DrawRelayConfigurationProgrammDetailScreen();
void GUI_UpdateRelayConfigurationProgrammDetailScreen();
void GUI_TouchRelayConfigurationProgrammDetailScreen();

void GUI_DrawProgrammStepScreen();
void GUI_UpdateProgrammStepScreen();
void GUI_TouchProgrammStepScreen();

void GUI_DrawCurrentConfigurationCommonScreen();
void GUI_UpdateCurrentConfigurationCommonScreen();
void GUI_TouchCurrentConfigurationCommonScreen();
void GUI_DrawCurrentConfigurationLogic1Screen();
void GUI_UpdateCurrentConfigurationLogic1Screen();
void GUI_TouchCurrentConfigurationLogic1Screen();
void GUI_DrawCurrentConfigurationLogic2Screen();
void GUI_UpdateCurrentConfigurationLogic2Screen();
void GUI_TouchCurrentConfigurationLogic2Screen();
void GUI_DrawCurrentConfigurationLineoutScreen();
void GUI_UpdateCurrentConfigurationLineoutScreen();
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

void GUI_InitializeRects();

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

extern const tFont Tahoma10B;
extern const tFont Tahoma19B;
extern const tFont Tahoma43;

extern COLOR_SCHEME tft_color_scheme;

extern const char* PARAM_NAMES[];
extern const char* PARAM_TYPES[];
extern const char* UNIT_TYPES[];

#define RS_TFT_LOW              GPIOx_RESET(GPIOD, 2)
#define RS_TFT_HIGH             GPIOx_SET(GPIOD, 2)
#define CS_TFT_LOW              GPIOx_RESET(GPIOD, 7)
#define CS_TFT_HIGH             GPIOx_SET(GPIOD, 7)
#define RD_TFT_LOW              { GPIOx_RESET(GPIOD, 0); __no_operation(); __no_operation(); __no_operation(); __no_operation(); __no_operation(); __no_operation(); }
#define RD_TFT_HIGH             { GPIOx_SET(GPIOD, 0);   __no_operation(); __no_operation(); __no_operation(); __no_operation(); __no_operation(); __no_operation(); }
#define WR_TFT_LOW              { GPIOx_RESET(GPIOD, 1); __no_operation(); }
#define WR_TFT_HIGH             GPIOx_SET(GPIOD, 1)
#define RES_TFT_LOW             GPIOx_RESET(GPIOD, 3)
#define RES_TFT_HIGH            GPIOx_SET(GPIOD, 3)
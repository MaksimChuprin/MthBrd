#ifndef __LED_H_
#define __LED_H_

// LCD definition
#define   c_CHAR      16
#define   L_CHAR      17
#define   o_CHAR      18
#define   t_CHAR      19
#define   P_CHAR      20
#define   n_CHAR      21
#define   U_CHAR      22
#define   u_CHAR      23
#define   r_CHAR      24
#define   Y_CHAR      25
#define   H_CHAR      26
#define   l_CHAR      27
#define   N_CHAR      28
#define   LOW_LINE_C  29
#define   MINUS_C     30
#define   SPACE_C     31
#define   POINT_C     32
#define   R_CHAR      33
#define   g_CHAR      9
#define   J_CHAR      34
#define   h_CHAR      35
#define   i_CHAR      36

#define SHORTPRESS_KEYTIME      5
#define LONGPRESS_KEYTIME       100
#define CONSTPRESS_KEYTIME      200
#define SHORTPRESS_MASK         BIT13
#define LONGPRESS_MASK          BIT14
#define CONSTPRESS_MASK         BIT15

#define SET_FLASH(A)            FlashMaskDisplay|= (Int32U)(A)
#define CLR_FLASH(A)            FlashMaskDisplay&= ~(Int32U)(A)
#define CLR_ALL_FLASH           FlashMaskDisplay=  0

#define DISPLAY_LEN             16

#define KEY_1                   BIT0
#define KEY_2                   BIT1
#define KEY_3                   BIT2
#define KEY_4                   BIT3
#define KEY_5                   BIT4
#define KEY_6                   BIT5

#define   ABORT                 4
#define   FAIL_KEY              3
#define   NO_KEY                2
#define   ENTR                  0

void led_init ();

void led_timer ();
extern OS_TIMER LEDTimer;

//Название
//Есть индикатор "Параметр" и светодиоды
//Есть светодиоды "Каналы управления"
//Ширина индикатора "Температура" 3 или 4
struct Device
{
  char DeviceName[32];
  Int8U param_ind_and_leds_exist;
  Int8U control_leds_exist;
  Int8U temp_ind_width;
};

// Базовые цвета
#define RED                     0xF800
#define ORANGE                  0xF9E0
#define YELLOW                  0xFE00
#define GREEN                   0x07E0
#define DARKGREEN               RGB565(60, 160, 40)
#define LIGHTGREEN              0xDFFB
#define BLUE                    0x19FF
#define DEEPBLUE                RGB565(40, 40, 160)//0x001F
#define PURPLE                  0xF86F
#define WHITE                   0xFFFF
#define BLACK                   0x0000
#define LIGHTRED                RGB565(255, 100, 100)
#define BROWN                   RGB565(200, 145, 75)

extern const char*              PARAM_NAMES[];
extern const char*              PARAM_TYPES[];
extern const char*              UNIT_TYPES[];

#define RESOLUTION_X            320
#define RESOLUTION_Y            240

extern OS_TASK                  OS_TFT;

void strdel_(char *st, char c);

#endif
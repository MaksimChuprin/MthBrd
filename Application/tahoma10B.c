/*******************************************************************************
* filename: H:/Projects/IVG-1_TFT/Application/Tahoma10B.xml
* name: Tahoma10B
* family: Tahoma
* size: 8
* style: Bold
* included characters: *0123456789
* antialiasing: no
* type: proportional
* encoding: Windows-1251
* unicode bom: no
*
* preset name: Monochrome
* data block size: 8 bit(s), U8
* RLE compression enabled: no
* conversion type: Monochrome, Diffuse Dither 128
* bits per pixel: 1
*
* preprocess:
*  main scan direction: top to bottom
*  line scan direction: backward
*  inverse: yes
*******************************************************************************/

/*
 typedef struct {
     long int code;
     const tImage *image;
     } tChar;
 typedef struct {
     int length;
     const tChar *chars;
     } tFont;
*/

#include "defines.h"


static const U8 image_data_Tahoma10B_0x2a[10] = {
    0x00, 
    0x00, 
    0x20, 
    0x70, 
    0x70, 
    0x20, 
    0x00, 
    0x00, 
    0x00, 
    0x00
};
static const tImage Tahoma10B_0x2a = { image_data_Tahoma10B_0x2a, 5, 10};
static const U8 image_data_Tahoma10B_0x30[10] = {
    0x00, 
    0x00, 
    0x70, 
    0x58, 
    0xd8, 
    0xd8, 
    0xd8, 
    0x70, 
    0x00, 
    0x00
};
static const tImage Tahoma10B_0x30 = { image_data_Tahoma10B_0x30, 5, 10};
static const U8 image_data_Tahoma10B_0x31[10] = {
    0x00, 
    0x00, 
    0x20, 
    0x60, 
    0x20, 
    0x20, 
    0x20, 
    0x78, 
    0x00, 
    0x00
};
static const tImage Tahoma10B_0x31 = { image_data_Tahoma10B_0x31, 5, 10};
static const U8 image_data_Tahoma10B_0x32[10] = {
    0x00, 
    0x00, 
    0x70, 
    0x18, 
    0x18, 
    0x30, 
    0x60, 
    0x78, 
    0x00, 
    0x00
};
static const tImage Tahoma10B_0x32 = { image_data_Tahoma10B_0x32, 5, 10};
static const U8 image_data_Tahoma10B_0x33[10] = {
    0x00, 
    0x00, 
    0x70, 
    0x18, 
    0x30, 
    0x18, 
    0x98, 
    0xf0, 
    0x00, 
    0x00
};
static const tImage Tahoma10B_0x33 = { image_data_Tahoma10B_0x33, 5, 10};
static const U8 image_data_Tahoma10B_0x34[10] = {
    0x00, 
    0x00, 
    0x30, 
    0x30, 
    0x50, 
    0x90, 
    0xf8, 
    0x10, 
    0x00, 
    0x00
};
static const tImage Tahoma10B_0x34 = { image_data_Tahoma10B_0x34, 5, 10};
static const U8 image_data_Tahoma10B_0x35[10] = {
    0x00, 
    0x00, 
    0x78, 
    0x40, 
    0x70, 
    0x18, 
    0x98, 
    0xf0, 
    0x00, 
    0x00
};
static const tImage Tahoma10B_0x35 = { image_data_Tahoma10B_0x35, 5, 10};
static const U8 image_data_Tahoma10B_0x36[10] = {
    0x00, 
    0x00, 
    0x30, 
    0x40, 
    0xf0, 
    0xd8, 
    0x58, 
    0x70, 
    0x00, 
    0x00
};
static const tImage Tahoma10B_0x36 = { image_data_Tahoma10B_0x36, 5, 10};
static const U8 image_data_Tahoma10B_0x37[10] = {
    0x00, 
    0x00, 
    0xf8, 
    0x18, 
    0x10, 
    0x30, 
    0x60, 
    0x60, 
    0x00, 
    0x00
};
static const tImage Tahoma10B_0x37 = { image_data_Tahoma10B_0x37, 5, 10};
static const U8 image_data_Tahoma10B_0x38[10] = {
    0x00, 
    0x00, 
    0x70, 
    0xd8, 
    0x70, 
    0x78, 
    0xd8, 
    0x70, 
    0x00, 
    0x00
};
static const tImage Tahoma10B_0x38 = { image_data_Tahoma10B_0x38, 5, 10};
static const U8 image_data_Tahoma10B_0x39[10] = {
    0x00, 
    0x00, 
    0x70, 
    0xd8, 
    0xd8, 
    0x78, 
    0x10, 
    0x70, 
    0x00, 
    0x00
};
static const tImage Tahoma10B_0x39 = { image_data_Tahoma10B_0x39, 5, 10};

static const tChar Tahoma10B_array[] = {

// character: '*'
  {0x2a, &Tahoma10B_0x2a},
// character: '0'
  {0x30, &Tahoma10B_0x30},
// character: '1'
  {0x31, &Tahoma10B_0x31},
// character: '2'
  {0x32, &Tahoma10B_0x32},
// character: '3'
  {0x33, &Tahoma10B_0x33},
// character: '4'
  {0x34, &Tahoma10B_0x34},
// character: '5'
  {0x35, &Tahoma10B_0x35},
// character: '6'
  {0x36, &Tahoma10B_0x36},
// character: '7'
  {0x37, &Tahoma10B_0x37},
// character: '8'
  {0x38, &Tahoma10B_0x38},
// character: '9'
  {0x39, &Tahoma10B_0x39}
};

const tFont Tahoma10B = { 11, Tahoma10B_array };

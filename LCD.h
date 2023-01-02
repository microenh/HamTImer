/*****************************************************************************
* | File        :   LCD_1IN14_V2.h
* | Function    :   test Demo
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2021-03-16
* | Info        :   
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/pwm.h"
#include "fonts/fonts.h"

#define LCD_1IN14_V2_HEIGHT 240
#define LCD_1IN14_V2_WIDTH  135

#define LCD_RST_PIN  12
#define LCD_DC_PIN    8
#define LCD_BL_PIN   13
    
#define LCD_CS_PIN    9
#define LCD_CLK_PIN  10
#define LCD_MOSI_PIN 11
    
// GPIO pins for buttons
#define BTN_A_PIN     15 
#define BTN_B_PIN     17 
#define BTN_UP_PIN     2
#define BTN_DOWN_PIN  18
#define BTN_LEFT_PIN  16
#define BTN_RIGHT_PIN 20
#define BTN_CTRL_PIN   3

#define SPI_PORT spi1

// Note: colors are designed for the LCD to be inverted (Invert(true))
#define RGB_565(r,g,b) (((((r)) & 0xf8) << 8) | (((( g)) & 0xfc) << 3) | ((( b)) >> 3))

#define BLACK    RGB_565(0,0,0)
#define RED      RGB_565(127,0,0)    
#define BRED     RGB_565(255,0,0)    
#define GREEN    RGB_565(0,127,0)    
#define BGREEN   RGB_565(0,255,0)    
#define BLUE     RGB_565(0,0,127)    
#define BBLUE    RGB_565(0,0,255)    
#define CYAN     RGB_565(0,127,127)  
#define BCYAN    RGB_565(0,255,255)  
#define MAGENTA  RGB_565(127,0,127)  
#define BMAGENTA RGB_565(255,0,255)  
#define YELLOW   RGB_565(127,127,0)
#define BYELLOW  RGB_565(255,255,0)  
#define GRAY     RGB_565(127,127,127)
#define WHITE    RGB_565(255,255,255)
#define ORANGE   RGB_565(255,127,0)  
#define PINK     RGB_565(255,0,127)  
#define LGREEN   RGB_565(0,255,127)
#define LBLUE    RGB_565(0,127,255)

#define HORIZONTAL 0
#define VERTICAL   1

typedef struct{
    uint16_t width;
    uint16_t height;
    uint8_t  scanDir;
} LCD_Attributes;

extern LCD_Attributes lcd;
extern uint8_t slice_num;

// ------------------------------------------------------------------------------------------------------
void InitHardware(void);
void InitLCD(uint8_t scan_dir);
void DrawChar(const uint16_t x, const uint16_t y, const sFONT *font, const uint16_t foreground, const uint16_t background, const uint8_t character);
void DrawString(uint16_t x, uint16_t y, const char * pString, const sFONT* font, uint16_t foreground, uint16_t background);
void DrawSeconds(uint16_t x, uint16_t y, uint16_t seconds, const sFONT *font,
                 uint16_t foreground, uint16_t background, uint16_t prev_seconds);
void Invert(bool invert);
void ClearWindow(uint16_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
#define Clear(Color) ClearWindow(Color, 0, 0, lcd.width, lcd.height)
#define BacklightLevel(Value) pwm_set_chan_level(slice_num, PWM_CHAN_B, Value)

#ifndef NDEBUG
	#include <stdio.h>
	#define Debug(__info,...) printf("Debug: " __info,##__VA_ARGS__)
#else
	#define Debug(__info,...)  
#endif

#endif // __LCD_H
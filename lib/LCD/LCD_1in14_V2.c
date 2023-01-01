/*****************************************************************************
* | File        :   LCD_1IN14_V2.C
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
#include <stdarg.h>
#include "LCD_1in14_V2.h"
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "Debug.h"


LCD_1IN14_V2_ATTRIBUTES LCD_1IN14_V2;

/******************************************************************************
function :	Hardware reset
parameter:
******************************************************************************/
static void LCD_1IN14_V2_Reset(void)
{
    gpio_put(LCD_CS_PIN, 1);
    gpio_put(LCD_RST_PIN, 1);
    sleep_ms(100);
    gpio_put(LCD_RST_PIN, 0);
    sleep_ms(100);
    gpio_put(LCD_RST_PIN, 1);
    sleep_ms(100);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
static void LCD_1IN14_V2_SendCommand(uint8_t command)
{
    gpio_put(LCD_DC_PIN, 0);
    gpio_put(LCD_CS_PIN, 0);
    spi_write_blocking(SPI_PORT, &command, 1);
    gpio_put(LCD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void LCD_1IN14_V2_SendData_8Bit(uint8_t data)
{
    gpio_put(LCD_DC_PIN, 1);
    gpio_put(LCD_CS_PIN, 0);
    spi_write_blocking(SPI_PORT, &data, 1);
    gpio_put(LCD_CS_PIN, 1);
}

static void LCD_1IN14_V2_SendCommandData(uint8_t command, uint16_t num, ...) {
    gpio_put(LCD_DC_PIN, 0);
    gpio_put(LCD_CS_PIN, 0);
    spi_write_blocking(SPI_PORT, &command, 1);
    gpio_put(LCD_DC_PIN, 1);
    va_list valist;
    va_start(valist, num);
    for (uint16_t i = 0; i < num; i++) {
        uint8_t data = va_arg(valist, int);
        spi_write_blocking(SPI_PORT, &data, 1);
    }
    va_end(valist);
    gpio_put(LCD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void LCD_1IN14_V2_SendData_16Bit(uint16_t data)
{
    gpio_put(LCD_DC_PIN, 1);
    gpio_put(LCD_CS_PIN, 0);
    spi_write_blocking(SPI_PORT, (uint8_t *) &data + 1, 1);
    spi_write_blocking(SPI_PORT, (uint8_t *) &data, 1);
    gpio_put(LCD_CS_PIN, 1);
}

/******************************************************************************
function :	Initialize the lcd register
parameter:
******************************************************************************/
static void LCD_1IN14_V2_InitReg(void)
{
    LCD_1IN14_V2_SendCommandData(0x3A, 1, 0x55);

    // LCD_1IN14_V2_SendCommandData(0xb2, 5,  // PORTCTRL (porch)
    //    0x0c, 0x0c, 0x00, 0x33, 0x33);      // these are the defaults on init / reset  

    LCD_1IN14_V2_SendCommandData(0xb7, 1, 0x35);        // Gate Control
    LCD_1IN14_V2_SendCommandData(0xbb, 1, 0x19);        // VCOM Setting
    LCD_1IN14_V2_SendCommandData(0xc0, 1, 0x2c);        // LCM Control
    LCD_1IN14_V2_SendCommandData(0xc2, 1, 0x01);        // VDV, VRH Enable
    LCD_1IN14_V2_SendCommandData(0xc3, 1, 0x12);        // VRH Set
    LCD_1IN14_V2_SendCommandData(0xc4, 1, 0x20);        // VDV Set
    LCD_1IN14_V2_SendCommandData(0xc6, 1, 0x0f);        // frame rate control in normal mode
    LCD_1IN14_V2_SendCommandData(0xd0, 2, 0xa4, 0xa1);  // power control 1

    LCD_1IN14_V2_SendCommandData(0xe0, 14, //Positive Voltage Gamma Control
        0xd0, 0x04, 0x0d, 0x11, 0x13, 0x2b, 0x3f, 0x54, 0x4c, 0x18, 0x0d, 0x0b, 0x1f, 0x23);

    LCD_1IN14_V2_SendCommandData(0xe1, 14, //Negative Voltage Gamma Control
        0xd0, 0x04, 0x0c, 0x11, 0x13, 0x2c, 0x3f, 0x44, 0x51, 0x2f, 0x1f, 0x1f, 0x20, 0x23);

    LCD_1IN14_V2_SendCommand(0x21);  // Display Inversion: Off (0x20), On (0x21)
    LCD_1IN14_V2_SendCommand(0x11);  // Sleep Out
    LCD_1IN14_V2_SendCommand(0x29);  // Display On
}

/********************************************************************************
function:	Set the resolution and scanning method of the screen
parameter:
		Scan_dir:   Scan direction
********************************************************************************/
static void LCD_1IN14_V2_SetAttributes(uint8_t Scan_dir)
{
    //Get the screen scan direction
    LCD_1IN14_V2.SCAN_DIR = Scan_dir;
    uint8_t MemoryAccessReg = 0x00;

    //Get GRAM and LCD width and height
    if(Scan_dir == HORIZONTAL) {
        LCD_1IN14_V2.HEIGHT	= LCD_1IN14_V2_WIDTH;
        LCD_1IN14_V2.WIDTH   = LCD_1IN14_V2_HEIGHT;
        MemoryAccessReg = 0X70;
    } else {
        LCD_1IN14_V2.HEIGHT	= LCD_1IN14_V2_HEIGHT;       
        LCD_1IN14_V2.WIDTH   = LCD_1IN14_V2_WIDTH;
        MemoryAccessReg = 0X00;
    }

    // Set the read / write scan direction of the frame memory
    LCD_1IN14_V2_SendCommandData(0x36, 1, MemoryAccessReg); //MX, MY, RGB mode (0x08 set RGB)
}

/********************************************************************************
function :	Initialize the lcd
parameter:
********************************************************************************/
void LCD_1IN14_V2_Init(uint8_t Scan_dir)
{
    //Hardware reset
    LCD_1IN14_V2_Reset();

    //Set the resolution and scanning method of the screen
    LCD_1IN14_V2_SetAttributes(Scan_dir);
    
    //Set the initialization register
    LCD_1IN14_V2_InitReg();
}

/********************************************************************************
function:	Sets the start position and size of the display area
parameter:
		Xstart 	:   X direction Start coordinates
		Ystart  :   Y direction Start coordinates
		Xend    :   X direction end coordinates
		Yend    :   Y direction end coordinates
********************************************************************************/
void LCD_1IN14_V2_SetWindows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend)
{
    uint8_t x,y;

    if(LCD_1IN14_V2.SCAN_DIR == HORIZONTAL){
        x=40;
        y=53;
    } else {
        x=52;
        y=40;
    }
    
    //set the X coordinates
    LCD_1IN14_V2_SendCommand(0x2A); // CASET
    
    LCD_1IN14_V2_SendData_16Bit(Xstart + x);
    LCD_1IN14_V2_SendData_16Bit(Xend - 1 + x);
    //set the Y coordinates
    LCD_1IN14_V2_SendCommand(0x2B);         // RASET
    LCD_1IN14_V2_SendData_16Bit(Ystart + y);
    LCD_1IN14_V2_SendData_16Bit(Yend - 1 + y);

    LCD_1IN14_V2_SendCommand(0X2C);  // RAMWR
    // Debug("%d %d\r\n",x,y);
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void LCD_1IN14_V2_Clear(uint16_t Color)
{
    LCD_1IN14_V2_ClearWindow(Color, 0, 0, LCD_1IN14_V2.WIDTH, LCD_1IN14_V2.HEIGHT);
}

/******************************************************************************
function :	Clear window
parameter:
		Xstart 	:   X direction Start coordinates
		Ystart  :   Y direction Start coordinates
		Xend    :   X direction end coordinates
		Yend    :   Y direction end coordinates
******************************************************************************/
void LCD_1IN14_V2_ClearWindow(uint16_t Color, uint16_t Xstart, uint16_t Ystart, uint16_t width, uint16_t height)
{
    uint8_t low = Color;
    uint8_t high = Color >> 8;
   
    LCD_1IN14_V2_SetWindows(Xstart, Ystart, Xstart + width, Ystart + height);
    gpio_put(LCD_DC_PIN, 1);
    gpio_put(LCD_CS_PIN, 0);
    for (int j=0; j<width * height; j++) {
        spi_write_blocking(SPI_PORT, &high, 1);
        spi_write_blocking(SPI_PORT, &low, 1);
    }
    gpio_put(LCD_CS_PIN, 1);
}

void LCD_1IN14_V2_Char(const uint16_t x, const uint16_t y, const sFONT *font, const uint16_t foreground, const uint16_t background, const u_char character)
{
    uint8_t fg_low = foreground;
    uint8_t fg_high = foreground >> 8;
    uint8_t bg_low = background;
    uint8_t bg_high = background >> 8;
   
    LCD_1IN14_V2_SetWindows(x, y, x + font->Width, y + font->Height);
    gpio_put(LCD_DC_PIN, 1);
    gpio_put(LCD_CS_PIN, 0);
    uint32_t offset = (character - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const unsigned char *ptr = &font->table[offset];

    for (uint16_t page = 0; page < font->Height; page++)
    {
        for (uint16_t column = 0; column < font->Width; column++)
        {
            if (*ptr & (0x80 >> (column % 8))) {
                spi_write_blocking(SPI_PORT, &fg_high, 1);
                spi_write_blocking(SPI_PORT, &fg_low, 1);
            } else {
                spi_write_blocking(SPI_PORT, &bg_high, 1);
                spi_write_blocking(SPI_PORT, &bg_low, 1);
            }
            ptr += (column %8 == 7);
        } // Write a line
        ptr += (font->Width % 8 != 0);
    } // Write all
    gpio_put(LCD_CS_PIN, 1);
}

void LCD_1IN14_V2_Invert(bool invert) {
    LCD_1IN14_V2_SendCommand(invert ? 0x21 : 0x20);
}

static uint slice_num;

/******************************************************************************
function:	Module Initialize, the library and initialize the pins, SPI protocol
parameter:
Info:
******************************************************************************/
uint8_t DEV_Module_Init(void)
{
    stdio_init_all();

    // SPI Config
    spi_init(SPI_PORT, 10000 * 1000);
    gpio_set_function(LCD_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);
    
    // GPIO Config
    gpio_init(LCD_RST_PIN);
    gpio_init(LCD_DC_PIN);
    gpio_init(LCD_CS_PIN);

    gpio_set_dir(LCD_RST_PIN, GPIO_OUT);
    gpio_set_dir(LCD_DC_PIN, GPIO_OUT);
    gpio_set_dir(LCD_CS_PIN, GPIO_OUT);
    
    // PWM Config
    gpio_set_function(LCD_BL_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(LCD_BL_PIN);
    pwm_set_wrap(slice_num, 100);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1);
    pwm_set_clkdiv(slice_num,50);
    pwm_set_enabled(slice_num, true);
    
    Debug("DEV_Module_Init OK \r\n");
    return 0;
}

void DEV_SET_PWM(uint8_t Value){
    if (Value < 0 || Value > 100) {
        Debug("DEV_SET_PWM Error \r\n");
    } else {
        pwm_set_chan_level(slice_num, PWM_CHAN_B, Value);
    }    
}

/******************************************************************************
function:	Display the string directly to the controller RAM
parameter:
    Xstart           ：X coordinate
    Ystart           ：Y coordinate
    pString          ：The first address of the English string to be displayed
    Font             ：A structure pointer that displays a character size
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
void Paint_DrawStringDirect(uint16_t Xstart, uint16_t Ystart, const char *pString,
                         const sFONT *Font, uint16_t Color_Foreground, uint16_t Color_Background)
{
    uint16_t Xpoint = Xstart;
    uint16_t Ypoint = Ystart;

    if (Xstart > LCD_1IN14_V2.WIDTH || Ystart > LCD_1IN14_V2.HEIGHT)
    {
        Debug("Paint_DrawString Input exceeds the normal display range\r\n");
        return;
    }

    while (*pString != '\0')
    {
        // if X direction filled , reposition to(Xstart,Ypoint),Ypoint is Y direction plus the Height of the character
        if ((Xpoint + Font->Width) > LCD_1IN14_V2.WIDTH)
        {
            Xpoint = Xstart;
            Ypoint += Font->Height;
        }

        // If the Y direction is full, reposition to(Xstart, Ystart)
        if ((Ypoint + Font->Height) > LCD_1IN14_V2_WIDTH)
        {
            Xpoint = Xstart;
            Ypoint = Ystart;
        }
        LCD_1IN14_V2_Char(Xpoint, Ypoint, Font, Color_Foreground, Color_Background, *pString);
 
        // The next character of the address
        pString++;

        // The next word of the abscissa increases the font of the broadband
        Xpoint += Font->Width;
    }
}

static void do_digit(const sFONT *font, const uint16_t xstart, const uint16_t ystart, const uint16_t background, uint16_t const foreground, const u_char digit) {
    LCD_1IN14_V2_Char(xstart, ystart, font, foreground, background, digit);
    // Paint_DrawChar(xstart, ystart, digit, font, background, foreground);
    // LCD_1IN14_V2_DisplayWindows(xstart, ystart, xstart + font->Width, ystart + font->Height, (uint16_t *) Paint.Image);
    // LCD_1IN14_V2_DisplayWindows(xstart, ystart, xstart + font->Width, ystart + font->Height, (uint16_t *) Paint.Image);
}

void Paint_DrawSeconds(uint16_t Xstart, uint16_t Ystart, uint16_t seconds, const sFONT *Font,
                    uint16_t Color_Foreground, uint16_t Color_Background, uint16_t prev_seconds)
{
    uint16_t dx = Font->Width;
    
    uint8_t sec = seconds % 60;
    uint8_t prev_sec = prev_seconds % 60;
    uint8_t min = seconds / 60;
    uint8_t prev_min = prev_seconds / 60;

    uint8_t sec01 = sec % 10;
    uint8_t sec10 = sec / 10;
    uint8_t min01 = min % 10;
    uint8_t min10 = min / 10;

    uint8_t prev_sec01 = prev_sec % 10;
    uint8_t prev_sec10 = prev_sec / 10;
    uint8_t prev_min01 = prev_min % 10;
    uint8_t prev_min10 = prev_min / 10;

    // Update display where needed by digit
    if (!prev_seconds) {
        do_digit(Font, Xstart + 7 * dx / 4, Ystart, Color_Background, Color_Foreground, ':');
    }
    if (!prev_seconds || prev_min10 != min10) {
        do_digit(Font, Xstart, Ystart, Color_Background, Color_Foreground, min10 ? min10 + '0' : ' ');
    }
    if (!prev_seconds || prev_min01 != min01) {
        do_digit(Font, Xstart + dx, Ystart, Color_Background, Color_Foreground, min01 + '0');
    }
    if (!prev_seconds || prev_sec10 != sec10) {
        do_digit(Font, Xstart + 5 * dx / 2, Ystart, Color_Background, Color_Foreground, sec10 + '0');
    }
    if (!prev_seconds || prev_sec01 != sec01) {
        do_digit(Font, Xstart + 7 * dx / 2, Ystart, Color_Background, Color_Foreground, sec01 + '0');
    }
}
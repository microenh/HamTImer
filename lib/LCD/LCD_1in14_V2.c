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
#include "LCD_1in14_V2.h"

LCD_1IN14_V2_ATTRIBUTES LCD_1IN14_V2;

/******************************************************************************
function :	Hardware reset
parameter:
******************************************************************************/
static void LCD_1IN14_V2_Reset(void)
{
    DEV_Digital_Write(LCD_RST_PIN, 1);
    DEV_Delay_ms(100);
    DEV_Digital_Write(LCD_RST_PIN, 0);
    DEV_Delay_ms(100);
    DEV_Digital_Write(LCD_RST_PIN, 1);
    DEV_Delay_ms(100);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
static void LCD_1IN14_V2_SendCommand(UBYTE Reg)
{
    DEV_Digital_Write(LCD_DC_PIN, 0);
    DEV_Digital_Write(LCD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(LCD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void LCD_1IN14_V2_SendData_8Bit(UBYTE Data)
{
    DEV_Digital_Write(LCD_DC_PIN, 1);
    DEV_Digital_Write(LCD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(LCD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void LCD_1IN14_V2_SendData_16Bit(UWORD Data)
{
    DEV_Digital_Write(LCD_DC_PIN, 1);
    DEV_Digital_Write(LCD_CS_PIN, 0);
    DEV_SPI_WriteByte((Data >> 8) & 0xFF);
    DEV_SPI_WriteByte(Data & 0xFF);
    DEV_Digital_Write(LCD_CS_PIN, 1);
}

/******************************************************************************
function :	Initialize the lcd register
parameter:
******************************************************************************/
static void LCD_1IN14_V2_InitReg(void)
{
    LCD_1IN14_V2_SendCommand(0x3A);
    LCD_1IN14_V2_SendData_8Bit(0x55);  // 0x05

    // LCD_1IN14_V2_SendCommand(0xB2);     // PORTCTRL (porch)
    // LCD_1IN14_V2_SendData_8Bit(0x0C);    // defaults
    // LCD_1IN14_V2_SendData_8Bit(0x0C);
    // LCD_1IN14_V2_SendData_8Bit(0x00);
    // LCD_1IN14_V2_SendData_8Bit(0x33);
    // LCD_1IN14_V2_SendData_8Bit(0x33);

    LCD_1IN14_V2_SendCommand(0xB7);  //Gate Control
    LCD_1IN14_V2_SendData_8Bit(0x35);

    LCD_1IN14_V2_SendCommand(0xBB);  //VCOM Setting
    LCD_1IN14_V2_SendData_8Bit(0x19);

    LCD_1IN14_V2_SendCommand(0xC0); //LCM Control     
    LCD_1IN14_V2_SendData_8Bit(0x2C);

    LCD_1IN14_V2_SendCommand(0xC2);  //VDV and VRH Command Enable
    LCD_1IN14_V2_SendData_8Bit(0x01);
    LCD_1IN14_V2_SendCommand(0xC3);  //VRH Set
    LCD_1IN14_V2_SendData_8Bit(0x12);
    LCD_1IN14_V2_SendCommand(0xC4);  //VDV Set
    LCD_1IN14_V2_SendData_8Bit(0x20);

    LCD_1IN14_V2_SendCommand(0xC6);  //Frame Rate Control in Normal Mode
    LCD_1IN14_V2_SendData_8Bit(0x0F);
    
    LCD_1IN14_V2_SendCommand(0xD0);  // Power Control 1
    LCD_1IN14_V2_SendData_8Bit(0xA4);
    LCD_1IN14_V2_SendData_8Bit(0xA1);

    LCD_1IN14_V2_SendCommand(0xE0);  //Positive Voltage Gamma Control
    LCD_1IN14_V2_SendData_8Bit(0xD0);
    LCD_1IN14_V2_SendData_8Bit(0x04);
    LCD_1IN14_V2_SendData_8Bit(0x0D);
    LCD_1IN14_V2_SendData_8Bit(0x11);
    LCD_1IN14_V2_SendData_8Bit(0x13);
    LCD_1IN14_V2_SendData_8Bit(0x2B);
    LCD_1IN14_V2_SendData_8Bit(0x3F);
    LCD_1IN14_V2_SendData_8Bit(0x54);
    LCD_1IN14_V2_SendData_8Bit(0x4C);
    LCD_1IN14_V2_SendData_8Bit(0x18);
    LCD_1IN14_V2_SendData_8Bit(0x0D);
    LCD_1IN14_V2_SendData_8Bit(0x0B);
    LCD_1IN14_V2_SendData_8Bit(0x1F);
    LCD_1IN14_V2_SendData_8Bit(0x23);

    LCD_1IN14_V2_SendCommand(0xE1);  //Negative Voltage Gamma Control
    LCD_1IN14_V2_SendData_8Bit(0xD0);
    LCD_1IN14_V2_SendData_8Bit(0x04);
    LCD_1IN14_V2_SendData_8Bit(0x0C);
    LCD_1IN14_V2_SendData_8Bit(0x11);
    LCD_1IN14_V2_SendData_8Bit(0x13);
    LCD_1IN14_V2_SendData_8Bit(0x2C);
    LCD_1IN14_V2_SendData_8Bit(0x3F);
    LCD_1IN14_V2_SendData_8Bit(0x44);
    LCD_1IN14_V2_SendData_8Bit(0x51);
    LCD_1IN14_V2_SendData_8Bit(0x2F);
    LCD_1IN14_V2_SendData_8Bit(0x1F);
    LCD_1IN14_V2_SendData_8Bit(0x1F);
    LCD_1IN14_V2_SendData_8Bit(0x20);
    LCD_1IN14_V2_SendData_8Bit(0x23);

    LCD_1IN14_V2_SendCommand(0x21);  //Display Inversion Off (0x20), On (0x21)

    LCD_1IN14_V2_SendCommand(0x11);  //Sleep Out

    LCD_1IN14_V2_SendCommand(0x29);  //Display On
}

/********************************************************************************
function:	Set the resolution and scanning method of the screen
parameter:
		Scan_dir:   Scan direction
********************************************************************************/
static void LCD_1IN14_V2_SetAttributes(UBYTE Scan_dir)
{
    //Get the screen scan direction
    LCD_1IN14_V2.SCAN_DIR = Scan_dir;
    UBYTE MemoryAccessReg = 0x00;

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
    LCD_1IN14_V2_SendCommand(0x36); //MX, MY, RGB mode
    LCD_1IN14_V2_SendData_8Bit(MemoryAccessReg);	//0x08 set RGB
}

/********************************************************************************
function :	Initialize the lcd
parameter:
********************************************************************************/
void LCD_1IN14_V2_Init(UBYTE Scan_dir)
{
    // DEV_SET_PWM(90);
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
void LCD_1IN14_V2_SetWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend)
{
    UBYTE x,y;
    if(LCD_1IN14_V2.SCAN_DIR == HORIZONTAL){x=40;y=53;}
    else{ x=52; y=40; }
    
    //set the X coordinates
    LCD_1IN14_V2_SendCommand(0x2A); // CASET
    
    
    LCD_1IN14_V2_SendData_16Bit(Xstart	+x);
    LCD_1IN14_V2_SendData_16Bit(Xend-1	+x);
    //set the Y coordinates
    LCD_1IN14_V2_SendCommand(0x2B);         // RASET
    LCD_1IN14_V2_SendData_16Bit(Ystart +y);
    LCD_1IN14_V2_SendData_16Bit(Yend-1	  +y);

    LCD_1IN14_V2_SendCommand(0X2C);  // RAMWR
    // Debug("%d %d\r\n",x,y);
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void LCD_1IN14_V2_Clear(UWORD Color)
{
    UBYTE low = Color;
    UBYTE high = Color >> 8;
   
    LCD_1IN14_V2_SetWindows(0, 0, LCD_1IN14_V2.WIDTH, LCD_1IN14_V2.HEIGHT);
    DEV_Digital_Write(LCD_DC_PIN, 1);
    DEV_Digital_Write(LCD_CS_PIN, 0);
    for (int j=0; j<LCD_1IN14_V2.HEIGHT * LCD_1IN14_V2.WIDTH; j++) {
        DEV_SPI_WriteByte(high);
        DEV_SPI_WriteByte(low);
    }
    DEV_Digital_Write(LCD_CS_PIN, 1);
}

/******************************************************************************
function :	Clear window
parameter:
		Xstart 	:   X direction Start coordinates
		Ystart  :   Y direction Start coordinates
		Xend    :   X direction end coordinates
		Yend    :   Y direction end coordinates
******************************************************************************/
void LCD_1IN14_V2_ClearWindow(UWORD Color, UWORD Xstart, UWORD Ystart, UWORD width, UWORD height)
{
    UBYTE low = Color;
    UBYTE high = Color >> 8;
   
    LCD_1IN14_V2_SetWindows(Xstart, Ystart, Xstart + width, Ystart + height);
    DEV_Digital_Write(LCD_DC_PIN, 1);
    DEV_Digital_Write(LCD_CS_PIN, 0);
    for (int j=0; j<width * height; j++) {
        DEV_SPI_WriteByte(high);
        DEV_SPI_WriteByte(low);
    }
    DEV_Digital_Write(LCD_CS_PIN, 1);
}

void LCD_1IN14_V2_Char(const uint16_t x, const uint16_t y, const sFONT *font, const uint16_t foreground, const uint16_t background, const u_char character)
{
    UBYTE fg_low = foreground;
    UBYTE fg_high = foreground >> 8;
    UBYTE bg_low = background;
    UBYTE bg_high = background >> 8;
   
    LCD_1IN14_V2_SetWindows(x, y, x + font->Width, y + font->Height);
    DEV_Digital_Write(LCD_DC_PIN, 1);
    DEV_Digital_Write(LCD_CS_PIN, 0);
    uint32_t offset = (character - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const unsigned char *ptr = &font->table[offset];

    for (UWORD page = 0; page < font->Height; page++)
    {
        for (UWORD column = 0; column < font->Width; column++)
        {
            if (*ptr & (0x80 >> (column % 8))) {
                DEV_SPI_WriteByte(fg_high);
                DEV_SPI_WriteByte(fg_low);
            } else {
                DEV_SPI_WriteByte(bg_high);
                DEV_SPI_WriteByte(bg_low);
            }
            ptr += (column %8 == 7);
        } // Write a line
        ptr += (font->Width % 8 != 0);
    } // Write all    DEV_Digital_Write(LCD_CS_PIN, 1);
}

void LCD_1IN14_V2_Invert(bool invert) {
    LCD_1IN14_V2_SendCommand(invert ? 0x21 : 0x20);
}

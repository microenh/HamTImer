#include "GUI_Paint.h"
#include "Debug.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h> //memset()
#include <math.h>


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
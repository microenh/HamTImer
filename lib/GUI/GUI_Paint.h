#ifndef __GUI_PAINT_H
#define __GUI_PAINT_H

#include "../Fonts/fonts.h"
#include "../LCD/LCD_1in14_V2.h"

/**
 * image color
**/

#define RGB_565(r,g,b) (((((r)) & 0xf8) << 8) | (((( g)) & 0xfc) << 3) | ((( b)) >> 3))

#define BLACK          RGB_565(0,0,0)       // 0x0000
#define RED            RGB_565(127,0,0)     // 0xF800
#define BRED           RGB_565(255,0,0)     // 0XF81F
#define GREEN          RGB_565(0,127,0)     // 0x07E0
#define BGREEN         RGB_565(0,255,0)     // 0x07E0
#define BLUE           RGB_565(0,0,127)     // 0x001F
#define BBLUE          RGB_565(0,0,255)     // 0x001F
#define CYAN           RGB_565(0,127,127)   // 0x7FFF
#define BCYAN          RGB_565(0,255,255)   // 0x7FFF
#define MAGENTA        RGB_565(127,0,127)   // 0xF81F
#define BMAGENTA       RGB_565(255,0,255)   // 0xF81F
#define YELLOW         RGB_565(127,127,0)
#define BYELLOW        RGB_565(255,255,0)   // 0xFFE0
#define GRAY           RGB_565(127,127,127) // 0X8430
#define WHITE          RGB_565(255,255,255) // 0xFFFF

#define ORANGE         RGB_565(255,127,0)   // 0XFFE0
#define PINK           RGB_565(255,0,127)   // 0XFC07
#define LGREEN         RGB_565(0,255,127)
#define LBLUE          RGB_565(0,127,255)

void Paint_DrawStringDirect(UWORD Xstart, UWORD Ystart, const char * pString, const sFONT* Font, UWORD Color_Foreground, UWORD Color_Background);
void Paint_DrawSeconds(UWORD Xstart, UWORD Ystart, uint16_t seconds, const sFONT *Font,
                    UWORD Color_Foreground, UWORD Color_Background, uint16_t prev_seconds);

#endif
#include "display.h"
#include "config.h"
#include "lcd.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


enum DISPLAYING {
    DISP_NONE,
    DISP_TIME,
    DISP_STRING
};

typedef struct LAYOUT {
    uint16_t color;
    uint8_t timeLen;
    uint8_t digitX[4];
    uint8_t colonX;
    uint8_t *cache;
    uint8_t y;
    uint8_t count;
    uint8_t oldLen;
    uint8_t oldLeft;
    enum DISPLAYING displaying;
    const sFONT* font;
} LAYOUT;

static LAYOUT topLayout;
static LAYOUT bottomLayout;
static bool showBottom;
static uint16_t yTop, yMiddle;


static void initLayout(LAYOUT *layout, const sFONT *font) {
    layout->font = font;
    layout->timeLen = font->width * 9 / 2;
    layout->digitX[0] = (lcd.width - layout->timeLen) / 2;
    layout->digitX[1] = layout->digitX[0] + font->width;
    layout->digitX[2] = layout->digitX[1] + 3 * font->width / 2;
    layout->digitX[3] = layout->digitX[2] + font->width;
    layout->colonX = layout->digitX[0] + 7 * font->width / 4; // ':'
    layout->cache = malloc(lcd.width / font->width);
    layout->oldLeft = lcd.width / 2;
}

void displayInit(void) {
    yTop = (lcd.height - fontTop->height - fontBottom->height) / 3;
    yMiddle = (lcd.height - fontTop->height) / 2;
    initLayout(&topLayout, fontTop);
    topLayout.y = showBottom ? yTop : yMiddle;
    topLayout.color = TOP_FOREGROUND;
    initLayout(&bottomLayout, fontBottom);
    bottomLayout.y = (lcd.height - fontBottom->height - yTop);
    bottomLayout.color = BOTTOM_FOREGROUND;
}



void displayTime(const bool top, const uint16_t time) {
    if (top || showBottom) {
        LAYOUT *layout = top ? &topLayout : &bottomLayout;
        uint8_t temp = time / 60;
        uint8_t cache[4];
        cache[0] = temp / 10;
        cache[1] = temp % 10;
        temp = time % 60;
        cache[2] = temp / 10;
        cache[3] = temp % 10;

        if (layout->displaying == DISP_TIME) {
            for (uint8_t i = 0; i < 4; i++) {
                if (cache[i] != layout->cache[i]) {
                    layout->cache[i] = cache[i];
                    DrawChar(layout->digitX[i], layout->y, layout->font, layout->color, BACKGROUND, cache[i] + '0');
                }
            }
        } else {
            // clear as needed - prefix
            int16_t delta2 = (layout->oldLen - layout->timeLen);
            int8_t delta = delta2 / 2 + delta2 % 2;
            if (delta > 0) {
                ClearWindow(BACKGROUND, layout->oldLeft, layout->y, delta, layout->font->height);
            }
            DrawChar(layout->colonX, layout->y, layout->font, layout->color, BACKGROUND, ':');
            for (uint8_t i = 0; i < 4; i++) {
                layout->cache[i] = cache[i];
                DrawChar(layout->digitX[i], layout->y, layout->font, layout->color, BACKGROUND, cache[i] + '0');
            }
            // clear as needed - suffix
            if (delta > 0) {
                ClearWindow(BACKGROUND, layout->oldLeft + layout->oldLen - delta, layout->y, delta, layout->font->height);
            }
            layout->displaying = DISP_TIME;
            layout->oldLen = layout->timeLen;
            layout->oldLeft = layout->digitX[0];
            layout->count = 0;
        }
    }
}

void displayString(const bool top, const char * pString) {
    if (top || showBottom) {
        LAYOUT *layout = top ? &topLayout : &bottomLayout;
        uint8_t count = strlen(pString);
        if ((layout->displaying == DISP_STRING) && (count == layout->count)) {
            for (uint8_t i = 0; i < count; i++) {
                if (pString[i] != layout->cache[i]) {
                    layout->cache[i] = pString[i];
                    DrawChar(layout->oldLeft + layout->font->width * i, layout->y, layout->font, layout->color, BACKGROUND, pString[i]);
                }
            }
        } else {
            uint8_t newLen = count * layout->font->width;
            uint8_t newLeft = (lcd.width - newLen) / 2;
            int16_t delta2 = (layout->oldLen - newLen);
            int8_t delta = delta2 / 2 + delta2 % 2;

            if (delta > 0) {
                // clear as needed - prefix
                ClearWindow(BACKGROUND, layout->oldLeft, layout->y, delta, layout->font->height);        }
            for (uint8_t i = 0; i < count; i++) {
                layout->cache[i] = pString[i];
                DrawChar(newLeft + layout->font->width * i, layout->y, layout->font, layout->color, BACKGROUND, pString[i]);
            }
            if (delta > 0) {
                // clear as needed - suffix
                ClearWindow(BACKGROUND, layout->oldLeft + layout->oldLen - delta, layout->y, delta, layout->font->height);
            }
            layout->count = count;
            layout->oldLen = newLen;
            layout->oldLeft = newLeft;
            layout->displaying = DISP_STRING;
        }
    }
}

static void clear(const bool top) {
    if (top || showBottom) {
        LAYOUT *layout = top ? &topLayout : &bottomLayout;
        if (layout->displaying != DISP_NONE) {
            ClearWindow(BACKGROUND, layout->oldLeft, layout->y, layout->oldLen, layout->font->height);
            layout->count = 0;
            layout->displaying = DISP_NONE;
            layout->oldLeft = lcd.width / 2;
            layout->oldLen = 0;
        }
    }
}

void displayShowBottom(const bool show) {
    if (show != showBottom) {
        clear(true);
        clear(false);
        showBottom = show;
        topLayout.y = showBottom ? yTop : yMiddle;
    }
}
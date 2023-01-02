#include "display.h"
#include "config.h"
#include "lcd.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint16_t yTop, yMiddle, yBottom;

typedef struct LAYOUT {
    uint8_t count;
    uint8_t *cache;
    bool time;
    uint8_t digitX[5];
    uint8_t timeLen;
} LAYOUT;

LAYOUT topLayout;
LAYOUT bottomLayout;
bool singleDisplay;


static void initLayout(LAYOUT *layout, const sFONT *font) {
    layout->timeLen = font->width * 9 / 2;
    layout->digitX[0] = (lcd.width - layout->timeLen) / 2;
    layout->digitX[1] = layout->digitX[0] + font->width;
    layout->digitX[2] = layout->digitX[1] + 3 * font->width / 2;
    layout->digitX[3] = layout->digitX[2] + font->width;
    layout->digitX[4] = layout->digitX[0] + 7 * font->width / 4; // ':'
    layout->cache = malloc(lcd.width / font->width);
}

void displayInit(void) {
    yTop = (lcd.height - fontTop->height - fontBottom->height) / 3;
    yMiddle = (lcd.height - fontTop->height) / 2;
    yBottom = (lcd.height - fontBottom->height - yTop);
    initLayout(&topLayout, fontTop);
    initLayout(&bottomLayout, fontBottom);
}

void displayTimeTop(const uint16_t time) {
    uint8_t temp = time / 60;
    uint8_t cache[4];
    cache[0] = temp / 10;
    cache[1] = temp % 10;
    temp = time % 60;
    cache[2] = temp / 10;
    cache[3] = temp % 10;

    uint8_t y = singleDisplay ? yMiddle : yTop;
    if (topLayout.time) {
        for (uint8_t i = 0; i < 4; i++) {
            if (cache[i] != topLayout.cache[i]) {
                topLayout.cache[i] = cache[i];
                DrawChar(topLayout.digitX[i], y, fontTop, TOP_FOREGROUND, BACKGROUND, cache[i] + '0');
            }
        }
    } else {
        // clear as needed - prefix
        int8_t delta = (topLayout.count * fontTop->width - topLayout.timeLen) / 2;
        if (delta > 0) {
            ClearWindow(BACKGROUND, topLayout.digitX[0] - delta, y, delta, fontTop->height);
        }
        DrawChar(topLayout.digitX[4], y, fontTop, TOP_FOREGROUND, BACKGROUND, ':');
        for (uint8_t i = 0; i < 4; i++) {
            topLayout.cache[i] = cache[i];
            DrawChar(topLayout.digitX[i], y, fontTop, TOP_FOREGROUND, BACKGROUND, cache[i] + '0');
        }
        // clear as needed - suffix
        if (delta > 0) {
            ClearWindow(BACKGROUND, topLayout.digitX[3] + fontTop->width, y, delta, fontTop->height);
        }
        topLayout.count = 4;
        topLayout.time = true;
    }
}

void displayTimeBottom(const uint16_t time) {
    if (!singleDisplay) {
        uint8_t temp = time / 60;
        uint8_t cache[4];
        cache[0] = temp / 10;
        cache[1] = temp % 10;
        temp = time % 60;
        cache[2] = temp / 10;
        cache[3] = temp % 10;

        if (bottomLayout.time) {
            for (uint8_t i = 0; i < 4; i++) {
                if (cache[i] != bottomLayout.cache[i]) {
                    bottomLayout.cache[i] = cache[i];
                    DrawChar(bottomLayout.digitX[i], yBottom, fontBottom, BOTTOM_FOREGROUND, BACKGROUND, cache[i] + '0');
                }
            }
        } else {
            // clear as needed - prefix
            int8_t delta = (bottomLayout.count * fontBottom->width - bottomLayout.timeLen) / 2;
            if (delta > 0) {
                ClearWindow(BACKGROUND, bottomLayout.digitX[0] - delta, yBottom, delta, fontBottom->height);
            }
            DrawChar(bottomLayout.digitX[4], yBottom, fontBottom, BOTTOM_FOREGROUND, BACKGROUND, ':');
            for (uint8_t i = 0; i < 4; i++) {
                bottomLayout.cache[i] = cache[i];
                DrawChar(bottomLayout.digitX[i], yBottom, fontBottom, BOTTOM_FOREGROUND, BACKGROUND, cache[i] + '0');
            }
            // clear as needed - suffix
            if (delta > 0) {
                ClearWindow(BACKGROUND, bottomLayout.digitX[3] + fontBottom->width, yBottom, delta, fontBottom->height);
            }
            bottomLayout.count = 4;
            bottomLayout.time = true;
        }
    }
}

void displayStringTop(const char * pString) {
    uint8_t count = strlen(pString);
    uint8_t y = singleDisplay ? yMiddle : yTop;
    uint8_t newlen = count * fontTop->width;
    uint8_t left = (lcd.width - newlen) / 2;
    if (topLayout.time || topLayout.count != count) {
        uint8_t oldlen = topLayout.time ? topLayout.timeLen : topLayout.count * fontTop->width;
        int8_t delta = (oldlen - newlen) / 2;

        if (delta > 0) {
            // clear as needed - prefix
            ClearWindow(BACKGROUND, topLayout.digitX[0], y, delta, fontTop->height);
        }
        for (uint8_t i = 0; i < count; i++) {
            topLayout.cache[i] = pString[i];
            DrawChar(left + fontTop->width * i, y, fontTop, TOP_FOREGROUND, BACKGROUND, pString[i]);
        }
        if (delta > 0) {
            // clear as needed - suffix
            ClearWindow(BACKGROUND, left + newlen, y, delta, fontTop->height);
        }
        topLayout.count = count;
        topLayout.time = false;
    } else {
        for (uint8_t i = 0; i < count; i++) {
            if (pString[i] != topLayout.cache[i]) {
                topLayout.cache[i] = pString[i];
                DrawChar(left + fontTop->width * i, y, fontTop, TOP_FOREGROUND, BACKGROUND, pString[i]);
            }
        }
    }
}

void displayStringBottom(const char * pString) {
    if (!singleDisplay) {
        uint8_t count = strlen(pString);
        uint8_t newlen = count * fontBottom->width;
        uint8_t left = (lcd.width - newlen) / 2;
        if (bottomLayout.time || bottomLayout.count != count) {
            uint8_t oldlen = bottomLayout.time ? bottomLayout.timeLen : bottomLayout.count * fontBottom->width;
            int8_t delta = (oldlen - newlen) / 2;

            if (delta > 0) {
                // clear as needed - prefix
                ClearWindow(BACKGROUND, bottomLayout.digitX[0], yBottom, delta, fontBottom->height);
            }
            for (uint8_t i = 0; i < count; i++) {
                topLayout.cache[i] = pString[i];
                DrawChar(left + fontBottom->width * i, yBottom, fontBottom, BOTTOM_FOREGROUND, BACKGROUND, pString[i]);
            }
            if (delta > 0) {
                // clear as needed - suffix
                ClearWindow(BACKGROUND, left + newlen, yBottom, delta, fontBottom->height);
            }
            bottomLayout.count = count;
            bottomLayout.time = false;
        } else {
            for (uint8_t i = 0; i < count; i++) {
                if (pString[i] != topLayout.cache[i]) {
                    topLayout.cache[i] = pString[i];
                    DrawChar(left + fontBottom->width * i, yBottom, fontBottom, BOTTOM_FOREGROUND, BACKGROUND, pString[i]);
                }
            }
        }
    }

}

void clearTop(void) {
    uint8_t y = singleDisplay ? yMiddle : yTop;
    if (topLayout.time) {
        ClearWindow(BACKGROUND, topLayout.digitX[0], y, topLayout.timeLen, fontTop->height);
        topLayout.time = false;
    } else {
        if (topLayout.count) {
            uint8_t len = topLayout.count * fontTop->width;
            uint8_t left = (lcd.width - len) / 2;
            ClearWindow(BACKGROUND, left, y, len, fontTop->height);
            topLayout.count = 0;
        }
    }
}

void clearBottom(void) {
    if (!singleDisplay) {
        if (bottomLayout.time) {
            ClearWindow(BACKGROUND, bottomLayout.digitX[0], yBottom, bottomLayout.timeLen, fontBottom->height);
            bottomLayout.time = false;
        } else {
            if (bottomLayout.count) {
                uint8_t len = bottomLayout.count * fontBottom->width;
                uint8_t left = (lcd.width - len) / 2;
                ClearWindow(BACKGROUND, left, yBottom, len, fontBottom->height);
                bottomLayout.count = 0;
            }
        }
    }
}

static void redrawTop() {
    uint8_t y = singleDisplay ? yMiddle : yTop;
    if (topLayout.time) {
        DrawChar(topLayout.digitX[4], y, fontTop, TOP_FOREGROUND, BACKGROUND, ':');
        for (uint8_t i = 0; i < 4; i++) {
            DrawChar(topLayout.digitX[i], y, fontTop, TOP_FOREGROUND, BACKGROUND, topLayout.cache[i] + '0');
        }
    } else {
        uint8_t left = (lcd.width - topLayout.count * fontTop->width) / 2;
        for (uint8_t i = 0; i < topLayout.count; i++) {
            DrawChar(left + fontTop->width * i, y, fontTop, TOP_FOREGROUND, BACKGROUND, topLayout.cache[i]);
        }
    }
}

static void redrawBottom() {

}

void setSingle(const bool single) {
    if (single != singleDisplay) {
        clearTop();
        if (!singleDisplay) {
            clearBottom();
        }
        singleDisplay = single;
        redrawTop();
        if (!single) {
            redrawBottom();
        }
    }
}
#include "config.h"
#include "lcd.h"

const uint8_t DEBOUNCE = 1;   // 125 mSec ticks for btn debounce
const uint8_t FLASH_CTR = 1;  // 125 mSec ticks for btn detect flash
const sFONT* fontTop = &Liberation48;
const sFONT* fontBottom = &Liberation36;
const char* TO_MSG = "TIMEOUT";  // 8 char max w/ 36 pt
const char* ID_MSG = "ID";       // 6 char max w/ 48 pt

// colors
const uint16_t BACKGROUND = BLACK;
const uint16_t TOP_FOREGROUND = WHITE;
const uint16_t BOTTOM_FOREGROUND = BYELLOW;

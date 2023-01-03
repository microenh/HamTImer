#pragma once

#include <stdint.h>
#include <stdbool.h>

void displayInit(void);
void displayDualTimer(const bool dual);
void displayTime(const bool top, const uint16_t time);
void displayString(const bool top, const char *pString);
void displayShowBottom(const bool show);
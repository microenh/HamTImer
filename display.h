#pragma once

#include <stdint.h>
#include <stdbool.h>

void displayInit(void);
void displayTimeTop(const uint16_t time);
void displayTimeBottom(const uint16_t time);
void displayStringTop(const char * pString);
void displayStringBottom(const char * pString);
void setSingle(const bool single, const bool redraw);
#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    unsigned int CTRA:12;
    unsigned int CTRB:12;
    unsigned int pwm:7;
    bool flash:1;
} SETTINGS;

extern SETTINGS settings;

void readFlash(void);
void writeFlash(void);
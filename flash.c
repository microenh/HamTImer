#include <stdio.h>
#include "flash.h"
#include "hardware/sync.h"
#include "hardware/flash.h"

enum FLASH_STATUS {
    FLASH_GOOD_DATA,
    FLASH_BAD_DATA,
    FLASH_EMPTY,
    FLASH_FULL
};

enum FLASH_STATUS flashStatus;
uint16_t flashIndex;


    // PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)
    // FLASH_SECTOR_SIZE (1 << 12) 4096
    // FLASH_PAGE_SIZE (1<< 8) 256

typedef union {
    SETTINGS s;
    int32_t i;
} READER;

SETTINGS settings;
SETTINGS savedSettings;

void setDefaults() {
    settings.CTRA = 30;
    settings.CTRB = 15;
    settings.pwm = 5;
    settings.flash = true;
    savedSettings = settings;
}

bool check(const SETTINGS s) {
    return s.CTRA > 0 && s.CTRA <= 6000 && s.CTRB >= 0 && s.CTRB <= 6000 && s.pwm >= 1 && s.pwm <= 100;
}

void readFlash(void) {
    #define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
    int addr = XIP_BASE + FLASH_TARGET_OFFSET;

    READER *p = (READER *) addr;
    uint16_t i;
    setDefaults();
    for (i = 0; i<FLASH_SECTOR_SIZE / sizeof(READER) && !p[i].i; i++);    // self-contained loop
    flashIndex = i;
    if (i == FLASH_SECTOR_SIZE / sizeof(READER)) {
        printf("No data found.\r\n");  
        flashStatus = FLASH_FULL;      
    } else if (p[i].i == -1) {
        printf("Empty data slot at %d\r\n", i);
        flashStatus = FLASH_EMPTY;
    } else if (check(p[i].s)) {
        printf("Good data at %d\r\n", i);
        flashStatus = FLASH_GOOD_DATA;
        settings = p[i].s;
        savedSettings = settings;
    } else {
        printf("Bad data at %d\r\n", i);
        flashStatus = FLASH_BAD_DATA;
     }
}

void writeFlash(void) {

}
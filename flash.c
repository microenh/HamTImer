#include <stdio.h>
#include "flash.h"
#include "debug.h"
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

typedef union {
    SETTINGS s;
    int32_t i;
} READER;

SETTINGS settings;
SETTINGS savedSettings;

const SETTINGS DEFAULT_SETTINGS = {
    .CTRA = 600,
    .CTRB = 120,
    .pwm = 5,
    .flash = true
};

bool check(const SETTINGS s) {
    return s.CTRA >= 1 && s.CTRA <= 6000
        && s.CTRB >= 0 && s.CTRB <= 6000
        && s.pwm >= 1 && s.pwm <= 100;
}

// PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)
// FLASH_SECTOR_SIZE (1 << 12) 4096
// FLASH_PAGE_SIZE (1<< 8) 256
const uint16_t MAX_SAVES = FLASH_SECTOR_SIZE / sizeof (READER);
const uint16_t SAVES_PER_SECTOR = FLASH_PAGE_SIZE / sizeof(READER);
const uint32_t FLASH_TARGET_OFFSET = (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE);

void readFlash(void) {
    int addr = XIP_BASE + FLASH_TARGET_OFFSET;

    READER *p = (READER *) addr;
    uint16_t i;
    settings = DEFAULT_SETTINGS;
    savedSettings = DEFAULT_SETTINGS;
    for (i = 0; i < MAX_SAVES && !p[i].i; i++) {
        Debug("Old data at %d\r\n", i);
    }
    flashIndex = i;
    if (i == MAX_SAVES) {
        Debug("No data found.\r\n");  
        flashStatus = FLASH_FULL;      
    } else if (p[i].i == -1) {
        Debug("Empty data slot at %d\r\n", i);
        flashStatus = FLASH_EMPTY;
    } else if (check(p[i].s)) {
        Debug("Good data at %d\r\n", i);
        flashStatus = FLASH_GOOD_DATA;
        settings = p[i].s;
        savedSettings = settings;
    } else {
        Debug("Bad data at %d\r\n", i);
        flashStatus = FLASH_BAD_DATA;
     }
}


static bool settingsChanged(const SETTINGS settings1, const SETTINGS settings2) {
    return settings1.CTRA != settings2.CTRA
        || settings1.CTRB != settings2.CTRB
        || settings1.pwm != settings2.pwm
        || settings1.flash != settings2.flash;
}

void writeFlash(void) {
    READER buf[SAVES_PER_SECTOR];
    for (int i = 0; i< SAVES_PER_SECTOR; i++) {
        buf[i].i = -1;
    }
    int flashPage = flashIndex / SAVES_PER_SECTOR;
    int flashPageIndex = flashIndex % SAVES_PER_SECTOR;

    switch (flashStatus) {
        case FLASH_GOOD_DATA:
        case FLASH_BAD_DATA:
            if (settingsChanged(settings, savedSettings)) {
                if (flashIndex == MAX_SAVES - 1) {
                    Debug("Clearing flash.\r\n");
                    buf[0].s = settings;
                    uint32_t ints = save_and_disable_interrupts();
                    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
                    flash_range_program(FLASH_TARGET_OFFSET, (uint8_t *) buf, FLASH_PAGE_SIZE);
                    restore_interrupts(ints);
                    flashIndex = 0;
                } else {
                    // write 0 to old settings, write new settings next
                    int flashSectorOffset;
                    if (flashPageIndex == SAVES_PER_SECTOR - 1) {
                        Debug("Old and new in different pages: %d, %d\r\n", flashPage, flashPage+1);
                        buf[0].i = 0;
                        buf[1].s = settings;
                        flashSectorOffset = (flashPage + 1) * FLASH_PAGE_SIZE - sizeof(READER);
                    } else {
                        Debug("Old and new in same page: %d\r\n", flashPage);
                        flashSectorOffset = flashPage * FLASH_PAGE_SIZE;
                        buf[flashPageIndex].i = 0;
                        buf[flashPageIndex + 1].s = settings;
                    }
                    uint32_t ints = save_and_disable_interrupts();
                    flash_range_program(FLASH_TARGET_OFFSET + flashSectorOffset, (uint8_t *) buf, FLASH_PAGE_SIZE);
                    restore_interrupts(ints);
                    flashIndex++;
                }
                savedSettings = settings;
                flashStatus = FLASH_GOOD_DATA;
            }
            break;
        case FLASH_EMPTY:
            if (settingsChanged(settings, savedSettings)) {
                int flashSectorOffset = flashPage * FLASH_PAGE_SIZE;
                buf[flashPageIndex].s = settings;
                uint32_t ints = save_and_disable_interrupts();
                flash_range_program(FLASH_TARGET_OFFSET + flashSectorOffset, (uint8_t *) buf, FLASH_PAGE_SIZE);
                restore_interrupts(ints);
                savedSettings = settings;
                flashStatus = FLASH_GOOD_DATA;
            }
            break;
        case FLASH_FULL:
            if (settingsChanged(settings, savedSettings)) {
                buf[0].s = settings;
                uint32_t ints = save_and_disable_interrupts();
                flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
                flash_range_program(FLASH_TARGET_OFFSET, (uint8_t *) buf, FLASH_PAGE_SIZE);
                restore_interrupts(ints);
                flashIndex = 0;
                savedSettings = settings;
                flashStatus = FLASH_GOOD_DATA;
            }
            break;
    }
}
#include <string.h>
#include <stdint.h>

#include "LCD.h"
#include "pico/stdlib.h"

const uint8_t DEBOUNCE = 1;   // 125 mSec ticks for btn debounce
const uint8_t FLASH_CTR = 1;  // 125 mSec ticks for btn detect flash
const sFONT* fontA = &Liberation48;
const sFONT* fontB = &Liberation36;
const char* TO_MSG = "TIMEOUT";  // 8 char max w/ 36 pt
const char* ID_MSG = "ID";       // 6 char max w/ 48 pt

const uint8_t yStartTop = 10;

// TBA: EEPROM
const uint16_t CTRA = 600;
const uint16_t CTRB = 120;
const uint8_t pwm = 5;
const bool flash = true;
const bool twoTimers = false;


// colors
const uint16_t BACKGROUND = BLACK;
const uint16_t A_FOREGROUND = WHITE;
const uint16_t B_FOREGROUND = BYELLOW;

// updated by irq
volatile uint16_t ctrA = CTRA;
volatile uint16_t ctrB = CTRB;
volatile bool do_tickA = false;
volatile bool do_tickB = false;
volatile bool do_invert = false;
volatile bool in_flash_ctr = 0;
volatile bool do_clear_flash = false;
volatile uint8_t tick_ctr = 0;

enum {
    KEY_NONE = -1,
    KEY_A = 0,
    KEY_B,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_CTRL,
    KEY_COUNT,
};

int gpio_index(uint8_t gpio) {
    switch (gpio) {
        case BTN_A_PIN: return KEY_A;
        case BTN_B_PIN: return KEY_B;
        case BTN_UP_PIN: return KEY_UP;
        case BTN_DOWN_PIN: return KEY_DOWN;
        case BTN_LEFT_PIN: return KEY_LEFT;
        case BTN_RIGHT_PIN: return KEY_RIGHT;
        case BTN_CTRL_PIN: return KEY_CTRL;
        default: return KEY_NONE;
    }
}

struct BTN_IRQ_DATA {
    uint32_t last_state;
    uint8_t ctr;
    bool triggered;
};

struct BTN_IRQ_DATA irq_data[KEY_COUNT];

void gpio_irq(uint gpio, uint32_t events) {
    int8_t irq = gpio_index(gpio);
    if (irq != KEY_NONE) {
        irq_data[irq].last_state = events;
        irq_data[irq].ctr = DEBOUNCE;
        irq_data[irq].triggered = false;
    }
}

void init_key_irq(void) {
    for (uint8_t i = 0; i<KEY_COUNT; i++) {
        irq_data[i].last_state = 0;
        irq_data[i].ctr = 0;
        irq_data[i].triggered = false;
    }
    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_irq);
    gpio_set_irq_enabled(BTN_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_UP_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_DOWN_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_LEFT_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_RIGHT_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_CTRL_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

bool repeating_timer_callback(struct repeating_timer *t) {
    if (in_flash_ctr) {
        if (!--in_flash_ctr) {
            do_clear_flash = true;
        }
    }
    if (tick_ctr) {
        tick_ctr--;
        for (uint8_t i=0; i<KEY_COUNT; i++) {
            if (irq_data[i].ctr) {
                if (!--irq_data[i].ctr) {
                    irq_data[i].triggered = true;
                }
            }
        }
    } else {
        tick_ctr = 7;
        if (ctrA) {
            ctrA--;
            do_tickA = true;
        } else {
            do_invert = flash;
        }
        if (twoTimers && ctrB) {
            ctrB--;
            do_tickB = true;
        }
    }
}

void do_flash() {
    Invert(false);
    in_flash_ctr = FLASH_CTR;    
}

int main(void)
{
    static struct repeating_timer timer;

    set_sys_clock_48mhz();
    add_repeating_timer_ms(125, repeating_timer_callback, NULL, &timer);
    const uint yStartB = (LCD_1IN14_V2_WIDTH) - fontB->Height - yStartTop;

    InitHardware();

    BacklightLevel(0);
    /* LCD Init */
    InitLCD(HORIZONTAL);
    Clear(BACKGROUND);

    init_key_irq();

    bool inverse = false;
 
    uint widthA = 4 * fontA->Width + fontA->Width / 2;
    uint xStartA = (lcd.width - widthA) / 2;
    uint xStartAT = (lcd.width - strlen(ID_MSG) * fontA->Width) / 2;
    const uint yStartA = twoTimers ? yStartTop : (lcd.height - fontA->Height) / 2;

    uint widthB = 4 * fontB->Width + fontB->Width / 2;
    uint xStartB = (lcd.width - widthB) / 2;
    uint xStartBT = (lcd.width - strlen(TO_MSG) * fontB->Width) / 2;
    
    uint16_t prev_ctrA;
    uint16_t prev_ctrB;

    BacklightLevel(pwm);

    while(1){
        if (do_clear_flash) {
            do_clear_flash = false;
            Invert(true);
        }
        if (do_tickA)
        {
            do_tickA = false;
            if (ctrA)
            {
                DrawSeconds(xStartA, yStartA, ctrA, fontA, A_FOREGROUND, BACKGROUND, prev_ctrA);
                prev_ctrA = ctrA;
            } else {
                ClearWindow(BACKGROUND, 0, yStartA, lcd.width, fontA->Height);
                DrawString(xStartAT, yStartA, ID_MSG, fontA, A_FOREGROUND, BACKGROUND);
            }
       }

        if (do_tickB)
        {
            do_tickB = false;
            if (ctrB) {
                DrawSeconds(xStartB, yStartB, ctrB, fontB, B_FOREGROUND, BACKGROUND, prev_ctrB);
                prev_ctrB = ctrB;
            } else {
                ClearWindow(BACKGROUND, 0, yStartB, lcd.width, fontB->Height);
                DrawString(xStartBT, yStartB, TO_MSG, fontB, B_FOREGROUND, BACKGROUND);
            }
        }

        if (do_invert) {
            do_invert = false;
            Invert(inverse);
            inverse = !inverse;
        }

        if (irq_data[KEY_A].triggered) {
            irq_data[KEY_A].triggered = false;
            if (irq_data[KEY_A].last_state == GPIO_IRQ_EDGE_FALL) {
                do_flash();
                ctrA = CTRA;
                prev_ctrA = 0;
                ClearWindow(BACKGROUND, 0, yStartA, lcd.width, fontA->Height);
                DrawSeconds(xStartA, yStartA, ctrA, fontA, A_FOREGROUND, BACKGROUND, prev_ctrA);
                if (!ctrB) {
                    ctrB = CTRB;
                    prev_ctrB = 0;
                    ClearWindow(BACKGROUND, 0, yStartB, lcd.width, fontB->Height);
                    DrawSeconds(xStartB, yStartB, ctrB, fontB, B_FOREGROUND, BACKGROUND, prev_ctrB);
                }
            }
        }
        if (irq_data[KEY_B].triggered) {
            irq_data[KEY_B].triggered = false;
            if (irq_data[KEY_B].last_state == GPIO_IRQ_EDGE_FALL && twoTimers) {
                do_flash();
                ctrB = CTRB;
                prev_ctrB = 0;
                ClearWindow(BACKGROUND, 0, yStartB, lcd.width, fontB->Height);
                DrawSeconds(xStartB, yStartB, ctrB, fontB, B_FOREGROUND, BACKGROUND, prev_ctrB);
            }
        }
        if (irq_data[KEY_UP].triggered) {
            irq_data[KEY_UP].triggered = false;
        }
        if (irq_data[KEY_DOWN].triggered) {
            irq_data[KEY_DOWN].triggered = false;
        }
        if (irq_data[KEY_LEFT].triggered) {
            irq_data[KEY_LEFT].triggered = false;
        }
        if (irq_data[KEY_RIGHT].triggered) {
            irq_data[KEY_RIGHT].triggered = false;
        }
        if (irq_data[KEY_CTRL].triggered) {
            irq_data[KEY_CTRL].triggered = false;
        }
    }
    return 0;
}
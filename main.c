#include <string.h>
#include <stdint.h>

#include "lcd.h"
#include "display.h"
#include "config.h"
#include "pico/stdlib.h"


// TBA: EEPROM
const uint16_t CTRA = 10;
const uint16_t CTRB = 5;
const uint8_t pwm = 5;
const bool flash = true;
const bool twoTimers = true;



// updated by irq
volatile uint16_t ctrA = CTRA;
volatile uint16_t ctrB = CTRB;
volatile bool do_tickA = false;
volatile bool do_tickB = false;
volatile bool do_invert = false;
volatile bool in_flash_ctr = 0;
volatile bool do_clear_flash = false;


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

enum OPR_MODE {
    OPR_RUN,
    OPR_TIMER_A,
    OPR_TIMER_B,
    OPR_FLASH,
    OPR_PWM,
    OPR_COUNT
};

enum OPR_MODE oprMode = OPR_RUN;

bool heartbeat(struct repeating_timer *t) {
    static uint8_t tick_ctr = 7;

    for (uint8_t i=0; i<KEY_COUNT; i++) {
        if (irq_data[i].ctr) {
            if (!--irq_data[i].ctr) {
                irq_data[i].triggered = true;
            }
        }
    }

    if (in_flash_ctr) {
        if (!--in_flash_ctr) {
            do_clear_flash = true;
        }
    }

    if (!--tick_ctr) {
        tick_ctr = 7;
        if (oprMode == OPR_RUN) {
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
}

void do_flash() {
    Invert(false);
    in_flash_ctr = FLASH_CTR;    
}

void do_btn_a(void) {
    do_flash();
    ctrA = CTRA;
    if (twoTimers) {
        displayStringTop("");
        setSingle(false, true);
    }
    displayTimeTop(ctrA);
    if (!ctrB) {
        ctrB = CTRB;
        displayTimeBottom(ctrB);
    }
}

void do_btn_b(void) {
    if (twoTimers) {
        do_flash();
        ctrB = CTRB;
        displayTimeBottom(ctrB);
    }
}

void do_btn_up(void) {}
void do_btn_down(void) {}
void do_btn_left(void) {}
void do_btn_right(void) {}

void do_btn_ctrl(void) {
    oprMode++;
    Invert(true);
    if (oprMode == OPR_COUNT) {
        oprMode = OPR_RUN;
    }
    setSingle(oprMode == OPR_RUN && !twoTimers, false);
    switch (oprMode) {
        case OPR_RUN:
            break;
        case OPR_TIMER_A:
            displayTimeTop(CTRA);
            displayStringBottom("TIMER A");
            break;
        case OPR_TIMER_B:
            displayTimeTop(CTRB);
            displayStringBottom("TIMER B");
            break;
        case OPR_FLASH:
            displayStringTop("ON");
            displayStringBottom("FLASH");
            break;
        case OPR_PWM:
            displayStringTop("5");
            displayStringBottom("BKLGT");
            break;
    }
}



const void (*btn_handler[])(void) = {
    &do_btn_a,
    &do_btn_b,
    &do_btn_up,
    &do_btn_down,
    &do_btn_left,
    &do_btn_right,
    &do_btn_ctrl
};


void tickA_handler(void) {
    if (ctrA) {
        displayTimeTop(ctrA);
    } else {
        if (twoTimers) {
            displayStringTop("");
            setSingle(true, false);
        }
        displayStringTop(ID_MSG);
    }
}

void tickB_handler(void) {
    if (ctrB) {
        displayTimeBottom(ctrB);
    } else {
        displayStringBottom(TO_MSG);
    }
}

void setup() {
    static struct repeating_timer timer;

    set_sys_clock_48mhz();
    add_repeating_timer_ms(125, heartbeat, NULL, &timer);
    
    InitHardware();

    BacklightLevel(0);
    /* LCD Init */
    InitLCD(HORIZONTAL);
    Invert(true);
    Clear(BACKGROUND);

    init_key_irq();

    BacklightLevel(pwm);

    displayInit();
    setSingle(!twoTimers, false);

}

void loop() {
    static bool inverse = false;

    if (do_clear_flash) {
        do_clear_flash = false;
        Invert(true);
    }
    if (do_tickA)
    {
        do_tickA = false;
        tickA_handler();
    }

    if (do_tickB)
    {
        do_tickB = false;
        tickB_handler();
    }

    if (do_invert && oprMode == OPR_RUN) {
        do_invert = false;
        Invert(inverse);
        inverse = !inverse;
    }

    for (uint8_t i = 0; i < KEY_COUNT; i++) {
        if (irq_data[i].triggered) {
            irq_data[i].triggered = false;
            if (irq_data[i].last_state == GPIO_IRQ_EDGE_FALL) {
                btn_handler[i]();
            }
        }
    }
}

int main(void)
{
    setup();
    while(1){
        loop();
    }
    return 0;
}
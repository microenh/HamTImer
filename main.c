#include <string.h>
#include <stdint.h>

#include "lcd.h"
#include "display.h"
#include "config.h"
#include "pico/stdlib.h"


// TBA: EEPROM
uint16_t CTRA = 600;
uint16_t CTRB = 120;
uint8_t pwm = 5;
bool flash = true;

// updated by irq
volatile uint16_t ctrA;
volatile uint16_t ctrB;
volatile bool do_tickA = false;
volatile bool do_tickB = false;
volatile bool do_invert = false;
volatile bool in_flash_ctr = 0;
volatile bool do_clear_flash = false;

const char *off_on[] = {"OFF", "ON"};

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
            if (CTRB && ctrB) {
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
    displayShowBottom(CTRB);
    displayTime(true, ctrA);
    if (!ctrB) {
        ctrB = CTRB;
        displayTime(false, ctrB);
    }
}

void do_btn_b(void) {
    if (CTRB) {
        do_flash();
        ctrB = CTRB;
        displayTime(false, ctrB);
    }
}

static void updateTimerA(const bool up) {
    int16_t new = CTRA + (up ? 15 : -15);
    if (new > (60 * 99)) {
        new = 60 * 99;
    } else if (new < 15) {
        new = 15;
    }
    if (new != CTRA) {
        CTRA = new;
        ctrA = CTRA;
        displayTime(true, CTRA);
    }
}

static void updateTimerB(const bool up) {
    int16_t new = CTRB + (up ? 15 : -15);
    if (new > (60 * 99)) {
        new = 60 * 99;
    } else if (new < 0) {
        new = 0;
    }
    if (new != CTRB) {
        CTRB = new;
        ctrB = CTRB;
        displayTime(true, CTRB);
    }

}

static char buffer[4];

void valueStr(uint8_t value) {
    uint8_t i = 0;
    
    uint8_t x = value / 100;
    if (x) {
        buffer[i++] = x + '0';
    }
    x = value % 100 / 10;
    if (i || x) {
        buffer[i++] = x + '0';
    }
    buffer[i++] = value % 10 + '0';
    buffer[i] = 0;
}

static void toggleFlash(void) {
    flash = !flash;
    displayString(true, off_on[flash]);
}

static void updatePWM(const bool up) {
    int16_t new = pwm + (up ? 1 : -1);

    if (new > 100) {
        new = 100;
    } else if (new < 0) {
        new = 0;
    }
    if (new != pwm) {
        pwm = new;
        valueStr(pwm);
        displayString(true, buffer);
        BacklightLevel(pwm);
    }
}

static void do_up_down(const bool up) {
    switch (oprMode) {
        case OPR_TIMER_A:
            updateTimerA(up);
            break;
        case OPR_TIMER_B:
            updateTimerB(up);
            break;
        case OPR_FLASH:
            toggleFlash();
            break;
        case OPR_PWM:
            updatePWM(up);
            break;
    }   
}

void do_btn_up(void) {
    do_up_down(true);
}

void do_btn_down(void) {
    do_up_down(false);
}

void do_btn_left(void) {}
void do_btn_right(void) {}

void do_btn_ctrl(void) {
    oprMode++;
    Invert(true);
    if (oprMode == OPR_COUNT) {
        oprMode = OPR_RUN;
    }
    switch (oprMode) {
        case OPR_RUN:
            if (ctrA) {
                displayShowBottom(CTRB);
                displayTime(true, ctrA);
            } else {
                displayShowBottom(false);
                displayString(true, ID_MSG);
            }
            if (ctrB) {
                displayTime(false, ctrB);
            } else {
                displayString(false, TO_MSG);
            }
            break;
        case OPR_TIMER_A:
            displayShowBottom(true);
            displayTime(true, CTRA);
            displayString(false, "TIMER A");
            break;
        case OPR_TIMER_B:
            displayTime(true, CTRB);
            displayString(false, "TIMER B");
            break;
        case OPR_FLASH:
            displayString(true, off_on[flash]);
            displayString(false, "FLASH");
            break;
        case OPR_PWM:
            valueStr(pwm);
            displayString(true, buffer);
            displayString(false, "BKLGT");
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
        displayTime(true, ctrA);
    } else {
        if (CTRB) {
            displayShowBottom(false);
        }
        displayString(true, ID_MSG);
    }
}

void tickB_handler(void) {
    if (ctrB) {
        displayTime(false, ctrB);
    } else {
        displayString(false, TO_MSG);
    }
}

void setup() {
    static struct repeating_timer timer;

    ctrA = CTRA;
    ctrB = CTRB;

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
    displayShowBottom(CTRB);
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
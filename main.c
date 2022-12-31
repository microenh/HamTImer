#include "LCD_1in14_V2.h"

#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include "Debug.h"
#include <stdlib.h> // malloc() free()
#include <string.h>
#include "pico/stdlib.h"

#include "Infrared.h"

const uint8_t DEBOUNCE = 1;  // 125 Hz ticks
const uint8_t FLASH_CTR = 1;  // 125 Hz ticks
const sFONT* fontA = &Liberation48;
const sFONT* fontB = &Liberation36;
const char* TO_MSG = "TIME OUT";  // 8 char max w/ 36 pt
const char* ID_MSG = "ID";  // 6 char max w / 48 pt

const uint yStartTop = 10;


// GPIO pins for buttons
const uint8_t keyA = 15; 
const uint8_t keyB = 17; 
const uint8_t up = 2;
const uint8_t down = 18;
const uint8_t left = 16;
const uint8_t right = 20;
const uint8_t ctrl = 3;


// TBA: EEPROM
const u_int16_t CTRA = 600;
const u_int16_t CTRB = 120;
const u_int8_t pwm = 5;
const bool flash = true;
const bool twoTimers = true;


// colors
const u_int16_t BACKGROUND = BLACK;
const u_int16_t A_FOREGROUND = WHITE;
const u_int16_t B_FOREGROUND = BYELLOW;

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

int gpio_index(uint gpio) {
    switch (gpio) {
        case keyA: return KEY_A;
        case keyB: return KEY_B;
        case up: return KEY_UP;
        case down: return KEY_DOWN;
        case left: return KEY_LEFT;
        case right: return KEY_RIGHT;
        case ctrl: return KEY_CTRL;
        default: return KEY_NONE;
    }
}

struct IRQ_DATA {
    uint32_t last_state;
    uint8_t ctr;
    bool triggered;
};

struct IRQ_DATA irq_data[KEY_COUNT];


void gpio_irq(uint gpio, uint32_t events) {
    int8_t irq = gpio_index(gpio);
    if (irq != KEY_NONE) {
        irq_data[irq].last_state = events;
        irq_data[irq].ctr = DEBOUNCE;
        irq_data[irq].triggered = false;
    }
}


void init_irq(void) {
    for (uint8_t i = 0; i<KEY_COUNT; i++) {
        irq_data[i].last_state = 0;
        irq_data[i].ctr = 0;
        irq_data[i].triggered = false;
    }
    gpio_set_irq_enabled_with_callback(keyA, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_irq);
    gpio_set_irq_enabled(keyB, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(up, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(down, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(left, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(right, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(ctrl, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
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
    LCD_1IN14_V2_Invert(false);
    in_flash_ctr = FLASH_CTR;    
}

int main(void)
{
    static struct repeating_timer timer;

    set_sys_clock_48mhz();
    add_repeating_timer_ms(125, repeating_timer_callback, NULL, &timer);
    const uint yStartB = (LCD_1IN14_V2_WIDTH) - fontB->Height - yStartTop;

    sleep_ms(100);
    printf("LCD_1in14_test Demo\r\n");
    if(DEV_Module_Init()!=0){
        return -1;
    }
    /* LCD Init */
    printf("1.14inch LCD demo...\r\n");
    LCD_1IN14_V2_Init(HORIZONTAL);

    DEV_SET_PWM(0);
    LCD_1IN14_V2_Clear(BACKGROUND);

    // LCD_1IN14_V2_Clear(BLACK);
    // LCD_1IN14_V2_Clear(RED);
    // LCD_1IN14_V2_Clear(BRED);
    // LCD_1IN14_V2_Clear(GREEN);
    // LCD_1IN14_V2_Clear(BGREEN);
    // LCD_1IN14_V2_Clear(BLUE);
    // LCD_1IN14_V2_Clear(BBLUE);
    // LCD_1IN14_V2_Clear(CYAN);
    // LCD_1IN14_V2_Clear(BCYAN);
    // LCD_1IN14_V2_Clear(MAGENTA);
    // LCD_1IN14_V2_Clear(BMAGENTA);
    // LCD_1IN14_V2_Clear(YELLOW);
    // LCD_1IN14_V2_Clear(BYELLOW);
    // LCD_1IN14_V2_Clear(GRAY);
    // LCD_1IN14_V2_Clear(WHITE);
    
    // LCD_1IN14_V2_Clear(ORANGE);
    // LCD_1IN14_V2_Clear(PINK);

    UWORD Imagesize = LCD_1IN14_V2_HEIGHT * LCD_1IN14_V2_WIDTH * 2;
    UWORD *imageBuffer;
    if((imageBuffer = (UWORD *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        exit(0);
    }
    // /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
    Paint_NewImage((UBYTE *)imageBuffer, LCD_1IN14_V2.WIDTH, LCD_1IN14_V2.HEIGHT, 0, BACKGROUND);
    Paint_SetScale(65);
    
    // /*2.Drawing on the image*/
    SET_Infrared_PIN(keyA);    
    SET_Infrared_PIN(keyB);
		 
	SET_Infrared_PIN(up);
    SET_Infrared_PIN(down);
    SET_Infrared_PIN(left);
    SET_Infrared_PIN(right);
    SET_Infrared_PIN(ctrl);

    init_irq();
        
    bool inverse = false;
 
    uint widthA = 4 * fontA->Width + fontA->Width / 2;
    uint xStartA = (LCD_1IN14_V2_HEIGHT - widthA) / 2;
    uint xStartAT = (LCD_1IN14_V2_HEIGHT - strlen(ID_MSG) * fontA->Width) / 2;
    const uint yStartA = twoTimers ? yStartTop : (LCD_1IN14_V2_WIDTH - fontA->Height) / 2;

    uint widthB = 4 * fontB->Width + fontB->Width / 2;
    uint xStartB = (LCD_1IN14_V2_HEIGHT - widthB) / 2;
    uint xStartBT = (LCD_1IN14_V2_HEIGHT - strlen(TO_MSG) * fontB->Width) / 2;
    
    uint16_t prev_ctrA;
    uint16_t prev_ctrB;

    DEV_SET_PWM(pwm);

    Paint_Clear(BACKGROUND);

    while(1){
        if (do_clear_flash) {
            do_clear_flash = false;
            LCD_1IN14_V2_Invert(true);
        }
        if (do_tickA)
        {
            do_tickA = false;
            if (ctrA)
            {
                Paint_DrawSeconds(xStartA, yStartA, ctrA, fontA, A_FOREGROUND, BACKGROUND, prev_ctrA);
                prev_ctrA = ctrA;
            } else {
                Paint_ClearWindows(0, yStartA, LCD_1IN14_V2_HEIGHT, yStartA + fontA->Height, BACKGROUND);
                Paint_DrawString(xStartAT, yStartA, ID_MSG, fontA, A_FOREGROUND, BLACK);
                LCD_1IN14_V2_DisplayWindows(0, yStartA, LCD_1IN14_V2_HEIGHT, yStartA + fontA->Height, imageBuffer);
            }
       }

        if (do_tickB)
        {
            do_tickB = false;
            if (ctrB) {
                Paint_DrawSeconds(xStartB, yStartB, ctrB, fontB, B_FOREGROUND, BACKGROUND, prev_ctrB);
                prev_ctrB = ctrB;
            } else {
                Paint_ClearWindows(0, yStartB, LCD_1IN14_V2_HEIGHT, yStartB + fontB->Height, BACKGROUND);
                Paint_DrawString(xStartBT, yStartB, TO_MSG, fontB, B_FOREGROUND, BACKGROUND);
                LCD_1IN14_V2_DisplayWindows(0, yStartB, LCD_1IN14_V2_HEIGHT, yStartB + fontB->Height, imageBuffer);
            }
        }

        if (do_invert) {
            do_invert = false;
            LCD_1IN14_V2_Invert(inverse);
            inverse = !inverse;
        }

        if (irq_data[KEY_A].triggered) {
            irq_data[KEY_A].triggered = false;
            if (irq_data[KEY_A].last_state == GPIO_IRQ_EDGE_FALL) {
                do_flash();
                ctrA = CTRA;
                prev_ctrA = 0;
                Paint_ClearWindows(0, yStartA, LCD_1IN14_V2_HEIGHT, yStartA + fontA->Height, BACKGROUND);
                LCD_1IN14_V2_DisplayWindows(0, yStartA, LCD_1IN14_V2_HEIGHT, yStartA + fontA->Height, imageBuffer);
                if (!ctrB) {
                    ctrB = CTRB;
                    prev_ctrB = 0;
                    Paint_ClearWindows(0, yStartB, LCD_1IN14_V2_HEIGHT, yStartB + fontB->Height, BACKGROUND);
                    LCD_1IN14_V2_DisplayWindows(0, yStartB, LCD_1IN14_V2_HEIGHT, yStartB + fontB->Height, imageBuffer);
                }
            }
        }
        if (irq_data[KEY_B].triggered) {
            irq_data[KEY_B].triggered = false;
            if (irq_data[KEY_B].last_state == GPIO_IRQ_EDGE_FALL && twoTimers) {
                do_flash();
                ctrB = CTRB;
                prev_ctrB = 0;
                Paint_ClearWindows(0, yStartB, LCD_1IN14_V2_HEIGHT, yStartB + fontB->Height, BACKGROUND);
                LCD_1IN14_V2_DisplayWindows(0, yStartB, LCD_1IN14_V2_HEIGHT, yStartB + fontB->Height, imageBuffer);
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

    /* Module Exit */
    free(imageBuffer);
    imageBuffer = NULL;
    
    
    DEV_Module_Exit();
    return 0;
}
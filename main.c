#include "LCD_1in14_V2.h"

#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include "Debug.h"
#include <stdlib.h> // malloc() free()
#include "pico/stdlib.h"

#include "Infrared.h"

struct repeating_timer timer;

volatile UWORD ctr = 0;
volatile bool do_tick = 0;
volatile UBYTE tick_ctr = 0;

const uint8_t keyA = 15; 
const uint8_t keyB = 17; 

const uint8_t up = 2;
const uint8_t down = 18;
const uint8_t left = 16;
const uint8_t right = 20;
const uint8_t ctrl = 3;

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

const uint8_t DEBOUNCE = 2;

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
        if (ctr) {
            ctr--;
            do_tick = 1;
        }
    }
}

int main(void)
{
    set_sys_clock_48mhz();
    add_repeating_timer_ms(125, repeating_timer_callback, NULL, &timer);


    DEV_Delay_ms(100);
    printf("LCD_1in14_test Demo\r\n");
    if(DEV_Module_Init()!=0){
        return -1;
    }
    /* LCD Init */
    printf("1.14inch LCD demo...\r\n");
    LCD_1IN14_V2_Init(HORIZONTAL);
    DEV_SET_PWM(50);
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
    Paint_NewImage((UBYTE *)imageBuffer, LCD_1IN14_V2.WIDTH, LCD_1IN14_V2.HEIGHT, 0, WHITE);
    Paint_SetScale(65);
    Paint_Clear(WHITE);
    // Paint_SetRotate(ROTATE_0);
    // Paint_Clear(BLACK);
    
    // /* GUI */
    printf("drawing...\r\n");
    // /*2.Drawing on the image*/
#if 0
    Paint_DrawPoint(2,1, BLACK, DOT_PIXEL_1X1,  DOT_FILL_RIGHTUP);//240 240
    Paint_DrawPoint(2,6, BLACK, DOT_PIXEL_2X2,  DOT_FILL_RIGHTUP);
    Paint_DrawPoint(2,11, BLACK, DOT_PIXEL_3X3, DOT_FILL_RIGHTUP);
    Paint_DrawPoint(2,16, BLACK, DOT_PIXEL_4X4, DOT_FILL_RIGHTUP);
    Paint_DrawPoint(2,21, BLACK, DOT_PIXEL_5X5, DOT_FILL_RIGHTUP);
    Paint_DrawLine( 10,  5, 40, 35, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    Paint_DrawLine( 10, 35, 40,  5, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);

    Paint_DrawLine( 80,  20, 110, 20, CYAN, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine( 95,   5,  95, 35, CYAN, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

    Paint_DrawRectangle(10, 5, 40, 35, RED, DOT_PIXEL_2X2,DRAW_FILL_EMPTY);
    Paint_DrawRectangle(45, 5, 75, 35, BLUE, DOT_PIXEL_2X2,DRAW_FILL_FULL);

    Paint_DrawCircle(95, 20, 15, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(130, 20, 15, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);


    Paint_DrawNum (50, 40 ,9.87654321, &Font20,3,  WHITE,  BLACK);
    Paint_DrawString_EN(1, 40, "ABC", &Font24, 0x000f, 0xfff0);
    Paint_DrawString_EN(1, 60, "Pico-LCD-1.14", &Font16, RED, WHITE); 

    // /*3.Refresh the picture in RAM to LCD*/
    LCD_1IN14_V2_Display(imageBuffer);
    DEV_Delay_ms(2000);
    // DEV_SET_PWM(10);
    /*
    DEV_Digital_Write(LCD_DC_PIN, 0);
    DEV_Digital_Write(LCD_CS_PIN, 0);
    DEV_SPI_WriteByte(0x28);
    DEV_Digital_Write(LCD_CS_PIN, 1);
    DEV_SET_PWM(0);
    */

#endif
#if 0
     
     Paint_DrawImage(gImage_1inch14_1,0,0,240,135);
    
     LCD_1IN14_V2_Display(imageBuffer);
     DEV_Delay_ms(2000);
     
#endif
   

    SET_Infrared_PIN(keyA);    
    SET_Infrared_PIN(keyB);
		 
	SET_Infrared_PIN(up);
    SET_Infrared_PIN(down);
    SET_Infrared_PIN(left);
    SET_Infrared_PIN(right);
    SET_Infrared_PIN(ctrl);

    init_irq();
    
    Paint_Clear(WHITE);
    Paint_DrawRectangle(208,12,229,32, BRED, DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
    Paint_DrawRectangle(208,103,229,123, BRED, DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
    Paint_DrawRectangle(37,35,58,55, BRED, DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
    Paint_DrawRectangle(37,85,58,105, BRED, DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
    Paint_DrawRectangle(12,60,33,80, BRED, DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
    Paint_DrawRectangle(62,60,83,80, BRED, DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
    Paint_DrawRectangle(37,60,58,80, BRED, DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
    LCD_1IN14_V2_Display(imageBuffer);
    
    
    while(1){
        if (do_tick)
        {
            do_tick = 0;

            PAINT_TIME time;
            time.Hour = ctr / 3600;
            time.Min = (ctr % 3600) / 60;
            time.Sec = (ctr % 3600) % 60;

            Paint_DrawTime(0, 0, &time, &Font20, BLACK, WHITE);


           LCD_1IN14_V2_DisplayWindows(0, 0, 8 * 17, 24,imageBuffer);
        }
        if (irq_data[KEY_A].triggered) {
            irq_data[KEY_A].triggered = false;
            if (irq_data[KEY_A].last_state == GPIO_IRQ_EDGE_FALL) {
                ctr = 600;
            }
            Paint_DrawRectangle(208,12,228,32, irq_data[KEY_A].last_state == GPIO_IRQ_EDGE_FALL ? BRED : WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(208,12,228,32, imageBuffer);
        }
        if (irq_data[KEY_B].triggered) {
            irq_data[KEY_B].triggered = false;
            Paint_DrawRectangle(208,103,228,123, irq_data[KEY_B].last_state == GPIO_IRQ_EDGE_FALL ? BRED : WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(208,103,228,123, imageBuffer);
        }
        if (irq_data[KEY_UP].triggered) {
            irq_data[KEY_UP].triggered = false;
            Paint_DrawRectangle(37,35,57,55, irq_data[KEY_UP].last_state == GPIO_IRQ_EDGE_FALL ? BRED : WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(37,35,57,55, imageBuffer);
        }
        if (irq_data[KEY_DOWN].triggered) {
            irq_data[KEY_DOWN].triggered = false;
            Paint_DrawRectangle(37,85,57,105, irq_data[KEY_DOWN].last_state == GPIO_IRQ_EDGE_FALL ? BRED : WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(37,85,57,105, imageBuffer);
        }
        if (irq_data[KEY_LEFT].triggered) {
            irq_data[KEY_LEFT].triggered = false;
            Paint_DrawRectangle(12,60,32,80, irq_data[KEY_LEFT].last_state == GPIO_IRQ_EDGE_FALL ? BRED : WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(12,60,32,80, imageBuffer);
        }
        if (irq_data[KEY_RIGHT].triggered) {
            irq_data[KEY_RIGHT].triggered = false;
            Paint_DrawRectangle(62,60,82,80, irq_data[KEY_RIGHT].last_state == GPIO_IRQ_EDGE_FALL ? BRED : WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(62,60,82,80, imageBuffer);
        }
        if (irq_data[KEY_CTRL].triggered) {
            irq_data[KEY_CTRL].triggered = false;
            Paint_DrawRectangle(37,60,57,80, irq_data[KEY_CTRL].last_state == GPIO_IRQ_EDGE_FALL ? BRED : WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(37,60,57,80, imageBuffer);
        }
        #if 0
        if(DEV_Digital_Read(keyA ) == 0){
            ctr = 600;
            Paint_DrawRectangle(208,12,228,32, 0xF800, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(208,12,228,32,imageBuffer);
            printf("gpio =%d\r\n",keyA);
        }
        else{
            Paint_DrawRectangle(208,12,228,32, WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(208,12,228,32,imageBuffer);
        }
            
        if(DEV_Digital_Read(keyB ) == 0){
            Paint_DrawRectangle(208,103,228,123, 0xF800, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(208,103,228,123,imageBuffer);
            printf("gpio =%d\r\n",keyB);
        }
        else{
            Paint_DrawRectangle(208,103,228,123, WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(208,103,228,123,imageBuffer);
        }
        
        if(DEV_Digital_Read(up ) == 0){
            Paint_DrawRectangle(37,35,57,55, 0xF800, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(37,35,57,55,imageBuffer);
            printf("gpio =%d\r\n",up);
        }
        else{
            Paint_DrawRectangle(37,35,57,55, WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(37,35,57,55,imageBuffer);
        }

        if(DEV_Digital_Read(down ) == 0){
            Paint_DrawRectangle(37,85,57,105, 0xF800, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(37,85,57,105,imageBuffer);
            printf("gpio =%d\r\n",down);
        }
        else{
            Paint_DrawRectangle(37,85,57,105, WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(37,85,57,105,imageBuffer);
        }
        
        if(DEV_Digital_Read(left ) == 0){
            Paint_DrawRectangle(12,60,32,80, 0xF800, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(12,60,32,80,imageBuffer);
            printf("gpio =%d\r\n",left);
        }
        else{
            Paint_DrawRectangle(12,60,32,80, WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(12,60,32,80,imageBuffer);
        }
            
        if(DEV_Digital_Read(right ) == 0){
            Paint_DrawRectangle(62,60,82,80, 0xF800, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(62,60,82,80,imageBuffer);
            printf("gpio =%d\r\n",right);
        }
        else{
            Paint_DrawRectangle(62,60,82,80, WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(62,60,82,80,imageBuffer);
        }
        
        if(DEV_Digital_Read(ctrl ) == 0){
            Paint_DrawRectangle(37,60,57,80, 0xF800, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(37,60,57,80,imageBuffer);
            printf("gpio =%d\r\n",ctrl);
        }
        else{
            Paint_DrawRectangle(37,60,57,80, WHITE, DOT_PIXEL_2X2,DRAW_FILL_FULL);
            LCD_1IN14_V2_DisplayWindows(37,60,57,80,imageBuffer);
        }
        #endif
    }

    /* Module Exit */
    free(imageBuffer);
    imageBuffer = NULL;
    
    
    DEV_Module_Exit();
    return 0;
}
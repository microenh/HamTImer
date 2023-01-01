#include "Infrared.h"
#include "Debug.h"


void SET_Infrared_PIN(uint8_t PIN)
{
    int Infrared_PIN;

    Infrared_PIN = PIN;
    DEV_GPIO_Mode(Infrared_PIN, 0);
    gpio_pull_up(Infrared_PIN); //Need to pull up
}

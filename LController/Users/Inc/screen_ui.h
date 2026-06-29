#ifndef _LCD_H
#define _LCD_H

#include "tft_driver.h"

#define BEEPGPIOx GPIOC
#define BEEPGPIO_Pin GPIO_PIN_15

void LCD_ShowHome();
void R1_Interface();
void R2_Interface();
// void Toggle_status_show();
void LCD_flash();
void JS_measure_show();
void Auto_Connect_show();
void Beep(uint8_t times, uint16_t delaytime);
void HalBeep(uint8_t times, uint16_t delaytime);
#endif
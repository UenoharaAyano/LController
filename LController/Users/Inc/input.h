#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "main.h"
#include "stdbool.h"

#define Kv_fast 800
#define Kv_slow 80

#define joystickmeasureFlag 0 // 摇杆校准
/*
右侧3x3 矩阵键盘
两个顶部按键（按压拉低）
四个拨动开关（向上拉高 向下拉低）
一个左摇杆（含按键）
一个右摇杆（含按键）
*/
/*用于定义按键对应引脚的GPIO结构体*/
typedef struct Key_S
{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
} Key_S;
/***********************************key Name User Define Begin********************************/
/*枚举按键 用于定义按键对应引脚*/
typedef enum Key_E
{
    K_C1,
    K_C2,
    K_C3,
    K_C4,
    K_R1,
    K_R2,
    K_R3,
    bottom_1,
    bottom_2,
    bottom_3,
    bottom_4,
    TOP_LEFT,
    TOP_RIGHT,
    JOYSTICK_LEFT,   // 左摇杆按键 (PB10)
    JOYSTICK_RIGHT,  // 右摇杆按键 (PA15)
    TOGGLE_1,
    TOGGLE_2,
    TOGGLE_3,
    TOGGLE_4,
} Key_E;
// 存储19个按键引脚的结构体数组
/***********************************key Name User Define end********************************/

void KeyInit();                   // 初始化按键
GPIO_PinState KeyRead(Key_E key); // 返回按键状态
void KRSet(Key_E key);            // 拉高按键对应引脚
void KRReset(Key_E key);          // 拉低按键对应引脚

uint8_t KeyScan();    // 右边矩阵键盘的扫描
uint8_t ToggleScan(); // 返回拨动开关状态

typedef struct JS_Data
{
    int16_t x;
    int16_t y;
} JS;
typedef struct JScontrol_S
{
    short velX;
    short velY;
    short angW;
} JScontrol_S;

extern JScontrol_S JScontrolmsg; // 摇杆控制消息
extern short Kv;                 // 左摇杆速度系数
extern short Kw;                 // 右摇杆角速度系数

bool JoystickInit();
void joystickLeft_scan();  // 左边霍尔摇杆的扫描
void joystickRight_scan(); // 右边摇杆的扫描

// 需要满电状态测
/*
max 8.4v *1/4 2.1v  4096*(2.1/3.3)=
min 3.7v *1/4 0.9v
*/
// 理论值
#define MAX_power 2430
#define MIN_power 2160

// uint8_t power_scan(); // 返回电量

void R1_key_function(uint8_t keynum);
void R1_toggle_function(uint8_t togglenum);
void R2_key_function(uint8_t keynum);
void R2_toggle_function(uint8_t togglenum);
#endif

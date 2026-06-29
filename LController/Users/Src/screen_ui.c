#include "screen_ui.h"
#include "robot_msg.h"
#include "app_state.h"
#include "cmsis_os.h"
#include "input.h"
#include "app_init.h"

void LCD_ShowHome()
{
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
}
// 摇杆模式显示 - 仅显示快慢模式
void JS_mode_show()
{
    // 绘制摇杆显示圆圈
    if (flag.LCDfirstshow)
    {
        Draw_Circle(25, 40, 25, WHITE);
        LCD_DrawRectangle(17, 15, 33, 65, BLACK);
        LCD_DrawRectangle(0, 32, 50, 48, BLACK);
    }
    // 快慢模式切换显示
    if (flag.JSfast_slow != flag.JSfast_slow_last || flag.LCDfirstshow)
    {
        if (flag.JSfast_slow == 1)		// 快速模式
            Draw_Circle(25, 40, 15, BLACK);
        else                          // 慢速模式
            Draw_Circle(25, 40, 15, BRRED);
        flag.JSfast_slow_last = flag.JSfast_slow;
    }
}
void R1_Interface()
{
    LCD_ShowHome();
    JS_mode_show();
    LCD_DrawRectangle(90, 104, 150, 116, WHITE);
	LCD_ShowString(5, 5, (uint8_t *)"R1",WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 10, (uint8_t *)"disX:", WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 30, (uint8_t *)"disY:", WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 50, (uint8_t *)"Yaw:", WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 70, (uint8_t *)"Pitch:", WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 90, (uint8_t *)"YIS:", WHITE, BLACK, 12, 0);
//    LCD_ShowString(10, 75, (uint8_t *)"RX", WHITE, BLACK, 12, 0);
//    LCD_ShowString(25, 75, (uint8_t *)"TX", WHITE, BLACK, 12, 0);
}
void R2_Interface()
{
    LCD_ShowHome();
    JS_mode_show();
    LCD_DrawRectangle(90, 84, 150, 96, WHITE);
		LCD_ShowString(5, 5, (uint8_t *)"R2",WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 10, (uint8_t *)"Pos_X:", WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 30, (uint8_t *)"Pos_Y:", WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 50, (uint8_t *)"Pos_A:", WHITE, BLACK, 12, 0);
//    LCD_ShowString(10, 75, (uint8_t *)"RX", WHITE, BLACK, 12, 0);
//    LCD_ShowString(25, 75, (uint8_t *)"TX", WHITE, BLACK, 12, 0);
}

/*
void Toggle_status_show()
{
    LCD_ShowIntNum(15, 103, (temp.ToggleNum & 0x80) >> 7, 1, WHITE, BLACK, 24);
    LCD_ShowIntNum(55, 103, (temp.ToggleNum & 0x40) >> 6, 1, WHITE, BLACK, 24);
    LCD_ShowIntNum(93, 103, (temp.ToggleNum & 0x20) >> 5, 1, WHITE, BLACK, 24);
    LCD_ShowIntNum(133, 103, (temp.ToggleNum & 0x10) >> 4, 1, WHITE, BLACK, 24);
}
// 连接状态显示
*/

/*
void status_show()
{
    if (g_rfRxFlag)
        Draw_Circle(15, 90, 3, YELLOW);
    if (g_rfTxBusy)
        Draw_Circle(30, 90, 3, GREEN);
}
*/

void LCD_flash()
{
    JS_mode_show();
    temp.L1xNow = 25 + 20 * JScontrolmsg.velY / K;
    temp.L1yNow = 40 + 20 * JScontrolmsg.velX / K;
    temp.RxNow = 120 + 20 * JScontrolmsg.angW / Kw;
    if (flag.RobotMode == 0)
    {
        LCD_ShowFloatNum1(91, 10, r1backmsg.disX, 7, WHITE, BLACK, 12);	// 位数不算符号
        LCD_ShowFloatNum1(91, 30, r1backmsg.disY, 7, WHITE, BLACK, 12);
        LCD_ShowFloatNum1(91, 50, r1backmsg.Yaw, 7, WHITE, BLACK, 12);
        LCD_ShowFloatNum1(91, 70, r1backmsg.pitch, 7, WHITE, BLACK, 12);
        LCD_ShowFloatNum1(91, 90, r1backmsg.YIS, 7, WHITE, BLACK, 12);
		}
    // else
    // {
    //     LCD_ShowFloatNum1(91, 10, r2backmsg.X, 7, WHITE, BLACK, 12);
    //     LCD_ShowFloatNum1(91, 35, r2backmsg.Y, 7, WHITE, BLACK, 12);
    //     LCD_ShowFloatNum1(91, 60, r2backmsg.angle, 7, WHITE, BLACK, 12);
    // }
    Draw_Circle(temp.L1xlast, temp.L1ylast, 1, BLACK);
    Draw_Circle(temp.Rxlast, 110, 3, BLACK);
    Draw_Circle(temp.L1xNow, temp.L1yNow, 1, LIGHTGREEN);
    Draw_Circle(temp.RxNow, 110, 3, LIGHTGREEN);
    temp.L1xlast = temp.L1xNow;
    temp.L1ylast = temp.L1yNow;
    temp.Rxlast = temp.RxNow;
}

void JS_measure_show()
{
    LCD_ShowHome();
    LCD_ShowString(20, 50, (uint8_t *)"joystick measure...", WHITE, BLACK, 12, 0);
}


//void Auto_Connect_show()
//{
//    LCD_ShowHome();
//    LCD_ShowString(0, 50, (uint8_t *)"Auto connect", WHITE, BLACK, 24, 0);
//    while (bw.autoconnect)
//    {
//        osDelay(100);
//    }
//}

void Beep(uint8_t times, uint16_t delaytime)
{
    for (uint8_t i = 0; i < times; i++)
    {
        HAL_GPIO_WritePin(BEEPGPIOx, BEEPGPIO_Pin, GPIO_PIN_SET);
        osDelay(delaytime);
        HAL_GPIO_WritePin(BEEPGPIOx, BEEPGPIO_Pin, GPIO_PIN_RESET);
        osDelay(delaytime);
    }
}
void HalBeep(uint8_t times, uint16_t delaytime)
{
    for (uint8_t i = 0; i < times; i++)
    {
        HAL_GPIO_WritePin(BEEPGPIOx, BEEPGPIO_Pin, GPIO_PIN_SET);
        HAL_Delay(delaytime);
        HAL_GPIO_WritePin(BEEPGPIOx, BEEPGPIO_Pin, GPIO_PIN_RESET);
        HAL_Delay(delaytime);
    }
}

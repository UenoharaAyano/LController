#include "app_init.h"
#include "tft_driver.h"
#include "input.h"
#include "touchscreen.h"
#include "robot_msg.h"
#include "uart_protocol.h"
#include "app_state.h"
#include "screen_ui.h"
#include "fifo_queue.h"

void Handle_Start(void)
{
	FlagInit();
	TempInit();
	KeyInit();
	r1controlmsg.area = 0;
	LCD_ShowHome();
	LCD_Init();
	JoystickInit();	// 初始化滤波器，开启摇杆ADC采样
	Usart2DataFrameInit(USART_SUM_CHECK);	// 初始化U2串口数据包框架
	LCD_ShowHome();

#if joystickmeasureFlag==1	// 摇杆极限值校准，日常使用不编译
	JS_measure_show();
    while (joystickmeasureFlag)
    {
        joystickLeft_scan();
        joystickRight_scan();
    }
	LCD_ShowHome();
#else
	// 设置模式,默认为R1
	LCD_ShowString(10 ,10 ,(uint8_t *)"R1",WHITE ,BLACK ,32 ,0 );
	// LCD_ShowString(10 ,42 ,(uint8_t *)"Select Mode...",WHITE ,BLACK ,12 ,0);	// 删除选择步骤
	LCD_ShowString(10 ,42 ,(uint8_t *)"Waiting Enter...",WHITE ,BLACK ,12 ,0);
	HAL_Delay(50);
	for(;;)	// 此时串口屏处于start页面，等待点击
		{
			// if(KeyRead(TOP_LEFT)==GPIO_PIN_RESET)	// 切换模式,左肩键
			// {
			// 	flag.RobotMode=!flag.RobotMode;
			// 	if(!flag.RobotMode)
			// 	{
			// 		LCD_ShowString(10 ,10 ,(uint8_t *)"R1",WHITE ,BLACK ,32 ,0 );
			// 		MsgSend("RobotMode=0");	// 同步控制串口屏(小屏显示为主)
			// 	}
			// 	else
			// 	{
			// 		LCD_ShowString(10 ,10 ,(uint8_t *)"R2",WHITE ,BLACK ,32 ,0 );
			// 		MsgSend("RobotMode=1");
			// 	}
			// 	HAL_Delay(300);
			// }
			if(KeyRead(TOP_RIGHT)==GPIO_PIN_RESET) // 确定模式，右肩键
			{
        MsgSend("click m0,1");// 此时串口屏位于操作页面1/2，点击返回控件，清除旧数据(MAP保留)返回到start页面
				HAL_Delay(20);
				// if(!flag.RobotMode)	  // 点击main页面控件，进入操作页面
				MsgSend("click R1,0");
				// else
				// 	MsgSend("click R2,0");
				break;
			}
		}
    LCD_ShowHome();
#endif

  	if (!QueueInit())// 队列初始化
 	 {
		LCD_ShowString(20, 45, (uint8_t *)"Queue init error", WHITE, BLACK, 12, 1);
		while (1)
		LCD_ShowString(80, 58, (uint8_t *)"Queue!", WHITE, BLACK, 12, 1);
  	}
  	else
    	LCD_ShowString(20, 45, (uint8_t *)"Queue init success", WHITE, BLACK, 12, 1);
	if (!UartDMAStart())// DMA初始化
 	 {
		LCD_ShowString(20, 65, (uint8_t *)"DMA init error", WHITE, BLACK, 12, 1);
		while (1)
		LCD_ShowString(80, 78, (uint8_t *)"DMA!", WHITE, BLACK, 12, 1);
  	}
  	else
    	LCD_ShowString(20, 65, (uint8_t *)"DMA init success", WHITE, BLACK, 12, 1);
	HAL_Delay(500);
	LCD_ShowHome();// 清屏
}
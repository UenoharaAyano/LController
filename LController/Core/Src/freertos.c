/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "robot_msg.h"
#include "input.h"
#include "uart_protocol.h"
#include "fifo_queue.h"
#include "touchscreen.h"
#include "string.h"
#include "app_state.h"
#include "screen_ui.h"
#include "myRadio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

static uint8_t TJCtxbuffer[128];
static uint8_t LORATxBuffer[128];
static rfRxPacket_ts g_rfRxPacket;
volatile uint8_t g_rfRxFlag = 0;
volatile uint8_t g_rfTxBusy = 0;
osMessageQueueId_t LoRaTxQueueHandle;

char TJCrxmsg;
/* USER CODE END Variables */
/* Definitions for TJCshow_Task */
osThreadId_t TJCshow_TaskHandle;
const osThreadAttr_t TJCshow_Task_attributes = {
  .name = "TJCshow_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow2,
};
/* Definitions for Key_Task */
osThreadId_t Key_TaskHandle;
const osThreadAttr_t Key_Task_attributes = {
  .name = "Key_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow3,
};
/* Definitions for Joystick_Task */
osThreadId_t Joystick_TaskHandle;
const osThreadAttr_t Joystick_Task_attributes = {
  .name = "Joystick_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow3,
};
/* Definitions for TJCmsg_Task */
osThreadId_t TJCmsg_TaskHandle;
const osThreadAttr_t TJCmsg_Task_attributes = {
  .name = "TJCmsg_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow2,
};
/* Definitions for LORAmsg_Task */
osThreadId_t LORAmsg_TaskHandle;
const osThreadAttr_t LORAmsg_Task_attributes = {
  .name = "LORAmsg_Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow3,
};
/* Definitions for LCDLED_Task */
osThreadId_t LCDLED_TaskHandle;
const osThreadAttr_t LCDLED_Task_attributes = {
  .name = "LCDLED_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow3,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void rfRxCallback(uint8_t status, rfRxPacket_ts packet);
static void LoRa_SendControlMsg(void);
static void LoRa_RxDataProcess(rfRxPacket_ts *rxPacket);
/* USER CODE END FunctionPrototypes */

void TJCshowTask(void *argument);
void KeyTask(void *argument);
void JoystickTask(void *argument);
void TJCmsgTask(void *argument);
void LORAmsgTask(void *argument);
void LCDLEDTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook(void)
{
  /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
  to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
  task. It is essential that code added to this hook function never attempts
  to block in any way (for example, call xQueueReceive() with a block time
  specified, or call vTaskDelay()). If the application makes use of the
  vTaskDelete() API function (as this demo application does) then it is also
  important that vApplicationIdleHook() is permitted to return to its calling
  function, because it is the responsibility of the idle task to clean up
  memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
  /* Run time stack overflow checking is performed if
  configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
  called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  LoRaTxQueueHandle = osMessageQueueNew(4, sizeof(R1ControlMsg_S), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of TJCshow_Task */
  TJCshow_TaskHandle = osThreadNew(TJCshowTask, NULL, &TJCshow_Task_attributes);

  /* creation of Key_Task */
  Key_TaskHandle = osThreadNew(KeyTask, NULL, &Key_Task_attributes);

  /* creation of Joystick_Task */
  Joystick_TaskHandle = osThreadNew(JoystickTask, NULL, &Joystick_Task_attributes);

  /* creation of TJCmsg_Task */
  TJCmsg_TaskHandle = osThreadNew(TJCmsgTask, NULL, &TJCmsg_Task_attributes);

  /* creation of LORAmsg_Task */
  LORAmsg_TaskHandle = osThreadNew(LORAmsgTask, NULL, &LORAmsg_Task_attributes);

  /* creation of LCDLED_Task */
  LCDLED_TaskHandle = osThreadNew(LCDLEDTask, NULL, &LCDLED_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_TJCshowTask */
/**
 * @brief  Function implementing the TJCshow_Task thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_TJCshowTask */
void TJCshowTask(void *argument)
{
  /* USER CODE BEGIN TJCshowTask */
  /* Infinite loop */
  for (;;)
  {
    if (flag.TJCvalrefresh)
    {
      if (flag.RobotMode== 0)
        R1_TJC_val_show();
//      else
//        R2_TJC_val_show();
     	flag.TJCvalrefresh = false;
    }
		
		// if (temp.POWERflashtic++ == 100)	// 检测电量
    // {
    //   powerSend(power_scan());
    //   temp.POWERflashtic = 0;
    // }
    osDelay(40);
  }
  /* USER CODE END TJCshowTask */
}

/* USER CODE BEGIN Header_KeyTask */
/**
 * @brief Function implementing the Key_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_KeyTask */
void KeyTask(void *argument)
{
  /* USER CODE BEGIN KeyTask */
  static uint8_t keyTaskLastToggleNum = 0x0F; // 用来储存上一次的拨码开关状态，初始值为0x0F表示全开
  /* Infinite loop */
  for (;;)
  {
      temp.key_num = KeyScan();      	  // 按键扫描
		  temp.ToggleNum = ToggleScan();		// 拨动开关扫描
			if (temp.key_num != 0)	//扫描到按键变化
			{
				if (flag.RobotMode== 0)
					R1_key_function(temp.key_num);
//				else
//					R2_key_function(temp.key_num);
			}
			if(temp.ToggleNum != keyTaskLastToggleNum && (!flag.LCDfirstshow))
			{
        keyTaskLastToggleNum = temp.ToggleNum; // 更新上一次的拨码开关状态
				if (flag.RobotMode== 0)
					R1_toggle_function(temp.ToggleNum);
//			else
//				R2_toggle_function(temp.ToggleNum);
			}
      osDelay(20);
  }
  /* USER CODE END KeyTask */
}

/* USER CODE BEGIN Header_JoystickTask */
/**
 * @brief Function implementing the Joystick_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_JoystickTask */
void JoystickTask(void *argument)
{
  /* USER CODE BEGIN JoystickTask */
  uint8_t lora_queue_msg[sizeof(R1ControlMsg_S)];
  static JScontrol_S js_last = {0}; // 记录上一次的摇杆状态
  static R1ControlMsg_S r1_last = {0}; // 记录上次发送的R1按键状态
  // static R2ControlMsg_S r2_last = {0}; // 记录上次发送的R2按键状态
  static TickType_t last_send_tick = 0; // 上次发送的时间戳
  static TickType_t xPeriod = pdMS_TO_TICKS(1);
  TickType_t xLastWakeTime = osKernelGetTickCount();
  
  /* Infinite loop */
  for (;;)
  {
    vTaskDelayUntil(&xLastWakeTime, xPeriod); // 消抖
    joystickLeft_scan();   // 单左摇杆扫描
    joystickRight_scan();
		
    // 检查是否有变化
    bool is_changed = false;
    
    // 每次先将最新读取的摇杆数据同步到 controlmsg 中，然后再做整体的 memcmp 比较
    if (flag.RobotMode == 0)
    {
      r1controlmsg.setx = JScontrolmsg.velX; 
      r1controlmsg.sety = JScontrolmsg.velY; 
      r1controlmsg.setw = JScontrolmsg.angW;

      if (memcmp(&r1controlmsg, &r1_last, sizeof(R1ControlMsg_S)) != 0)
        is_changed = true;
    }
    // else
    // {
    //   r2controlmsg.Vx = JScontrolmsg.velX; 
    //   r2controlmsg.Vy = JScontrolmsg.velY;
    //   r2controlmsg.Vw = JScontrolmsg.angW;

    //   if (memcmp(&r2controlmsg, &r2_last, sizeof(R2ControlMsg_S)) != 0)
    //     is_changed = true;
    // }

    // 获取当前系统时间
    xLastWakeTime = osKernelGetTickCount();

    // 触发发送条件：1. 数据发生变化  2. 超过200ms没有发送数据(防丢包)
    if (is_changed || (xLastWakeTime - last_send_tick > 200))
    {
      last_send_tick = xLastWakeTime;
      js_last = JScontrolmsg;

      if (flag.RobotMode == 0)
      {
        r1_last = r1controlmsg; // 记录已发送的R1状态
        memcpy(lora_queue_msg, &r1controlmsg, sizeof(R1ControlMsg_S));
        osMessageQueuePut(LoRaTxQueueHandle, lora_queue_msg, 0, 0);
      }
      // else
      // {
      //   r2_last = r2controlmsg; // 记录已发送的R2状态
      //   memcpy(lora_queue_msg, &r2controlmsg, sizeof(R2ControlMsg_S));
      //   osMessageQueuePut(LoRaTxQueueHandle, lora_queue_msg, 0, 0);
      // }
		
    //    uint8_t *data;		
    //    if (flag.RobotMode == 0)
    //       data = dataEncode(R1all_ID, &U2DataFrame); // 编码
    //    else
    //       data = dataEncode(R2all_ID, &U2DataFrame); 
    //		
    //    enQueueLen(USARTmsgTxQueue, data, U2DataFrame.sendlen); // 入队
    }
    
    osDelay(15);
  }
  /* USER CODE END JoystickTask */
}

/* USER CODE BEGIN Header_TJCmsgTask */
/**
 * @brief Function implementing the TJCmsg_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_TJCmsgTask */
void TJCmsgTask(void *argument)
{
  /* USER CODE BEGIN TJCmsgTask */
  /* Infinite loop */
  for (;;)
  {
    while (deQueue(TJCMsgRxQueue, (uint8_t *)&TJCrxmsg))
		GetMsg(&tjcMsg, TJCrxmsg); // 解析触摸屏反馈
    osDelay(30);
  }
  /* USER CODE END TJCmsgTask */
}

/* USER CODE BEGIN Header_LORAmsgTask */
/**
* @brief Function implementing the LORAmsg_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LORAmsgTask */
void LORAmsgTask(void *argument)
{
  /* USER CODE BEGIN LORAmsgTask */
  
  //  const uint32_t SEND_INTERVAL_MS = 10;  /* 发送间隔10ms */

  myRadio_init(0, (void *)rfRxCallback);
  myRadio_setFrequency(915000000);
  myRadio_setTxPower(8);
  myRadio_setBaudrate(RF_BAUDRATE_62500);
  myRadio_receiver();

  uint8_t tx_buffer[sizeof(R1ControlMsg_S)];
  static rfTxPacket_ts txPacket;

  /* Infinite loop */
  for(;;)
  {
    myRadio_process();

    /* 检查并处理接收到的数据 */
    if (g_rfRxFlag)
    {
      g_rfRxFlag = 0;
      LoRa_RxDataProcess(&g_rfRxPacket);
    }

    /* 数据流驱动发送控制消息 */
    if (osMessageQueueGet(LoRaTxQueueHandle, tx_buffer, NULL, 5) == osOK)
    {
      while (g_rfTxBusy)
      {
        myRadio_process();
        osDelay(1);
      }

      /* 直接发送裸 R1ControlMsg_S 结构体，无 MsgId */
      memcpy(txPacket.payload, tx_buffer, sizeof(R1ControlMsg_S));
      txPacket.len = sizeof(R1ControlMsg_S);

      g_rfTxBusy = 1;
      myRadio_transmit(&txPacket);
    }

//    osDelay(15);
  }
  /* USER CODE END LORAmsgTask */
}

/* USER CODE BEGIN Header_LCDLEDTask */
/**
 * @brief Function implementing the LCDLED_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_LCDLEDTask */
void LCDLEDTask(void *argument)
{
  /* USER CODE BEGIN LCDLEDTask */
  /* Infinite loop */
  for (;;)
  {
      if (flag.LCDfirstshow) // 第一次LCD刷新界面基本文字要素
      {
				if (flag.RobotMode == 0) // R1
					R1_Interface();
				// else // R2
        //  R2_Interface();
				
				// Toggle_status_show();
				temp.ToggleNum=0x0F;	
				temp.ToggleNumlast=temp.ToggleNum;
        flag.LCDfirstshow = false;
      }
      else
      {
        if(temp.ToggleNum!=temp.ToggleNumlast)
        {
          // Toggle_status_show();
          temp.ToggleNumlast=temp.ToggleNum;
        }
        LCD_flash(); // 动态显示
      }
    osDelay(30);
  }
  /* USER CODE END LCDLEDTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
static void rfRxCallback(uint8_t status, rfRxPacket_ts packet)
{
  switch (status)
  {
    case RX_STA_SECCESS:
      g_rfRxPacket = packet;
      g_rfRxFlag = 1;
      /* 接收成功后重新进入接收模式 */
      myRadio_receiver();
      break;
    case RX_STA_TIMEOUT:
      /* 接收超时，重新进入接收 */
      myRadio_receiver();
      break;
    case TX_STA_SECCESS:
      /* 发送完成，清除忙标志并切换回接收 */
      g_rfTxBusy = 0;
      myRadio_receiver();
      break;
    default:
      g_rfTxBusy = 0;
      myRadio_receiver();
      break;
  }
}

/**
 * @brief LoRa发送控制消息（改为队列后不需要）
 *   帧格式: [MsgId(1B)] [Data(NB)]
 *   根据当前机器人模式发送R1或R2控制数据
 */
//  static void LoRa_SendControlMsg(void)
//  {
//     static rfTxPacket_ts txPacket;
//     uint8_t offset = 0;

//     if (flag.RobotMode == 0)
//      {
//       txPacket.payload[offset++] = (uint8_t)R1all_ID;
//       memcpy(&txPacket.payload[offset], &r1controlmsg, sizeof(R1ControlMsg_S));
//       offset += sizeof(R1ControlMsg_S);
//      }
//     else
//      {
//       txPacket.payload[offset++] = (uint8_t)R2all_ID;
//       memcpy(&txPacket.payload[offset], &r2controlmsg, sizeof(R2ControlMsg_S));
//       offset += sizeof(R2ControlMsg_S);
//      }

//     txPacket.len = offset;
//     myRadio_transmit(&txPacket);
//  }

/**
 * @brief LoRa接收数据处理
 *   解析机器人返回的状态反馈消息
 *   帧格式: 裸 R1BackMsg_S 结构体，无帧头/校验/帧尾（LORA链路层自带CRC）
 */
static void LoRa_RxDataProcess(rfRxPacket_ts *rxPacket)
{
    /* 校验数据长度是否等于 R1BackMsg_S 结构体大小（33字节） */
    if (rxPacket->len != sizeof(R1BackMsg_S))
        return;

    /* 校验通过，直接整包拷贝到 r1backmsg */
    memcpy(&r1backmsg, rxPacket->payload, sizeof(R1BackMsg_S));
    flag.TJCvalrefresh = true;  /* 触发界面刷新 */
}

/* USER CODE END Application */


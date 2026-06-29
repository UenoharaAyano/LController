/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
extern int worldtime;
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Input_RT_Pin GPIO_PIN_13
#define Input_RT_GPIO_Port GPIOC
#define INput_LT_Pin GPIO_PIN_14
#define INput_LT_GPIO_Port GPIOC
#define BEEP_Pin GPIO_PIN_15
#define BEEP_GPIO_Port GPIOC
#define LoRa_DIO1_Pin GPIO_PIN_0
#define LoRa_DIO1_GPIO_Port GPIOC
#define LoRa_DIO1_EXTI_IRQn EXTI0_IRQn
#define Output_LCDCS_Pin GPIO_PIN_4
#define Output_LCDCS_GPIO_Port GPIOA
#define Output_LCDDC_Pin GPIO_PIN_6
#define Output_LCDDC_GPIO_Port GPIOA
#define Output_LCDRES_Pin GPIO_PIN_4
#define Output_LCDRES_GPIO_Port GPIOC
#define Output_LCDBLK_Pin GPIO_PIN_5
#define Output_LCDBLK_GPIO_Port GPIOC
#define LoRa_RXEN_Pin GPIO_PIN_0
#define LoRa_RXEN_GPIO_Port GPIOB
#define LoRa_TXEN_Pin GPIO_PIN_1
#define LoRa_TXEN_GPIO_Port GPIOB
#define Input_LK_Pin GPIO_PIN_10
#define Input_LK_GPIO_Port GPIOB
#define LoRa_BUSY_Pin GPIO_PIN_11
#define LoRa_BUSY_GPIO_Port GPIOB
#define Input_B1_Pin GPIO_PIN_6
#define Input_B1_GPIO_Port GPIOC
#define Input_B2_Pin GPIO_PIN_7
#define Input_B2_GPIO_Port GPIOC
#define Input_K1_Pin GPIO_PIN_8
#define Input_K1_GPIO_Port GPIOC
#define Input_K2_Pin GPIO_PIN_8
#define Input_K2_GPIO_Port GPIOA
#define Input_K3_Pin GPIO_PIN_9
#define Input_K3_GPIO_Port GPIOA
#define Input_K4_Pin GPIO_PIN_10
#define Input_K4_GPIO_Port GPIOA
#define Input_B3_Pin GPIO_PIN_11
#define Input_B3_GPIO_Port GPIOA
#define Input_B4_Pin GPIO_PIN_12
#define Input_B4_GPIO_Port GPIOA
#define Input_RK_Pin GPIO_PIN_15
#define Input_RK_GPIO_Port GPIOA
#define LoRa_RST_Pin GPIO_PIN_2
#define LoRa_RST_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */
/* LoRa SPI NSS 软件片选 (PB12) */
#define LORA_NSS_Pin GPIO_PIN_12
#define LORA_NSS_GPIO_Port GPIOB
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

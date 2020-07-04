/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "stm32f1xx_hal.h"

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

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SW_Pin GPIO_PIN_1
#define SW_GPIO_Port GPIOA
#define LSM_INT1_Pin GPIO_PIN_2
#define LSM_INT1_GPIO_Port GPIOA
#define LSM_INT2_Pin GPIO_PIN_3
#define LSM_INT2_GPIO_Port GPIOA
#define LSM_CS_Pin GPIO_PIN_4
#define LSM_CS_GPIO_Port GPIOA
#define TF_CS_Pin GPIO_PIN_12
#define TF_CS_GPIO_Port GPIOB
#define TF_CLK_Pin GPIO_PIN_13
#define TF_CLK_GPIO_Port GPIOB
#define TF_MISO_Pin GPIO_PIN_14
#define TF_MISO_GPIO_Port GPIOB
#define TF_MOSI_Pin GPIO_PIN_15
#define TF_MOSI_GPIO_Port GPIOB
#define STAT_G_Pin GPIO_PIN_15
#define STAT_G_GPIO_Port GPIOA
#define STAT_B_Pin GPIO_PIN_3
#define STAT_B_GPIO_Port GPIOB
#define L_B_Pin GPIO_PIN_4
#define L_B_GPIO_Port GPIOB
#define L_R_Pin GPIO_PIN_5
#define L_R_GPIO_Port GPIOB
#define L_G_Pin GPIO_PIN_6
#define L_G_GPIO_Port GPIOB
#define L3_EN_Pin GPIO_PIN_7
#define L3_EN_GPIO_Port GPIOB
#define L2_EN_Pin GPIO_PIN_8
#define L2_EN_GPIO_Port GPIOB
#define L1_EN_Pin GPIO_PIN_9
#define L1_EN_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
void assert_failed(uint8_t *file, uint32_t line);
#define ASSERT     assert_failed(__FILE__, __LINE__)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

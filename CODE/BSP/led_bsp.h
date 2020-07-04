#ifndef __LED_BSP__
#define __LED_BSP__

#ifdef __cplusplus
  extern "C" {
#endif

#include "main.h"

#define LED_ON          GPIO_PIN_RESET
#define LED_OFF         GPIO_PIN_SET

#define STAT_B(__S)       HAL_GPIO_WritePin(STAT_B_GPIO_Port, STAT_B_Pin, __S);
#define STAT_B_TOGGLE   HAL_GPIO_TogglePin(STAT_B_GPIO_Port, STAT_B_Pin);

#define STAT_G(__S)       HAL_GPIO_WritePin(STAT_G_GPIO_Port, STAT_G_Pin, __S);
#define STAT_G_TOGGLE   HAL_GPIO_TogglePin(STAT_G_GPIO_Port, STAT_G_Pin);

typedef enum {
    BLACK = 0,
    RED,
    GREEN,
    BLUE,
    WHITE,
    YELLOW,
    PURPLE,
    YCAN
}Color_e;

typedef enum {
    LALL = 0,
    L1,
    L2,
    L3,
    NONE
}L_e;

//void LED_Init(void);
void LED_Set(L_e Ln,Color_e c);
void LED_On_Set(L_e Ln,Color_e c,uint16_t on);
void LED_Loop(uint8_t time);


#ifdef __cplusplus
}
#endif

#endif

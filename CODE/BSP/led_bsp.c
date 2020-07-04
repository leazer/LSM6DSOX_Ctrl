#include "led_bsp.h"

//extern TIM_HandleTypeDef htim4;

//#define L1_ENABLE                       HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);   //L1_EN
//#define L1_DISABLE                      HAL_TIM_PWM_StOP(&htim4, TIM_CHANNEL_4);   //L1_EN
//#define L1_SET(_BRIGHTNESS_)            __HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_4,_BRIGHTNESS_);

//#define L2_ENABLE                       HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);   //L1_EN
//#define L2_DISABLE                      HAL_TIM_PWM_StOP(&htim4, TIM_CHANNEL_3);   //L1_EN
//#define L2_SET(_BRIGHTNESS_)            __HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_3,_BRIGHTNESS_);

//#define L3_ENABLE                       HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);   //L1_EN
//#define L3_DISABLE                      HAL_TIM_PWM_StOP(&htim4, TIM_CHANNEL_2);   //L1_EN
//#define L3_SET(_BRIGHTNESS_)            __HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2,_BRIGHTNESS_);


#define L1(__S)       HAL_GPIO_WritePin(L1_EN_GPIO_Port, L1_EN_Pin, __S);
#define L2(__S)       HAL_GPIO_WritePin(L2_EN_GPIO_Port, L2_EN_Pin, __S);
#define L3(__S)       HAL_GPIO_WritePin(L3_EN_GPIO_Port, L3_EN_Pin, __S);

#define L_BLUE_ON    HAL_GPIO_WritePin(L_B_GPIO_Port, L_B_Pin, GPIO_PIN_RESET)
#define L_BLUE_OFF   HAL_GPIO_WritePin(L_B_GPIO_Port, L_B_Pin, GPIO_PIN_SET)
#define L_GREEN_ON   HAL_GPIO_WritePin(L_G_GPIO_Port, L_G_Pin, GPIO_PIN_RESET)
#define L_GREEN_OFF  HAL_GPIO_WritePin(L_G_GPIO_Port, L_G_Pin, GPIO_PIN_SET)
#define L_RED_ON     HAL_GPIO_WritePin(L_R_GPIO_Port, L_R_Pin, GPIO_PIN_RESET)
#define L_RED_OFF    HAL_GPIO_WritePin(L_R_GPIO_Port, L_R_Pin, GPIO_PIN_SET)

void white_fuc  (void)    {L_RED_ON;L_GREEN_ON;L_BLUE_ON;};
void black_fuc  (void)    {L_RED_OFF;L_GREEN_OFF;L_BLUE_OFF;};
void red_fuc    (void)      {L_RED_ON;L_GREEN_OFF;L_BLUE_OFF;};
void green_fuc  (void)    {L_RED_OFF;L_GREEN_ON;L_BLUE_OFF;};
void blue_fuc   (void)     {L_RED_OFF;L_GREEN_OFF;L_BLUE_ON;};
void yellow_fuc (void)   {L_RED_ON;L_GREEN_ON;L_BLUE_OFF;};
void purple_fuc (void)   {L_RED_ON;L_GREEN_OFF;L_BLUE_ON;};
void cyan_fuc   (void)     {L_RED_OFF;L_GREEN_ON;L_BLUE_ON;};
void (*color[8])(void) = {black_fuc,red_fuc,green_fuc,blue_fuc,white_fuc,yellow_fuc,purple_fuc,cyan_fuc};

static uint8_t l_color[3];

void LED_Set(L_e Ln,Color_e c){
    if(Ln >= NONE) return;
    if(Ln == LALL)
    {
        l_color[0] = c;
        l_color[1] = c;
        l_color[2] = c;
    }
    else
        l_color[Ln-1] = c;
}
static uint8_t led_on_flag = 0;
static uint16_t on_time = 0;
static L_e led_on_num = NONE;

void LED_On_Set(L_e Ln,Color_e c,uint16_t on){
    
    LED_Set(Ln,c);
    led_on_num = Ln;
    led_on_flag = 1;
    on_time = on;
    
}

void LED_Loop(uint8_t time)
{
    if(led_on_flag == 1)
    {
        if(on_time ==0 )
        {
            LED_Set(led_on_num,BLACK);
            led_on_num = NONE;
            led_on_flag = 0;
        }
        else
            on_time--;
    }
    static uint16_t loop_time;
    if(loop_time <= time)
    {
        L2(LED_ON);
        L3(LED_ON);
        L1(LED_OFF);
        color[l_color[0]]();
    }
    else if( loop_time > time && loop_time <= 2*time)
    {
        L1(LED_ON);
        L3(LED_ON);
        L2(LED_OFF);
        color[l_color[1]]();
    }
    else if( loop_time > 2*time && loop_time <= 3*time)
    {
        L1(LED_ON);
        L2(LED_ON);
        L3(LED_OFF);
        color[l_color[2]]();
    }
    else
    {
        loop_time = 0;
    }
    loop_time++;

}



































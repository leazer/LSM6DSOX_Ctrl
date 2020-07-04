
/* Private includes ----------------------------------------------------------*/
#include "app.h"
#include "main.h"
#include "fatfs.h"
#include "lsm6dsox_bsp.h"
#include "led_bsp.h"
#include "multi_button.h"
#include "usbd_cdc_if.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
extern stmdev_ctx_t dev_ctx;

FATFS                       fs;
FIL                         fil;

char                        buffer[100];

Button                      SW_button;

lsm6dsox_all_sources_t      status;
uint8_t                     mlc_out[8];
uint8_t                     mode = 0;
/* Private function prototypes -----------------------------------------------*/

static uint8_t sw_read(void);
static void signel_click(void* handle);
static void double_click(void* handle);
static void long_press_start(void* handle);
static void LoadingConfig(void);
//bsp init 
void BSP_Init(void)
{
    STAT_B(LED_ON);
    button_init(&SW_button,sw_read, RESET);
    button_attach(&SW_button, SINGLE_CLICK, signel_click);
    button_attach(&SW_button, DOUBLE_CLICK, double_click);
    button_attach(&SW_button, LONG_RRESS_START, long_press_start);
    button_start(&SW_button);
    //lsm6dsox init and load config file.
    lsm6dsox_init();
    STAT_B(LED_OFF);
    //Mount SD Card
    if(f_mount(&fs, "", 0) != FR_OK)
    {
        LED_On_Set(LALL,WHITE,1000);
    }
}

//灯光扫描时基
void BSP_loop(void)
{
    LED_Loop(1);
}

//主逻辑
static char data[100];
void APP_loop(void)
{
    static uint8_t last_mlc1,last_fsm1;
    
    lsm6dsox_all_sources_get(&dev_ctx, &status);
    if (status.mlc1&&!last_mlc1){
        lsm6dsox_mlc_out_get(&dev_ctx, mlc_out);
        switch(mode){
          case 1:   //LED 显示动作
//          LED_On_Set(LALL,mlc_out[0],700);
//          switch(mlc_out[0]){
//              case 1:
//                  usb_printf("传感器方向 向左\n");
//              break;
//              case 2:
//                  usb_printf("传感器方向 向右\n");
//              break;
//              case 3:
//                  usb_printf("传感器方向 向上\n");
//              break;
//              case 4:
//                  usb_printf("传感器方向 向下\n");
//              break;
//              case 5:
//                  usb_printf("传感器方向 正向\n");
//              break;
//              case 6:
//                  usb_printf("传感器方向 反向\n");
//              break;
//          }
              if(mlc_out[0] == 4)               //下
                  LED_On_Set(LALL,YELLOW,400);
              else if(mlc_out[0] == 12)          //上
                  LED_On_Set(LALL,PURPLE,400);
              else if(mlc_out[0] == 8)          //左
                  LED_On_Set(LALL,YCAN,  400);
          break;
          case 2:   //串口输出
              if(mlc_out[0] == 4)
                  usb_printf("%d:下侧上钩\n",HAL_GetTick());
              else if(mlc_out[0] == 12)
                  usb_printf("%d:上方扣杀\n",HAL_GetTick());
              else if(mlc_out[0] == 8)
                  usb_printf("%d:左侧回击\n",HAL_GetTick());
          break;
          case 3:   //写入文件
              if(mlc_out[0] == 4)
                  sprintf(data,"%d:下侧上钩\n",HAL_GetTick());
              else if(mlc_out[0] == 12)
                  sprintf(data,"%d:上方扣杀\n",HAL_GetTick());
              else if(mlc_out[0] == 8)
                  sprintf(data,"%d:左侧回击\n",HAL_GetTick());
              f_puts(data, &fil);
          break;
          default:
              
          break;
        }
    }
    last_mlc1 = status.mlc1;
    
//    if (status.fsm1&&!last_fsm1){   
//        LED_On_Set(LALL,PURPLE,1000);
//    }
//    last_fsm1 = status.fsm1;
    button_ticks();
}

//上电自动加载ucf配置模型文件
#include <string.h>
void LoadingConfig(void)
{
    uint16_t line_num = 0;
    char temp_str[8] = "";
    char temp_str2[2][5];
    uint8_t add = 0;
    uint8_t vel = 0;
    if(f_open(&fil, "config.h", FA_READ) != FR_OK)
    {
        LED_On_Set(LALL,WHITE,1000);
    }
    else
    {
        while(f_gets(buffer, sizeof(buffer), &fil)) //读取文件
        {
            memcpy(temp_str,buffer+4,7);
            temp_str[7] = '\0';
            if(strcmp(temp_str,"address")==0)
            {
                memcpy(temp_str2[0], buffer+14, 4);
                memcpy(temp_str2[1],buffer+28, 4);
                temp_str2[0][4] = '\0';
                temp_str2[1][4] = '\0';
                add = strtol(temp_str2[0],NULL,16);
                vel = strtol(temp_str2[1],NULL,16);
                lsm6dsox_write_reg(&dev_ctx, add,&vel, 1);
                //usb_printf("%d:%d   %d\n",line_num,add,vel);
                STAT_G_TOGGLE;
            }
            line_num++;
        }
        f_close(&fil);
    }
    STAT_G(LED_OFF);
}


//按键读取回调
uint8_t  sw_read(void)
{
    return HAL_GPIO_ReadPin(SW_GPIO_Port, SW_Pin);
}
//单击回调
static uint8_t f_is_open = 0;
void signel_click(void* handle)
{
    //切换模式
    static Color_e c = RED;
    
    LED_On_Set(LALL,c,500);
    mode = c;
    if(mode == 3&&!f_is_open){
        
        if(f_open(&fil, "data.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE) != FR_OK){
            LED_On_Set(LALL,WHITE,500);
        }
        else
            f_is_open = 1;
        
    }
    c++;
    if(c > BLUE)c = RED;
}
//双击回调
void double_click(void* handle)
{
    //关闭数据记录文件
    if(f_is_open){
        STAT_B_TOGGLE;
        f_close(&fil);
        f_is_open = 0;
        STAT_B_TOGGLE;
    }
}

void long_press_start(void* handle)
{
    LoadingConfig();
}


//FATFS                       *pfs;
//FRESULT                     fres;
//DWORD                       fre_clust;
//uint32_t                    total,freespace;
//        /* Open file to write */
//        if(f_open(&fil, "first.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE) != FR_OK)
//            ASSERT;
//        
        /* Open file to read */
//        if(f_open(&fol, "my.txt", FA_READ) != FR_OK)
//            ASSERT;
//        
//        while(f_gets(buffer, sizeof(buffer), &fol))
//        {
//            //f_puts(buffer, &fil);
//                i++;
//            if(i==100)
//                STAT_G_TOGGLE;
//            if(i>101)i=0;
//             usb_printf("%s",buffer);
//        }
//        if(f_close(&fil) != FR_OK)
//            ASSERT;  
//        if(f_close(&fol) != FR_OK)
//            ASSERT;  
        /* Check free space */
//        if(f_getfree("", &fre_clust, &pfs) != FR_OK)
//            ASSERT;

//        total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
//        freespace = (uint32_t)(fre_clust * pfs->csize * 0.5);   
//        usb_printf("freespace :%d \r\n",freespace);
//        /* Free space is less than 1kb */
//        if(freespace < 1)
//            ASSERT; 

//        /* Writing text */
//        f_puts("STM32 SD Card I/O Example via SPI\n", &fil);  
//        f_puts("Save the world!!!", &fil);

//        /* Close file */
//        if(f_close(&fil) != FR_OK)
//            ASSERT;  

//        /* Close file */
//        if(f_close(&fil) != FR_OK)
//            ASSERT;     

        /* Unmount SDCARD */
//        if(f_mount(NULL, "", 1) != FR_OK)
//            ASSERT; 

















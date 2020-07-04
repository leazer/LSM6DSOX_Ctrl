#ifndef __APP_H__
#define __APP_H__

#ifdef __cplusplus
  extern "C" {
#endif

#include "usbd_cdc_if.h"    //usb_printf

void BSP_Init(void);
void BSP_loop(void);
void APP_loop(void);
#ifdef __cplusplus
}
#endif

#endif


#ifndef _TOUCH_H_
#define _TOUCH_H_ 
#include "main.h"

void delay_ms(uint16_t nms);
uint8_t TP_Init(void);
void ctp_test(void);
uint8_t touch_port_for_lv_is_pressed(void);
uint16_t touch_getX(void);
uint16_t touch_getY(void);
#endif 

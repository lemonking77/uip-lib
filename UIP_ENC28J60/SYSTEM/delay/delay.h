#ifndef __DELAY_H
#define __DELAY_H 			   
#include "sys.h"  
 
void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);

void TIME2_Init(u16 arr,u16 psc);

#endif






























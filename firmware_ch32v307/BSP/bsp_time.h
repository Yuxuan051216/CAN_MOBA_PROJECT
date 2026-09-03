#ifndef BSP_TIME_H
#define BSP_TIME_H

#include <stdint.h>

void BSP_Time_Init(void);
void BSP_Time_On1msInterrupt(void);
uint32_t BSP_Time_Millis(void);
uint32_t millis(void);
void delay_ms(uint32_t ms);

#endif

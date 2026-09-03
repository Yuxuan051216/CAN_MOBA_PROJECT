#ifndef BSP_LCD1602_H
#define BSP_LCD1602_H

#include <stdint.h>

void BSP_LCD1602_Init(void);
void BSP_LCD1602_WriteLine(uint8_t row, const char *text);

#endif

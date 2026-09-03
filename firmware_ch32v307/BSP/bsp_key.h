#ifndef BSP_KEY_H
#define BSP_KEY_H

#include <stdint.h>

void BSP_Key_Init(void);
void BSP_Key_Update(void);
uint8_t BSP_Key_WasPressed(uint8_t key_id);

#endif

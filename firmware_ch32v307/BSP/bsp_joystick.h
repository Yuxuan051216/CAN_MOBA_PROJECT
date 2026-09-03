#ifndef BSP_JOYSTICK_H
#define BSP_JOYSTICK_H

#include <stdint.h>

void BSP_Joystick_Init(void);
void BSP_Joystick_Update(void);
int8_t BSP_Joystick_GetXDir(void);
int8_t BSP_Joystick_GetYDir(void);

#endif

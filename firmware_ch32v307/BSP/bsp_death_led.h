#ifndef BSP_DEATH_LED_H
#define BSP_DEATH_LED_H

#include <stdint.h>

void BSP_DeathLED_Init(void);
void BSP_DeathLED_SetPc(uint8_t on);
void BSP_DeathLED_SetBoard(uint8_t on);

#endif

#ifndef RGB_EFFECT_H
#define RGB_EFFECT_H

#include <stdint.h>

typedef enum {
    RGB_EFFECT_NONE = 0,
    RGB_EFFECT_SKILL_1,
    RGB_EFFECT_SKILL_2_HEAL,
    RGB_EFFECT_SKILL_3,
    RGB_EFFECT_CRYSTAL_BLUE,
    RGB_EFFECT_CRYSTAL_RED,
    RGB_EFFECT_DEATH,
    RGB_EFFECT_MASTER_A,
    RGB_EFFECT_MASTER_B
} RgbEffectType;

void RgbEffect_Init(void);
void RgbEffect_Start(RgbEffectType effect);
void RgbEffect_Update(void);
void RgbEffect_Reset(void);
void RgbEffect_DmaIrqHandler(void);
RgbEffectType RgbEffect_GetActive(void);

#endif

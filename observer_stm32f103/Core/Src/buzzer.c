#include "buzzer.h"

#include "main.h"

static BuzzerPattern active_pattern;
static uint8_t buzzer_active;
static uint32_t pattern_started_ms;

void Buzzer_Init(void)
{
    Buzzer_Reset();
}

void Buzzer_Play(BuzzerPattern pattern)
{
    active_pattern = pattern;
    buzzer_active = 1U;
    pattern_started_ms = HAL_GetTick();
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

void Buzzer_Update(void)
{
    uint32_t elapsed;
    uint8_t on = 0U;

    if(buzzer_active == 0U)
    {
        return;
    }

    elapsed = HAL_GetTick() - pattern_started_ms;
    if(active_pattern == BUZZER_PATTERN_HIT)
    {
        on = (uint8_t)(elapsed < 80U);
        if(elapsed >= 80U)
        {
            buzzer_active = 0U;
        }
    }
    else if(active_pattern == BUZZER_PATTERN_SWITCH)
    {
        on = (uint8_t)(elapsed < 150U);
        if(elapsed >= 150U)
        {
            buzzer_active = 0U;
        }
    }
    else
    {
        on = (uint8_t)(((elapsed / 100U) & 1U) == 0U);
        if(elapsed >= 600U)
        {
            buzzer_active = 0U;
            on = 0U;
        }
    }

    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN,
                      on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Buzzer_Reset(void)
{
    buzzer_active = 0U;
    active_pattern = BUZZER_PATTERN_HIT;
    pattern_started_ms = 0U;
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

#include "bsp_death_led.h"

#include "app_config.h"
#include "ch32v30x.h"

void BSP_DeathLED_Init(void)
{
#if NODE_ID == NODE_ID_BOARD_A
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_ResetBits(PC_DEATH_LED_PORT, PC_DEATH_LED_PIN);
    GPIO_ResetBits(BOARD_DEATH_LED_PORT, BOARD_DEATH_LED_PIN);

    gpio.GPIO_Pin = PC_DEATH_LED_PIN | BOARD_DEATH_LED_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &gpio);

    BSP_DeathLED_SetPc(0U);
    BSP_DeathLED_SetBoard(0U);
#endif
}

void BSP_DeathLED_SetPc(uint8_t on)
{
#if NODE_ID == NODE_ID_BOARD_A
    GPIO_WriteBit(PC_DEATH_LED_PORT,
                  PC_DEATH_LED_PIN,
                  on ? Bit_SET : Bit_RESET);
#else
    (void)on;
#endif
}

void BSP_DeathLED_SetBoard(uint8_t on)
{
#if NODE_ID == NODE_ID_BOARD_A
    GPIO_WriteBit(BOARD_DEATH_LED_PORT,
                  BOARD_DEATH_LED_PIN,
                  on ? Bit_SET : Bit_RESET);
#else
    (void)on;
#endif
}

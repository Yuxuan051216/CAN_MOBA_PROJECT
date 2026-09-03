#include "bsp_led.h"

#include "app_config.h"
#include "bsp_time.h"
#include "ch32v30x.h"

static uint8_t run_led_on;
static uint8_t role_led_on;
static uint32_t last_run_toggle_ms;

static void BSP_LED_Write(GPIO_TypeDef *port, uint16_t pin, uint8_t on)
{
#if LED_ACTIVE_LOW
    GPIO_WriteBit(port, pin, on ? Bit_RESET : Bit_SET);
#else
    GPIO_WriteBit(port, pin, on ? Bit_SET : Bit_RESET);
#endif
}

void BSP_LED_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    gpio.GPIO_Pin = LED_RUN_PIN | LED_ROLE_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &gpio);

    run_led_on = 0U;
    role_led_on = 0U;
    last_run_toggle_ms = millis();
    BSP_LED_SetRun(0U);
    BSP_LED_SetRole(0U);
}

void BSP_LED_SetRun(uint8_t on)
{
    run_led_on = (uint8_t)(on != 0U);
    BSP_LED_Write(LED_RUN_PORT, LED_RUN_PIN, run_led_on);
}

void BSP_LED_SetRole(uint8_t on)
{
    role_led_on = (uint8_t)(on != 0U);
    BSP_LED_Write(LED_ROLE_PORT, LED_ROLE_PIN, role_led_on);
}

void BSP_LED_ToggleRun(void)
{
    BSP_LED_SetRun((uint8_t)!run_led_on);
}

void BSP_LED_ToggleRole(void)
{
    BSP_LED_SetRole((uint8_t)!role_led_on);
}

void BSP_LED_UpdateByRole(NodeRole_t role, HeroState_t hero_state)
{
    uint32_t now = millis();
    uint32_t phase;

    if((uint32_t)(now - last_run_toggle_ms) >= 500U)
    {
        last_run_toggle_ms = now;
        BSP_LED_ToggleRun();
    }

    if(hero_state == HERO_COOLDOWN)
    {
        phase = now % 1000U;
        BSP_LED_SetRole((uint8_t)((phase < 100U) ||
                                  ((phase >= 200U) && (phase < 300U))));
    }
    else if((role == ROLE_PLAYER) || (role == ROLE_MASTER_PLAYER))
    {
        BSP_LED_SetRole((uint8_t)(((now / 150U) & 1U) != 0U));
    }
    else if(role == ROLE_MASTER)
    {
        BSP_LED_SetRole((uint8_t)(((now / 500U) & 1U) != 0U));
    }
    else
    {
        BSP_LED_SetRole((uint8_t)(((now / 1000U) & 1U) != 0U));
    }
}

#include "bsp_key.h"

#include "app_config.h"
#include "bsp_time.h"
#include "ch32v30x.h"

#define KEY_COUNT          3U
#define KEY_RELEASED       1U
#define KEY_PRESSED        0U

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t raw_state;
    uint8_t stable_state;
    uint8_t pressed_event;
    uint32_t raw_changed_ms;
} KeyState_t;

static KeyState_t keys[KEY_COUNT] = {
    {KEY1_PORT, KEY1_PIN, KEY_RELEASED, KEY_RELEASED, 0U, 0U},
    {KEY2_PORT, KEY2_PIN, KEY_RELEASED, KEY_RELEASED, 0U, 0U},
    {KEY3_PORT, KEY3_PIN, KEY_RELEASED, KEY_RELEASED, 0U, 0U}
};

static uint8_t BSP_Key_ReadLevel(const KeyState_t *key)
{
    return GPIO_ReadInputDataBit(key->port, key->pin);
}

void BSP_Key_Init(void)
{
    GPIO_InitTypeDef gpio;
    uint8_t i;
    uint32_t now;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    gpio.GPIO_Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &gpio);

    now = BSP_Time_Millis();
    for(i = 0U; i < KEY_COUNT; i++)
    {
        keys[i].raw_state = BSP_Key_ReadLevel(&keys[i]);
        keys[i].stable_state = keys[i].raw_state;
        keys[i].pressed_event = 0U;
        keys[i].raw_changed_ms = now;
    }
}

void BSP_Key_Update(void)
{
    uint8_t i;
    uint8_t level;
    uint32_t now = BSP_Time_Millis();

    for(i = 0U; i < KEY_COUNT; i++)
    {
        level = BSP_Key_ReadLevel(&keys[i]);

        if(level != keys[i].raw_state)
        {
            keys[i].raw_state = level;
            keys[i].raw_changed_ms = now;
        }

        if((keys[i].raw_state != keys[i].stable_state) &&
           ((uint32_t)(now - keys[i].raw_changed_ms) >= KEY_DEBOUNCE_MS))
        {
            keys[i].stable_state = keys[i].raw_state;
            if(keys[i].stable_state == KEY_PRESSED)
            {
                keys[i].pressed_event = 1U;
            }
        }
    }
}

uint8_t BSP_Key_WasPressed(uint8_t key_id)
{
    uint8_t index;
    uint8_t event;

    if((key_id == 0U) || (key_id > KEY_COUNT))
    {
        return 0U;
    }

    index = (uint8_t)(key_id - 1U);
    event = keys[index].pressed_event;
    keys[index].pressed_event = 0U;
    return event;
}

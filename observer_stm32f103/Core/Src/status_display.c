#include "status_display.h"

#include <string.h>

#include "observer_config.h"
#include "stm32f1xx_hal.h"

#if OBSERVER_USE_OLED

#define OLED_WIDTH  128U
#define OLED_PAGES  8U
#define OLED_SCL    GPIO_PIN_8
#define OLED_SDA    GPIO_PIN_9

static const uint8_t digits[10][5] = {
    {0x3EU,0x51U,0x49U,0x45U,0x3EU},
    {0x00U,0x42U,0x7FU,0x40U,0x00U},
    {0x42U,0x61U,0x51U,0x49U,0x46U},
    {0x21U,0x41U,0x45U,0x4BU,0x31U},
    {0x18U,0x14U,0x12U,0x7FU,0x10U},
    {0x27U,0x45U,0x45U,0x45U,0x39U},
    {0x3CU,0x4AU,0x49U,0x49U,0x30U},
    {0x01U,0x71U,0x09U,0x05U,0x03U},
    {0x36U,0x49U,0x49U,0x49U,0x36U},
    {0x06U,0x49U,0x49U,0x29U,0x1EU}
};

static const uint8_t letters[26][5] = {
    {0x7EU,0x11U,0x11U,0x11U,0x7EU},
    {0x7FU,0x49U,0x49U,0x49U,0x36U},
    {0x3EU,0x41U,0x41U,0x41U,0x22U},
    {0x7FU,0x41U,0x41U,0x22U,0x1CU},
    {0x7FU,0x49U,0x49U,0x49U,0x41U},
    {0x7FU,0x09U,0x09U,0x09U,0x01U},
    {0x3EU,0x41U,0x49U,0x49U,0x7AU},
    {0x7FU,0x08U,0x08U,0x08U,0x7FU},
    {0x00U,0x41U,0x7FU,0x41U,0x00U},
    {0x20U,0x40U,0x41U,0x3FU,0x01U},
    {0x7FU,0x08U,0x14U,0x22U,0x41U},
    {0x7FU,0x40U,0x40U,0x40U,0x40U},
    {0x7FU,0x02U,0x0CU,0x02U,0x7FU},
    {0x7FU,0x04U,0x08U,0x10U,0x7FU},
    {0x3EU,0x41U,0x41U,0x41U,0x3EU},
    {0x7FU,0x09U,0x09U,0x09U,0x06U},
    {0x3EU,0x41U,0x51U,0x21U,0x5EU},
    {0x7FU,0x09U,0x19U,0x29U,0x46U},
    {0x46U,0x49U,0x49U,0x49U,0x31U},
    {0x01U,0x01U,0x7FU,0x01U,0x01U},
    {0x3FU,0x40U,0x40U,0x40U,0x3FU},
    {0x1FU,0x20U,0x40U,0x20U,0x1FU},
    {0x3FU,0x40U,0x38U,0x40U,0x3FU},
    {0x63U,0x14U,0x08U,0x14U,0x63U},
    {0x07U,0x08U,0x70U,0x08U,0x07U},
    {0x61U,0x51U,0x49U,0x45U,0x43U}
};

static const uint8_t init_commands[] = {
    0xAEU, 0x20U, 0x02U, 0xB0U, 0xC8U, 0x00U, 0x10U,
    0x40U, 0x81U, 0x7FU, 0xA1U, 0xA6U, 0xA8U, 0x3FU,
    0xA4U, 0xD3U, 0x00U, 0xD5U, 0x80U, 0xD9U, 0xF1U,
    0xDAU, 0x12U, 0xDBU, 0x40U, 0x8DU, 0x14U, 0xAFU
};

static ObserverState display_state;
static uint8_t framebuffer[OLED_WIDTH * OLED_PAGES];
static uint8_t init_index;
static uint8_t tx_active;
static uint8_t tx_page;
static uint8_t tx_column;
static uint8_t tx_phase;
static uint8_t display_dirty;
static uint32_t last_render_ms;

static void Oled_Delay(void)
{
    uint8_t i;
    for(i = 0U; i < 30U; i++)
    {
        __NOP();
    }
}

static void Oled_Sda(uint8_t high)
{
    HAL_GPIO_WritePin(GPIOB, OLED_SDA,
                      high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void Oled_Scl(uint8_t high)
{
    HAL_GPIO_WritePin(GPIOB, OLED_SCL,
                      high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void Oled_Start(void)
{
    Oled_Sda(1U);
    Oled_Scl(1U);
    Oled_Delay();
    Oled_Sda(0U);
    Oled_Delay();
    Oled_Scl(0U);
}

static void Oled_Stop(void)
{
    Oled_Sda(0U);
    Oled_Scl(1U);
    Oled_Delay();
    Oled_Sda(1U);
    Oled_Delay();
}

static void Oled_WriteByte(uint8_t value)
{
    uint8_t bit;

    for(bit = 0U; bit < 8U; bit++)
    {
        Oled_Sda((uint8_t)((value & 0x80U) != 0U));
        Oled_Scl(1U);
        Oled_Delay();
        Oled_Scl(0U);
        Oled_Delay();
        value <<= 1;
    }
    Oled_Sda(1U);
    Oled_Scl(1U);
    Oled_Delay();
    Oled_Scl(0U);
    Oled_Delay();
}

static void Oled_Write(uint8_t control, uint8_t value)
{
    Oled_Start();
    Oled_WriteByte((uint8_t)(OBSERVER_OLED_ADDRESS << 1));
    Oled_WriteByte(control);
    Oled_WriteByte(value);
    Oled_Stop();
}

static const uint8_t *StatusDisplay_Glyph(char ch)
{
    static const uint8_t blank[5] = {0U,0U,0U,0U,0U};
    static const uint8_t colon[5] = {0U,0x36U,0x36U,0U,0U};
    static const uint8_t dash[5] = {0x08U,0x08U,0x08U,0x08U,0x08U};

    if((ch >= '0') && (ch <= '9'))
    {
        return digits[(uint8_t)(ch - '0')];
    }
    if((ch >= 'A') && (ch <= 'Z'))
    {
        return letters[(uint8_t)(ch - 'A')];
    }
    if(ch == ':')
    {
        return colon;
    }
    if(ch == '-')
    {
        return dash;
    }
    return blank;
}

static void StatusDisplay_DrawText(uint8_t page,
                                   uint8_t column,
                                   const char *text)
{
    while((*text != '\0') && (column < (OLED_WIDTH - 5U)))
    {
        const uint8_t *glyph = StatusDisplay_Glyph(*text++);
        uint8_t i;
        for(i = 0U; i < 5U; i++)
        {
            framebuffer[(uint16_t)page * OLED_WIDTH + column++] = glyph[i];
        }
        framebuffer[(uint16_t)page * OLED_WIDTH + column++] = 0U;
    }
}

static char *StatusDisplay_AppendU8(char *out, uint8_t value)
{
    if(value >= 100U)
    {
        *out++ = (char)('0' + (value / 100U));
        value %= 100U;
        *out++ = (char)('0' + (value / 10U));
    }
    else if(value >= 10U)
    {
        *out++ = (char)('0' + (value / 10U));
    }
    *out++ = (char)('0' + (value % 10U));
    return out;
}

static void StatusDisplay_Render(void)
{
    char line[22];
    char *p;
    const char *state_name;

    memset(framebuffer, 0, sizeof(framebuffer));

    p = line;
    *p++ = 'P'; *p++ = 'C'; *p++ = ' '; *p++ = 'H'; *p++ = 'P';
    *p++ = ':'; p = StatusDisplay_AppendU8(p, display_state.pc_hp);
    *p++ = ' '; *p++ = 'S'; *p++ = ':';
    p = StatusDisplay_AppendU8(p, display_state.last_pc_score);
    *p = '\0';
    StatusDisplay_DrawText(0U, 0U, line);

    p = line;
    *p++ = 'B'; *p++ = 'D'; *p++ = ' '; *p++ = 'H'; *p++ = 'P';
    *p++ = ':'; p = StatusDisplay_AppendU8(p, display_state.embedded_hp);
    *p++ = ' '; *p++ = 'S'; *p++ = ':';
    p = StatusDisplay_AppendU8(p, display_state.last_embedded_score);
    *p = '\0';
    StatusDisplay_DrawText(2U, 0U, line);

    p = line;
    *p++ = 'M'; *p++ = ':';
    *p++ = (display_state.last_master == NODE_ID_BOARD_A) ? 'A' : 'B';
    *p++ = ' '; *p++ = 'P'; *p++ = ':';
    *p++ = (display_state.last_player == NODE_ID_BOARD_A) ? 'A' : 'B';
    *p++ = ' '; *p++ = 'T'; *p++ = ':';
    p = StatusDisplay_AppendU8(p, display_state.last_term);
    *p = '\0';
    StatusDisplay_DrawText(4U, 0U, line);

    switch(display_state.last_game_state)
    {
        case GAME_RUNNING:
            state_name = "RUNNING";
            break;
        case GAME_PAUSED:
            state_name = "PAUSED";
            break;
        case GAME_OVER:
            state_name = "GAME OVER";
            break;
        default:
            state_name = "IDLE";
            break;
    }
    StatusDisplay_DrawText(6U, 0U, state_name);
}

void StatusDisplay_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    gpio.Pin = OLED_SCL | OLED_SDA;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOB, &gpio);
    HAL_GPIO_WritePin(GPIOB, OLED_SCL | OLED_SDA, GPIO_PIN_SET);

    memset(&display_state, 0, sizeof(display_state));
    memset(framebuffer, 0, sizeof(framebuffer));
    init_index = 0U;
    tx_active = 0U;
    tx_page = 0U;
    tx_column = 0U;
    tx_phase = 0U;
    display_dirty = 1U;
    last_render_ms = 0U;
}

void StatusDisplay_SetState(const ObserverState *state)
{
    if(state != 0)
    {
        display_state = *state;
        display_dirty = 1U;
    }
}

void StatusDisplay_Update(void)
{
    uint32_t now = HAL_GetTick();

    if(init_index < sizeof(init_commands))
    {
        Oled_Write(0x00U, init_commands[init_index++]);
        return;
    }

    if((tx_active == 0U) && (display_dirty != 0U) &&
       ((uint32_t)(now - last_render_ms) >=
        OBSERVER_DISPLAY_REFRESH_MS))
    {
        StatusDisplay_Render();
        display_dirty = 0U;
        last_render_ms = now;
        tx_active = 1U;
        tx_page = 0U;
        tx_column = 0U;
        tx_phase = 0U;
    }

    if(tx_active == 0U)
    {
        return;
    }

    if(tx_phase == 0U)
    {
        Oled_Write(0x00U, (uint8_t)(0xB0U + tx_page));
        tx_phase = 1U;
    }
    else if(tx_phase == 1U)
    {
        Oled_Write(0x00U, 0x00U);
        tx_phase = 2U;
    }
    else if(tx_phase == 2U)
    {
        Oled_Write(0x00U, 0x10U);
        tx_phase = 3U;
    }
    else
    {
        Oled_Write(0x40U,
                   framebuffer[(uint16_t)tx_page * OLED_WIDTH +
                               tx_column]);
        tx_column++;
        if(tx_column >= OLED_WIDTH)
        {
            tx_column = 0U;
            tx_page++;
            tx_phase = 0U;
            if(tx_page >= OLED_PAGES)
            {
                tx_active = 0U;
            }
        }
    }
}

void StatusDisplay_ForceRefresh(void)
{
    display_dirty = 1U;
}

#else

void StatusDisplay_Init(void)
{
}

void StatusDisplay_SetState(const ObserverState *state)
{
    (void)state;
}

void StatusDisplay_Update(void)
{
}

void StatusDisplay_ForceRefresh(void)
{
}

#endif

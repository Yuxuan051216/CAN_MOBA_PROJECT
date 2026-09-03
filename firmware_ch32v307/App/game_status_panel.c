#include "game_status_panel.h"

#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "bsp_death_led.h"
#include "bsp_lcd1602.h"
#include "bsp_time.h"
#include "game_types.h"

#if NODE_ID == NODE_ID_BOARD_A
#define STATUS_PANEL_ROW_LEN    16U

typedef struct {
    uint16_t pc_hp;
    uint16_t board_hp;
    uint8_t game_state;
    uint8_t pc_dead;
    uint8_t board_dead;
    uint32_t pc_dead_until_ms;
    uint32_t board_dead_until_ms;
    uint8_t pc_led_on;
    uint8_t board_led_on;
    uint8_t last_death_event_valid;
    uint8_t last_dead_node;
    uint32_t pc_led_off_ms;
    uint32_t board_led_off_ms;
    uint32_t last_death_event_ms;
    char shown_pc[STATUS_PANEL_ROW_LEN + 1U];
    char shown_board[STATUS_PANEL_ROW_LEN + 1U];
} GameStatusPanel_t;

static GameStatusPanel_t panel;

static uint8_t GameStatusPanel_TimeReached(uint32_t now, uint32_t deadline)
{
    return (uint8_t)((int32_t)(now - deadline) >= 0);
}

static void GameStatusPanel_FormatLine(char *line,
                                       const char *label,
                                       uint16_t hp,
                                       uint8_t dead)
{
    if(dead)
    {
        (void)snprintf(line,
                       STATUS_PANEL_ROW_LEN + 1U,
                       "%s:DIED",
                       label);
    }
    else
    {
        (void)snprintf(line,
                       STATUS_PANEL_ROW_LEN + 1U,
                       "%s:%u",
                       label,
                       (unsigned int)hp);
    }
}

static void GameStatusPanel_DrawIfChanged(uint8_t row,
                                          char *shown,
                                          const char *next)
{
    if(strncmp(shown, next, STATUS_PANEL_ROW_LEN) != 0)
    {
        BSP_LCD1602_WriteLine(row, next);
        strncpy(shown, next, STATUS_PANEL_ROW_LEN);
        shown[STATUS_PANEL_ROW_LEN] = '\0';
    }
}
#endif

void GameStatusPanel_Init(void)
{
#if NODE_ID == NODE_ID_BOARD_A
    memset(&panel, 0, sizeof(panel));
    panel.pc_hp = PC_INIT_HP;
    panel.board_hp = EMBEDDED_INIT_HP;
    memset(panel.shown_pc, 0xFF, STATUS_PANEL_ROW_LEN);
    memset(panel.shown_board, 0xFF, STATUS_PANEL_ROW_LEN);
    panel.shown_pc[STATUS_PANEL_ROW_LEN] = '\0';
    panel.shown_board[STATUS_PANEL_ROW_LEN] = '\0';

    BSP_DeathLED_Init();
    BSP_LCD1602_Init();
    GameStatusPanel_Update();
#endif
}

void GameStatusPanel_Update(void)
{
#if NODE_ID == NODE_ID_BOARD_A
    uint32_t now = millis();
    uint8_t pc_dead;
    uint8_t board_dead;
    char next_pc[STATUS_PANEL_ROW_LEN + 1U];
    char next_board[STATUS_PANEL_ROW_LEN + 1U];

    if(panel.pc_led_on && GameStatusPanel_TimeReached(now,
                                                      panel.pc_led_off_ms))
    {
        panel.pc_led_on = 0U;
        BSP_DeathLED_SetPc(0U);
    }
    if(panel.board_led_on && GameStatusPanel_TimeReached(now,
                                                         panel.board_led_off_ms))
    {
        panel.board_led_on = 0U;
        BSP_DeathLED_SetBoard(0U);
    }

    pc_dead = panel.pc_dead;
    board_dead = panel.board_dead;
    if(panel.game_state != (uint8_t)GAME_OVER)
    {
        if(pc_dead && GameStatusPanel_TimeReached(now,
                                                  panel.pc_dead_until_ms))
        {
            panel.pc_dead = 0U;
            pc_dead = 0U;
        }
        if(board_dead && GameStatusPanel_TimeReached(now,
                                                     panel.board_dead_until_ms))
        {
            panel.board_dead = 0U;
            board_dead = 0U;
        }
    }

    GameStatusPanel_FormatLine(next_pc, "PC", panel.pc_hp, pc_dead);
    GameStatusPanel_FormatLine(next_board, "BOARD", panel.board_hp, board_dead);
    GameStatusPanel_DrawIfChanged(0U, panel.shown_pc, next_pc);
    GameStatusPanel_DrawIfChanged(1U, panel.shown_board, next_board);
#endif
}

void GameStatusPanel_SetHealth(uint16_t pc_hp, uint16_t board_hp)
{
#if NODE_ID == NODE_ID_BOARD_A
    panel.pc_hp = pc_hp;
    panel.board_hp = board_hp;
#else
    (void)pc_hp;
    (void)board_hp;
#endif
}

void GameStatusPanel_SetGameState(uint8_t game_state)
{
#if NODE_ID == NODE_ID_BOARD_A
    panel.game_state = game_state;
#else
    (void)game_state;
#endif
}

void GameStatusPanel_OnDeathEvent(uint8_t dead_node, uint8_t cooldown_s)
{
#if NODE_ID == NODE_ID_BOARD_A
    uint32_t now = millis();
    uint32_t died_ms = (uint32_t)cooldown_s * 1000U;

    if(panel.last_death_event_valid &&
       (dead_node == panel.last_dead_node) &&
       ((uint32_t)(now - panel.last_death_event_ms) < 100U))
    {
        return;
    }
    panel.last_death_event_valid = 1U;
    panel.last_dead_node = dead_node;
    panel.last_death_event_ms = now;

    if(died_ms < DEATH_LED_ON_MS)
    {
        died_ms = DEATH_LED_ON_MS;
    }

    if(dead_node == NODE_ID_PC)
    {
        panel.pc_dead = 1U;
        panel.pc_dead_until_ms = now + died_ms;
        panel.pc_led_on = 1U;
        panel.pc_led_off_ms = now + DEATH_LED_ON_MS;
        BSP_DeathLED_SetPc(1U);
    }
    else if((dead_node == NODE_ID_BOARD_A) ||
            (dead_node == NODE_ID_BOARD_B))
    {
        panel.board_dead = 1U;
        panel.board_dead_until_ms = now + died_ms;
        panel.board_led_on = 1U;
        panel.board_led_off_ms = now + DEATH_LED_ON_MS;
        BSP_DeathLED_SetBoard(1U);
    }
#else
    (void)dead_node;
    (void)cooldown_s;
#endif
}

void GameStatusPanel_OnReset(void)
{
#if NODE_ID == NODE_ID_BOARD_A
    panel.pc_dead = 0U;
    panel.board_dead = 0U;
    panel.pc_led_on = 0U;
    panel.board_led_on = 0U;
    panel.last_death_event_valid = 0U;
    BSP_DeathLED_SetPc(0U);
    BSP_DeathLED_SetBoard(0U);
#endif
}

#include "game_player.h"

#include <stdio.h>

#include "app_config.h"
#include "bsp_joystick.h"
#include "bsp_key.h"
#include "bsp_time.h"
#include "game_master.h"
#include "game_protocol.h"
#include "node_role.h"

static uint32_t last_player_update_ms;
static uint32_t last_move_send_ms;
static int8_t last_x_dir;
static int8_t last_y_dir;

void GamePlayer_Init(void)
{
    last_player_update_ms = millis();
    last_move_send_ms = last_player_update_ms;
    last_x_dir = 0;
    last_y_dir = 0;
}

void GamePlayer_Update(void)
{
    uint32_t now;
    uint8_t key1;
    uint8_t key2;
    uint8_t key3;
    int8_t x_dir;
    int8_t y_dir;

    now = millis();
    if((uint32_t)(now - last_player_update_ms) <
       PLAYER_INPUT_PERIOD_MS)
    {
        return;
    }
    last_player_update_ms = now;

    /* 即使当前不是 Player，也读取并清除旧按键事件，避免换角后误触发。 */
    key1 = BSP_Key_WasPressed(1U);
    key2 = BSP_Key_WasPressed(2U);
    key3 = BSP_Key_WasPressed(3U);

    if(key1 || key2 || key3)
    {
        printf("KEY event node=%u player=%u cooldown=%u game_state=%u\r\n",
               (unsigned int)NODE_ID,
               (unsigned int)NodeRole_IsPlayer(),
               (unsigned int)NodeRole_IsCooldown(),
               (unsigned int)g_game_master.game_state);
    }

    if(!NodeRole_IsPlayer())
    {
        if(key1 || key2 || key3)
        {
            printf("KEY ignored: node=%u is Master/standby, current Player=%u\r\n",
                   (unsigned int)NODE_ID,
                   (unsigned int)g_node_role.current_player);
        }
        return;
    }

    if(NodeRole_IsCooldown())
    {
        if(key1 || key2 || key3)
        {
            printf("KEY ignored: node=%u is in death cooldown\r\n",
                   (unsigned int)NODE_ID);
        }
        return;
    }

    /*
     * Player 的按键始终发送到 Master。游戏未开始或技能冷却时，
     * 由 Master 返回明确的拒绝结果，避免按键在本地静默丢失。
     */
    if(key1)
    {
        printf("KEY1 -> SKILL_1 TX\r\n");
        Protocol_SendSkillInput(SKILL_1, NODE_ID_PC);
    }
    if(key2)
    {
        printf("KEY2 -> SKILL_2 TX\r\n");
        Protocol_SendSkillInput(SKILL_2, NODE_ID);
    }
    if(key3)
    {
        printf("KEY3 -> SKILL_3 TX\r\n");
        Protocol_SendSkillInput(SKILL_3, NODE_ID_PC);
    }

    if(g_game_master.game_state != GAME_RUNNING)
    {
        return;
    }

    x_dir = BSP_Joystick_GetXDir();
    y_dir = BSP_Joystick_GetYDir();

    if((x_dir != last_x_dir) ||
       (y_dir != last_y_dir) ||
       ((uint32_t)(now - last_move_send_ms) >=
        MOVE_INPUT_REFRESH_MS))
    {
        last_x_dir = x_dir;
        last_y_dir = y_dir;
        last_move_send_ms = now;
        Protocol_SendMoveInput((uint8_t)(x_dir + 1),
                               (uint8_t)(y_dir + 1));
    }
}

void GamePlayer_OnGlobalState(const CanFrame_t *frame)
{
    (void)frame;
    /*
     * 全局状态由 GameMaster_OnGlobalState 保存为本地影子状态。
     * Player 读取同一份 g_game_master，避免维护两份可能不同步的数据。
     */
}

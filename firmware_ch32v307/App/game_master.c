#include "game_master.h"

#include <stdio.h>

#include "app_config.h"
#include "bsp_time.h"
#include "game_common.h"
#include "game_protocol.h"
#include "node_role.h"

GameMasterContext_t g_game_master;

#define GAME_WINNER_NONE       0U
#define GAME_WINNER_PC         1U
#define GAME_WINNER_EMBEDDED   2U

static void GameMaster_ClearCooldowns(uint32_t *cooldowns)
{
    uint8_t i;

    for(i = 0U; i < 4U; i++)
    {
        cooldowns[i] = 0U;
    }
}

static void GameMaster_ClearInputSequences(void)
{
    uint8_t i;

    for(i = 0U; i < NODE_COUNT; i++)
    {
        g_game_master.last_skill_input_seq[i] = 0U;
        g_game_master.skill_input_seq_valid[i] = 0U;
    }
}

static uint8_t GameMaster_IsDuplicateSkill(uint8_t node_id,
                                            uint8_t input_seq)
{
    if(node_id >= NODE_COUNT)
    {
        return 1U;
    }
    if(g_game_master.skill_input_seq_valid[node_id] &&
       (g_game_master.last_skill_input_seq[node_id] == input_seq))
    {
        return 1U;
    }
    g_game_master.skill_input_seq_valid[node_id] = 1U;
    g_game_master.last_skill_input_seq[node_id] = input_seq;
    return 0U;
}

static uint8_t GameMaster_CanUseSkill(uint32_t *cooldowns,
                                     uint8_t skill_id,
                                     uint32_t now)
{
    if(!GameCommon_IsValidSkill(skill_id))
    {
        return 0U;
    }

    return GameCommon_TimeReached(now, cooldowns[skill_id]);
}

static void GameMaster_StartCooldown(uint32_t *cooldowns,
                                     uint8_t skill_id,
                                     uint32_t now)
{
    cooldowns[skill_id] =
        now + GameCommon_SkillCooldownMs(skill_id);
}

static int16_t GameMaster_ClampCoordinate(int16_t value,
                                          int16_t minimum,
                                          int16_t maximum)
{
    if(value < minimum)
    {
        return minimum;
    }
    if(value > maximum)
    {
        return maximum;
    }
    return value;
}

static void GameMaster_ClearMovement(void)
{
    g_game_master.pc_move_x = 0;
    g_game_master.pc_move_y = 0;
    g_game_master.embedded_move_x = 0;
    g_game_master.embedded_move_y = 0;
}

static void GameMaster_ResetPositions(void)
{
    g_game_master.pc_x = PC_SPAWN_X;
    g_game_master.pc_y = PC_SPAWN_Y;
    g_game_master.embedded_x = EMBEDDED_SPAWN_X;
    g_game_master.embedded_y = EMBEDDED_SPAWN_Y;
    GameMaster_ClearMovement();
}

static void GameMaster_ResetCrystalTimers(uint32_t now)
{
    g_game_master.crystal_last_attack_ms[0] = now;
    g_game_master.crystal_last_attack_ms[1] = now;
    g_game_master.crystal_in_range[0] = 0U;
    g_game_master.crystal_in_range[1] = 0U;
    g_game_master.crystal_hit_seq = 0U;
}

static uint8_t GameMaster_UpdatePositions(uint32_t now)
{
    uint32_t elapsed;
    uint32_t ticks;

    elapsed = (uint32_t)(now - g_game_master.last_master_tick_ms);
    if(elapsed < MASTER_TICK_PERIOD_MS)
    {
        return 0U;
    }

    ticks = elapsed / MASTER_TICK_PERIOD_MS;
    if(ticks > 4U)
    {
        ticks = 4U;
    }
    g_game_master.last_master_tick_ms += ticks * MASTER_TICK_PERIOD_MS;

    if(g_game_master.game_state != GAME_RUNNING)
    {
        return 1U;
    }

    while(ticks > 0U)
    {
        g_game_master.pc_x = GameMaster_ClampCoordinate(
            (int16_t)(g_game_master.pc_x +
                      g_game_master.pc_move_x * MOVE_STEP_X_PER_TICK),
            WORLD_X_MIN,
            WORLD_X_MAX);
        g_game_master.pc_y = GameMaster_ClampCoordinate(
            (int16_t)(g_game_master.pc_y +
                      g_game_master.pc_move_y * MOVE_STEP_Y_PER_TICK),
            WORLD_Y_MIN,
            WORLD_Y_MAX);
        g_game_master.embedded_x = GameMaster_ClampCoordinate(
            (int16_t)(g_game_master.embedded_x +
                      g_game_master.embedded_move_x *
                      MOVE_STEP_X_PER_TICK),
            WORLD_X_MIN,
            WORLD_X_MAX);
        g_game_master.embedded_y = GameMaster_ClampCoordinate(
            (int16_t)(g_game_master.embedded_y +
                      g_game_master.embedded_move_y *
                      MOVE_STEP_Y_PER_TICK),
            WORLD_Y_MIN,
            WORLD_Y_MAX);
        ticks--;
    }

    return 1U;
}

static uint64_t GameMaster_DistanceSquaredScaled(int16_t first_x,
                                                 int16_t first_y,
                                                 int16_t second_x,
                                                 int16_t second_y)
{
    int64_t dx;
    int64_t dy;

    dx = ((int64_t)first_x - (int64_t)second_x) *
         (int64_t)WORLD_MAP_X_PIXELS;
    dy = ((int64_t)first_y - (int64_t)second_y) *
         (int64_t)WORLD_MAP_Y_PIXELS;
    return (uint64_t)(dx * dx + dy * dy);
}

static uint8_t GameMaster_IsInRange(int16_t first_x,
                                    int16_t first_y,
                                    int16_t second_x,
                                    int16_t second_y,
                                    uint32_t range_map_pixels)
{
    uint64_t range_scaled;

    range_scaled = (uint64_t)range_map_pixels *
                   (uint64_t)WORLD_COORD_SCALE;
    return (uint8_t)(
        GameMaster_DistanceSquaredScaled(first_x,
                                         first_y,
                                         second_x,
                                         second_y) <=
        range_scaled * range_scaled);
}

static uint32_t GameMaster_IntegerSqrt(uint64_t value)
{
    uint64_t result = 0U;
    uint64_t bit = (uint64_t)1U << 62;

    while(bit > value)
    {
        bit >>= 2;
    }
    while(bit != 0U)
    {
        if(value >= result + bit)
        {
            value -= result + bit;
            result = (result >> 1) + bit;
        }
        else
        {
            result >>= 1;
        }
        bit >>= 2;
    }
    return (uint32_t)result;
}

static void GameMaster_LogCrystal(uint8_t crystal_id,
                                  uint8_t target_node,
                                  int16_t hero_x,
                                  int16_t hero_y,
                                  int16_t crystal_x,
                                  int16_t crystal_y,
                                  uint8_t in_range)
{
    uint32_t distance;

    distance = GameMaster_IntegerSqrt(
        GameMaster_DistanceSquaredScaled(hero_x,
                                         hero_y,
                                         crystal_x,
                                         crystal_y)) /
               WORLD_COORD_SCALE;
    printf("Crystal crystal_id=%u target_node=%u hero_x=%d hero_y=%d "
           "crystal_x=%d crystal_y=%d distance=%lu in_range=%u damage=%u\r\n",
           (unsigned int)crystal_id,
           (unsigned int)target_node,
           (int)hero_x,
           (int)hero_y,
           (int)crystal_x,
           (int)crystal_y,
           (unsigned long)distance,
           (unsigned int)in_range,
           (unsigned int)CRYSTAL_DAMAGE);
}

static uint8_t GameMaster_IsAttackSkillInRange(uint8_t skill_id)
{
    uint32_t range;

    if(skill_id == SKILL_2)
    {
        return 1U;
    }
    range = (skill_id == SKILL_1) ?
            SKILL1_RANGE_MAP_PIXELS :
            SKILL3_RANGE_MAP_PIXELS;
    return GameMaster_IsInRange(g_game_master.pc_x,
                                g_game_master.pc_y,
                                g_game_master.embedded_x,
                                g_game_master.embedded_y,
                                range);
}

static void GameMaster_EnterGameOver(uint8_t winner)
{
    if(g_game_master.game_state == GAME_OVER)
    {
        return;
    }

    g_game_master.winner = winner;
    g_game_master.game_state = GAME_OVER;
    GameMaster_ClearMovement();
    GameMaster_ResetCrystalTimers(millis());
    Protocol_SendGlobalState();
    printf("GAME_OVER winner=%s pc_score=%u embedded_score=%u\r\n",
           (winner == GAME_WINNER_PC) ? "PC" : "EMBEDDED",
           (unsigned int)g_game_master.pc_score,
           (unsigned int)g_game_master.embedded_score);
}

static uint8_t GameMaster_CanAttackEmbedded(void)
{
    HeroState_t player_state;

    if(g_game_master.embedded_hp == 0U)
    {
        return 0U;
    }
    if(!NodeRole_IsNodeOnline(g_node_role.current_player))
    {
        return 0U;
    }
    player_state = NodeRole_GetNodeHeroState(
        g_node_role.current_player);
    if((player_state == HERO_DEAD) ||
       (player_state == HERO_COOLDOWN))
    {
        return 0U;
    }
    return 1U;
}

static void GameMaster_HandleOneCrystal(uint32_t now,
                                        uint8_t timer_index,
                                        uint8_t crystal_id,
                                        uint8_t target_node,
                                        int16_t hero_x,
                                        int16_t hero_y,
                                        int16_t crystal_x,
                                        int16_t crystal_y,
                                        uint8_t attackable)
{
    uint8_t in_range;
    uint8_t was_in_range;

    was_in_range = g_game_master.crystal_in_range[timer_index];
    in_range = (uint8_t)(
        attackable &&
        GameMaster_IsInRange(hero_x,
                             hero_y,
                             crystal_x,
                             crystal_y,
                             CRYSTAL_ATTACK_RANGE_MAP_PIXELS));
    if(in_range != g_game_master.crystal_in_range[timer_index])
    {
        g_game_master.crystal_in_range[timer_index] = in_range;
        GameMaster_LogCrystal(crystal_id,
                              target_node,
                              hero_x,
                              hero_y,
                              crystal_x,
                              crystal_y,
                              in_range);
    }
    if(!in_range)
    {
        g_game_master.crystal_last_attack_ms[timer_index] = now;
        return;
    }
    if(!was_in_range)
    {
        g_game_master.crystal_last_attack_ms[timer_index] = now;
        return;
    }
    if((uint32_t)(now -
                  g_game_master.crystal_last_attack_ms[timer_index]) <
       CRYSTAL_ATTACK_MS)
    {
        return;
    }

    g_game_master.crystal_last_attack_ms[timer_index] = now;
    GameMaster_LogCrystal(crystal_id,
                          target_node,
                          hero_x,
                          hero_y,
                          crystal_x,
                          crystal_y,
                          1U);
    if(timer_index == 0U)
    {
        g_game_master.embedded_hp = GameCommon_ClampHp(
            (int16_t)g_game_master.embedded_hp -
            (int16_t)CRYSTAL_DAMAGE);
    }
    else
    {
        g_game_master.pc_hp = GameCommon_ClampHp(
            (int16_t)g_game_master.pc_hp -
            (int16_t)CRYSTAL_DAMAGE);
    }
    Protocol_SendCrystalAttack(crystal_id,
                               target_node,
                               CRYSTAL_DAMAGE,
                               g_game_master.crystal_hit_seq++);
}

static void GameMaster_HandleCrystalAttacks(uint32_t now)
{
    uint8_t pc_attackable;

    if(g_game_master.game_state != GAME_RUNNING)
    {
        GameMaster_ResetCrystalTimers(now);
        return;
    }

    GameMaster_HandleOneCrystal(now,
                                0U,
                                CRYSTAL_BLUE,
                                g_node_role.current_player,
                                g_game_master.embedded_x,
                                g_game_master.embedded_y,
                                BLUE_CRYSTAL_X,
                                BLUE_CRYSTAL_Y,
                                GameMaster_CanAttackEmbedded());

    pc_attackable = (uint8_t)(
        (g_game_master.pc_hp > 0U) &&
        GameCommon_TimeReached(now,
                               g_game_master.pc_respawn_end_ms));
    GameMaster_HandleOneCrystal(now,
                                1U,
                                CRYSTAL_RED,
                                NODE_ID_PC,
                                g_game_master.pc_x,
                                g_game_master.pc_y,
                                RED_CRYSTAL_X,
                                RED_CRYSTAL_Y,
                                pc_attackable);
}

static void GameMaster_HandlePcDeath(void)
{
    if(g_game_master.pc_hp != 0U)
    {
        return;
    }

    g_game_master.embedded_score++;
    Protocol_SendDeathEvent(NODE_ID_PC,
                            g_node_role.current_player,
                            PC_RESPAWN_SECONDS);
    GameMaster_OnDeathEvent(NODE_ID_PC);
    g_game_master.pc_hp = PC_INIT_HP;
    GameMaster_ClearCooldowns(g_game_master.pc_skill_cd_end);
    Protocol_SendPositionState();

    if(g_game_master.embedded_score >= WIN_SCORE)
    {
        GameMaster_EnterGameOver(GAME_WINNER_EMBEDDED);
    }

    printf("PC hero defeated, embedded score=%u\r\n",
           (unsigned int)g_game_master.embedded_score);
}

static void GameMaster_HandleEmbeddedDeath(void)
{
    uint8_t old_master;
    uint8_t old_player;
    uint8_t new_master;
    uint8_t new_player;
    uint8_t new_term;
    uint8_t cooldown_s;

    if(g_game_master.embedded_hp != 0U)
    {
        return;
    }

    old_master = g_node_role.current_master;
    old_player = g_node_role.current_player;
    new_master = old_player;
    new_player = old_master;

    /*
     * 降级态可能由同一节点兼任 Master 和 Player。若另一块板在线，
     * 阵亡节点转为新 Master，另一块板立即接管 Player。
     */
    if(old_master == old_player)
    {
        new_player = (old_player == NODE_ID_BOARD_A) ?
                     NODE_ID_BOARD_B : NODE_ID_BOARD_A;
        if(!NodeRole_IsNodeOnline(new_player))
        {
            new_player = old_master;
        }
    }

    new_term = (uint8_t)(g_node_role.current_term + 1U);
    cooldown_s = (uint8_t)(DEATH_COOLDOWN_MS / 1000U);

    g_game_master.pc_score++;
    Protocol_SendDeathEvent(old_player, NODE_ID_PC, cooldown_s);
    GameMaster_OnDeathEvent(old_player);

    g_game_master.embedded_hp = EMBEDDED_INIT_HP;
    GameMaster_ClearCooldowns(g_game_master.embedded_skill_cd_end);
    Protocol_SendPositionState();

    NodeRole_OnRoleSwitch(new_term,
                          new_master,
                          new_player,
                          SWITCH_BY_PLAYER_DEATH,
                          old_player,
                          cooldown_s);

    g_game_master.term = new_term;
    g_game_master.master_node = new_master;
    g_game_master.embedded_player_node = new_player;

    Protocol_SendRoleSwitch(new_master,
                            new_player,
                            SWITCH_BY_PLAYER_DEATH,
                            old_player,
                            cooldown_s);
    if(g_game_master.pc_score >= WIN_SCORE)
    {
        GameMaster_EnterGameOver(GAME_WINNER_PC);
    }
    else
    {
        Protocol_SendGlobalState();
    }

    printf("Embedded hero defeated, PC score=%u\r\n",
           (unsigned int)g_game_master.pc_score);
}

void GameMaster_Init(void)
{
    uint32_t now = millis();

    g_game_master.game_state = GAME_IDLE;
    g_game_master.term = g_node_role.current_term;
    g_game_master.master_node = g_node_role.current_master;
    g_game_master.embedded_player_node = g_node_role.current_player;
    g_game_master.pc_hp = PC_INIT_HP;
    g_game_master.embedded_hp = EMBEDDED_INIT_HP;
    g_game_master.pc_score = 0U;
    g_game_master.embedded_score = 0U;
    g_game_master.winner = GAME_WINNER_NONE;
    GameMaster_ClearCooldowns(g_game_master.pc_skill_cd_end);
    GameMaster_ClearCooldowns(g_game_master.embedded_skill_cd_end);
    GameMaster_ClearInputSequences();
    GameMaster_ResetPositions();
    GameMaster_ResetCrystalTimers(now);
    g_game_master.pc_respawn_end_ms = 0U;
    g_game_master.last_global_state_ms = now;
    g_game_master.last_position_state_ms = now;
    g_game_master.last_master_tick_ms = now;
}

void GameMaster_Update(void)
{
    uint32_t now;

    now = millis();
    if(!NodeRole_IsMaster())
    {
        g_game_master.last_master_tick_ms = now;
        return;
    }
    if(!GameMaster_UpdatePositions(now))
    {
        return;
    }

    g_game_master.term = g_node_role.current_term;
    g_game_master.master_node = g_node_role.current_master;
    g_game_master.embedded_player_node = g_node_role.current_player;

    if(g_game_master.game_state == GAME_RUNNING)
    {
        GameMaster_HandleCrystalAttacks(now);
        GameMaster_HandlePcDeath();
        if(g_game_master.game_state == GAME_RUNNING)
        {
            GameMaster_HandleEmbeddedDeath();
        }
    }

    if((uint32_t)(now - g_game_master.last_position_state_ms) >=
       POSITION_STATE_PERIOD_MS)
    {
        g_game_master.last_position_state_ms = now;
        Protocol_SendPositionState();
    }
    if((uint32_t)(now - g_game_master.last_global_state_ms) >=
       GLOBAL_STATE_PERIOD_MS)
    {
        g_game_master.last_global_state_ms = now;
        Protocol_SendGlobalState();
    }
}

void GameMaster_OnPcSkill(uint8_t skill_id, uint8_t input_seq)
{
    uint32_t now;

    if(!NodeRole_IsMaster())
    {
        return;
    }
    if(GameMaster_IsDuplicateSkill(NODE_ID_PC, input_seq))
    {
        return;
    }
    printf("Master RX PC skill=%u seq=%u pc_hp=%u embedded_hp=%u\r\n",
           (unsigned int)skill_id,
           (unsigned int)input_seq,
           (unsigned int)g_game_master.pc_hp,
           (unsigned int)g_game_master.embedded_hp);
    if(g_game_master.game_state != GAME_RUNNING)
    {
        printf("PC skill %u ignored: game state=%u is not RUNNING\r\n",
               (unsigned int)skill_id,
               (unsigned int)g_game_master.game_state);
        Protocol_SendSkillResult(NODE_ID_PC,
                                 skill_id,
                                 SKILL_RESULT_GAME_NOT_RUNNING,
                                 input_seq);
        return;
    }

    now = millis();
    if(!GameMaster_CanUseSkill(g_game_master.pc_skill_cd_end,
                               skill_id,
                               now))
    {
        printf("PC skill %u ignored: cooldown\r\n",
               (unsigned int)skill_id);
        Protocol_SendSkillResult(NODE_ID_PC,
                                 skill_id,
                                 SKILL_RESULT_COOLDOWN,
                                 input_seq);
        return;
    }
    if(!GameMaster_IsAttackSkillInRange(skill_id))
    {
        printf("PC skill %u ignored: target out of range\r\n",
               (unsigned int)skill_id);
        Protocol_SendSkillResult(NODE_ID_PC,
                                 skill_id,
                                 SKILL_RESULT_OUT_OF_RANGE,
                                 input_seq);
        return;
    }

    GameMaster_StartCooldown(g_game_master.pc_skill_cd_end,
                             skill_id,
                             now);

    if(skill_id == SKILL_1)
    {
        g_game_master.embedded_hp =
            GameCommon_ClampHp((int16_t)g_game_master.embedded_hp -
                               (int16_t)SKILL1_DAMAGE);
    }
    else if(skill_id == SKILL_2)
    {
        g_game_master.pc_hp =
            GameCommon_ClampHp((int16_t)g_game_master.pc_hp +
                               (int16_t)SKILL2_HEAL);
    }
    else if(skill_id == SKILL_3)
    {
        g_game_master.embedded_hp =
            GameCommon_ClampHp((int16_t)g_game_master.embedded_hp -
                               (int16_t)SKILL3_DAMAGE);
    }

    printf("PC skill=%u pc_hp=%u embedded_hp=%u\r\n",
           (unsigned int)skill_id,
           (unsigned int)g_game_master.pc_hp,
           (unsigned int)g_game_master.embedded_hp);
    Protocol_SendSkillResult(NODE_ID_PC,
                             skill_id,
                             SKILL_RESULT_ACCEPTED,
                             input_seq);
    if(g_game_master.embedded_hp == 0U)
    {
        GameMaster_HandleEmbeddedDeath();
    }
    else
    {
        Protocol_SendGlobalState();
    }
}

void GameMaster_OnEmbeddedSkill(uint8_t node_id,
                                uint8_t skill_id,
                                uint8_t input_seq)
{
    uint32_t now;

    if(!NodeRole_IsMaster())
    {
        return;
    }
    if(GameMaster_IsDuplicateSkill(node_id, input_seq))
    {
        return;
    }
    printf("Master RX node=%u skill=%u seq=%u pc_hp=%u embedded_hp=%u\r\n",
           (unsigned int)node_id,
           (unsigned int)skill_id,
           (unsigned int)input_seq,
           (unsigned int)g_game_master.pc_hp,
           (unsigned int)g_game_master.embedded_hp);
    if(g_game_master.game_state != GAME_RUNNING)
    {
        printf("Node %u skill %u ignored: game state=%u is not RUNNING\r\n",
               (unsigned int)node_id,
               (unsigned int)skill_id,
               (unsigned int)g_game_master.game_state);
        Protocol_SendSkillResult(node_id,
                                 skill_id,
                                 SKILL_RESULT_GAME_NOT_RUNNING,
                                 input_seq);
        return;
    }
    if(node_id != g_node_role.current_player)
    {
        printf("Node %u skill %u ignored: current Player=%u\r\n",
               (unsigned int)node_id,
               (unsigned int)skill_id,
               (unsigned int)g_node_role.current_player);
        Protocol_SendSkillResult(node_id,
                                 skill_id,
                                 SKILL_RESULT_NOT_CURRENT_PLAYER,
                                 input_seq);
        return;
    }

    now = millis();
    if(!GameMaster_CanUseSkill(g_game_master.embedded_skill_cd_end,
                               skill_id,
                               now))
    {
        printf("Node %u skill %u ignored: cooldown\r\n",
               (unsigned int)node_id,
               (unsigned int)skill_id);
        Protocol_SendSkillResult(node_id,
                                 skill_id,
                                 SKILL_RESULT_COOLDOWN,
                                 input_seq);
        return;
    }
    if(!GameMaster_IsAttackSkillInRange(skill_id))
    {
        printf("Node %u skill %u ignored: target out of range\r\n",
               (unsigned int)node_id,
               (unsigned int)skill_id);
        Protocol_SendSkillResult(node_id,
                                 skill_id,
                                 SKILL_RESULT_OUT_OF_RANGE,
                                 input_seq);
        return;
    }

    GameMaster_StartCooldown(g_game_master.embedded_skill_cd_end,
                             skill_id,
                             now);

    if(skill_id == SKILL_1)
    {
        g_game_master.pc_hp =
            GameCommon_ClampHp((int16_t)g_game_master.pc_hp -
                               (int16_t)SKILL1_DAMAGE);
    }
    else if(skill_id == SKILL_2)
    {
        g_game_master.embedded_hp =
            GameCommon_ClampHp((int16_t)g_game_master.embedded_hp +
                               (int16_t)SKILL2_HEAL);
    }
    else if(skill_id == SKILL_3)
    {
        g_game_master.pc_hp =
            GameCommon_ClampHp((int16_t)g_game_master.pc_hp -
                               (int16_t)SKILL3_DAMAGE);
    }

    printf("Node %u skill=%u pc_hp=%u embedded_hp=%u\r\n",
           (unsigned int)node_id,
           (unsigned int)skill_id,
           (unsigned int)g_game_master.pc_hp,
           (unsigned int)g_game_master.embedded_hp);
    Protocol_SendSkillResult(node_id,
                             skill_id,
                             SKILL_RESULT_ACCEPTED,
                             input_seq);
    if(g_game_master.pc_hp == 0U)
    {
        GameMaster_HandlePcDeath();
    }
    else
    {
        Protocol_SendGlobalState();
    }
}

void GameMaster_OnMoveInput(uint8_t node_id,
                            uint8_t x_dir,
                            uint8_t y_dir)
{
    int8_t decoded_x;
    int8_t decoded_y;
    uint8_t changed;

    if((x_dir > 2U) || (y_dir > 2U))
    {
        return;
    }
    if(!NodeRole_IsMaster() ||
       (g_game_master.game_state != GAME_RUNNING))
    {
        return;
    }

    if((node_id != NODE_ID_PC) &&
       (node_id != g_node_role.current_player))
    {
        return;
    }

    decoded_x = (int8_t)x_dir - 1;
    decoded_y = 1 - (int8_t)y_dir;
    if(node_id == NODE_ID_PC)
    {
        if(!GameCommon_TimeReached(millis(),
                                  g_game_master.pc_respawn_end_ms))
        {
            return;
        }
        changed = (uint8_t)(
            (g_game_master.pc_move_x != decoded_x) ||
            (g_game_master.pc_move_y != decoded_y));
        g_game_master.pc_move_x = decoded_x;
        g_game_master.pc_move_y = decoded_y;
    }
    else
    {
        changed = (uint8_t)(
            (g_game_master.embedded_move_x != decoded_x) ||
            (g_game_master.embedded_move_y != decoded_y));
        g_game_master.embedded_move_x = decoded_x;
        g_game_master.embedded_move_y = decoded_y;
    }

    if(changed)
    {
        printf("Move input node=%u x=%u y=%u\r\n",
               (unsigned int)node_id,
               (unsigned int)x_dir,
               (unsigned int)y_dir);
    }
}

void GameMaster_OnGameControl(uint8_t command)
{
    if(!NodeRole_IsMaster())
    {
        return;
    }

    if(command == GAME_CTRL_START)
    {
        if(g_game_master.game_state == GAME_IDLE)
        {
            g_game_master.game_state = GAME_RUNNING;
            printf("Game started\r\n");
            Protocol_SendGlobalState();
        }
    }
    else if(command == GAME_CTRL_PAUSE)
    {
        if(g_game_master.game_state == GAME_RUNNING)
        {
            g_game_master.game_state = GAME_PAUSED;
            GameMaster_ClearMovement();
            GameMaster_ResetCrystalTimers(millis());
            printf("Game paused\r\n");
            Protocol_SendGlobalState();
        }
    }
    else if(command == GAME_CTRL_RESET)
    {
        GameMaster_ResetGame();
    }
}

void GameMaster_OnGlobalState(const CanFrame_t *frame)
{
    if((frame == 0) || (frame->dlc != 8U))
    {
        return;
    }

    if((frame->data[0] < g_game_master.term) ||
       ((frame->data[0] == g_game_master.term) &&
        (frame->data[1] != g_game_master.master_node)))
    {
        return;
    }

    g_game_master.term = frame->data[0];
    g_game_master.master_node = frame->data[1];
    g_game_master.embedded_player_node = frame->data[2];
    g_game_master.pc_hp = frame->data[3];
    g_game_master.embedded_hp = frame->data[4];
    g_game_master.pc_score = frame->data[5];
    g_game_master.embedded_score = frame->data[6];
    g_game_master.game_state = frame->data[7];
    if(g_game_master.game_state == GAME_OVER)
    {
        g_game_master.winner =
            (g_game_master.pc_score >= WIN_SCORE) ?
            GAME_WINNER_PC : GAME_WINNER_EMBEDDED;
        GameMaster_ClearMovement();
    }
}

void GameMaster_OnPositionState(const CanFrame_t *frame)
{
    if((frame == 0) || (frame->dlc != 8U) ||
       NodeRole_IsMaster())
    {
        return;
    }

    g_game_master.pc_x = GameMaster_ClampCoordinate(
        Protocol_ReadInt16LE(&frame->data[0]),
        WORLD_X_MIN,
        WORLD_X_MAX);
    g_game_master.pc_y = GameMaster_ClampCoordinate(
        Protocol_ReadInt16LE(&frame->data[2]),
        WORLD_Y_MIN,
        WORLD_Y_MAX);
    g_game_master.embedded_x = GameMaster_ClampCoordinate(
        Protocol_ReadInt16LE(&frame->data[4]),
        WORLD_X_MIN,
        WORLD_X_MAX);
    g_game_master.embedded_y = GameMaster_ClampCoordinate(
        Protocol_ReadInt16LE(&frame->data[6]),
        WORLD_Y_MIN,
        WORLD_Y_MAX);
    g_game_master.last_master_tick_ms = millis();
}

void GameMaster_OnDeathEvent(uint8_t dead_node)
{
    uint32_t now = millis();

    if(dead_node == NODE_ID_PC)
    {
        g_game_master.pc_x = PC_SPAWN_X;
        g_game_master.pc_y = PC_SPAWN_Y;
        g_game_master.pc_move_x = 0;
        g_game_master.pc_move_y = 0;
        g_game_master.crystal_last_attack_ms[1] = now;
        g_game_master.crystal_in_range[1] = 0U;
        g_game_master.pc_respawn_end_ms =
            now + (uint32_t)PC_RESPAWN_SECONDS * 1000U;
    }
    else if((dead_node == NODE_ID_BOARD_A) ||
            (dead_node == NODE_ID_BOARD_B))
    {
        g_game_master.embedded_x = EMBEDDED_SPAWN_X;
        g_game_master.embedded_y = EMBEDDED_SPAWN_Y;
        g_game_master.embedded_move_x = 0;
        g_game_master.embedded_move_y = 0;
        g_game_master.crystal_last_attack_ms[0] = now;
        g_game_master.crystal_in_range[0] = 0U;
    }
}

void GameMaster_OnRoleSwitch(uint8_t reason)
{
    if(reason == SWITCH_BY_MANUAL_RESET)
    {
        GameMaster_ResetPositions();
        GameMaster_ResetCrystalTimers(millis());
    }
}

void GameMaster_ResetGame(void)
{
    uint8_t new_term;
    uint8_t current_master;
    uint8_t current_player;

    if(!NodeRole_IsMaster())
    {
        return;
    }

    new_term = (uint8_t)(g_node_role.current_term + 1U);
    current_master = g_node_role.current_master;
    current_player = g_node_role.current_player;

    /* Reset 保留正常角色；若处于同节点降级态且双板在线，则恢复双角色。 */
    if(current_master == current_player)
    {
        current_player = (current_master == NODE_ID_BOARD_A) ?
                         NODE_ID_BOARD_B : NODE_ID_BOARD_A;
        if(!NodeRole_IsNodeOnline(current_player))
        {
            current_player = current_master;
        }
    }

    g_game_master.game_state = GAME_IDLE;
    g_game_master.pc_hp = PC_INIT_HP;
    g_game_master.embedded_hp = EMBEDDED_INIT_HP;
    g_game_master.pc_score = 0U;
    g_game_master.embedded_score = 0U;
    g_game_master.winner = GAME_WINNER_NONE;
    GameMaster_ClearCooldowns(g_game_master.pc_skill_cd_end);
    GameMaster_ClearCooldowns(g_game_master.embedded_skill_cd_end);
    GameMaster_ClearInputSequences();
    GameMaster_ResetPositions();
    GameMaster_ResetCrystalTimers(millis());
    g_game_master.pc_respawn_end_ms = 0U;

    NodeRole_OnRoleSwitch(new_term,
                          current_master,
                          current_player,
                          SWITCH_BY_MANUAL_RESET,
                          NODE_ID_PC,
                          0U);

    g_game_master.term = new_term;
    g_game_master.master_node = current_master;
    g_game_master.embedded_player_node = current_player;

    Protocol_SendRoleSwitch(current_master,
                            current_player,
                            SWITCH_BY_MANUAL_RESET,
                            NODE_ID_PC,
                            0U);
    Protocol_SendPositionState();
    Protocol_SendGlobalState();
    printf("Game reset: keep Master=%u Player=%u\r\n",
           (unsigned int)current_master,
           (unsigned int)current_player);
}

uint8_t GameMaster_GetEmbeddedHp(void)
{
    return g_game_master.embedded_hp;
}

uint8_t GameMaster_GetPcHp(void)
{
    return g_game_master.pc_hp;
}

#include "game_common.h"

#include "app_config.h"
#include "game_types.h"

uint8_t GameCommon_IsValidNode(uint8_t node_id)
{
    return (uint8_t)(node_id < NODE_COUNT);
}

uint8_t GameCommon_IsValidSkill(uint8_t skill_id)
{
    return (uint8_t)((skill_id >= (uint8_t)SKILL_1) &&
                     (skill_id <= (uint8_t)SKILL_3));
}

uint8_t GameCommon_ClampHp(int16_t hp)
{
    if(hp < 0)
    {
        return 0U;
    }
    if(hp > (int16_t)GAME_INIT_HP)
    {
        return GAME_INIT_HP;
    }
    return (uint8_t)hp;
}

uint32_t GameCommon_SkillCooldownMs(uint8_t skill_id)
{
    switch(skill_id)
    {
        case SKILL_1:
            return SKILL1_CD_MS;
        case SKILL_2:
            return SKILL2_CD_MS;
        case SKILL_3:
            return SKILL3_CD_MS;
        default:
            return 0U;
    }
}

uint8_t GameCommon_TimeReached(uint32_t now, uint32_t deadline)
{
    return (uint8_t)((int32_t)(now - deadline) >= 0);
}

uint8_t GameCommon_RemainingSeconds(uint32_t end_ms, uint32_t now_ms)
{
    uint32_t remaining;

    if(GameCommon_TimeReached(now_ms, end_ms))
    {
        return 0U;
    }

    remaining = end_ms - now_ms;
    remaining = (remaining + 999U) / 1000U;
    return (remaining > 255U) ? 255U : (uint8_t)remaining;
}

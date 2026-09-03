#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "buzzer.h"
#include "can_moba_protocol.h"
#include "observer_controller.h"
#include "rgb_effect.h"
#include "voice_queue.h"

extern uint16_t test_voice_tracks[32];
extern uint8_t test_voice_count;
extern uint8_t test_rgb_count;
extern RgbEffectType test_last_rgb;
extern uint8_t test_buzzer_count;
extern BuzzerPattern test_last_buzzer;
void TestMocks_Reset(void);

static ObserverCanFrame frame(uint16_t id,
                              uint8_t d0, uint8_t d1,
                              uint8_t d2, uint8_t d3,
                              uint8_t d4, uint8_t d5,
                              uint8_t d6, uint8_t d7)
{
    ObserverCanFrame result;
    uint8_t data[8] = {d0,d1,d2,d3,d4,d5,d6,d7};

    result.id = id;
    result.dlc = 8U;
    memcpy(result.data, data, sizeof(data));
    return result;
}

static void reset_controller(void)
{
    TestMocks_Reset();
    ObserverController_Init();
    TestMocks_Reset();
}

static void test_protocol_filter(void)
{
    assert(ObserverProtocol_IsObservedId(0x001U));
    assert(ObserverProtocol_IsObservedId(0x010U));
    assert(ObserverProtocol_IsObservedId(0x080U));
    assert(ObserverProtocol_IsObservedId(0x100U));
    assert(ObserverProtocol_IsObservedId(0x110U));
    assert(ObserverProtocol_IsObservedId(0x120U));
    assert(ObserverProtocol_IsObservedId(0x381U));
    assert(ObserverProtocol_IsObservedId(0x382U));
    assert(!ObserverProtocol_IsObservedId(0x020U));
    assert(!ObserverProtocol_IsObservedId(0x701U));
}

static void test_death_first_blood_and_duplicate(void)
{
    ObserverCanFrame death = frame(CAN_ID_DEATH_EVENT,
                                   NODE_ID_PC, NODE_ID_BOARD_A,
                                   3U, 1U, 0U, 0U, 0U, 0U);

    reset_controller();
    Observer_HandleFrame(&death);
    assert(test_voice_count == 2U);
    assert(test_voice_tracks[0] == VOICE_FIRST_BLOOD);
    assert(test_voice_tracks[1] == VOICE_PC_DEFEATED);
    assert(test_rgb_count == 1U);
    assert(test_last_rgb == RGB_EFFECT_DEATH);
    assert(test_buzzer_count == 1U);
    assert(test_last_buzzer == BUZZER_PATTERN_DEATH);

    Observer_HandleFrame(&death);
    assert(test_voice_count == 2U);
    assert(test_rgb_count == 1U);
    assert(ObserverController_GetState()->duplicate_frames == 1U);
}

static void test_old_term_is_ignored(void)
{
    ObserverCanFrame global = frame(CAN_ID_GLOBAL_STATE,
                                    5U, NODE_ID_BOARD_A,
                                    NODE_ID_BOARD_B, 90U,
                                    80U, 1U, 1U, GAME_RUNNING);
    ObserverCanFrame old_death = frame(CAN_ID_DEATH_EVENT,
                                      NODE_ID_BOARD_B,
                                      NODE_ID_PC, 10U, 4U,
                                      0U, 0U, 0U, 0U);

    reset_controller();
    Observer_HandleFrame(&global);
    TestMocks_Reset();
    Observer_HandleFrame(&old_death);
    assert(test_voice_count == 0U);
    assert(test_rgb_count == 0U);
    assert(ObserverController_GetState()->last_term == 5U);
    assert(ObserverController_GetState()->stale_frames == 1U);
}

static void test_global_state_does_not_repeat_audio(void)
{
    ObserverCanFrame state2 = frame(CAN_ID_GLOBAL_STATE,
                                    1U, NODE_ID_BOARD_A,
                                    NODE_ID_BOARD_B, 100U,
                                    100U, 2U, 0U, GAME_RUNNING);
    ObserverCanFrame state3 = state2;
    state3.data[5] = 3U;
    state3.data[7] = GAME_OVER;

    reset_controller();
    Observer_HandleFrame(&state2);
    TestMocks_Reset();
    Observer_HandleFrame(&state2);
    assert(test_voice_count == 0U);

    Observer_HandleFrame(&state3);
    assert(test_voice_count == 1U);
    assert(test_voice_tracks[0] == VOICE_PC_VICTORY);
    Observer_HandleFrame(&state3);
    assert(test_voice_count == 1U);
}

static void test_skill_requires_accepted_result(void)
{
    ObserverCanFrame global = frame(CAN_ID_GLOBAL_STATE,
                                    1U, NODE_ID_BOARD_A,
                                    NODE_ID_BOARD_B, 100U,
                                    100U, 0U, 0U, GAME_RUNNING);
    ObserverCanFrame rejected = frame(CAN_ID_SKILL_RESULT_BOARD_A,
                                      NODE_ID_BOARD_A, NODE_ID_PC,
                                      SKILL_1,
                                      SKILL_RESULT_COOLDOWN,
                                      8U, 100U, 100U, 1U);
    ObserverCanFrame accepted = frame(CAN_ID_SKILL_RESULT_BOARD_A,
                                      NODE_ID_BOARD_A, NODE_ID_PC,
                                      SKILL_2,
                                      SKILL_RESULT_ACCEPTED,
                                      9U, 100U, 100U, 1U);

    reset_controller();
    Observer_HandleFrame(&global);
    TestMocks_Reset();
    Observer_HandleFrame(&rejected);
    assert(test_rgb_count == 0U);
    Observer_HandleFrame(&accepted);
    assert(test_rgb_count == 1U);
    assert(test_last_rgb == RGB_EFFECT_SKILL_2_HEAL);
}

static void test_crystal_and_role_switch_deduplication(void)
{
    ObserverCanFrame crystal = frame(CAN_ID_CRYSTAL_ATTACK,
                                     CRYSTAL_BLUE,
                                     NODE_ID_BOARD_B,
                                     10U, 7U, 100U, 90U, 1U, 0U);
    ObserverCanFrame role = frame(CAN_ID_ROLE_SWITCH,
                                  2U, NODE_ID_BOARD_B,
                                  NODE_ID_BOARD_A, 1U,
                                  NODE_ID_BOARD_B, 10U, 0U, 0U);

    reset_controller();
    Observer_HandleFrame(&crystal);
    assert(test_voice_count == 1U);
    assert(test_voice_tracks[0] == VOICE_CRYSTAL_ATTACK);
    assert(test_last_rgb == RGB_EFFECT_CRYSTAL_BLUE);
    Observer_HandleFrame(&crystal);
    assert(test_voice_count == 1U);

    TestMocks_Reset();
    Observer_HandleFrame(&role);
    assert(test_voice_count == 1U);
    assert(test_voice_tracks[0] == VOICE_MASTER_B);
    assert(ObserverController_GetState()->last_master ==
           NODE_ID_BOARD_B);
    Observer_HandleFrame(&role);
    assert(test_voice_count == 1U);
}

int main(void)
{
    test_protocol_filter();
    test_death_first_blood_and_duplicate();
    test_old_term_is_ignored();
    test_global_state_does_not_repeat_audio();
    test_skill_requires_accepted_result();
    test_crystal_and_role_switch_deduplication();
    puts("observer host tests: PASS");
    return 0;
}

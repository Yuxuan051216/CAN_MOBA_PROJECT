#include "observer_controller.h"

#include <string.h>

#include "buzzer.h"
#include "event_queue.h"
#include "observer_config.h"
#include "rgb_effect.h"
#include "status_display.h"
#include "voice_queue.h"

static ObserverState observer_state;

static int8_t Observer_TermRelation(uint8_t term)
{
    if(observer_state.term_valid == 0U)
    {
        return 1;
    }
    if(term < observer_state.last_term)
    {
        return -1;
    }
    if(term > observer_state.last_term)
    {
        return 1;
    }
    return 0;
}

static void Observer_CommitTerm(uint8_t term)
{
    observer_state.last_term = term;
    observer_state.term_valid = 1U;
}

static uint8_t Observer_AuthorityMatches(uint8_t term, uint8_t master)
{
    if((observer_state.authority_term_valid != 0U) &&
       (observer_state.authority_term == term))
    {
        return (uint8_t)(observer_state.last_master == master);
    }
    return 1U;
}

static void Observer_CommitAuthority(uint8_t term,
                                     uint8_t master,
                                     uint8_t player)
{
    Observer_CommitTerm(term);
    observer_state.authority_term = term;
    observer_state.authority_term_valid = 1U;
    observer_state.last_master = master;
    observer_state.last_player = player;
}

static void Observer_ResetTransient(uint8_t announce_reset)
{
    observer_state.first_blood_played = 0U;
    observer_state.winner_announced = 0U;
    observer_state.death_fingerprint_valid = 0U;
    observer_state.crystal_fingerprint_valid = 0U;
    observer_state.skill_fingerprint_valid = 0U;
    observer_state.last_death_fingerprint = 0U;
    observer_state.last_crystal_fingerprint = 0U;
    observer_state.last_skill_fingerprint = 0U;
    VoiceQueue_Reset();
    RgbEffect_Reset();
    Buzzer_Reset();
    if(announce_reset != 0U)
    {
        (void)VoiceQueue_Enqueue(VOICE_GAME_RESET,
                                 VOICE_PRIORITY_GAME_STATE);
    }
}

static void Observer_HandleGameControl(const ObserverCanFrame *frame)
{
    if(frame->data[0] == GAME_CTRL_RESET)
    {
        Observer_ResetTransient(1U);
        observer_state.last_game_state = GAME_IDLE;
    }
}

static void Observer_HandleRoleSwitch(const ObserverCanFrame *frame)
{
    uint8_t term = frame->data[0];
    uint8_t master = frame->data[1];
    uint8_t player = frame->data[2];
    int8_t relation = Observer_TermRelation(term);

    if((relation < 0) ||
       !ObserverProtocol_IsBoardNode(master) ||
       !ObserverProtocol_IsBoardNode(player) ||
       !Observer_AuthorityMatches(term, master))
    {
        observer_state.stale_frames++;
        return;
    }

    if((relation == 0) &&
       (observer_state.last_master == master) &&
       (observer_state.last_player == player))
    {
        observer_state.duplicate_frames++;
        return;
    }

    Observer_CommitAuthority(term, master, player);
    if(master == NODE_ID_BOARD_A)
    {
        (void)VoiceQueue_Enqueue(VOICE_MASTER_A,
                                 VOICE_PRIORITY_MASTER_SWITCH);
        RgbEffect_Start(RGB_EFFECT_MASTER_A);
    }
    else
    {
        (void)VoiceQueue_Enqueue(VOICE_MASTER_B,
                                 VOICE_PRIORITY_MASTER_SWITCH);
        RgbEffect_Start(RGB_EFFECT_MASTER_B);
    }
    Buzzer_Play(BUZZER_PATTERN_SWITCH);
    StatusDisplay_SetState(&observer_state);
}

static void Observer_HandleDeath(const ObserverCanFrame *frame)
{
    uint8_t dead_node = frame->data[0];
    uint8_t killer_node = frame->data[1];
    uint8_t term = frame->data[3];
    uint32_t fingerprint;

    if(Observer_TermRelation(term) < 0)
    {
        observer_state.stale_frames++;
        return;
    }
    if((dead_node >= NODE_COUNT) || (killer_node >= NODE_COUNT))
    {
        return;
    }

    fingerprint = ((uint32_t)term << 16) |
                  ((uint32_t)dead_node << 8) |
                  killer_node;
    if((observer_state.death_fingerprint_valid != 0U) &&
       (observer_state.last_death_fingerprint == fingerprint))
    {
        observer_state.duplicate_frames++;
        return;
    }

    Observer_CommitTerm(term);
    observer_state.last_death_fingerprint = fingerprint;
    observer_state.death_fingerprint_valid = 1U;

    if(observer_state.first_blood_played == 0U)
    {
        observer_state.first_blood_played = 1U;
        (void)VoiceQueue_Enqueue(VOICE_FIRST_BLOOD,
                                 VOICE_PRIORITY_DEATH);
    }

    (void)VoiceQueue_Enqueue(
        (dead_node == NODE_ID_PC) ?
        VOICE_PC_DEFEATED : VOICE_EMBEDDED_DEFEATED,
        VOICE_PRIORITY_DEATH);
    RgbEffect_Start(RGB_EFFECT_DEATH);
    Buzzer_Play(BUZZER_PATTERN_DEATH);
}

static void Observer_AnnounceWinner(uint8_t previous_pc_score,
                                    uint8_t previous_embedded_score,
                                    uint8_t had_state)
{
    if(observer_state.winner_announced != 0U)
    {
        return;
    }

    if(((had_state != 0U) &&
        (previous_pc_score < OBSERVER_VICTORY_SCORE) &&
        (observer_state.last_pc_score >= OBSERVER_VICTORY_SCORE)) ||
       ((had_state == 0U) &&
        (observer_state.last_pc_score >= OBSERVER_VICTORY_SCORE) &&
        (observer_state.last_pc_score >
         observer_state.last_embedded_score)))
    {
        observer_state.winner_announced = 1U;
        (void)VoiceQueue_Enqueue(VOICE_PC_VICTORY,
                                 VOICE_PRIORITY_VICTORY);
    }
    else if(((had_state != 0U) &&
             (previous_embedded_score < OBSERVER_VICTORY_SCORE) &&
             (observer_state.last_embedded_score >=
              OBSERVER_VICTORY_SCORE)) ||
            ((had_state == 0U) &&
             (observer_state.last_embedded_score >=
              OBSERVER_VICTORY_SCORE) &&
             (observer_state.last_embedded_score >
              observer_state.last_pc_score)))
    {
        observer_state.winner_announced = 1U;
        (void)VoiceQueue_Enqueue(VOICE_EMBEDDED_VICTORY,
                                 VOICE_PRIORITY_VICTORY);
    }
}

static void Observer_HandleGlobalState(const ObserverCanFrame *frame)
{
    uint8_t term = frame->data[0];
    uint8_t master = frame->data[1];
    uint8_t player = frame->data[2];
    uint8_t game_state = frame->data[7];
    uint8_t previous_game_state = observer_state.last_game_state;
    uint8_t previous_pc_score = observer_state.last_pc_score;
    uint8_t previous_embedded_score =
        observer_state.last_embedded_score;
    uint8_t had_state = observer_state.global_state_seen;
    int8_t relation = Observer_TermRelation(term);

    if((relation < 0) ||
       !ObserverProtocol_IsBoardNode(master) ||
       !ObserverProtocol_IsBoardNode(player) ||
       (game_state > GAME_OVER) ||
       !Observer_AuthorityMatches(term, master))
    {
        observer_state.stale_frames++;
        return;
    }

    Observer_CommitAuthority(term, master, player);
    observer_state.pc_hp = frame->data[3];
    observer_state.embedded_hp = frame->data[4];
    observer_state.last_pc_score = frame->data[5];
    observer_state.last_embedded_score = frame->data[6];
    observer_state.last_game_state = game_state;

    if((observer_state.global_state_seen != 0U) &&
       (previous_game_state != game_state))
    {
        if(game_state == GAME_RUNNING)
        {
            (void)VoiceQueue_Enqueue(VOICE_GAME_START,
                                     VOICE_PRIORITY_GAME_STATE);
        }
        else if(game_state == GAME_PAUSED)
        {
            (void)VoiceQueue_Enqueue(VOICE_GAME_PAUSED,
                                     VOICE_PRIORITY_GAME_STATE);
        }
        else if(game_state == GAME_IDLE)
        {
            Observer_ResetTransient(1U);
        }
    }
    else if((observer_state.global_state_seen == 0U) &&
            (game_state == GAME_RUNNING))
    {
        (void)VoiceQueue_Enqueue(VOICE_GAME_START,
                                 VOICE_PRIORITY_GAME_STATE);
    }

    observer_state.global_state_seen = 1U;
    if(game_state == GAME_OVER)
    {
        Observer_AnnounceWinner(previous_pc_score,
                                previous_embedded_score,
                                had_state);
    }
    StatusDisplay_SetState(&observer_state);
}

static void Observer_HandleCrystalAttack(const ObserverCanFrame *frame)
{
    uint8_t crystal = frame->data[0];
    uint8_t target = frame->data[1];
    uint8_t hit_seq = frame->data[3];
    uint8_t term = frame->data[6];
    uint32_t fingerprint;

    if(Observer_TermRelation(term) < 0)
    {
        observer_state.stale_frames++;
        return;
    }
    if(((crystal != CRYSTAL_BLUE) && (crystal != CRYSTAL_RED)) ||
       (target >= NODE_COUNT))
    {
        return;
    }

    fingerprint = ((uint32_t)term << 24) |
                  ((uint32_t)crystal << 16) |
                  ((uint32_t)target << 8) |
                  hit_seq;
    if((observer_state.crystal_fingerprint_valid != 0U) &&
       (observer_state.last_crystal_fingerprint == fingerprint))
    {
        observer_state.duplicate_frames++;
        return;
    }

    Observer_CommitTerm(term);
    observer_state.last_crystal_fingerprint = fingerprint;
    observer_state.crystal_fingerprint_valid = 1U;
    (void)VoiceQueue_Enqueue(VOICE_CRYSTAL_ATTACK,
                             VOICE_PRIORITY_CRYSTAL);
    RgbEffect_Start((crystal == CRYSTAL_BLUE) ?
                    RGB_EFFECT_CRYSTAL_BLUE :
                    RGB_EFFECT_CRYSTAL_RED);
    Buzzer_Play(BUZZER_PATTERN_HIT);
}

static void Observer_HandleSkillResult(const ObserverCanFrame *frame)
{
    uint8_t master = frame->data[0];
    uint8_t source = frame->data[1];
    uint8_t skill = frame->data[2];
    uint8_t result = frame->data[3];
    uint8_t input_seq = frame->data[4];
    uint8_t term = frame->data[7];
    uint8_t id_master = (uint8_t)(frame->id -
                                  CAN_ID_SKILL_RESULT_BASE);
    uint32_t fingerprint;
    int8_t relation = Observer_TermRelation(term);

    if((relation < 0) ||
       !ObserverProtocol_IsBoardNode(master) ||
       (master != id_master) ||
       (source >= NODE_COUNT) ||
       (skill < SKILL_1) ||
       (skill > SKILL_3) ||
       !Observer_AuthorityMatches(term, master))
    {
        observer_state.stale_frames++;
        return;
    }

    fingerprint = ((uint32_t)term << 24) |
                  ((uint32_t)source << 16) |
                  ((uint32_t)skill << 8) |
                  input_seq;
    if((observer_state.skill_fingerprint_valid != 0U) &&
       (observer_state.last_skill_fingerprint == fingerprint))
    {
        observer_state.duplicate_frames++;
        return;
    }

    Observer_CommitAuthority(term, master,
                             observer_state.last_player);
    observer_state.last_skill_fingerprint = fingerprint;
    observer_state.skill_fingerprint_valid = 1U;

    if(result != SKILL_RESULT_ACCEPTED)
    {
        return;
    }

    if(skill == SKILL_1)
    {
        RgbEffect_Start(RGB_EFFECT_SKILL_1);
    }
    else if(skill == SKILL_2)
    {
        RgbEffect_Start(RGB_EFFECT_SKILL_2_HEAL);
    }
    else
    {
        RgbEffect_Start(RGB_EFFECT_SKILL_3);
    }
}

void ObserverController_Init(void)
{
    memset(&observer_state, 0, sizeof(observer_state));
    observer_state.last_master = NODE_ID_BOARD_A;
    observer_state.last_player = NODE_ID_BOARD_B;
    observer_state.last_game_state = GAME_IDLE;
    observer_state.pc_hp = 100U;
    observer_state.embedded_hp = 100U;
    StatusDisplay_SetState(&observer_state);
    (void)VoiceQueue_Enqueue(VOICE_WELCOME,
                             VOICE_PRIORITY_GAME_STATE);
}

void Observer_ProcessEvents(void)
{
    ObserverCanFrame frame;

    while(EventQueue_Pop(&frame))
    {
        Observer_HandleFrame(&frame);
    }
}

void Observer_HandleFrame(const ObserverCanFrame *frame)
{
    if((frame == 0) ||
       (frame->dlc != 8U) ||
       !ObserverProtocol_IsObservedId(frame->id))
    {
        return;
    }

    observer_state.handled_frames++;
    switch(frame->id)
    {
        case CAN_ID_ROLE_SWITCH:
            Observer_HandleRoleSwitch(frame);
            break;
        case CAN_ID_GAME_CTRL:
            Observer_HandleGameControl(frame);
            break;
        case CAN_ID_DEATH_EVENT:
            Observer_HandleDeath(frame);
            break;
        case CAN_ID_GLOBAL_STATE:
            Observer_HandleGlobalState(frame);
            break;
        case CAN_ID_CRYSTAL_ATTACK:
            Observer_HandleCrystalAttack(frame);
            break;
        case CAN_ID_SKILL_RESULT_BOARD_A:
        case CAN_ID_SKILL_RESULT_BOARD_B:
            Observer_HandleSkillResult(frame);
            break;
        default:
            break;
    }
}

const ObserverState *ObserverController_GetState(void)
{
    return &observer_state;
}

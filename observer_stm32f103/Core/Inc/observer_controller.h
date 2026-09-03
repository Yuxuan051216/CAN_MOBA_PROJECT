#ifndef OBSERVER_CONTROLLER_H
#define OBSERVER_CONTROLLER_H

#include <stdint.h>

#include "can_moba_protocol.h"

typedef struct {
    uint8_t last_term;
    uint8_t last_master;
    uint8_t last_player;
    uint8_t last_game_state;
    uint8_t last_pc_score;
    uint8_t last_embedded_score;
    uint8_t first_blood_played;
    uint8_t winner_announced;
    uint32_t last_death_fingerprint;
    uint32_t last_crystal_fingerprint;
    uint32_t last_skill_fingerprint;

    uint8_t pc_hp;
    uint8_t embedded_hp;
    uint8_t term_valid;
    uint8_t authority_term_valid;
    uint8_t authority_term;
    uint8_t death_fingerprint_valid;
    uint8_t crystal_fingerprint_valid;
    uint8_t skill_fingerprint_valid;
    uint8_t global_state_seen;
    uint32_t handled_frames;
    uint32_t stale_frames;
    uint32_t duplicate_frames;
} ObserverState;

void ObserverController_Init(void);
void Observer_ProcessEvents(void);
void Observer_HandleFrame(const ObserverCanFrame *frame);
const ObserverState *ObserverController_GetState(void);

#endif

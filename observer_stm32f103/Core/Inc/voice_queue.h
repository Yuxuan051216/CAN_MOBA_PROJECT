#ifndef VOICE_QUEUE_H
#define VOICE_QUEUE_H

#include <stdint.h>

typedef enum {
    VOICE_PRIORITY_CRYSTAL = 1,
    VOICE_PRIORITY_GAME_STATE = 2,
    VOICE_PRIORITY_MASTER_SWITCH = 3,
    VOICE_PRIORITY_DEATH = 4,
    VOICE_PRIORITY_VICTORY = 5
} VoicePriority;

enum {
    VOICE_WELCOME = 1,
    VOICE_GAME_START = 2,
    VOICE_FIRST_BLOOD = 3,
    VOICE_PC_DEFEATED = 4,
    VOICE_EMBEDDED_DEFEATED = 5,
    VOICE_CRYSTAL_ATTACK = 6,
    VOICE_MASTER_A = 7,
    VOICE_MASTER_B = 8,
    VOICE_PC_VICTORY = 9,
    VOICE_EMBEDDED_VICTORY = 10,
    VOICE_GAME_PAUSED = 11,
    VOICE_GAME_RESET = 12
};

void VoiceQueue_Init(void);
uint8_t VoiceQueue_Enqueue(uint16_t track, VoicePriority priority);
void VoiceQueue_Update(void);
void VoiceQueue_Reset(void);
uint8_t VoiceQueue_GetCount(void);

#endif

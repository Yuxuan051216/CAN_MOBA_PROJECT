#include <stdint.h>

#include "buzzer.h"
#include "event_queue.h"
#include "rgb_effect.h"
#include "status_display.h"
#include "voice_queue.h"

uint16_t test_voice_tracks[32];
uint8_t test_voice_count;
uint8_t test_rgb_count;
RgbEffectType test_last_rgb;
uint8_t test_buzzer_count;
BuzzerPattern test_last_buzzer;

void TestMocks_Reset(void)
{
    test_voice_count = 0U;
    test_rgb_count = 0U;
    test_last_rgb = RGB_EFFECT_NONE;
    test_buzzer_count = 0U;
    test_last_buzzer = BUZZER_PATTERN_HIT;
}

uint8_t EventQueue_Pop(ObserverCanFrame *frame)
{
    (void)frame;
    return 0U;
}

uint8_t VoiceQueue_Enqueue(uint16_t track, VoicePriority priority)
{
    (void)priority;
    if(test_voice_count < 32U)
    {
        test_voice_tracks[test_voice_count++] = track;
        return 1U;
    }
    return 0U;
}

void VoiceQueue_Reset(void)
{
    test_voice_count = 0U;
}

void RgbEffect_Start(RgbEffectType effect)
{
    test_rgb_count++;
    test_last_rgb = effect;
}

void RgbEffect_Reset(void)
{
    test_last_rgb = RGB_EFFECT_NONE;
}

void Buzzer_Play(BuzzerPattern pattern)
{
    test_buzzer_count++;
    test_last_buzzer = pattern;
}

void Buzzer_Reset(void)
{
}

void StatusDisplay_SetState(const ObserverState *state)
{
    (void)state;
}

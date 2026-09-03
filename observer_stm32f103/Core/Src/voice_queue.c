#include "voice_queue.h"

#include "dfplayer.h"
#include "observer_config.h"
#include "stm32f1xx_hal.h"

typedef struct {
    uint16_t track;
    uint8_t priority;
    uint32_t sequence;
} VoiceItem;

static VoiceItem voice_items[OBSERVER_VOICE_QUEUE_CAPACITY];
static uint8_t voice_count;
static uint32_t next_sequence;
static uint32_t last_command_ms;
static uint8_t command_sent;

void VoiceQueue_Init(void)
{
    VoiceQueue_Reset();
    last_command_ms = 0U;
    command_sent = 0U;
}

static uint8_t VoiceQueue_FindLowest(void)
{
    uint8_t i;
    uint8_t lowest = 0U;

    for(i = 1U; i < voice_count; i++)
    {
        if((voice_items[i].priority < voice_items[lowest].priority) ||
           ((voice_items[i].priority == voice_items[lowest].priority) &&
            (voice_items[i].sequence > voice_items[lowest].sequence)))
        {
            lowest = i;
        }
    }
    return lowest;
}

uint8_t VoiceQueue_Enqueue(uint16_t track, VoicePriority priority)
{
    uint8_t index;

    if(voice_count >= OBSERVER_VOICE_QUEUE_CAPACITY)
    {
        index = VoiceQueue_FindLowest();
        if((uint8_t)priority <= voice_items[index].priority)
        {
            return 0U;
        }
    }
    else
    {
        index = voice_count++;
    }

    voice_items[index].track = track;
    voice_items[index].priority = (uint8_t)priority;
    voice_items[index].sequence = next_sequence++;
    return 1U;
}

static uint8_t VoiceQueue_FindNext(void)
{
    uint8_t i;
    uint8_t next = 0U;

    for(i = 1U; i < voice_count; i++)
    {
        if((voice_items[i].priority > voice_items[next].priority) ||
           ((voice_items[i].priority == voice_items[next].priority) &&
            (voice_items[i].sequence < voice_items[next].sequence)))
        {
            next = i;
        }
    }
    return next;
}

void VoiceQueue_Update(void)
{
    uint32_t now = HAL_GetTick();
    uint8_t index;

    if((voice_count == 0U) || !DFPlayer_IsReady())
    {
        return;
    }
    if((command_sent != 0U) &&
       ((uint32_t)(now - last_command_ms) <
        OBSERVER_VOICE_COMMAND_GAP_MS))
    {
        return;
    }

    index = VoiceQueue_FindNext();
    if(DFPlayer_PlayTrack(voice_items[index].track))
    {
        voice_count--;
        voice_items[index] = voice_items[voice_count];
        last_command_ms = now;
        command_sent = 1U;
    }
}

void VoiceQueue_Reset(void)
{
    voice_count = 0U;
    next_sequence = 0U;
    last_command_ms = 0U;
    command_sent = 0U;
}

uint8_t VoiceQueue_GetCount(void)
{
    return voice_count;
}

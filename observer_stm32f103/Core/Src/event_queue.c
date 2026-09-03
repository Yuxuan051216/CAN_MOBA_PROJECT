#include "event_queue.h"

#include "observer_config.h"
#include "stm32f1xx.h"

#if (OBSERVER_EVENT_QUEUE_CAPACITY < 2U) || \
    ((OBSERVER_EVENT_QUEUE_CAPACITY & (OBSERVER_EVENT_QUEUE_CAPACITY - 1U)) != 0U)
#error "OBSERVER_EVENT_QUEUE_CAPACITY must be a power of two"
#endif

static ObserverCanFrame event_queue[OBSERVER_EVENT_QUEUE_CAPACITY];
static volatile uint8_t event_head;
static volatile uint8_t event_tail;
static volatile uint32_t dropped_count;

void EventQueue_Init(void)
{
    event_head = 0U;
    event_tail = 0U;
    dropped_count = 0U;
}

uint8_t EventQueue_PushFromIsr(const ObserverCanFrame *frame)
{
    uint8_t next;

    if(frame == 0)
    {
        return 0U;
    }

    next = (uint8_t)((event_head + 1U) &
                     (OBSERVER_EVENT_QUEUE_CAPACITY - 1U));
    if(next == event_tail)
    {
        dropped_count++;
        return 0U;
    }

    event_queue[event_head] = *frame;
    __DMB();
    event_head = next;
    return 1U;
}

uint8_t EventQueue_Pop(ObserverCanFrame *frame)
{
    if((frame == 0) || (event_tail == event_head))
    {
        return 0U;
    }

    *frame = event_queue[event_tail];
    __DMB();
    event_tail = (uint8_t)((event_tail + 1U) &
                           (OBSERVER_EVENT_QUEUE_CAPACITY - 1U));
    return 1U;
}

uint32_t EventQueue_GetDroppedCount(void)
{
    return dropped_count;
}

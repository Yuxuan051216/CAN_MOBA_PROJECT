#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <stdint.h>

#include "can_moba_protocol.h"

void EventQueue_Init(void);
uint8_t EventQueue_PushFromIsr(const ObserverCanFrame *frame);
uint8_t EventQueue_Pop(ObserverCanFrame *frame);
uint32_t EventQueue_GetDroppedCount(void);

#endif

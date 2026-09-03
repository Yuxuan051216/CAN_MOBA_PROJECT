#ifndef STATUS_DISPLAY_H
#define STATUS_DISPLAY_H

#include "observer_controller.h"

void StatusDisplay_Init(void);
void StatusDisplay_SetState(const ObserverState *state);
void StatusDisplay_Update(void);
void StatusDisplay_ForceRefresh(void);

#endif

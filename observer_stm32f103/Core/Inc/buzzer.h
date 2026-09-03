#ifndef BUZZER_H
#define BUZZER_H

typedef enum {
    BUZZER_PATTERN_HIT = 0,
    BUZZER_PATTERN_DEATH,
    BUZZER_PATTERN_SWITCH
} BuzzerPattern;

void Buzzer_Init(void);
void Buzzer_Play(BuzzerPattern pattern);
void Buzzer_Update(void);
void Buzzer_Reset(void);

#endif

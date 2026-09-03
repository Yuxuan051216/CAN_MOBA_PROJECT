#include "rgb_effect.h"

#include "observer_config.h"
#include "stm32f1xx_hal.h"

#define RGB_BUFFER_LENGTH \
    ((OBSERVER_WS2812_LED_COUNT * 24U) + OBSERVER_WS2812_RESET_SLOTS)

static uint16_t pwm_buffer[RGB_BUFFER_LENGTH];
static volatile uint8_t dma_busy;
static RgbEffectType active_effect;
static uint32_t effect_started_ms;
static uint8_t active_priority;
static uint8_t last_r;
static uint8_t last_g;
static uint8_t last_b;

static uint8_t RgbEffect_Priority(RgbEffectType effect)
{
    switch(effect)
    {
        case RGB_EFFECT_DEATH:
            return 5U;
        case RGB_EFFECT_CRYSTAL_BLUE:
        case RGB_EFFECT_CRYSTAL_RED:
            return 4U;
        case RGB_EFFECT_MASTER_A:
        case RGB_EFFECT_MASTER_B:
            return 3U;
        case RGB_EFFECT_SKILL_1:
        case RGB_EFFECT_SKILL_2_HEAL:
        case RGB_EFFECT_SKILL_3:
            return 2U;
        default:
            return 0U;
    }
}

static void RgbEffect_StartTransfer(uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t led;
    uint32_t bit;
    uint32_t index = 0U;
    uint32_t color = ((uint32_t)g << 16) |
                     ((uint32_t)r << 8) |
                     b;

    if(dma_busy != 0U)
    {
        return;
    }

    for(led = 0U; led < OBSERVER_WS2812_LED_COUNT; led++)
    {
        for(bit = 0U; bit < 24U; bit++)
        {
            pwm_buffer[index++] =
                ((color & (1UL << (23U - bit))) != 0U) ?
                OBSERVER_WS2812_ONE_DUTY :
                OBSERVER_WS2812_ZERO_DUTY;
        }
    }
    while(index < RGB_BUFFER_LENGTH)
    {
        pwm_buffer[index++] = 0U;
    }

    DMA1_Channel1->CCR &= ~DMA_CCR_EN;
    DMA1->IFCR = DMA_IFCR_CGIF1;
    DMA1_Channel1->CMAR = (uint32_t)pwm_buffer;
    DMA1_Channel1->CNDTR = RGB_BUFFER_LENGTH;
    TIM4->CNT = 0U;
    TIM4->CCR1 = 0U;
    dma_busy = 1U;
    DMA1_Channel1->CCR |= DMA_CCR_EN;
    TIM4->CR1 |= TIM_CR1_CEN;
}

void RgbEffect_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    gpio.Pin = GPIO_PIN_6;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);

    TIM4->CR1 = TIM_CR1_ARPE;
    TIM4->PSC = 0U;
    TIM4->ARR = OBSERVER_WS2812_TIMER_PERIOD - 1U;
    TIM4->CCR1 = 0U;
    TIM4->CCMR1 = TIM_CCMR1_OC1M_PWM1 | TIM_CCMR1_OC1PE;
    TIM4->CCER = TIM_CCER_CC1E;
    TIM4->DIER = TIM_DIER_CC1DE;
    TIM4->EGR = TIM_EGR_UG;

    DMA1_Channel1->CCR = DMA_CCR_TCIE |
                         DMA_CCR_DIR |
                         DMA_CCR_MINC |
                         DMA_CCR_PSIZE_0 |
                         DMA_CCR_MSIZE_0 |
                         DMA_CCR_PL_1;
    DMA1_Channel1->CPAR = (uint32_t)&TIM4->CCR1;
    DMA1_Channel1->CMAR = (uint32_t)pwm_buffer;
    DMA1_Channel1->CNDTR = 0U;

    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 2U, 0U);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    active_effect = RGB_EFFECT_NONE;
    active_priority = 0U;
    effect_started_ms = 0U;
    dma_busy = 0U;
    last_r = 0xFFU;
    last_g = 0xFFU;
    last_b = 0xFFU;
}

void RgbEffect_Start(RgbEffectType effect)
{
    uint8_t priority = RgbEffect_Priority(effect);

    if((active_effect != RGB_EFFECT_NONE) &&
       (priority < active_priority))
    {
        return;
    }
    active_effect = effect;
    active_priority = priority;
    effect_started_ms = HAL_GetTick();
}

static void RgbEffect_Color(uint32_t elapsed,
                            uint8_t *r,
                            uint8_t *g,
                            uint8_t *b,
                            uint8_t *finished)
{
    uint8_t phase;

    *r = 0U;
    *g = 0U;
    *b = 0U;
    *finished = 0U;
    phase = (uint8_t)((elapsed / 100U) & 1U);

    switch(active_effect)
    {
        case RGB_EFFECT_SKILL_1:
            *r = (elapsed < 140U) ? 180U : 0U;
            *g = *r;
            *b = *r;
            *finished = (uint8_t)(elapsed >= 260U);
            break;
        case RGB_EFFECT_SKILL_2_HEAL:
            *g = (phase == 0U) ? 180U : 45U;
            *r = (phase == 0U) ? 10U : 0U;
            *finished = (uint8_t)(elapsed >= 1000U);
            break;
        case RGB_EFFECT_SKILL_3:
            if(((elapsed / 110U) & 1U) == 0U)
            {
                *r = 220U;
                *g = 220U;
                *b = 255U;
            }
            *finished = (uint8_t)(elapsed >= 660U);
            break;
        case RGB_EFFECT_CRYSTAL_BLUE:
            if(phase == 0U)
            {
                *b = 220U;
                *g = 50U;
            }
            else
            {
                *r = 160U;
                *g = 160U;
                *b = 200U;
            }
            *finished = (uint8_t)(elapsed >= 800U);
            break;
        case RGB_EFFECT_CRYSTAL_RED:
            if(phase == 0U)
            {
                *r = 220U;
                *g = 15U;
            }
            else
            {
                *r = 200U;
                *g = 160U;
                *b = 160U;
            }
            *finished = (uint8_t)(elapsed >= 800U);
            break;
        case RGB_EFFECT_DEATH:
            *r = (phase == 0U) ? 220U : 20U;
            *finished = (uint8_t)(elapsed >= 1400U);
            break;
        case RGB_EFFECT_MASTER_A:
            *b = 180U;
            *g = 70U;
            *finished = (uint8_t)(elapsed >= 700U);
            break;
        case RGB_EFFECT_MASTER_B:
            *r = 180U;
            *g = 25U;
            *finished = (uint8_t)(elapsed >= 700U);
            break;
        default:
            *finished = 1U;
            break;
    }
}

void RgbEffect_Update(void)
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t finished;

    RgbEffect_Color(HAL_GetTick() - effect_started_ms,
                    &r, &g, &b, &finished);
    if(finished != 0U)
    {
        active_effect = RGB_EFFECT_NONE;
        active_priority = 0U;
        r = 0U;
        g = 0U;
        b = 0U;
    }

    if((dma_busy == 0U) &&
       ((r != last_r) || (g != last_g) || (b != last_b)))
    {
        last_r = r;
        last_g = g;
        last_b = b;
        RgbEffect_StartTransfer(r, g, b);
    }
}

void RgbEffect_Reset(void)
{
    active_effect = RGB_EFFECT_NONE;
    active_priority = 0U;
    effect_started_ms = HAL_GetTick();
    last_r = 0xFFU;
    last_g = 0xFFU;
    last_b = 0xFFU;
}

void RgbEffect_DmaIrqHandler(void)
{
    if((DMA1->ISR & DMA_ISR_TCIF1) != 0U)
    {
        DMA1->IFCR = DMA_IFCR_CGIF1;
        DMA1_Channel1->CCR &= ~DMA_CCR_EN;
        TIM4->CR1 &= ~TIM_CR1_CEN;
        TIM4->CCR1 = 0U;
        dma_busy = 0U;
    }
}

RgbEffectType RgbEffect_GetActive(void)
{
    return active_effect;
}

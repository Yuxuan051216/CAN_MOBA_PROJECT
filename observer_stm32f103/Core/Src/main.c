#include "main.h"

#include "buzzer.h"
#include "can_receiver.h"
#include "dfplayer.h"
#include "event_queue.h"
#include "observer_config.h"
#include "observer_controller.h"
#include "rgb_effect.h"
#include "status_display.h"
#include "voice_queue.h"

CAN_HandleTypeDef hcan;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

int main(void)
{
    uint32_t last_led_ms;

    if(HAL_Init() != HAL_OK)
    {
        Error_Handler();
    }
    SystemClock_Config();
    MX_GPIO_Init();
    MX_CAN_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    EventQueue_Init();
    DFPlayer_Init(&huart1);
    VoiceQueue_Init();
    RgbEffect_Init();
    Buzzer_Init();
    StatusDisplay_Init();
    ObserverController_Init();

    if(CanReceiver_Init(&hcan) != HAL_OK)
    {
        Error_Handler();
    }

    last_led_ms = HAL_GetTick();
    while(1)
    {
        uint32_t now = HAL_GetTick();

        Observer_ProcessEvents();
        VoiceQueue_Update();
        RgbEffect_Update();
        Buzzer_Update();
        StatusDisplay_Update();

        if((uint32_t)(now - last_led_ms) >=
           OBSERVER_RUN_LED_PERIOD_MS)
        {
            last_led_ms = now;
            HAL_GPIO_TogglePin(RUN_LED_GPIO_PORT, RUN_LED_PIN);
        }
    }
}

static void SystemClock_Config(void)
{
    uint32_t timeout;

    RCC->CR |= RCC_CR_HSEON;
    timeout = 1000000UL;
    while(((RCC->CR & RCC_CR_HSERDY) == 0U) && (timeout-- != 0U))
    {
    }
    if(timeout == 0U)
    {
        Error_Handler();
    }

    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;
    RCC->CFGR = RCC_CFGR_HPRE_DIV1 |
                RCC_CFGR_PPRE1_DIV2 |
                RCC_CFGR_PPRE2_DIV1 |
                RCC_CFGR_PLLSRC |
                RCC_CFGR_PLLMULL9;
    RCC->CR |= RCC_CR_PLLON;
    timeout = 1000000UL;
    while(((RCC->CR & RCC_CR_PLLRDY) == 0U) && (timeout-- != 0U))
    {
    }
    if(timeout == 0U)
    {
        Error_Handler();
    }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    timeout = 1000000UL;
    while(((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) &&
          (timeout-- != 0U))
    {
    }
    if(timeout == 0U)
    {
        Error_Handler();
    }

    SystemCoreClockUpdate();
    if(SysTick_Config(SystemCoreClock / 1000UL) != 0U)
    {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN |
                    RCC_APB2ENR_IOPAEN |
                    RCC_APB2ENR_IOPBEN |
                    RCC_APB2ENR_IOPCEN;

    AFIO->MAPR = (AFIO->MAPR & ~(7UL << 24)) | (2UL << 24);

    HAL_GPIO_WritePin(RUN_LED_GPIO_PORT, RUN_LED_PIN, GPIO_PIN_SET);
    gpio.Pin = RUN_LED_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RUN_LED_GPIO_PORT, &gpio);

    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_RESET);
    gpio.Pin = BUZZER_PIN;
    HAL_GPIO_Init(BUZZER_GPIO_PORT, &gpio);

    gpio.Pin = GPIO_PIN_11;
    gpio.Mode = GPIO_MODE_INPUT_FLOATING;
    HAL_GPIO_Init(GPIOA, &gpio);
    gpio.Pin = GPIO_PIN_12;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_9 | GPIO_PIN_2;
    gpio.Mode = GPIO_MODE_AF_PP;
    HAL_GPIO_Init(GPIOA, &gpio);
    gpio.Pin = GPIO_PIN_10 | GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_INPUT_FLOATING;
    HAL_GPIO_Init(GPIOA, &gpio);
}

static void MX_CAN_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
    hcan.Instance = CAN1;
    hcan.Init.Prescaler = OBSERVER_CAN_PRESCALER;
    hcan.Init.Mode = CAN_MODE_NORMAL;
    hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
    hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
    hcan.Init.TimeTriggeredMode = DISABLE;
    hcan.Init.AutoBusOff = ENABLE;
    hcan.Init.AutoWakeUp = DISABLE;
    hcan.Init.AutoRetransmission = ENABLE;
    hcan.Init.ReceiveFifoLocked = DISABLE;
    hcan.Init.TransmitFifoPriority = DISABLE;
    if(HAL_CAN_Init(&hcan) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_USART1_UART_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600U;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if(HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_NVIC_SetPriority(USART1_IRQn, 3U, 0U);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

static void MX_USART2_UART_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200U;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if(HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    HAL_GPIO_WritePin(RUN_LED_GPIO_PORT, RUN_LED_PIN, GPIO_PIN_RESET);
    while(1)
    {
    }
}

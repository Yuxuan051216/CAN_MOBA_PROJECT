#ifndef STM32F1XX_HAL_H
#define STM32F1XX_HAL_H

#include <stddef.h>
#include <stdint.h>

#include "stm32f1xx.h"

#define __weak __attribute__((weak))

#define ENABLE  1U
#define DISABLE 0U

typedef enum {
    HAL_OK = 0x00U,
    HAL_ERROR = 0x01U,
    HAL_BUSY = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

typedef enum {
    GPIO_PIN_RESET = 0U,
    GPIO_PIN_SET
} GPIO_PinState;

#define GPIO_PIN_0                     (1U << 0)
#define GPIO_PIN_1                     (1U << 1)
#define GPIO_PIN_2                     (1U << 2)
#define GPIO_PIN_3                     (1U << 3)
#define GPIO_PIN_4                     (1U << 4)
#define GPIO_PIN_5                     (1U << 5)
#define GPIO_PIN_6                     (1U << 6)
#define GPIO_PIN_7                     (1U << 7)
#define GPIO_PIN_8                     (1U << 8)
#define GPIO_PIN_9                     (1U << 9)
#define GPIO_PIN_10                    (1U << 10)
#define GPIO_PIN_11                    (1U << 11)
#define GPIO_PIN_12                    (1U << 12)
#define GPIO_PIN_13                    (1U << 13)

#define GPIO_MODE_INPUT                0U
#define GPIO_MODE_OUTPUT_PP            1U
#define GPIO_MODE_OUTPUT_OD            2U
#define GPIO_MODE_AF_PP                3U
#define GPIO_MODE_AF_OD                4U
#define GPIO_MODE_INPUT_PULLUP         5U
#define GPIO_MODE_INPUT_FLOATING       6U

#define GPIO_NOPULL                    0U
#define GPIO_PULLUP                    1U
#define GPIO_PULLDOWN                  2U

#define GPIO_SPEED_FREQ_LOW            1U
#define GPIO_SPEED_FREQ_MEDIUM         2U
#define GPIO_SPEED_FREQ_HIGH           3U

typedef struct {
    uint32_t Pin;
    uint32_t Mode;
    uint32_t Pull;
    uint32_t Speed;
} GPIO_InitTypeDef;

#define CAN_MODE_NORMAL                0U
#define CAN_SJW_1TQ                    0U
#define CAN_BS1_13TQ                   12U
#define CAN_BS2_4TQ                    3U
#define CAN_FILTERMODE_IDLIST          1U
#define CAN_FILTERSCALE_16BIT          0U
#define CAN_FILTER_FIFO0               0U
#define CAN_ID_STD                     0U
#define CAN_ID_EXT                     4U
#define CAN_RTR_DATA                   0U
#define CAN_RX_FIFO0                   0U
#define CAN_IT_RX_FIFO0_MSG_PENDING    (1UL << 1)

typedef struct {
    uint32_t Prescaler;
    uint32_t Mode;
    uint32_t SyncJumpWidth;
    uint32_t TimeSeg1;
    uint32_t TimeSeg2;
    uint32_t TimeTriggeredMode;
    uint32_t AutoBusOff;
    uint32_t AutoWakeUp;
    uint32_t AutoRetransmission;
    uint32_t ReceiveFifoLocked;
    uint32_t TransmitFifoPriority;
} CAN_InitTypeDef;

typedef struct {
    CAN_TypeDef *Instance;
    CAN_InitTypeDef Init;
} CAN_HandleTypeDef;

typedef struct {
    uint32_t FilterIdHigh;
    uint32_t FilterIdLow;
    uint32_t FilterMaskIdHigh;
    uint32_t FilterMaskIdLow;
    uint32_t FilterFIFOAssignment;
    uint32_t FilterBank;
    uint32_t FilterMode;
    uint32_t FilterScale;
    uint32_t FilterActivation;
    uint32_t SlaveStartFilterBank;
} CAN_FilterTypeDef;

typedef struct {
    uint32_t StdId;
    uint32_t ExtId;
    uint32_t IDE;
    uint32_t RTR;
    uint32_t DLC;
    uint32_t Timestamp;
    uint32_t FilterMatchIndex;
} CAN_RxHeaderTypeDef;

#define UART_WORDLENGTH_8B              0U
#define UART_STOPBITS_1                 0U
#define UART_PARITY_NONE                0U
#define UART_MODE_TX_RX                 3U
#define UART_HWCONTROL_NONE             0U
#define UART_OVERSAMPLING_16            0U
#define HAL_UART_STATE_READY            0U
#define HAL_UART_STATE_BUSY_TX          1U

typedef struct {
    uint32_t BaudRate;
    uint32_t WordLength;
    uint32_t StopBits;
    uint32_t Parity;
    uint32_t Mode;
    uint32_t HwFlowCtl;
    uint32_t OverSampling;
} UART_InitTypeDef;

typedef struct {
    USART_TypeDef *Instance;
    UART_InitTypeDef Init;
    const uint8_t *pTxBuffPtr;
    uint16_t TxXferSize;
    volatile uint16_t TxXferCount;
    volatile uint32_t gState;
} UART_HandleTypeDef;

HAL_StatusTypeDef HAL_Init(void);
uint32_t HAL_GetTick(void);
void HAL_IncTick(void);
void HAL_Delay(uint32_t delay_ms);

void HAL_NVIC_SetPriority(IRQn_Type irqn,
                          uint32_t preempt_priority,
                          uint32_t sub_priority);
void HAL_NVIC_EnableIRQ(IRQn_Type irqn);

void HAL_GPIO_Init(GPIO_TypeDef *gpio, const GPIO_InitTypeDef *init);
void HAL_GPIO_WritePin(GPIO_TypeDef *gpio,
                       uint16_t pin,
                       GPIO_PinState state);
void HAL_GPIO_TogglePin(GPIO_TypeDef *gpio, uint16_t pin);

HAL_StatusTypeDef HAL_CAN_Init(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_ConfigFilter(
    CAN_HandleTypeDef *hcan,
    const CAN_FilterTypeDef *filter);
HAL_StatusTypeDef HAL_CAN_Start(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_ActivateNotification(
    CAN_HandleTypeDef *hcan,
    uint32_t notifications);
HAL_StatusTypeDef HAL_CAN_GetRxMessage(
    CAN_HandleTypeDef *hcan,
    uint32_t fifo,
    CAN_RxHeaderTypeDef *header,
    uint8_t data[8]);
void HAL_CAN_IRQHandler(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *huart,
                                       const uint8_t *data,
                                       uint16_t size);
void HAL_UART_IRQHandler(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

#endif

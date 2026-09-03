#include "bsp_can.h"

#include <stdio.h>

#include "app_config.h"
#include "bsp_time.h"
#include "ch32v30x.h"

#define CAN_BITRATE_HZ          500000UL
#define CAN_TOTAL_TQ            16UL
#define CAN_TX_TIMEOUT_MS       10U
#define CAN_TX_POLL_LIMIT       1000000UL

static uint8_t can_ready;

static void BSP_CAN_PrintData(const uint8_t *data)
{
#if CAN_DEBUG_PRINT
    uint8_t i;

    for(i = 0U; i < 8U; i++)
    {
        printf("%02X", (unsigned int)data[i]);
        if(i < 7U)
        {
            printf(" ");
        }
    }
#else
    (void)data;
#endif
}

static void BSP_CAN_GPIOInit(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

#if CAN1_USE_REMAP_PB8_PB9
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_8;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &gpio);
#else
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_11;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_12;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio);
#endif
}

void BSP_CAN_Init(void)
{
    CAN_InitTypeDef can_init;
    CAN_FilterInitTypeDef filter;
    RCC_ClocksTypeDef clocks;
    uint32_t bitrate_divisor;
    uint32_t prescaler;
    uint8_t init_status;

    can_ready = 0U;

    BSP_CAN_GPIOInit();
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    CAN_DeInit(CAN1);

    RCC_GetClocksFreq(&clocks);
    bitrate_divisor = CAN_BITRATE_HZ * CAN_TOTAL_TQ;

    if((clocks.PCLK1_Frequency % bitrate_divisor) != 0U)
    {
        printf("CAN1 init failed: PCLK1=%lu cannot use 16 tq at 500 kbps\r\n",
               (unsigned long)clocks.PCLK1_Frequency);
        return;
    }

    prescaler = clocks.PCLK1_Frequency / bitrate_divisor;
    if((prescaler == 0U) || (prescaler > 1024U))
    {
        printf("CAN1 init failed: invalid prescaler=%lu\r\n",
               (unsigned long)prescaler);
        return;
    }

    CAN_StructInit(&can_init);
    can_init.CAN_TTCM = DISABLE;
    can_init.CAN_ABOM = ENABLE;
    can_init.CAN_AWUM = DISABLE;
    can_init.CAN_NART = DISABLE;
    can_init.CAN_RFLM = DISABLE;
    can_init.CAN_TXFP = DISABLE;
    can_init.CAN_Mode = CAN_Mode_Normal;
    can_init.CAN_SJW = CAN_SJW_1tq;
    can_init.CAN_BS1 = CAN_BS1_12tq;
    can_init.CAN_BS2 = CAN_BS2_3tq;
    can_init.CAN_Prescaler = (uint16_t)prescaler;

    init_status = CAN_Init(CAN1, &can_init);
    if(init_status != CAN_InitStatus_Success)
    {
        printf("CAN1 init failed: peripheral timeout\r\n");
        return;
    }

    filter.CAN_FilterNumber = 0U;
    filter.CAN_FilterMode = CAN_FilterMode_IdMask;
    filter.CAN_FilterScale = CAN_FilterScale_32bit;
    filter.CAN_FilterIdHigh = 0U;
    filter.CAN_FilterIdLow = 0U;
    filter.CAN_FilterMaskIdHigh = 0U;
    filter.CAN_FilterMaskIdLow = 0U;
    filter.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    filter.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&filter);

    can_ready = 1U;

#if CAN1_USE_REMAP_PB8_PB9
    printf("CAN1 init success: PB8=RX PB9=TX, ");
#else
    printf("CAN1 init success: PA11=RX PA12=TX, ");
#endif
    printf("PCLK1=%lu prescaler=%lu 500 kbps\r\n",
           (unsigned long)clocks.PCLK1_Frequency,
           (unsigned long)prescaler);
}

uint8_t BSP_CAN_SendStd(uint16_t std_id, const uint8_t *data, uint8_t len)
{
    CanTxMsg tx;
    uint8_t mailbox;
    uint8_t status;
    uint8_t i;
    uint32_t start_ms;
    uint32_t poll_count;

    if((can_ready == 0U) || (std_id > 0x7FFU) ||
       (len > 8U) || ((data == 0) && (len != 0U)))
    {
        return 0U;
    }

    tx.StdId = std_id;
    tx.ExtId = 0U;
    tx.IDE = CAN_Id_Standard;
    tx.RTR = CAN_RTR_Data;
    tx.DLC = 8U;

    for(i = 0U; i < 8U; i++)
    {
        tx.Data[i] = (i < len) ? data[i] : 0U;
    }

    mailbox = CAN_Transmit(CAN1, &tx);
    if(mailbox == CAN_TxStatus_NoMailBox)
    {
        printf("CAN TX failed ID=0x%03X: no mailbox\r\n",
               (unsigned int)std_id);
        return 0U;
    }

    start_ms = BSP_Time_Millis();
    poll_count = 0U;
    do
    {
        status = CAN_TransmitStatus(CAN1, mailbox);
        poll_count++;
    } while((status == CAN_TxStatus_Pending) &&
            ((uint32_t)(BSP_Time_Millis() - start_ms) < CAN_TX_TIMEOUT_MS) &&
            (poll_count < CAN_TX_POLL_LIMIT));

    if(status != CAN_TxStatus_Ok)
    {
        CAN_CancelTransmit(CAN1, mailbox);
        printf("CAN TX failed ID=0x%03X status=%u\r\n",
               (unsigned int)std_id,
               (unsigned int)status);
        return 0U;
    }

#if CAN_DEBUG_PRINT
    printf("CAN TX ID=0x%03X DLC=8 DATA=", (unsigned int)std_id);
    BSP_CAN_PrintData(tx.Data);
    printf("\r\n");
#endif
    return 1U;
}

uint8_t BSP_CAN_Available(void)
{
    if(can_ready == 0U)
    {
        return 0U;
    }

    return (uint8_t)(CAN_MessagePending(CAN1, CAN_FIFO0) > 0U);
}

uint8_t BSP_CAN_Read(CanFrame_t *frame)
{
    CanRxMsg rx;
    uint8_t i;

    if((frame == 0) || (BSP_CAN_Available() == 0U))
    {
        return 0U;
    }

    CAN_Receive(CAN1, CAN_FIFO0, &rx);

    if((rx.IDE != CAN_Id_Standard) || (rx.RTR != CAN_RTR_Data))
    {
        printf("CAN RX ignored: only standard data frames are supported\r\n");
        return 0U;
    }

    frame->id = (uint16_t)rx.StdId;
    frame->dlc = rx.DLC;
    for(i = 0U; i < 8U; i++)
    {
        frame->data[i] = (i < rx.DLC) ? rx.Data[i] : 0U;
    }

#if CAN_DEBUG_PRINT
    printf("CAN RX ID=0x%03X DLC=%u DATA=",
           (unsigned int)frame->id,
           (unsigned int)frame->dlc);
    BSP_CAN_PrintData(frame->data);
    printf("\r\n");
#endif

    return 1U;
}

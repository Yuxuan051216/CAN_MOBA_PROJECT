#include "dfplayer.h"

static UART_HandleTypeDef *df_uart;
static uint8_t command_buffer[10];
static volatile uint8_t command_busy;

void DFPlayer_Init(UART_HandleTypeDef *huart)
{
    df_uart = huart;
    command_busy = 0U;
}

uint8_t DFPlayer_PlayTrack(uint16_t track)
{
    uint16_t checksum;
    uint8_t i;

    if((df_uart == 0) || (command_busy != 0U))
    {
        return 0U;
    }

    command_buffer[0] = 0x7EU;
    command_buffer[1] = 0xFFU;
    command_buffer[2] = 0x06U;
    command_buffer[3] = 0x03U;
    command_buffer[4] = 0x00U;
    command_buffer[5] = (uint8_t)(track >> 8);
    command_buffer[6] = (uint8_t)track;
    checksum = 0U;
    for(i = 1U; i <= 6U; i++)
    {
        checksum = (uint16_t)(checksum + command_buffer[i]);
    }
    checksum = (uint16_t)(0U - checksum);
    command_buffer[7] = (uint8_t)(checksum >> 8);
    command_buffer[8] = (uint8_t)checksum;
    command_buffer[9] = 0xEFU;

    command_busy = 1U;
    if(HAL_UART_Transmit_IT(df_uart,
                            command_buffer,
                            sizeof(command_buffer)) != HAL_OK)
    {
        command_busy = 0U;
        return 0U;
    }
    return 1U;
}

uint8_t DFPlayer_IsReady(void)
{
    return (uint8_t)(command_busy == 0U);
}

void DFPlayer_OnTxComplete(UART_HandleTypeDef *huart)
{
    if(huart == df_uart)
    {
        command_busy = 0U;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    DFPlayer_OnTxComplete(huart);
}

#include <stdio.h>

#include "debug.h"

#include "app_config.h"
#include "bsp_can.h"
#include "bsp_joystick.h"
#include "bsp_key.h"
#include "bsp_led.h"
#include "bsp_time.h"
#include "game_master.h"
#include "game_player.h"
#include "game_protocol.h"
#include "game_status_panel.h"
#include "node_role.h"

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    USART_Printf_Init(115200);

    BSP_Time_Init();
    BSP_LED_Init();
    BSP_Key_Init();
    BSP_Joystick_Init();
    BSP_CAN_Init();

    NodeRole_Init();
    GameMaster_Init();
    GamePlayer_Init();
    Protocol_Init();
    GameStatusPanel_Init();

    printf("\r\nCAN MOBA Node Start\r\n");
    printf("NODE_ID=%u SystemClk=%lu\r\n",
           (unsigned int)NODE_ID,
           (unsigned long)SystemCoreClock);
    printf("Firmware variant=%s CAN1=PB8_RX/PB9_TX\r\n",
           FIRMWARE_VARIANT_NAME);
    printf("Initial Master=%u Player=%u Term=%u\r\n",
           (unsigned int)g_node_role.current_master,
           (unsigned int)g_node_role.current_player,
           (unsigned int)g_node_role.current_term);
    printf("Joystick=%s, skill keys PB0/PB1/PB2 enabled\r\n",
           USE_JOYSTICK ? "ON" : "OFF");

    while(1)
    {
        CanFrame_t frame;
        uint8_t rx_count = 0U;

        /*
         * 每轮限制接收帧数量，确保心跳、角色检测和玩家输入始终能运行。
         * 下一轮会继续清空 FIFO，不会丢弃已经进入硬件 FIFO 的帧。
         */
        while(BSP_CAN_Available() &&
              (rx_count < CAN_RX_BUDGET_PER_LOOP))
        {
            if(BSP_CAN_Read(&frame))
            {
                Protocol_HandleRxFrame(&frame);
            }
            rx_count++;
        }

        BSP_Key_Update();
        BSP_Joystick_Update();

        Protocol_Update();
        NodeRole_Update();
        GameMaster_Update();
        GamePlayer_Update();
        GameStatusPanel_Update();

        BSP_LED_UpdateByRole(NodeRole_GetHeartbeatRole(),
                            g_node_role.self_hero_state);
    }
}

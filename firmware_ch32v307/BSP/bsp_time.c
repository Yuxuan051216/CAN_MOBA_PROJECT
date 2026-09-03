#include "bsp_time.h"

#include <stdint.h>

#include "ch32v30x.h"
#include "dmx_pit.h"

static volatile uint32_t system_millis;

void BSP_Time_Init(void)
{
    system_millis = 0U;

    /*
     * 使用独立 TIM2 产生 1 ms 时基，避免 SysTick 被 WCH Delay 或其他
     * 库代码重配后导致协议周期异常、心跳帧挤占总线。
     */
    init_pit(PIT2, 1000U);
}

void BSP_Time_On1msInterrupt(void)
{
    system_millis++;
}

uint32_t BSP_Time_Millis(void)
{
    return system_millis;
}

uint32_t millis(void)
{
    return BSP_Time_Millis();
}

void delay_ms(uint32_t ms)
{
    uint32_t start = millis();

    while((uint32_t)(millis() - start) < ms)
    {
        __NOP();
    }
}

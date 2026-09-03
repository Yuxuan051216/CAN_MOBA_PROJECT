#ifndef CORE_CM3_H
#define CORE_CM3_H

#include <stdint.h>

#define __I     volatile const
#define __O     volatile
#define __IO    volatile
#define __STATIC_INLINE static inline

typedef enum {
    NonMaskableInt_IRQn = -14,
    HardFault_IRQn = -13,
    MemoryManagement_IRQn = -12,
    BusFault_IRQn = -11,
    UsageFault_IRQn = -10,
    SVCall_IRQn = -5,
    DebugMonitor_IRQn = -4,
    PendSV_IRQn = -2,
    SysTick_IRQn = -1,
    WWDG_IRQn = 0,
    PVD_IRQn = 1,
    TAMPER_IRQn = 2,
    RTC_IRQn = 3,
    FLASH_IRQn = 4,
    RCC_IRQn = 5,
    EXTI0_IRQn = 6,
    EXTI1_IRQn = 7,
    EXTI2_IRQn = 8,
    EXTI3_IRQn = 9,
    EXTI4_IRQn = 10,
    DMA1_Channel1_IRQn = 11,
    DMA1_Channel2_IRQn = 12,
    DMA1_Channel3_IRQn = 13,
    DMA1_Channel4_IRQn = 14,
    DMA1_Channel5_IRQn = 15,
    DMA1_Channel6_IRQn = 16,
    DMA1_Channel7_IRQn = 17,
    ADC1_2_IRQn = 18,
    USB_HP_CAN1_TX_IRQn = 19,
    USB_LP_CAN1_RX0_IRQn = 20,
    CAN1_RX1_IRQn = 21,
    CAN1_SCE_IRQn = 22,
    EXTI9_5_IRQn = 23,
    TIM1_BRK_IRQn = 24,
    TIM1_UP_IRQn = 25,
    TIM1_TRG_COM_IRQn = 26,
    TIM1_CC_IRQn = 27,
    TIM2_IRQn = 28,
    TIM3_IRQn = 29,
    TIM4_IRQn = 30,
    I2C1_EV_IRQn = 31,
    I2C1_ER_IRQn = 32,
    I2C2_EV_IRQn = 33,
    I2C2_ER_IRQn = 34,
    SPI1_IRQn = 35,
    SPI2_IRQn = 36,
    USART1_IRQn = 37,
    USART2_IRQn = 38,
    USART3_IRQn = 39,
    EXTI15_10_IRQn = 40,
    RTCAlarm_IRQn = 41,
    USBWakeUp_IRQn = 42
} IRQn_Type;

typedef struct {
    __IO uint32_t CTRL;
    __IO uint32_t LOAD;
    __IO uint32_t VAL;
    __I  uint32_t CALIB;
} SysTick_Type;

typedef struct {
    __IO uint32_t ISER[8];
    uint32_t RESERVED0[24];
    __IO uint32_t ICER[8];
    uint32_t RESERVED1[24];
    __IO uint32_t ISPR[8];
    uint32_t RESERVED2[24];
    __IO uint32_t ICPR[8];
    uint32_t RESERVED3[24];
    __IO uint32_t IABR[8];
    uint32_t RESERVED4[56];
    __IO uint8_t IP[240];
} NVIC_Type;

typedef struct {
    __I  uint32_t CPUID;
    __IO uint32_t ICSR;
    __IO uint32_t VTOR;
    __IO uint32_t AIRCR;
    __IO uint32_t SCR;
    __IO uint32_t CCR;
    __IO uint8_t SHP[12];
    __IO uint32_t SHCSR;
} SCB_Type;

#define SCS_BASE            (0xE000E000UL)
#define SysTick_BASE        (SCS_BASE + 0x0010UL)
#define NVIC_BASE           (SCS_BASE + 0x0100UL)
#define SCB_BASE            (SCS_BASE + 0x0D00UL)
#define SysTick             ((SysTick_Type *)SysTick_BASE)
#define NVIC                ((NVIC_Type *)NVIC_BASE)
#define SCB                 ((SCB_Type *)SCB_BASE)

#define SysTick_CTRL_ENABLE_Msk       (1UL << 0)
#define SysTick_CTRL_TICKINT_Msk      (1UL << 1)
#define SysTick_CTRL_CLKSOURCE_Msk    (1UL << 2)

__STATIC_INLINE void __enable_irq(void)
{
    __asm volatile ("cpsie i" : : : "memory");
}

__STATIC_INLINE void __disable_irq(void)
{
    __asm volatile ("cpsid i" : : : "memory");
}

__STATIC_INLINE void __WFI(void)
{
    __asm volatile ("wfi");
}

__STATIC_INLINE void __NOP(void)
{
    __asm volatile ("nop");
}

__STATIC_INLINE void __DMB(void)
{
    __asm volatile ("dmb 0xF" : : : "memory");
}

__STATIC_INLINE void NVIC_EnableIRQ(IRQn_Type irqn)
{
    if((int32_t)irqn >= 0)
    {
        NVIC->ISER[(uint32_t)irqn >> 5U] =
            (uint32_t)(1UL << ((uint32_t)irqn & 0x1FU));
    }
}

__STATIC_INLINE void NVIC_DisableIRQ(IRQn_Type irqn)
{
    if((int32_t)irqn >= 0)
    {
        NVIC->ICER[(uint32_t)irqn >> 5U] =
            (uint32_t)(1UL << ((uint32_t)irqn & 0x1FU));
    }
}

__STATIC_INLINE void NVIC_SetPriority(IRQn_Type irqn, uint32_t priority)
{
    if((int32_t)irqn >= 0)
    {
        NVIC->IP[(uint32_t)irqn] = (uint8_t)((priority & 0x0FU) << 4U);
    }
    else
    {
        SCB->SHP[((uint32_t)irqn & 0x0FU) - 4U] =
            (uint8_t)((priority & 0x0FU) << 4U);
    }
}

__STATIC_INLINE uint32_t SysTick_Config(uint32_t ticks)
{
    if((ticks - 1UL) > 0x00FFFFFFUL)
    {
        return 1UL;
    }
    SysTick->LOAD = ticks - 1UL;
    NVIC_SetPriority(SysTick_IRQn, 15U);
    SysTick->VAL = 0UL;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
    return 0UL;
}

#endif

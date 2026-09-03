#include "stm32f1xx.h"

uint32_t SystemCoreClock = 8000000UL;

void SystemInit(void)
{
    RCC->CR |= RCC_CR_HSION;
    RCC->CFGR = 0U;
    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_PLLON);
    SCB->VTOR = 0x08000000UL;
    SystemCoreClock = 8000000UL;
}

void SystemCoreClockUpdate(void)
{
    SystemCoreClock = 72000000UL;
}

#ifndef STM32F103XB_H
#define STM32F103XB_H

#include "core_cm3.h"

typedef struct {
    __IO uint32_t CRL;
    __IO uint32_t CRH;
    __I  uint32_t IDR;
    __IO uint32_t ODR;
    __O  uint32_t BSRR;
    __O  uint32_t BRR;
    __IO uint32_t LCKR;
} GPIO_TypeDef;

typedef struct {
    __IO uint32_t EVCR;
    __IO uint32_t MAPR;
    __IO uint32_t EXTICR[4];
    uint32_t RESERVED0;
    __IO uint32_t MAPR2;
} AFIO_TypeDef;

typedef struct {
    __IO uint32_t CR;
    __IO uint32_t CFGR;
    __IO uint32_t CIR;
    __IO uint32_t APB2RSTR;
    __IO uint32_t APB1RSTR;
    __IO uint32_t AHBENR;
    __IO uint32_t APB2ENR;
    __IO uint32_t APB1ENR;
    __IO uint32_t BDCR;
    __IO uint32_t CSR;
} RCC_TypeDef;

typedef struct {
    __IO uint32_t ACR;
    __IO uint32_t KEYR;
    __IO uint32_t OPTKEYR;
    __IO uint32_t SR;
    __IO uint32_t CR;
    __IO uint32_t AR;
    uint32_t RESERVED;
    __IO uint32_t OBR;
    __IO uint32_t WRPR;
} FLASH_TypeDef;

typedef struct {
    __IO uint32_t CCR;
    __IO uint32_t CNDTR;
    __IO uint32_t CPAR;
    __IO uint32_t CMAR;
} DMA_Channel_TypeDef;

typedef struct {
    __I  uint32_t ISR;
    __O  uint32_t IFCR;
} DMA_TypeDef;

typedef struct {
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SMCR;
    __IO uint32_t DIER;
    __IO uint32_t SR;
    __IO uint32_t EGR;
    __IO uint32_t CCMR1;
    __IO uint32_t CCMR2;
    __IO uint32_t CCER;
    __IO uint32_t CNT;
    __IO uint32_t PSC;
    __IO uint32_t ARR;
    __IO uint32_t RCR;
    __IO uint32_t CCR1;
    __IO uint32_t CCR2;
    __IO uint32_t CCR3;
    __IO uint32_t CCR4;
    __IO uint32_t BDTR;
    __IO uint32_t DCR;
    __IO uint32_t DMAR;
} TIM_TypeDef;

typedef struct {
    __IO uint32_t SR;
    __IO uint32_t DR;
    __IO uint32_t BRR;
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t CR3;
    __IO uint32_t GTPR;
} USART_TypeDef;

typedef struct {
    __IO uint32_t TIR;
    __IO uint32_t TDTR;
    __IO uint32_t TDLR;
    __IO uint32_t TDHR;
} CAN_TxMailBox_TypeDef;

typedef struct {
    __I  uint32_t RIR;
    __I  uint32_t RDTR;
    __I  uint32_t RDLR;
    __I  uint32_t RDHR;
} CAN_FIFOMailBox_TypeDef;

typedef struct {
    __IO uint32_t FR1;
    __IO uint32_t FR2;
} CAN_FilterRegister_TypeDef;

typedef struct {
    __IO uint32_t MCR;
    __I  uint32_t MSR;
    __IO uint32_t TSR;
    __IO uint32_t RF0R;
    __IO uint32_t RF1R;
    __IO uint32_t IER;
    __IO uint32_t ESR;
    __IO uint32_t BTR;
    uint32_t RESERVED0[88];
    CAN_TxMailBox_TypeDef sTxMailBox[3];
    CAN_FIFOMailBox_TypeDef sFIFOMailBox[2];
    uint32_t RESERVED1[12];
    __IO uint32_t FMR;
    __IO uint32_t FM1R;
    uint32_t RESERVED2;
    __IO uint32_t FS1R;
    uint32_t RESERVED3;
    __IO uint32_t FFA1R;
    uint32_t RESERVED4;
    __IO uint32_t FA1R;
    uint32_t RESERVED5[8];
    CAN_FilterRegister_TypeDef sFilterRegister[14];
} CAN_TypeDef;

#define PERIPH_BASE             0x40000000UL
#define APB1PERIPH_BASE         PERIPH_BASE
#define APB2PERIPH_BASE         (PERIPH_BASE + 0x00010000UL)
#define AHBPERIPH_BASE          (PERIPH_BASE + 0x00020000UL)

#define TIM4_BASE               (APB1PERIPH_BASE + 0x0800UL)
#define USART2_BASE             (APB1PERIPH_BASE + 0x4400UL)
#define CAN1_BASE               (APB1PERIPH_BASE + 0x6400UL)
#define AFIO_BASE               (APB2PERIPH_BASE + 0x0000UL)
#define GPIOA_BASE              (APB2PERIPH_BASE + 0x0800UL)
#define GPIOB_BASE              (APB2PERIPH_BASE + 0x0C00UL)
#define GPIOC_BASE              (APB2PERIPH_BASE + 0x1000UL)
#define USART1_BASE             (APB2PERIPH_BASE + 0x3800UL)
#define DMA1_BASE               (AHBPERIPH_BASE + 0x0000UL)
#define DMA1_Channel1_BASE      (AHBPERIPH_BASE + 0x0008UL)
#define RCC_BASE                (AHBPERIPH_BASE + 0x1000UL)
#define FLASH_R_BASE            (AHBPERIPH_BASE + 0x2000UL)

#define TIM4                    ((TIM_TypeDef *)TIM4_BASE)
#define USART1                  ((USART_TypeDef *)USART1_BASE)
#define USART2                  ((USART_TypeDef *)USART2_BASE)
#define CAN1                    ((CAN_TypeDef *)CAN1_BASE)
#define AFIO                    ((AFIO_TypeDef *)AFIO_BASE)
#define GPIOA                   ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB                   ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC                   ((GPIO_TypeDef *)GPIOC_BASE)
#define DMA1                    ((DMA_TypeDef *)DMA1_BASE)
#define DMA1_Channel1           ((DMA_Channel_TypeDef *)DMA1_Channel1_BASE)
#define RCC                     ((RCC_TypeDef *)RCC_BASE)
#define FLASH                   ((FLASH_TypeDef *)FLASH_R_BASE)

#define RCC_CR_HSION            (1UL << 0)
#define RCC_CR_HSIRDY           (1UL << 1)
#define RCC_CR_HSEON            (1UL << 16)
#define RCC_CR_HSERDY           (1UL << 17)
#define RCC_CR_PLLON            (1UL << 24)
#define RCC_CR_PLLRDY           (1UL << 25)

#define RCC_CFGR_SW             (3UL << 0)
#define RCC_CFGR_SW_PLL         (2UL << 0)
#define RCC_CFGR_SWS            (3UL << 2)
#define RCC_CFGR_SWS_PLL        (2UL << 2)
#define RCC_CFGR_HPRE_DIV1      (0UL << 4)
#define RCC_CFGR_PPRE1_DIV2     (4UL << 8)
#define RCC_CFGR_PPRE2_DIV1     (0UL << 11)
#define RCC_CFGR_PLLSRC         (1UL << 16)
#define RCC_CFGR_PLLMULL9       (7UL << 18)

#define RCC_AHBENR_DMA1EN       (1UL << 0)
#define RCC_APB2ENR_AFIOEN      (1UL << 0)
#define RCC_APB2ENR_IOPAEN      (1UL << 2)
#define RCC_APB2ENR_IOPBEN      (1UL << 3)
#define RCC_APB2ENR_IOPCEN      (1UL << 4)
#define RCC_APB2ENR_USART1EN    (1UL << 14)
#define RCC_APB1ENR_TIM4EN      (1UL << 2)
#define RCC_APB1ENR_USART2EN    (1UL << 17)
#define RCC_APB1ENR_CAN1EN      (1UL << 25)

#define FLASH_ACR_LATENCY_2     (2UL << 0)
#define FLASH_ACR_PRFTBE        (1UL << 4)

#define DMA_CCR_EN              (1UL << 0)
#define DMA_CCR_TCIE            (1UL << 1)
#define DMA_CCR_DIR             (1UL << 4)
#define DMA_CCR_MINC            (1UL << 7)
#define DMA_CCR_PSIZE_0         (1UL << 8)
#define DMA_CCR_MSIZE_0         (1UL << 10)
#define DMA_CCR_PL_1            (1UL << 13)
#define DMA_ISR_TCIF1           (1UL << 1)
#define DMA_IFCR_CGIF1          (1UL << 0)

#define TIM_CR1_CEN             (1UL << 0)
#define TIM_CR1_ARPE            (1UL << 7)
#define TIM_DIER_CC1DE          (1UL << 9)
#define TIM_EGR_UG              (1UL << 0)
#define TIM_CCMR1_OC1PE         (1UL << 3)
#define TIM_CCMR1_OC1M_PWM1     (6UL << 4)
#define TIM_CCER_CC1E           (1UL << 0)

#define USART_SR_TXE            (1UL << 7)
#define USART_SR_TC             (1UL << 6)
#define USART_CR1_RE            (1UL << 2)
#define USART_CR1_TE            (1UL << 3)
#define USART_CR1_RXNEIE        (1UL << 5)
#define USART_CR1_TCIE          (1UL << 6)
#define USART_CR1_TXEIE         (1UL << 7)
#define USART_CR1_UE            (1UL << 13)

extern uint32_t SystemCoreClock;
void SystemInit(void);
void SystemCoreClockUpdate(void);

#endif

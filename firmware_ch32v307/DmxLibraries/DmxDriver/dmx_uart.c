/****************************************************************************************
 *     COPYRIGHT NOTICE
 *     Copyright (C) 2024,AS DAIMXA
 *     copyright Copyright (C) 呆萌侠DAIMXA,2024
 *     All rights reserved.
 *     技术讨论QQ群：710026750
 *
 *     除注明出处外，以下所有内容版权均属呆萌侠智能科技所有，未经允许，不得用于商业用途，
 *     修改内容时必须保留呆萌智能侠科技的版权声明。
 *      ____    _    ___ __  ____  __    _
 *     |  _ \  / \  |_ _|  \/  \ \/ /   / \
 *     | | | |/ _ \  | || |\/| |\  /   / _ \
 *     | |_| / ___ \ | || |  | |/  \  / ___ \
 *     |____/_/   \_\___|_|  |_/_/\_\/_/   \_\
 *
 * @file       dmx_uart.c
 * @brief      呆萌侠CH32V307VCT6开源库
 * @company    合肥呆萌侠智能科技有限公司
 * @author     呆萌侠科技（QQ：2453520483）
 * @MCUcore    CH32V307VCT6
 * @Software   MounRicer Stdio V191
 * @version    查看说明文档内version版本说明
 * @Taobao     https://daimxa.taobao.com/
 * @Openlib    https://gitee.com/daimxa
 * @date       2024-01-04
****************************************************************************************/

#include "dmx_uart.h"

void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART8_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

USART_TypeDef *UARTx[8] = {USART1, USART2, USART3, UART4, UART5, UART6, UART7, UART8};

/**
*
* @brief    串口模块初始化
* @param    uartpin     串口引脚
* @param    baudrate    波特率
* @return   void
* @notes
* Example:  init_uart(UART1_RM0_TX_A9_RX_A10 , 115200);
*
**/
void init_uart(UART_pin_enum uartpin,unsigned int baudrate)
{
    IRQn_Type irq_uart = 0;
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef  NVIC_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    switch((uartpin&0xF00000)>>20)
    {
        case 1:RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);irq_uart = USART1_IRQn;break;
        case 2:RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);irq_uart = USART2_IRQn;break;
        case 3:RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);irq_uart = USART3_IRQn;break;
        case 4:RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4 , ENABLE);irq_uart = UART4_IRQn;break;
        case 5:RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5 , ENABLE);irq_uart = UART5_IRQn;break;
        case 6:RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART6 , ENABLE);irq_uart = UART6_IRQn;break;
        case 7:RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7 , ENABLE);irq_uart = UART7_IRQn;break;
        case 8:RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8 , ENABLE);irq_uart = UART8_IRQn;break;
    }
    switch((uartpin&0xFF0000)>>16)
    {
        case 0x11:AFIO->PCFR2 &= ~(0x01<<26);AFIO -> PCFR1 |=  (0x01<<2);break;
        case 0x12:AFIO->PCFR2 |=  (0x01<<26);AFIO -> PCFR1 &= ~(0x01<<2);break;
        case 0x13:AFIO->PCFR2 |=  (0x01<<26);AFIO -> PCFR1 |=  (0x01<<2);break;

        case 0x21:GPIO_PinRemapConfig(GPIO_Remap_USART2 , ENABLE);break;

        case 0x31:GPIO_PinRemapConfig(GPIO_PartialRemap_USART3 , ENABLE);break;
        case 0x32:GPIO_PinRemapConfig(GPIO_PartialRemap1_USART3, ENABLE);break;
        case 0x33:GPIO_PinRemapConfig(GPIO_FullRemap_USART3    , ENABLE);break;

        case 0x41:GPIO_PinRemapConfig(GPIO_PartialRemap_USART4 , ENABLE);break;
        case 0x42:GPIO_PinRemapConfig(GPIO_FullRemap_USART4    , ENABLE);break;

        case 0x51:GPIO_PinRemapConfig(GPIO_PartialRemap_USART5 , ENABLE);break;
        case 0x52:GPIO_PinRemapConfig(GPIO_FullRemap_USART5    , ENABLE);break;

        case 0x61:GPIO_PinRemapConfig(GPIO_PartialRemap_USART6 , ENABLE);break;
        case 0x62:GPIO_PinRemapConfig(GPIO_FullRemap_USART6    , ENABLE);break;

        case 0x71:GPIO_PinRemapConfig(GPIO_PartialRemap_USART7 , ENABLE);break;
        case 0x72:GPIO_PinRemapConfig(GPIO_FullRemap_USART7    , ENABLE);break;

        case 0x81:GPIO_PinRemapConfig(GPIO_PartialRemap_USART8 , ENABLE);break;
        case 0x82:GPIO_PinRemapConfig(GPIO_FullRemap_USART8    , ENABLE);break;
    }

    init_gpio( (uartpin&0x00FF00)>>8 , GPO_AF_PP , Speed_50MHZ , 0);
    init_gpio( (uartpin&0x0000FF)    , GPI_PU    , Speed_50MHZ , 0);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(UARTx[((uartpin&0xF00000)>>20) - 1], &USART_InitStructure);
    USART_ITConfig(UARTx[((uartpin&0xF00000)>>20) - 1], USART_IT_RXNE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = irq_uart;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(UARTx[((uartpin&0xF00000)>>20) - 1], ENABLE);
}

/**
*
* @brief    串口发送字节
* @param    uartpin     串口引脚
* @param    ch          要发送的字符
* @return   void
* @notes    使用前请先初始化对应串口
* Example:  set_char_uart( UART1_RM0_TX_A9_RX_A10 ,'A');
*
**/
void set_char_uart(UART_pin_enum uartpin , char ch)
{
    unsigned char uartx = ((uartpin&0xF00000)>>20) - 1;
    while(((UARTx[uartx]) ->STATR & USART_FLAG_TXE)==0);
    (UARTx[uartx])->DATAR = ch;
}

/**
*
* @brief    串口发送字符串函数
* @param    uartpin     串口引脚
* @param    str         要发送的字符串首地址
* @return   void
* @notes    遇null停止发送,使用前请先初始化对应串口
* Example:  set_string_uart( UART1_RM0_TX_A9_RX_A10 ,"1234");
*
**/
void set_string_uart(UART_pin_enum uartpin , char *str)
{
   while(*str)
    {
       set_char_uart(uartpin, *str++);
    }
}

/**
*
* @brief    串口发送数组函数
* @param    uartpin     串口引脚
* @param    buf         要发送的字符串首地址
* @param    len         要发送的字符串长度
* @return   void
* @notes    使用前请先初始化对应串口
* Example:  set_buff_uart( UART1_RM0_TX_A9_RX_A10 , buf , 3 );
*
**/
void set_buff_uart(UART_pin_enum uartpin , unsigned char *buf, unsigned int len)
{
    while(len--)
    {
        set_char_uart(uartpin, *buf);
        buf++;
    }
}

/**
*
* @brief    串口接收字符(等待接收)
* @param    uartpin     串口引脚
* @return   char
* @notes    使用前请先初始化对应串口
* Example:  get1_char_uart(UART1_RM0_TX_A9_RX_A10);
*
**/
char get1_char_uart(UART_pin_enum uartpin)
{
    unsigned char uartx = ((uartpin&0xF00000)>>20) - 1;
    while(((UARTx[uartx])->STATR & USART_FLAG_RXNE) == 0);
    return ((UARTx[uartx])->DATAR & 0xFF);
}

/**
*
* @brief    串口接收字符(查询接收)
* @param    uartpin     串口引脚
* @param    ch          存储接收数据
* @return
* @notes    使用前请先初始化对应串口
* Example:  get2_char_uart( UART1_RM0_TX_A9_RX_A10 , data);
*
**/
char get2_char_uart(UART_pin_enum uartpin, unsigned char *ch)
{
    unsigned char uartx = ((uartpin&0xF00000)>>20) - 1;
    if(((UARTx[uartx])->STATR & USART_FLAG_RXNE) != 0)
    {
        *ch = ((UARTx[uartx])->DATAR & 0xFF);
        return 1;
    }
    return  0;
}

/**
*
* @brief    UART中断服务函数示例
* @param
* @return
* @notes
* Example:  直接使用以下函数即可
*
**/
void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        usart1_rx_irq();
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        usart2_rx_irq();
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
    {
        usart3_rx_irq();
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

void UART4_IRQHandler (void)
{
    if(USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
    {
        uart4_rx_irq();
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
    }
}

void UART5_IRQHandler (void)
{
    if(USART_GetITStatus(UART5, USART_IT_RXNE) == SET)
    {
        uart5_rx_irq();
        USART_ClearITPendingBit(UART5, USART_IT_RXNE);
    }
}

void UART6_IRQHandler (void)
{
    if(USART_GetITStatus(UART6, USART_IT_RXNE) == SET)
    {
        uart6_rx_irq();
        USART_ClearITPendingBit(UART6, USART_IT_RXNE);
    }
}

void UART7_IRQHandler (void)
{
    if(USART_GetITStatus(UART7, USART_IT_RXNE) == SET)
    {
        uart7_rx_irq();
        USART_ClearITPendingBit(UART7, USART_IT_RXNE);
    }
}

void UART8_IRQHandler (void)
{
    if(USART_GetITStatus(UART8, USART_IT_RXNE) == SET)
    {
        uart8_rx_irq();
        USART_ClearITPendingBit(UART8, USART_IT_RXNE);
    }
}

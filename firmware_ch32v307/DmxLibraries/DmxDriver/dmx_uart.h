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
 * @file       dmx_uart.h
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

#ifndef __DMX_UART_H
#define __DMX_UART_H

#include "dmx_gpio.h"

//串口引脚枚举
typedef enum
{
    //串口1
    UART1_RM0_TX_A9_RX_A10 = 0x10090A,
    UART1_RM1_TX_B6_RX_B7  = 0x111617,
    UART1_RM2_TX_B15_RX_A8 = 0x121F08,
    UART1_RM3_TX_A6_RX_A7  = 0x130607,

    //串口2
    UART2_RM0_TX_A2_RX_A3  = 0x200203,
    UART2_RM1_TX_D5_RX_D6  = 0x213536,

    //串口3
    UART3_RM0_TX_B10_RX_B11= 0x301A1B,
    UART3_RM1_TX_C10_RX_C11= 0x312A2B,
    UART3_RM2_TX_A13_RX_A14= 0x320D0E,
    UART3_RM3_TX_D8_RX_D9  = 0x333839,

    //串口4
    UART4_RM0_TX_C10_RX_C11= 0x402A2B,
    UART4_RM1_TX_B0_RX_B1  = 0x411011,
    UART4_RM2_TX_E0_RX_E1  = 0x424041,

    //串口5
    UART5_RM0_TX_C12_RX_D2 = 0x502C32,
    UART5_RM1_TX_B4_RX_B5  = 0x511415,
    UART5_RM2_TX_E8_RX_E9  = 0x524849,

    //串口6
    UART6_RM0_TX_C0_RX_C1  = 0x602021,
    UART6_RM1_TX_B8_RX_B9  = 0x611819,
    UART6_RM2_TX_E10_RX_E11= 0x624A4B,

    //串口7
    UART7_RM0_TX_C2_RX_C3  = 0x702223,
    UART7_RM1_TX_A6_RX_A7  = 0x710607,
    UART7_RM2_TX_E12_RX_E13= 0x724C4D,

    //串口8
    UART8_RM0_TX_C4_RX_C5  = 0x802425,
    UART8_RM1_TX_A14_RX_A15= 0x810E0F,
    UART8_RM2_TX_E14_RX_E15= 0x824E4F,

}UART_pin_enum;

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
void init_uart(UART_pin_enum uartpin,unsigned int baudrate);

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
void set_char_uart(UART_pin_enum uartpin , char ch);

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
void set_string_uart(UART_pin_enum uartpin , char *str);

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
void set_buff_uart(UART_pin_enum uartpin , unsigned char *buf, unsigned int len);

/**
*
* @brief    串口接收字符(等待接收)
* @param    uartpin     串口引脚
* @return   char
* @notes    使用前请先初始化对应串口
* Example:  get1_char_uart(UART1_RM0_TX_A9_RX_A10);
*
**/
char get1_char_uart(UART_pin_enum uartpin);

/**
*
* @brief    串口接收字符(查询接收)
* @param    uartpin     串口引脚
* @param    ch          存储接收数据
* @return
* @notes    使用前请先初始化对应串口
* Example:  get2_char_uart( UART1_RM0_TX_A9_RX_A10 , dat );
*
**/
char get2_char_uart(UART_pin_enum uartpin, unsigned char *ch);

#endif /* __DMX_UART_H */

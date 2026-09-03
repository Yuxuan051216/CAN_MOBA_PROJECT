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
 * @file       dmx_hard_spi.h
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

#ifndef __DMX_HARD_SPI_H
#define __DMX_HARD_SPI_H

// SPI模块
typedef enum
{
    SPI_1,
    SPI_2,
    SPI_3,
}SPI_module_enum;

// SPI模式
typedef enum
{
    SPI_MODE0,
    SPI_MODE1,
    SPI_MODE2,
    SPI_MODE3,
}SPI_mode_enum;

// SPI引脚枚举
typedef enum
{
    // SPI1
    SPI1_RM0_SCK_A5     = 0x1005,
    SPI1_RM0_MISO_A6    = 0x1006,
    SPI1_RM0_MOSI_A7    = 0x1007,

    SPI1_RM1_SCK_B3     = 0x1113,
    SPI1_RM1_MISO_B4    = 0x1114,
    SPI1_RM1_MOSI_B5    = 0x1115,

    // SPI2
    SPI2_RM0_SCK_B13    = 0x201D,
    SPI2_RM0_MISO_B14   = 0x201E,
    SPI2_RM0_MOSI_B15   = 0x201F,

    // SPI3
    SPI3_RM0_SCK_B3     = 0x3013,
    SPI3_RM0_MISO_B4    = 0x3014,
    SPI3_RM0_MOSI_B5    = 0x3015,

    SPI3_RM1_SCK_C10    = 0x312A,
    SPI3_RM1_MISO_C11   = 0x312B,
    SPI3_RM1_MOSI_C12   = 0x312C,

}SPI_pin_enum;

/**
*
* @brief    硬件SPI初始化
* @param    spi_n           SPI模块
* @param    mode            SPI模式
* @param    baud            波特率
* @param    sck_pin         SPI SCK引脚
* @param    mosi_pin        SPI MOSI引脚
* @param    miso_pin        SPI MISO引脚
* @return   void
* @notes
* Example:  init_hard_spi(SPI_1, SPI_MODE0, 72000000, SPI2_RM0_SCK_B13, SPI2_RM0_MOSI_B15 , SPI2_RM0_MISO_B14);
*
**/
void init_hard_spi (SPI_module_enum spi_n, SPI_mode_enum mode, unsigned int baud, SPI_pin_enum sck_pin, SPI_pin_enum mosi_pin, SPI_pin_enum miso_pin);

/**
*
* @brief    硬件SPI写8bit数据
* @param    spi_n           SPI模块
* @param    data            数据
* @return   void
* @notes
* Example:  write_8bit_hard_spi(SPI_1, 0x11);
*
**/
void write_8bit_hard_spi (SPI_module_enum spi_n, const unsigned char data);

/**
*
* @brief    硬件SPI写16bit数据
* @param    spi_n           SPI模块
* @param    data            数据
* @return   void
* @notes
* Example:  write_16bit_hard_spi(SPI_1, 0x1111);
*
**/
void write_16bit_hard_spi (SPI_module_enum spi_n, const unsigned int data);

/**
*
* @brief    硬件SPI写8bit寄存器
* @param    spi_n           SPI模块
* @param    register_name   寄存器地址
* @param    data            数据
* @return   void
* @notes
* Example:  write_8bit_reg_hard_spi(SPI_1,0XFB,0x11);
*
**/
void write_8bit_reg_hard_spi (SPI_module_enum spi_n, const unsigned char register_name, const unsigned char data);

/**
*
* @brief    硬件SPI写8bit寄存器数组
* @param    spi_n           SPI模块
* @param    register_name   寄存器地址
* @param    data            数据
* @param    len             长度
* @return   void
* @notes
* Example:  write_8bit_regs_hard_spi(SPI_1, 0x11, data, 32);
*
**/
void write_8bit_regs_hard_spi (SPI_module_enum spi_n, const unsigned char register_name, const unsigned char *data, unsigned int len);

/**
*
* @brief    硬件SPI读8bit寄存器数据
* @param    spi_n           SPI模块
* @return   uint8           数据
* @notes
* Example:  read_8bit_hard_spi(SPI_1);
*
**/
unsigned char read_8bit_hard_spi (SPI_module_enum spi_n);

/**
*
* @brief    硬件SPI读8bit寄存器数组
* @param    spi_n           SPI模块
* @param    register_name   寄存器地址
* @param    data            数据
* @param    len             长度
* @return   void
* @notes
* Example:  read_8bit_regs_hard_spi(SPI_1, 0x11, data, 32);
*
**/
void read_8bit_regs_hard_spi (SPI_module_enum spi_n, const unsigned char register_name, unsigned char *data, unsigned int len);

#endif /* __DMX_HARD_SPI_H */

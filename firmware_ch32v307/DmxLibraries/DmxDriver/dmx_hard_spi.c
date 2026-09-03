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
 * @file       dmx_hard_spi.c
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

#include "dmx_gpio.h"
#include "dmx_delay.h"
#include "dmx_hard_spi.h"

SPI_TypeDef *SPIx[3] = {SPI1, SPI2, SPI3};

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
void init_hard_spi (SPI_module_enum spi_n, SPI_mode_enum mode, unsigned int baud, SPI_pin_enum sck_pin, SPI_pin_enum mosi_pin, SPI_pin_enum miso_pin)
{
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    if(spi_n == 0)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE );
    else if(spi_n == 1)
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE );
    else if(spi_n == 2)
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE );

    if(sck_pin == SPI1_RM1_SCK_B3)
        GPIO_PinRemapConfig(GPIO_Remap_SPI1 , ENABLE);
    else if(sck_pin == SPI3_RM1_SCK_C10)
        GPIO_PinRemapConfig(GPIO_Remap_SPI3 , ENABLE);

    init_gpio( sck_pin&0x00FF  , GPO_AF_PP , Speed_50MHZ , 0);
    init_gpio( mosi_pin&0x00FF , GPO_AF_PP , Speed_50MHZ , 0);
    if(miso_pin != 0xff)
        init_gpio( miso_pin&0x00FF , GPI_PU , Speed_50MHZ , 0);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;

    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    if(mode == SPI_MODE2 || mode == SPI_MODE3)
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    if(mode == SPI_MODE1 || mode == SPI_MODE3)
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;

    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    if(baud >= SystemCoreClock / 2)
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    else if(baud >= SystemCoreClock / 4)
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    else if(baud >= SystemCoreClock / 8)
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    else if(baud >= SystemCoreClock / 16)
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    else if(baud >= SystemCoreClock / 32)
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    else if(baud >= SystemCoreClock / 64)
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    else if(baud >= SystemCoreClock / 128)
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
    else
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;

    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPIx[spi_n], &SPI_InitStructure);
    SPI_Cmd(SPIx[spi_n], ENABLE);
}

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
void write_8bit_hard_spi (SPI_module_enum spi_n, const unsigned char data)
{
    SPIx[spi_n]->DATAR = data;
    while((SPIx[spi_n]->STATR & SPI_I2S_FLAG_BSY) != RESET);
}

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
void write_16bit_hard_spi (SPI_module_enum spi_n, const unsigned int data)
{
    SPIx[spi_n]->DATAR = data >> 8;
    while((SPIx[spi_n]->STATR & SPI_I2S_FLAG_BSY) != RESET);
    SPIx[spi_n]->DATAR = data;
    while((SPIx[spi_n]->STATR & SPI_I2S_FLAG_BSY) != RESET);
}

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
void write_8bit_reg_hard_spi (SPI_module_enum spi_n, const unsigned char register_name, const unsigned char data)
{
    SPIx[spi_n]->DATAR = register_name;
    while((SPIx[spi_n]->STATR & SPI_I2S_FLAG_BSY) != RESET);
    SPIx[spi_n]->DATAR = data;
    while((SPIx[spi_n]->STATR & SPI_I2S_FLAG_BSY) != RESET);
}

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
void write_8bit_regs_hard_spi (SPI_module_enum spi_n, const unsigned char register_name, const unsigned char *data, unsigned int len)
{
    SPIx[spi_n]->DATAR = register_name;
    while((SPIx[spi_n]->STATR & SPI_I2S_FLAG_BSY) != RESET);
    do
    {
        SPIx[spi_n]->DATAR = *data ++;
        while((SPIx[spi_n]->STATR & SPI_I2S_FLAG_BSY) != RESET);
    }while(-- len);
}

/**
*
* @brief    硬件SPI读8bit寄存器数据
* @param    spi_n           SPI模块
* @return   uint8           数据
* @notes
* Example:  read_8bit_hard_spi(SPI_1);
*
**/
unsigned char read_8bit_hard_spi (SPI_module_enum spi_n)
{
    SPIx[spi_n]->DATAR = 0;
    while((SPIx[spi_n]->STATR & SPI_I2S_FLAG_BSY) != RESET);
    return (unsigned char)SPIx[spi_n]->DATAR;
}

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
void read_8bit_regs_hard_spi (SPI_module_enum spi_n, const unsigned char register_name, unsigned char *data, unsigned int len)
{
    SPIx[spi_n]->DATAR = register_name;
    while((SPIx[spi_n]->STATR & SPI_I2S_FLAG_BSY) != RESET);
    SPIx[spi_n]->DATAR;
    do
    {
        SPIx[spi_n]->DATAR = 0;
        while((SPIx[spi_n]->STATR & SPI_I2S_FLAG_BSY) != RESET);
        *data ++ = (unsigned char)SPIx[spi_n]->DATAR;  // 保存接收到的数据
    }while(-- len);
}



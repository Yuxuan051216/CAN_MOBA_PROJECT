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
 * @file       dmx_flash.c
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

#include "dmx_flash.h"

/**
*
* @brief    对指定页写入数据
* @param    sector      需要写入的扇区的编号,范围0~63
* @param    page        需要写入的页编号,参数范围0~15
* @param    buf         需要写入的数据地址,数组类型必须为unsigned int
* @param    len         一页256字节,根据数组类型为unsigned int则此处长度数据范围为1~64,(256/4 = 64)
* @return   void
* @notes    一个扇区中有16页 一个扇区4KB 一页256字节
* Example:  write_page(0,0, &buf , 64);
*
**/
void write_page(unsigned char sector, unsigned char page, unsigned int *buf,unsigned short len)
{
    volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
    unsigned long page_addr = 0x08000000 + (sector*1024*4) + (page*256);

    if(sector > 63)
        sector = 63;
    if(page > 15)
        page = 15;
    if(len > 64)
        len = 64;

    __disable_irq();
    erase_sector(sector,page);
    FLASH_Unlock();
    while(len--)
    {
        FLASHStatus = FLASH_ProgramWord(page_addr, *buf++);
        if(FLASHStatus != FLASH_COMPLETE)
        {
            // 写入失败
            break;
        }
        page_addr += 4;
    }
    FLASH_Lock();
    __enable_irq();
}

/**
*
* @brief    读取指定页数据
* @param    sector      需要读取的扇区的编号,范围0~63
* @param    page        需要读取的页编号,参数范围0-15
* @param    buf         需要读取的数据存放地址,数组类型必须为unsigned int
* @param    len         一页256字节,根据数组类型为unsigned int则此处长度数据范围为1~64,(256/4 = 64)
* @return   void
* @notes    一个扇区中有16页 一个扇区4KB 一页256字节
* Example:  read_page(0,0, &rbuf,64);
*
**/
void read_page(unsigned char sector, unsigned char page, unsigned int *rbuf, unsigned short len)
{
    unsigned short i;
    unsigned long page_addr = 0x08000000 + (sector*1024*4) + (page*256);
    if(sector > 63)
        sector = 63;
    if(page > 15)
        page = 15;
    if(len > 64)
        len = 64;

    __disable_irq();
    for (i = 0; i < len; i++)
    {
        rbuf[i] = *(__IO unsigned int*)(page_addr + i*4);
    }
    __enable_irq();
}

/**
*
* @brief    扇区擦除
* @param    sector      需要擦除扇区的编号,范围0~63
* @return   void
* @notes
* Example:  erase_sector(0);        // 擦除编号0的扇区
*
**/
void erase_sector(unsigned char sector , unsigned char page)
{
    volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
    unsigned long page_addr = 0x08000000 + (sector*1024*4) + (page*256);
    if(sector > 63)
        sector = 63;

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_WRPRTERR);
    FLASHStatus = FLASH_ErasePage(page_addr);
    FLASH_Lock();
    if(FLASHStatus != FLASH_COMPLETE)
    {
        // 擦除失败
    }
}

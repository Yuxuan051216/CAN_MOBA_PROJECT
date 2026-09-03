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
 * @file       dmx_flash.h
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

#ifndef __DMX_FLASH_H
#define __DMX_FLASH_H

#include "ch32v30x_flash.h"
#include "core_riscv.h"

/**
*
* @brief    对指定页写入数据
* @param    sector      需要写入的扇区的编号,范围0~63
* @param    page        需要写入的页编号,参数范围0~15
* @param    buf         需要写入的数据地址,数组类型必须为unsigned int
* @return   void
* @notes    一个扇区中有16页 一个扇区4KB 一页256字节
* Example:  write_page(0,0, &buf , 64);  // 在0扇区的第0页写入数据&buf,长度为64
*
**/
void write_page(unsigned char sector, unsigned char page, unsigned int *buf,unsigned short len);

/**
*
* @brief    读取指定页数据
* @param    sector      需要读取的扇区的编号,范围0~63
* @param    page        需要读取的页编号,参数范围0-15
* @param    buf         需要读取的数据存放地址,数组类型必须为unsigned int
* @return   void
* @notes
* Example:  read_page(0,0, &rbuf,64);  // 读取0扇区的第0页传入数组&rbuf,读取长度为64
*
**/
void read_page(unsigned char sector, unsigned char page, unsigned int *rbuf, unsigned short len);

/**
*
* @brief    扇区擦除
* @param    sector      需要擦除扇区的编号,范围0~63
* @return   void
* @notes
* Example:  erase_sector(63,15);        // 擦除编号0的扇区
*
**/
void erase_sector(unsigned char sector, unsigned char page);

#endif /* __DMX_FLASH_H */

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
 * @file       dmx_font.h
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

#ifndef __DMX_FONT_H
#define __DMX_FONT_H

// 输出设备,0:串口,1:oled屏幕,2:ips屏幕,3:tft屏幕
extern unsigned char output_device_flag;

//字体尺寸选择
typedef enum
{
    Show6x8,
    Show8x16,
}SHOW_size_enum;

//画笔颜色选择
typedef enum
{
    WHITE    = (0xFFFF),    // 白色
    BLACK    = (0x0000),    // 黑色
    BLUE     = (0x001F),    // 蓝色
    PURPLE   = (0xF81F),    // 紫色
    PINK     = (0xFE19),    // 粉色
    RED      = (0xF800),    // 红色
    MAGENTA  = (0xF81F),    // 品红
    GREEN    = (0x07E0),    // 绿色
    CYAN     = (0x07FF),    // 青色
    YELLOW   = (0xFFE0),    // 黄色
    BROWN    = (0xBC40),    // 棕色
    GRAY     = (0x8430),    // 灰色
}COLOR_enum;

extern const unsigned char Char6x8[][6];
extern const unsigned char Char8x16[][16];
extern const unsigned char OledChinese16x16[];
extern const unsigned char OledDMXLOGO128X54[896];
extern const unsigned char Chinese16x16[8][16];
extern const unsigned char Earth121x121[29282];

#endif /* __DMX_FONT_H */

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
 * @file       dmx_oled.h
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

#ifndef __DMX_OLED_H
#define __DMX_OLED_H

#include "dmx_font.h"

// 对OLED屏幕长宽进行设置
#define OLED_WIDTH          128
#define OLED_HEIGH          64

// 4脚OLED默认IIC地址,常见0.96寸SSD1306为0x3C
#define OLED_IIC_ADDR       0x3D
#define OLED_IIC_DELAY      200

// OLED,SCL管脚对应CH32V307VCT6引脚B13
#define OLED_SCL_PIN        B13
// OLED,SDA管脚对应CH32V307VCT6引脚B15
#define OLED_SDA_PIN        B15
// OLED屏幕延时
#define OLED_DELAY_MS(time)   (Delay_ms(time))
/**
*
* @brief    0.96寸4脚OLED屏幕初始化
* @param    void
* @return   void
* @notes    修改管脚定义可在dmx_oled.h文件里的宏定义管脚进行修改
* Example:  Init_OLED();
*
**/
void Init_OLED(void);

/**
*
* @brief    0.96寸4脚OLED屏幕画点
* @param    data
* @return   void
* @notes
* Example:  Draw_Point_OLED(0xff);
*
**/
void Draw_Point_OLED(unsigned char data);

/**
*
* @brief    0.96寸4脚OLED屏幕设置点坐标
* @param    x           x轴坐标数据(0~127)
* @param    y           y轴坐标数据(0~7)
* @return   void
* @notes    左上角坐标为(0,0)
* Example:  Set_Pos_OLED(0,0);
*
**/
void Set_Pos_OLED(unsigned char x, unsigned char y);

/**
*
* @brief    0.96寸4脚OLED屏幕点亮
* @param
* @return   void
* @notes
* Example:  Fill_OLED();
*
**/
void Fill_OLED(void);

/**
*
* @brief    0.96寸4脚OLED屏幕清屏
* @param
* @return   void
* @notes    执行后效果是全黑,黑屏
* Example:  Clean_OLED();
*
**/
void Clean_OLED(void);

/**
*
* @brief    0.96寸4脚OLED屏幕显示字符串
* @param    x           x轴坐标数据(0~127)
* @param    y           y轴坐标数据(0~7)
* @param    biglow      枚举字符显示大小
* @param    str         需要显示的字符串
* @return   void
* @notes
* Example:  Show_String_OLED(0,0,"123",Show8x16);    //在屏幕左上角显示8X16大小的“123”
*
**/
void Show_String_OLED(unsigned char x, unsigned char y, char str[], SHOW_size_enum fontsize);

/**
*
* @brief    0.96寸4脚OLED屏幕显示Int型数据
* @param    x                   x轴坐标数据(0~127)
* @param    y                   y轴坐标数据(0~7)
* @param    data                要显示的int型变量
* @param    num                 要显示的int型变量位数
* @param    fontsize            字体尺寸(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Show_Int_OLED(0,0,a,10,Show8x16);    //在屏幕左上角显示8x16大小的a变量的数据
*
**/
void Show_Int_OLED(char x, char y, const int data, unsigned char num, SHOW_size_enum fontsize);

/**
*
* @brief    0.96寸4脚OLED屏幕显示Float型数据
* @param    x                   x轴坐标数据(0~127)
* @param    y                   y轴坐标数据(0~7)
* @param    data                要显示的float型变量
* @param    num                 要显示的float型变量整数位数
* @param    pointnum            要显示的float型变量小数点位数
* @param    fontsize            字体尺寸(可在dmx_font.h文件里查看)
* @return   void
* @notes    Float型数据小数点后最多显示6位
* Example:  Show_Float_OLED(64,0,3.1415926,1,6,Show8x16);    //在屏幕左上角显示8X16大小的“3.142”
*
**/
void Show_Float_OLED(char x, char y, const double data, unsigned char num, unsigned char pointnum, SHOW_size_enum fontsize);

/**
*
* @brief    0.96寸4脚OLED屏幕显示16X16中文汉字
* @param    x           x轴坐标数据(0~127)
* @param    y           y轴坐标数据(0~7)
* @param    chinese     汉字数组的首地址
* @param    startnum    开始显示的汉字位号
* @param    endnum      结束显示的汉字位号
* @return   void
* @notes    如何取模在定义汉字字库时已说明
* Example:  Show_Chinese16x16_OLED(0,0,OledChinese16x16,0,3);   // 在屏幕左上角显示16X16大小的汉字“呆萌侠”
*
**/
void Show_Chinese16x16_OLED(char x, char y, const unsigned char *chinese, char startnum, char endnum);

/**
*
* @brief    0.96寸4脚OLED屏幕显示BMP图片
* @param    x           x轴坐标数据(0~127)
* @param    y           y轴坐标数据(0~7)
* @param    wide        图片实际像素宽度(1~127)
* @param    high        图片实际像素高度(1~64)
* @param    bmp         图片数组首地址
* @return   void
* @notes    图片最大像素为128×64
* Example:  Show_BMP_OLED(0,0,128,54,OledDMXLOGO128X54);    // 在屏幕显示呆萌侠LOGO
*
**/
void Show_BMP_OLED(unsigned char x, unsigned char y, unsigned char width, unsigned char height, const unsigned char *bmp);

/**
*
* @brief    OLED屏幕显示摄像头二值化图像
* @param    x                   x方向坐标
* @param    y                   y方向坐标
* @param    image               图像数组
* @param    width               图像数组实际宽度
* @param    heigh               图像数组实际高度
* @param    show_width          图像数组显示宽度
* @param    show_height         图像数组显示高度
* @param    threshold           二值化阈值
* @return   void
* @notes
* Example:  Show_Image_Oled(0,0,MT9V034_Image_Data[0],188,120,80,60,80);
*
**/
void Show_Image_Oled (unsigned short x, unsigned short y, const unsigned char *image, unsigned short width, unsigned short height, unsigned short show_width, unsigned short show_height, unsigned char threshold);

#endif /* __DMX_OLED_H */

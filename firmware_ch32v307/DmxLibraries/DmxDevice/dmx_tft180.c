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
 * @file       dmx_tft180.c
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

#include "dmx_tft180.h"
#include "dmx_gpio.h"
#include "dmx_delay.h"
#include "dmx_hard_spi.h"
#include "dmx_soft_spi.h"
#include "dmx_function.h"

// SPI协议读写操作宏定义
#if TFT180_HARD_SPI
#define TFT180_Write_8bitData(data)     (write_8bit_hard_spi(TFT180_SPI, (data)))
#define TFT180_Write_16bitData(data)    (write_16bit_hard_spi(TFT180_SPI, (data)))
#else
static  SOFT_SPI_struct                 TFT180_SPI;
#define TFT180_Write_8bitData(data)     (write_8bitdata_soft_spi(&TFT180_SPI, (data)))
#define TFT180_Write_16bitData(data)    (write_16bitdata_soft_spi(&TFT180_SPI, (data)))
#endif

// 静态函数声明,以下函数均为该.c文件内部调用
static void Write_Cmd_TFT180(unsigned char cmd);
static void Write_8bit_TFT180(unsigned char data);
static void Write_16bit_TFT180(unsigned short data);

/**
*
* @brief    TFT1.8寸屏幕初始化
* @param    void
* @return   void
* @notes    修改管脚定义可在dmx_tft180.h文件里的宏定义管脚进行修改
* Example:  Init_TFT180();
*
**/
void Init_TFT180(void)
{
    // SPI初始化
#if TFT180_HARD_SPI
    init_hard_spi(TFT180_SPI, SPI_MODE0, TFT180_SPI_SPEED, TFT180_SCL_PIN, TFT180_SDA_PIN, 0xff);
#else
    init_soft_spi(&TFT180_SPI, 0, TFT180_SCL_PIN, TFT180_SDA_PIN, 0xff);
#endif

    init_gpio( TFT180_RES_PIN , GPO_PP , Speed_50MHZ , 0);
    init_gpio( TFT180_DC_PIN  , GPO_PP , Speed_50MHZ , 0);
    init_gpio( TFT180_CS_PIN  , GPO_PP , Speed_50MHZ , 0);
    init_gpio( TFT180_BLK_PIN , GPO_PP , Speed_50MHZ , 1);

    TFT180_RES_LEVEL(0);
    TFT180_DELAY_MS(100);
    TFT180_RES_LEVEL(1);
    TFT180_DELAY_MS(100);
    Write_Cmd_TFT180(0x11);
    TFT180_DELAY_MS(100);

    Write_Cmd_TFT180(0x3A);
    Write_8bit_TFT180(0x55);
    Write_Cmd_TFT180(0x26);
    Write_8bit_TFT180(0x04);
    Write_Cmd_TFT180(0xF2);
    Write_8bit_TFT180(0x01);
    Write_Cmd_TFT180(0xE0);
    Write_8bit_TFT180(0x3F);
    Write_8bit_TFT180(0x25);
    Write_8bit_TFT180(0x1c);
    Write_8bit_TFT180(0x1e);
    Write_8bit_TFT180(0x20);
    Write_8bit_TFT180(0x12);
    Write_8bit_TFT180(0x2a);
    Write_8bit_TFT180(0x90);
    Write_8bit_TFT180(0x24);
    Write_8bit_TFT180(0x11);
    Write_8bit_TFT180(0x00);
    Write_8bit_TFT180(0x00);
    Write_8bit_TFT180(0x00);
    Write_8bit_TFT180(0x00);
    Write_8bit_TFT180(0x00);
    Write_Cmd_TFT180(0xe1);
    Write_8bit_TFT180(0x20);
    Write_8bit_TFT180(0x20);
    Write_8bit_TFT180(0x20);
    Write_8bit_TFT180(0x20);
    Write_8bit_TFT180(0x05);
    Write_8bit_TFT180(0x00);
    Write_8bit_TFT180(0x15);
    Write_8bit_TFT180(0xa7);
    Write_8bit_TFT180(0x3d);
    Write_8bit_TFT180(0x18);
    Write_8bit_TFT180(0x25);
    Write_8bit_TFT180(0x2a);
    Write_8bit_TFT180(0x2b);
    Write_8bit_TFT180(0x2b);
    Write_8bit_TFT180(0x3a);
    Write_Cmd_TFT180(0xb1);
    Write_8bit_TFT180(0x00);
    Write_8bit_TFT180(0x00);
    Write_Cmd_TFT180(0xb4);
    Write_8bit_TFT180(0x07);
    Write_Cmd_TFT180(0xc0);
    Write_8bit_TFT180(0x0a);
    Write_8bit_TFT180(0x02);
    Write_Cmd_TFT180(0xc1);
    Write_8bit_TFT180(0x02);
    Write_Cmd_TFT180(0xc5);
    Write_8bit_TFT180(0x4f);
    Write_8bit_TFT180(0x5a);
    Write_Cmd_TFT180(0xc7);
    Write_8bit_TFT180(0x40);
    Write_Cmd_TFT180(0x2a);
    Write_8bit_TFT180(0x00);
    Write_8bit_TFT180(0x00);
    Write_8bit_TFT180(0x00);
    Write_8bit_TFT180(0xa8);
    Write_Cmd_TFT180(0x2b);
    Write_8bit_TFT180(0x00);
    Write_8bit_TFT180(0x00);
    Write_8bit_TFT180(0x00);
    Write_8bit_TFT180(0xb3);
    Write_Cmd_TFT180(0x36);
    switch(TFT180_SHOW_DIR)
    {
    case 0:
        Write_8bit_TFT180(0xC0);
        break;
    case 1:
        Write_8bit_TFT180(0X00);
        break;
    case 2:
        Write_8bit_TFT180(0xA0);
        break;
    case 3:
        Write_8bit_TFT180(0x60);
        break;
    }
    Write_Cmd_TFT180(0xb7);
    Write_8bit_TFT180(0x00);
    Write_Cmd_TFT180(0x29);
    Write_Cmd_TFT180(0x2c);

    Clean_TFT180(WHITE);

    output_device_flag = 3;
}

/**
*
* @brief    TFT1.8寸屏幕设置起始和结束坐标
* @param    x1                x方向起始点
* @param    y1                y方向起始点
* @param    x2                x方向结束点
* @param    y2                y方向结束点
* @return   void
* @notes
* Example:  Set_Pos_TFT180(0,0,0,0);
*
**/
void Set_Pos_TFT180(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
    Write_Cmd_TFT180(0x2A);                      //Colulm addRSTs set
    Write_8bit_TFT180(0x00);                     //行起始坐标高8位，始终为零
    Write_8bit_TFT180((unsigned char)x1 + 2);    //行起始坐标低8位
    Write_8bit_TFT180(0x00);                     //行终止坐标高8位，始终为零
    Write_8bit_TFT180((unsigned char)x2 + 2);    //行终止坐标低8位
    Write_Cmd_TFT180(0x2B);                      //Colulm addRSTs set
    Write_8bit_TFT180(0x00);                     //列起始坐标高8位，始终为零
    Write_8bit_TFT180((unsigned char)y1 + 1);    //列起始坐标低8位
    Write_8bit_TFT180(0x00);                     //列终止坐标高8位，始终为零
    Write_8bit_TFT180((unsigned char)y2 + 1);    //列终止坐标低8位
    Write_Cmd_TFT180(0x2C);                      //GRAM接收MCU数据或命令
}

/**
*
* @brief    TFT1.8寸屏幕在指定位置画点
* @param    x                   x方向坐标
* @param    y                   y方向坐标
* @param    color               点的颜色(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Draw_Point_TFT180(0,0,0,0);
*
**/
void Draw_Point_TFT180(unsigned int x, unsigned int y, COLOR_enum color)
{
    Set_Pos_TFT180(x, y, x, y);
    Write_16bit_TFT180(color);
}

/**
*
* @brief    TFT1.8寸屏幕填充指定区域
* @param    x1                  x方向起始点
* @param    y1                  y方向起始点
* @param    x2                  x方向结束点
* @param    y2                  y方向结束点
* @param    color               填充颜色(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Fill_TFT180(0,0,128,160,WHITE);    //TFT18屏幕全白
*
**/
void Fill_TFT180(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, COLOR_enum color)
{
    unsigned int i, j;
    Set_Pos_TFT180(x1, y1, x2 - 1, y2 - 1); //设置显示范围
    for(i = y1; i < y2; i++)
    {
        for(j = x1; j < x2; j++)
        {
            Write_16bit_TFT180(color);
        }
    }
}

/**
*
* @brief    TFT1.8寸屏幕清屏
* @param    color               填充颜色(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Clean_TFT180(WHITE);   // TFT18屏幕全白
*
**/
void Clean_TFT180(COLOR_enum color)
{
    Fill_TFT180(0, 0, TFT180_WIDTH, TFT180_HEIGH, color);
}

/**
*
* @brief    TFT1.8寸屏幕显示单个字符
* @param    x                   x方向坐标
* @param    y                   y方向坐标
* @param    data                字符
* @param    fc                  字体颜色(可在dmx_font.h文件里查看)
* @param    bc                  背景颜色(可在dmx_font.h文件里查看)
* @param    fontsize            字体尺寸(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Show_Char_TFT180(0,0,'D',BLACK,WHITE,Show8x16);  // 在左上角显示白色背景黑色的8X16字符'D'
*
**/
void Show_Char_TFT180(unsigned int x, unsigned int y, unsigned char data, COLOR_enum fc, COLOR_enum bc, SHOW_size_enum fontsize)
{
    unsigned char i, j, temp_start, temp_end;
    switch(fontsize)
    {
    case Show6x8:
        for(i = 0; i < 6; i ++)
        {
            Set_Pos_TFT180(x + i, y, x + i, y + 8);
            temp_start = Char6x8[data - 32][i];
            for(j = 0; j < 8; j ++)
            {
                if(temp_start & 0x01)
                {
                    Write_16bit_TFT180(fc);
                }
                else
                {
                    Write_16bit_TFT180(bc);
                }
                temp_start >>= 1;
            }
        }
        break;
    case Show8x16:
        for(i = 0; i < 8; i ++)
        {
            Set_Pos_TFT180(x + i, y, x + i, y + 15);
            temp_start = Char8x16[data - 32][i];
            temp_end = Char8x16[data - 32][i + 8];
            for(j = 0; j < 8; j ++)
            {
                if(temp_start & 0x01)
                {
                    Write_16bit_TFT180(fc);
                }
                else
                {
                    Write_16bit_TFT180(bc);
                }
                temp_start >>= 1;
            }
            for(j = 0; j < 8; j ++)
            {
                if(temp_end & 0x01)
                {
                    Write_16bit_TFT180(fc);
                }
                else
                {
                    Write_16bit_TFT180(bc);
                }
                temp_end >>= 1;
            }
        }
        break;
    }
}

/**
*
* @brief    TFT1.8寸屏幕显示字符串
* @param    x                   x方向坐标
* @param    y                   y方向坐标
* @param    *p                  要显示的字符串
* @param    fc                  字体颜色(可在dmx_font.h文件里查看)
* @param    bc                  背景颜色(可在dmx_font.h文件里查看)
* @param    fontsize            字体尺寸(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Show_String_TFT180(0,0,"DMX",BLACK,WHITE,Show8x16);   // 在左上角显示白色背景黑色的8X16字符串'DMX'
*
**/
void Show_String_TFT180(unsigned int x, unsigned int y, char *p, COLOR_enum fc, COLOR_enum bc, SHOW_size_enum fontsize)
{
    while(*p != '\0')
    {
        Show_Char_TFT180(x, y, *p, fc, bc, fontsize);
        if(fontsize == Show6x8)
            x += 6;
        else if(fontsize == Show8x16)
            x += 8;
        p++;
    }
}

/**
*
* @brief    TFT1.8寸屏幕显示int型变量
* @param    x                   x方向坐标
* @param    y                   y方向坐标
* @param    data                要显示的int型变量
* @param    num                 要显示的int型变量位数
* @param    fc                  字体颜色(可在dmx_font.h文件里查看)
* @param    bc                  背景颜色(可在dmx_font.h文件里查看)
* @param    fontsize            字体尺寸(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Show_Int_TFT180(0,0,521,3,BLACK,WHITE,Show8x16);     // 在左上角显示白色背景黑色的8X16数字“521”
*
**/
void Show_Int_TFT180(unsigned short x, unsigned short y, const int data, unsigned char num, COLOR_enum fc, COLOR_enum bc, SHOW_size_enum fontsize)
{
    int multiple = 1;
    int remainder = data;
    char buff[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};

    if(num > 10)
        num = 10;
    if(num < 1)
        num = 1;

    buff[num + 1] = '\0';

    if(num<10)
    {
        while(num > 0)
        {
            num --;
            multiple *= 10;
        }
        remainder %= multiple;
    }

    int_to_str_func(remainder,buff);
    Show_String_TFT180(x, y, buff, fc, bc, fontsize);
}

/**
*
* @brief    TFT1.8寸屏幕显示float型变量
* @param    x                   x方向坐标
* @param    y                   y方向坐标
* @param    data                要显示的float型变量
* @param    num                 要显示的float型变量整数位数
* @param    pointnum            要显示的float型变量小数点位数
* @param    fc                  字体颜色(可在dmx_font.h文件里查看)
* @param    bc                  背景颜色(可在dmx_font.h文件里查看)
* @param    fontsize            字体尺寸(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Show_Float_TFT180(0,0,521.22,3,2,BLACK,WHITE,Show8x16);  // 在左上角显示白色背景黑色的8X16数字“521.22”
*
**/
void Show_Float_TFT180(unsigned short x, unsigned short y, const double data, unsigned char num, unsigned char pointnum, COLOR_enum fc, COLOR_enum bc, SHOW_size_enum fontsize)
{
    double multiple = 1;
    double remainder = data;
    char buff[17] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};

    if(num > 8)
        num = 8;
    if(num < 1)
        num = 1;
    if(pointnum > 6)
        pointnum = 6;
    if(pointnum < 1)
        pointnum = 1;

    buff[num + pointnum + 2] = '\0';

    while(num > 0)
    {
        num --;
        multiple *= 10;
    }
    remainder = remainder - ((int)remainder/(int)multiple)* multiple;

    float_to_str_func(remainder,pointnum,buff);
    Show_String_TFT180(x, y, buff, fc, bc, fontsize);
}

/**
*
* @brief    TFT1.8寸屏幕显示汉字
* @param    x                   x方向坐标
* @param    y                   y方向坐标
* @param    data                要显示的汉字所在数组
* @param    startnumber         第startnumber个汉字开始
* @param    number              第number个汉字结束
* @param    fc                  字体颜色(可在dmx_font.h文件里查看)
* @param    bc                  背景颜色(可在dmx_font.h文件里查看)
* @param    size                16
* @return   void
* @notes    使用PCtoLCD2002软件取模,阴码,逐行式,顺向,16*16
* Example:  Show_Chinese16x16_TFT180(0,0,Chinese16x16[0],0,3,BLACK,WHITE,16);    //在左上角显示Chinese16x16数组中的第0~3个汉字即“呆萌侠”
*
**/
void Show_Chinese16x16_TFT180(unsigned int x, unsigned int y, const unsigned char *data, unsigned char startnumber, unsigned char number, COLOR_enum fc, COLOR_enum bc, unsigned char size)
{
    int i, j, k;
    unsigned char temp;
    const unsigned char *pdata;

    Set_Pos_TFT180(x, y, (number - startnumber) * size - 1 + x, y + size - 1);

    for(i = 0; i < size; i ++)
    {
        temp = number;
        pdata = data + startnumber * 32 + i * (size / 8);
        for(; temp > startnumber; temp--)
        {
            for(k = 0; k < (size / 8); k ++)
            {
                for(j = 8; j > 0; j --)
                {
                    if((*pdata >> (j - 1)) & 0x01)
                    {
                        Write_16bit_TFT180(fc);
                    }
                    else
                    {
                        Write_16bit_TFT180(bc);
                    }
                }
                pdata ++;
            }
            pdata = pdata - (size / 8) + (size / 8) * size;
        }
    }
}

/**
*
* @brief    TFT1.8寸屏幕显示彩色图片
* @param    x                   x方向坐标
* @param    y                   y方向坐标
* @param    width               图片宽度
* @param    heigh               图片高度
* @param    pic                 图片数组
* @return   void
* @notes
* Example:  Show_Picture_TFT180(3,19,121,121,Earth121x121);  //在屏幕中间显示地球图片
*
**/
void Show_Picture_TFT180(unsigned int x, unsigned int y, unsigned int width, unsigned int heigh, const unsigned char pic[])
{
    unsigned int i, j;
    unsigned long k = 0;
    Set_Pos_TFT180(x, y, x + width - 1, y + heigh - 1);
    for(i = 0; i < width; i++)
    {
        for(j = 0; j < heigh; j++)
        {
            TFT180_Write_8bitData(pic[k * 2]);
            TFT180_Write_8bitData(pic[k * 2 + 1]);
            k++;
        }
    }
}

/**
*
* @brief    TFT1.8寸屏幕显示摄像头灰度图像
* @param    x                   x方向坐标
* @param    y                   y方向坐标
* @param    image               图像数组
* @param    width               图像数组实际宽度
* @param    heigh               图像数组实际高度
* @param    show_width           图像数组显示宽度
* @param    show_height          图像数组显示高度
* @param    threshold           二值化阈值为0时关闭二值化
* @return   void
* @notes
* Example:  Show_Image_TFT180(0,0,MT9V034_Image_Data[0],188,120,128,90,0); // 屏幕显示摄像头灰度图像
*
**/
void Show_Image_TFT180(unsigned short x, unsigned short y, unsigned char *image, unsigned short width, unsigned short height, unsigned short show_width, unsigned short show_height, unsigned char threshold)
{
    unsigned int i, j;
    unsigned short color, temp;
    unsigned int width_ratio, height_ratio;

    Set_Pos_TFT180(x, y, x + show_width - 1, y + show_height - 1);

    for(j = 0; j < show_height; j ++)
    {
        height_ratio = j * height / show_height;
        for(i = 0; i < show_width; i ++)
        {
            width_ratio = i * width / show_width;
            temp = *(image + height_ratio * width + width_ratio);
            if(threshold == 0)
            {
                color = (0x001f & ((temp) >> 3)) << 11;
                color = color | (((0x003f) & ((temp) >> 2)) << 5);
                color = color | (0x001f & ((temp) >> 3));
                Write_16bit_TFT180(color);
            }
            else if(temp < threshold)
            {
                Write_16bit_TFT180(BLACK);
            }
            else
            {
                Write_16bit_TFT180(WHITE);
            }

        }
    }
}

/**
*
* @brief    TFT1.8寸屏幕写命令
* @param    cmd             命令
* @return   void
* @notes
* Example:  Write_Cmd_TFT180(0x21);
*
**/
static void Write_Cmd_TFT180(unsigned char cmd)
{
    TFT180_DC_LEVEL(0);
    TFT180_Write_8bitData(cmd);
    TFT180_DC_LEVEL(1);
}

/**
*
* @brief    TFT1.8寸屏幕写8bit数据
* @param    data                8bit数据
* @return   void
* @notes
* Example:  Write_8bit_TFT180(0x23);
*
**/
static void Write_8bit_TFT180(unsigned char data)
{
    TFT180_DC_LEVEL(1);
    TFT180_Write_8bitData(data);
    TFT180_DC_LEVEL(0);
}

/**
*
* @brief    TFT1.8寸屏幕写16bit数据
* @param    data                16bit数据
* @return   void
* @notes
* Example:  Write_16bit_TFT180(0x0000);
*
**/
static void Write_16bit_TFT180(unsigned short data)
{
    TFT180_DC_LEVEL(1);
    TFT180_Write_16bitData(data);
    TFT180_DC_LEVEL(0);
}

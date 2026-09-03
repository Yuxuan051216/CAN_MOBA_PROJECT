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
 * @file       dmx_ips.c
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

#include "dmx_ips.h"
#include "dmx_gpio.h"
#include "dmx_delay.h"
#include "dmx_hard_spi.h"
#include "dmx_soft_spi.h"
#include "dmx_function.h"

// SPI协议读写操作宏定义
#if IPS_HARD_SPI
#define IPS_Write_8bitData(data)    (write_8bit_hard_spi(IPS_SPI, (data)))
#define IPS_Write_16bitData(data)   (write_16bit_hard_spi(IPS_SPI, (data)))
#else
static  SOFT_SPI_struct             IPS_SPI;
#define IPS_Write_8bitData(data)    (write_8bitdata_soft_spi(&IPS_SPI, (data)))
#define IPS_Write_16bitData(data)   (write_16bitdata_soft_spi(&IPS_SPI, (data)))
#endif

// 静态函数声明,以下函数均为该.c文件内部调用
static void Write_Cmd_IPS(unsigned char cmd);
static void Write_8bit_IPS(unsigned char data);
static void Write_16bit_IPS(unsigned short data);
// IPS屏幕宽和高,用户无需修改,在dmx_ips.h里选择正确的IPS屏幕型号后初始化会自动调整宽高
static unsigned short ips_width, ips_height;

/**
*
* @brief    初始化IPS屏幕
* @param    void
* @return   void
* @notes    修改管脚可在dmx_ips.h文件里的宏定义管脚进行修改
* Example:  Init_IPS();
*
**/
void Init_IPS(void)
{
    // SPI初始化
#if IPS_HARD_SPI
    init_hard_spi(IPS_SPI, SPI_MODE0, IPS_SPI_SPEED, IPS_SCL_PIN, IPS_SDA_PIN, 0xff);
#else
    init_soft_spi(&IPS_SPI, 0, IPS_SCL_PIN, IPS_SDA_PIN, 0xff);
#endif

    init_gpio( IPS_DC_PIN  , GPO_PP , Speed_50MHZ , 0);
    init_gpio( IPS_RES_PIN , GPO_PP , Speed_50MHZ , 0);
    init_gpio( IPS_CS_PIN  , GPO_PP , Speed_50MHZ , 1);
    init_gpio( IPS_BLK_PIN , GPO_PP , Speed_50MHZ , 1);

    // 根据屏幕型号自适应宽高
    if(IPS_MODEL == 0)
    {
        if(IPS_SHOW_DIR == 0 || IPS_SHOW_DIR == 1)
        {
            ips_width = 135;
            ips_height = 240;
        }
        else
        {
            ips_width = 240;
            ips_height = 135;
        }
    }
    else if(IPS_MODEL == 1)
    {
        ips_width = 240;
        ips_height = 240;
    }
    else if(IPS_MODEL == 2)
    {
        ips_width = 240;
        ips_height = 240;
    }
    else if(IPS_MODEL == 3)
    {
        if(IPS_SHOW_DIR == 0 || IPS_SHOW_DIR == 1)
        {
            ips_width = 240;
            ips_height = 320;
        }
        else
        {
            ips_width = 320;
            ips_height = 240;
        }
    }

    IPS_RES_LEVEL(0);
    IPS_DELAY_MS(100);
    IPS_RES_LEVEL(1);
    IPS_DELAY_MS(100);

    IPS_CS_LEVEL(0);
    IPS_DELAY_MS(150);
    Write_Cmd_IPS(0x11);
    IPS_DELAY_MS(150);
    Write_Cmd_IPS(0x36);

    if(IPS_SHOW_DIR == 0)
        Write_8bit_IPS(0x00);
    else if(IPS_SHOW_DIR == 1)
        Write_8bit_IPS(0xC0);
    else if(IPS_SHOW_DIR == 2)
        Write_8bit_IPS(0x70);
    else if(IPS_SHOW_DIR == 3)
        Write_8bit_IPS(0xA0);

    Write_Cmd_IPS(0x3A);
    Write_8bit_IPS(0x05);
    Write_Cmd_IPS(0xB2);
    Write_8bit_IPS(0x0C);
    Write_8bit_IPS(0x0C);
    Write_8bit_IPS(0x00);
    Write_8bit_IPS(0x33);
    Write_8bit_IPS(0x33);
    Write_Cmd_IPS(0xB7);
    Write_8bit_IPS(0x35);
    Write_Cmd_IPS(0xBB);
    Write_8bit_IPS(0x19);
    Write_Cmd_IPS(0xC0);
    Write_8bit_IPS(0x2C);
    Write_Cmd_IPS(0xC2);
    Write_8bit_IPS(0x01);
    Write_Cmd_IPS(0xC3);
    Write_8bit_IPS(0x12);
    Write_Cmd_IPS(0xC4);
    Write_8bit_IPS(0x20);
    Write_Cmd_IPS(0xC6);
    Write_8bit_IPS(0x0F);
    Write_Cmd_IPS(0xD0);
    Write_8bit_IPS(0xA4);
    Write_8bit_IPS(0xA1);
    Write_Cmd_IPS(0xE0);
    Write_8bit_IPS(0xD0);
    Write_8bit_IPS(0x04);
    Write_8bit_IPS(0x0D);
    Write_8bit_IPS(0x11);
    Write_8bit_IPS(0x13);
    Write_8bit_IPS(0x2B);
    Write_8bit_IPS(0x3F);
    Write_8bit_IPS(0x54);
    Write_8bit_IPS(0x4C);
    Write_8bit_IPS(0x18);
    Write_8bit_IPS(0x0D);
    Write_8bit_IPS(0x0B);
    Write_8bit_IPS(0x1F);
    Write_8bit_IPS(0x23);
    Write_Cmd_IPS(0xE1);
    Write_8bit_IPS(0xD0);
    Write_8bit_IPS(0x04);
    Write_8bit_IPS(0x0C);
    Write_8bit_IPS(0x11);
    Write_8bit_IPS(0x13);
    Write_8bit_IPS(0x2C);
    Write_8bit_IPS(0x3F);
    Write_8bit_IPS(0x44);
    Write_8bit_IPS(0x51);
    Write_8bit_IPS(0x2F);
    Write_8bit_IPS(0x1F);
    Write_8bit_IPS(0x1F);
    Write_8bit_IPS(0x20);
    Write_8bit_IPS(0x23);
    Write_Cmd_IPS(0x21);
    Write_Cmd_IPS(0x29);

    Clean_IPS(WHITE);

    output_device_flag = 2;
}

/**
*
* @brief    IPS屏幕设置起始和结束坐标
* @param    x1                x方向起始点(IPS1.14寸横屏模式0~240)
* @param    y1                y方向起始点(IPS1.14寸横屏模式0~135)
* @param    x2                x方向结束点(IPS1.14寸横屏模式0~240)
* @param    y2                y方向结束点(IPS1.14寸横屏模式0~135)
* @return   void
* @notes
* Example:  Set_Pos_IPS(0,0,0,0);
*
**/
void Set_Pos_IPS(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
    int offset_x = 0, offset_y = 0;
    if(IPS_SHOW_DIR == 0)
    {
        if(IPS_MODEL == 0)
        {
            offset_x = 52;
            offset_y = 40;
        }
        else
        {
            offset_x = 0;
            offset_y = 0;
        }
    }
    else if(IPS_SHOW_DIR == 1)
    {
        if(IPS_MODEL == 0)
        {
            offset_x = 53;
            offset_y = 40;
        }
        else if(IPS_MODEL == 1 || IPS_MODEL == 2)
        {
            offset_x = 0;
            offset_y = 80;
        }
        else
        {
            offset_x = 0;
            offset_y = 0;
        }
    }
    else if(IPS_SHOW_DIR == 2)
    {
        if(IPS_MODEL == 0)
        {
            offset_x = 40;
            offset_y = 53;
        }
        else
        {
            offset_x = 0;
            offset_y = 0;
        }
    }
    else if(IPS_SHOW_DIR == 3)
    {
        if(IPS_MODEL == 0)
        {
            offset_x = 40;
            offset_y = 52;
        }
        else if(IPS_MODEL == 1 || IPS_MODEL == 2)
        {
            offset_x = 80;
            offset_y = 0;
        }
        else
        {
            offset_x = 0;
            offset_y = 0;
        }
    }
    Write_Cmd_IPS(0x2A);    // 列地址设置
    Write_16bit_IPS(x1 + offset_x);
    Write_16bit_IPS(x2 + offset_x);
    Write_Cmd_IPS(0x2B);    // 行地址设置
    Write_16bit_IPS(y1 + offset_y);
    Write_16bit_IPS(y2 + offset_y);
    Write_Cmd_IPS(0x2C);    // 储存器写
}

/**
*
* @brief    IPS屏幕在指定位置画点
* @param    x                   x方向坐标(IPS1.14寸横屏模式0~240)
* @param    y                   y方向坐标(IPS1.14寸横屏模式0~135)
* @param    color               点的颜色(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Draw_Point_IPS(0,0,BLACK);
*
**/
void Draw_Point_IPS(unsigned int x, unsigned int y, COLOR_enum color)
{
    Set_Pos_IPS(x, y, x, y);
    Write_16bit_IPS(color);
}

/**
*
* @brief    IPS屏幕填充指定区域
* @param    x1                  x方向起始点(IPS1.14寸横屏模式0~240)
* @param    y1                  y方向起始点(IPS1.14寸横屏模式0~135)
* @param    x2                  x方向结束点(IPS1.14寸横屏模式0~240)
* @param    y2                  y方向结束点(IPS1.14寸横屏模式0~135)
* @param    color               填充颜色(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Fill_IPS(0,0,240,135,WHITE);    // IPS114屏幕全白
*
**/
void Fill_IPS(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, COLOR_enum color)
{
    unsigned int i, j;
    Set_Pos_IPS(x1, y1, x2 - 1, y2 - 1);
    for(i = y1; i < y2; i++)
    {
        for(j = x1; j < x2; j++)
        {
            Write_16bit_IPS(color);
        }
    }
}

/**
*
* @brief    IPS屏幕清屏
* @param    color               填充颜色(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Clean_IPS(WHITE);   // IPS114屏幕全白
*
**/
void Clean_IPS(COLOR_enum color)
{
    Fill_IPS(0, 0, ips_width, ips_height, color);
}

/**
*
* @brief    IPS屏幕显示单个字符
* @param    x                   x方向坐标(IPS1.14寸横屏模式0~240)
* @param    y                   y方向坐标(IPS1.14寸横屏模式0~135)
* @param    data                字符
* @param    fc                  字体颜色(可在dmx_font.h文件里查看)
* @param    bc                  背景颜色(可在dmx_font.h文件里查看)
* @param    fontsize            字体尺寸(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Show_Char_IPS(0,0,'D',BLACK,WHITE,Show8x16);    // 在左上角显示白色背景黑色的8x16字符'D'
*
**/
void Show_Char_IPS(unsigned int x, unsigned int y, unsigned char data, COLOR_enum fc, COLOR_enum bc, SHOW_size_enum fontsize)
{
    unsigned char i, j, temp_start, temp_end;
    switch(fontsize)
    {
    case Show6x8:
        for(i = 0; i < 6; i ++)
        {
            Set_Pos_IPS(x + i, y, x + i, y + 8);
            temp_start = Char6x8[data - 32][i];
            for(j = 0; j < 8; j ++)
            {
                if(temp_start & 0x01)
                {
                    Write_16bit_IPS(fc);
                }
                else
                {
                    Write_16bit_IPS(bc);
                }
                temp_start >>= 1;
            }
        }
        break;
    case Show8x16:
        for(i = 0; i < 8; i ++)
        {
            Set_Pos_IPS(x + i, y, x + i, y + 15);
            temp_start = Char8x16[data - 32][i];
            temp_end = Char8x16[data - 32][i + 8];
            for(j = 0; j < 8; j ++)
            {
                if(temp_start & 0x01)
                {
                    Write_16bit_IPS(fc);
                }
                else
                {
                    Write_16bit_IPS(bc);
                }
                temp_start >>= 1;
            }
            for(j = 0; j < 8; j ++)
            {
                if(temp_end & 0x01)
                {
                    Write_16bit_IPS(fc);
                }
                else
                {
                    Write_16bit_IPS(bc);
                }
                temp_end >>= 1;
            }
        }
        break;
    }
}

/**
*
* @brief    IPS屏幕显示字符串
* @param    x                   x方向坐标(IPS1.14寸横屏模式0~240)
* @param    y                   y方向坐标(IPS1.14寸横屏模式0~135)
* @param    *p                  要显示的字符串
* @param    fc                  字体颜色(可在dmx_font.h文件里查看)
* @param    bc                  背景颜色(可在dmx_font.h文件里查看)
* @param    fontsize            字体尺寸(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Show_String_IPS(0,0,"DMX",BLACK,WHITE,Show8x16);    // 在左上角显示白色背景黑色的8x16字符串'DMX'
*
**/
void Show_String_IPS(unsigned int x, unsigned int y, char *p, COLOR_enum fc, COLOR_enum bc, SHOW_size_enum fontsize)
{
    while(*p != '\0')
    {
        Show_Char_IPS(x, y, *p, fc, bc, fontsize);
        if(fontsize == Show6x8)
            x += 6;
        else if(fontsize == Show8x16)
            x += 8;
        p++;
    }
}

/**
*
* @brief    IPS屏幕显示Int型变量
* @param    x                   x方向坐标(IPS1.14寸横屏模式0~240)
* @param    y                   y方向坐标(IPS1.14寸横屏模式0~135)
* @param    data                要显示的int型变量
* @param    num                 要显示的int型变量位数
* @param    fc                  字体颜色(可在dmx_font.h文件里查看)
* @param    bc                  背景颜色(可在dmx_font.h文件里查看)
* @param    fontsize            字体尺寸(可在dmx_font.h文件里查看)
* @return   void
* @notes    "data = 1234,num = 3"则屏幕显示为"234"
* Example:  Show_Int_IPS(0,0,521,3,BLACK,WHITE,Show8x16);   // 在左上角显示白色背景黑色字体的8x16数字“521”
*
**/
void Show_Int_IPS (unsigned short x, unsigned short y, const int data, unsigned char num, COLOR_enum fc, COLOR_enum bc, SHOW_size_enum fontsize)
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
    Show_String_IPS(x, y, buff, fc, bc, fontsize);
}

/**
*
* @brief    IPS屏幕显示float型变量
* @param    x                   x方向坐标(IPS1.14寸横屏模式0~240)
* @param    y                   y方向坐标(IPS1.14寸横屏模式0~135)
* @param    data                要显示的float型变量
* @param    num                 要显示的float型变量整数位数
* @param    pointnum            要显示的float型变量小数点位数
* @param    fc                  字体颜色(可在dmx_font.h文件里查看)
* @param    bc                  背景颜色(可在dmx_font.h文件里查看)
* @param    fontsize            字体尺寸(可在dmx_font.h文件里查看)
* @return   void
* @notes
* Example:  Show_Float_IPS(0,0,521.22,3,2,BLACK,WHITE,Show8x16);    // 在左上角显示白色背景黑色字体的8x16数字“521.22”
*
**/
void Show_Float_IPS (unsigned short x, unsigned short y, const double data, unsigned char num, unsigned char pointnum, COLOR_enum fc, COLOR_enum bc, SHOW_size_enum fontsize)
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
    Show_String_IPS(x, y, buff, fc, bc, fontsize);
}

/**
*
* @brief    IPS屏幕显示汉字
* @param    x                   x方向坐标(IPS1.14寸横屏模式0~240)
* @param    y                   y方向坐标(IPS1.14寸横屏模式0~135)
* @param    data                要显示的汉字所在数组
* @param    startnumber         第startnumber个汉字开始
* @param    number              第startnumber+number个汉字结束
* @param    fc                  字体颜色(可在dmx_font.h文件里查看)
* @param    bc                  背景颜色(可在dmx_font.h文件里查看)
* @param    size                16
* @return   void
* @notes    使用PCtoLCD2002软件取模,阴码,逐行式,顺向,16*16
* Example:  Show_Chinese16x16_IPS(0,0,Chinese16x16[0],0,3,BLACK,WHITE,16);   // 在左上角显示Chinese16x16数组中的第0~3个汉字即“呆萌侠”
*
**/
void Show_Chinese16x16_IPS(unsigned int x, unsigned int y, const unsigned char *data, unsigned char startnumber, unsigned char number, COLOR_enum fc, COLOR_enum bc, unsigned char size)
{
    int i, j, k;
    const unsigned char *pdata;
    unsigned char temp;

    Set_Pos_IPS(x, y, (number - startnumber) * size - 1 + x, y + size - 1);

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
                        Write_16bit_IPS(fc);
                    }
                    else
                    {
                        Write_16bit_IPS(bc);
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
* @brief    IPS屏幕显示彩色图片
* @param    x                   x方向坐标(IPS1.14寸横屏模式0~240)
* @param    y                   y方向坐标(IPS1.14寸横屏模式0~135)
* @param    width               图片宽度
* @param    heigh               图片高度
* @param    pic                 图片数组
* @return   void
* @notes
* Example:  Show_Picture_IPS(60,7,121,121,Earth121x121); // 在屏幕中间显示地球图片
*
**/
void Show_Picture_IPS(unsigned int x, unsigned int y, unsigned int width, unsigned int heigh, const unsigned char pic[])
{
    unsigned int i, j;
    unsigned long k = 0;
    Set_Pos_IPS(x, y, x + width - 1, y + heigh - 1);
    for(i = 0; i < width; i++)
    {
        for(j = 0; j < heigh; j++)
        {
            Write_8bit_IPS(pic[k * 2]);
            Write_8bit_IPS(pic[k * 2 + 1]);
            k++;
        }
    }
}

/**
*
* @brief    IPS屏幕显示摄像头灰度图像
* @param    x                   x方向坐标(IPS1.14寸横屏模式0~240)
* @param    y                   y方向坐标(IPS1.14寸横屏模式0~135)
* @param    image               图像数组
* @param    width               图像数组实际宽度
* @param    heigh               图像数组实际高度
* @param    show_width          图像数组显示宽度
* @param    show_height         图像数组显示高度
* @param    threshold           为0时关闭二值化
* @return   void
* @notes
* Example:  Show_Image_IPS(0,0,MT9V034_Image_Data[0],188,120,188,120,0);
*
**/
void Show_Image_IPS(unsigned short x, unsigned short y, unsigned char *image, unsigned short width, unsigned short height, unsigned short show_width, unsigned short show_height, unsigned char threshold)
{
    unsigned int i, j;
    unsigned short color, temp;
    unsigned int width_ratio, height_ratio;

    Set_Pos_IPS(x, y, x + show_width - 1, y + show_height - 1);

    for(i = 0; i < show_height; i ++)
    {
        height_ratio = i * height / show_height;
        for(j = 0; j < show_width; j ++)
        {
            width_ratio = j * width / show_width;
            temp = *(image + height_ratio * width + width_ratio);

            if(threshold == 0)
            {
                color = (0x001f & ((temp) >> 3)) << 11;
                color = color | (((0x003f) & ((temp) >> 2)) << 5);
                color = color | (0x001f & ((temp) >> 3));
                Write_16bit_IPS(color);
            }
            else if(temp < threshold)
            {
                Write_16bit_IPS(BLACK);
            }
            else
            {
                Write_16bit_IPS(WHITE);
            }
        }
    }
}

/**
*
* @brief    IPS屏幕显示摄像头二值化图像
* @param    x                   x方向坐标(IPS1.14寸横屏模式0~240)
* @param    y                   y方向坐标(IPS1.14寸横屏模式0~135)
* @param    image               图像数组
* @param    width               图像数组实际宽度
* @param    heigh               图像数组实际高度
* @param    show_width          图像数组显示宽度
* @param    show_height         图像数组显示高度
* @return   void
* @notes
* Example:  Show_Image_Binary_IPS(0,0,Deal_Image_Binary[0],80,60,160,120);
*
**/
void Show_Image_Binary_IPS(unsigned short x, unsigned short y, unsigned char *image, unsigned short width, unsigned short height, unsigned short show_width, unsigned short show_height)
{
    unsigned int i, j;
    unsigned short temp;
    unsigned int width_ratio, height_ratio;

    Set_Pos_IPS(x, y, x + show_width - 1, y + show_height - 1);

    for(i = 0; i < show_height; i ++)
    {
        height_ratio = i * height / show_height;
        for(j = 0; j < show_width; j ++)
        {
            width_ratio = j * width / show_width;
            temp = *(image + height_ratio * width + width_ratio);

            switch(temp)
            {
            case 0:
                Write_16bit_IPS(BLACK);
                break;
            case 1:
                Write_16bit_IPS(WHITE);
                break;
            case 2:
                Write_16bit_IPS(BLUE);
                break;
            case 3:
                Write_16bit_IPS(RED);
                break;
            case 4:
                Write_16bit_IPS(GREEN);
                break;
            }
        }
    }
}

/**
*
* @brief    IPS屏幕写命令
* @param    cmd             命令
* @return   void
* @notes    IPS.c文件内部调用,用户无需调用尝试
* Example:  Write_Cmd_IPS(0x21);
*
**/
static void Write_Cmd_IPS(unsigned char cmd)
{
    IPS_DC_LEVEL(0);
    IPS_Write_8bitData(cmd);
}

/**
*
* @brief    IPS屏幕写8bit数据
* @param    data            8bit数据
* @return   void
* @notes    IPS.c文件内部调用,用户无需调用尝试
* Example:  Write_8bit_IPS(0x23);
*
**/
static void Write_8bit_IPS(unsigned char data)
{
    IPS_DC_LEVEL(1);
    IPS_Write_8bitData(data);
}

/**
*
* @brief    IPS屏幕写16bit数据
* @param    data            16bit数据
* @return   void
* @notes    IPS.c文件内部调用,用户无需调用尝试
* Example:  Write_16bit_IPS(0x0000);
*
**/
static void Write_16bit_IPS(unsigned short data)
{
    IPS_DC_LEVEL(1);
    IPS_Write_16bitData(data);
}

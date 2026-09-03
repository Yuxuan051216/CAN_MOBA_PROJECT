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
 * @file       dmx_oled.c
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

#include "dmx_oled.h"
#include "dmx_delay.h"
#include "dmx_soft_iic.h"
#include "dmx_function.h"

#define OLED_CMD_CTRL        0x00
#define OLED_DATA_CTRL       0x40

static SOFT_IIC_struct       OLED_IIC;

// 静态函数声明,以下函数均为该.c文件内部调用
static void Write_Cmd_OLED(unsigned char cmd);
static void Write_Data_OLED(unsigned char data);

/**
*
* @brief    0.96寸4脚OLED屏幕初始化
* @param    void
* @return   void
* @notes    修改管脚定义可在dmx_oled.h文件里的宏定义管脚进行修改
* Example:  Init_OLED();
*
**/
void Init_OLED(void)
{
    init_soft_iic(&OLED_IIC, OLED_IIC_DELAY, OLED_SCL_PIN, OLED_SDA_PIN);

    OLED_DELAY_MS(80);

    Write_Cmd_OLED(0xAE);
    Write_Cmd_OLED(0x00);
    Write_Cmd_OLED(0x10);
    Write_Cmd_OLED(0x40);
    Write_Cmd_OLED(0x81);
    Write_Cmd_OLED(0xCF);
    Write_Cmd_OLED(0xA1);
    Write_Cmd_OLED(0xC8);
    Write_Cmd_OLED(0xA6);
    Write_Cmd_OLED(0xA8);
    Write_Cmd_OLED(0x3F);
    Write_Cmd_OLED(0xD3);
    Write_Cmd_OLED(0x00);
    Write_Cmd_OLED(0xD5);
    Write_Cmd_OLED(0x80);
    Write_Cmd_OLED(0xD9);
    Write_Cmd_OLED(0xF1);
    Write_Cmd_OLED(0xDA);
    Write_Cmd_OLED(0x12);
    Write_Cmd_OLED(0xDB);
    Write_Cmd_OLED(0x40);
    Write_Cmd_OLED(0x20);
    Write_Cmd_OLED(0x02);
    Write_Cmd_OLED(0x8D);
    Write_Cmd_OLED(0x14);
    Write_Cmd_OLED(0xA4);
    Write_Cmd_OLED(0xA6);
    Write_Cmd_OLED(0xAF);
    Clean_OLED();
    Set_Pos_OLED(0, 0);

    output_device_flag = 1;
}

/**
*
* @brief    0.96寸4脚OLED屏幕画点
* @param    data
* @return   void
* @notes
* Example:  Draw_Point_OLED(0xff);
*
**/
void Draw_Point_OLED(unsigned char data)
{
    Write_Data_OLED(data);
}

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
void Set_Pos_OLED(unsigned char x, unsigned char y)
{
    Write_Cmd_OLED(0xb0 + y);
    Write_Cmd_OLED(((x & 0xF0) >> 4) | 0x10);
    Write_Cmd_OLED((x & 0x0F));
}

/**
*
* @brief    0.96寸4脚OLED屏幕点亮
* @param
* @return   void
* @notes
* Example:  Fill_OLED();
*
**/
void Fill_OLED(void)
{
    unsigned char y, x;
    for(y = 0; y < 8; y++)
    {
        Write_Cmd_OLED(0xB0 + y);
        Write_Cmd_OLED(0x01);
        Write_Cmd_OLED(0x10);
        for(x = 0; x < OLED_WIDTH; x++)
            Draw_Point_OLED(0xFF);
    }
}

/**
*
* @brief    0.96寸4脚OLED屏幕清屏
* @param
* @return   void
* @notes    执行后效果是全黑,黑屏
* Example:  Clean_OLED();
*
**/
void Clean_OLED(void)
{
    unsigned char y, x;
    for(y = 0; y < 8; y++)
    {
        Write_Cmd_OLED(0xB0 + y);
        Write_Cmd_OLED(0x01);
        Write_Cmd_OLED(0x10);
        for(x = 0; x < OLED_WIDTH; x++)
            Draw_Point_OLED(0);
    }
}

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
void Show_String_OLED(unsigned char x, unsigned char y, char str[], SHOW_size_enum fontsize)
{
    unsigned char c = 0, i = 0, j = 0;
    while (str[j] != '\0')
    {
        c = str[j] - 32;
        if(fontsize == Show6x8)
        {
            if(x > 126)
            {
                x = 0;
                y++;
            }
            Set_Pos_OLED(x, y);
            for(i = 0; i < 6; i++)
                Draw_Point_OLED(Char6x8[c][i]);
            x += 6;
        }
        else if(fontsize == Show8x16)
        {
            if(x > 120)
            {
                x = 0;
                y++;
            }
            Set_Pos_OLED(x, y);
            for(i = 0; i < 8; i++)
                Draw_Point_OLED(Char8x16[c][i]);
            Set_Pos_OLED(x, y + 1);
            for(i = 0; i < 8; i++)
                Draw_Point_OLED(Char8x16[c][i + 8]);
            x += 8;
        }
        j++;
    }
}

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
void Show_Int_OLED(char x, char y, const int data, unsigned char num, SHOW_size_enum fontsize)
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
    Show_String_OLED(x, y, buff, fontsize);
}

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
void Show_Float_OLED(char x, char y, const double data, unsigned char num, unsigned char pointnum, SHOW_size_enum fontsize)
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
    Show_String_OLED(x, y, buff, fontsize);
}

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
void Show_Chinese16x16_OLED(char x, char y, const unsigned char *chinese, char startnum, char endnum)
{
    unsigned int i = 0, j = 0, k = 0;

    for(j = startnum; j < endnum; j++)
    {
        Set_Pos_OLED(x + (j - startnum) * 16, y);
        for(i = 0; i < 16; i++)
        {
            Draw_Point_OLED(chinese[startnum * 16 * 2 + k]);
            k++;
        }
        Set_Pos_OLED(x + (j - startnum) * 16, y + 1);
        for(i = 0; i < 16; i++)
        {
            Draw_Point_OLED(chinese[startnum * 16 * 2 + k]);
            k++;
        }
    }
}

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
void Show_BMP_OLED(unsigned char x, unsigned char y, unsigned char width, unsigned char height, const unsigned char *bmp)
{
    unsigned int i = 0;
    unsigned char x0, y0, x1, y1;
    y1 = (y + height - 1) / 8;
    x1 = x + width;
    for(y0 = y / 8; y0 <= y1; y0++)
    {
        Set_Pos_OLED(x, y0);
        for(x0 = x; x0 < x1; x0++)
        {
            Draw_Point_OLED(bmp[i++]);
        }
    }
}

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
void Show_Image_Oled (unsigned short x, unsigned short y, const unsigned char *image, unsigned short width, unsigned short height, unsigned short show_width, unsigned short show_height, unsigned char threshold)
{
    short i, j;
    unsigned char dat;
    unsigned int width_ratio = 0, height_ratio = 0;

    show_height = show_height - show_height % 8;
    for(i = 0; i < show_height; i += 8)
    {
        Set_Pos_OLED(x + 0, y + i / 8);
        height_ratio = i * height / show_height;
        for(j = 0; j < show_width; j ++)
        {
            width_ratio = j * width / show_width;
            dat = 0;
            if(*(image + height_ratio * width + width_ratio + width * 0) > threshold)
                dat |= 0x01;
            if(*(image + height_ratio * width + width_ratio + width * 1) > threshold)
                dat |= 0x02;
            if(*(image + height_ratio * width + width_ratio + width * 2) > threshold)
                dat |= 0x04;
            if(*(image + height_ratio * width + width_ratio + width * 3) > threshold)
                dat |= 0x08;
            if(*(image + height_ratio * width + width_ratio + width * 4) > threshold)
                dat |= 0x10;
            if(*(image + height_ratio * width + width_ratio + width * 5) > threshold)
                dat |= 0x20;
            if(*(image + height_ratio * width + width_ratio + width * 6) > threshold)
                dat |= 0x40;
            if(*(image + height_ratio * width + width_ratio + width * 7) > threshold)
                dat |= 0x80;
            Draw_Point_OLED(dat);
        }
    }
}

/**
*
* @brief    0.96寸4脚OLED屏幕写命令
* @param    cmd         命令
* @return   void
* @notes    内部调用
* Example:  Write_Cmd_OLED(0x00);
*
**/
static void Write_Cmd_OLED(unsigned char cmd)
{
    write_data_soft_iic(&OLED_IIC, OLED_IIC_ADDR, OLED_CMD_CTRL, cmd);
}

static void Write_Data_OLED(unsigned char data)
{
    write_data_soft_iic(&OLED_IIC, OLED_IIC_ADDR, OLED_DATA_CTRL, data);
}

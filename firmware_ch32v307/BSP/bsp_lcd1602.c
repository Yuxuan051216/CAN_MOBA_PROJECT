#include "bsp_lcd1602.h"

#include <string.h>

#include "app_config.h"
#include "bsp_time.h"
#include "ch32v30x.h"

#if NODE_ID == NODE_ID_BOARD_A
#define LCD_BACKLIGHT           0x08U
#define LCD_ENABLE              0x04U
#define LCD_RS                  0x01U
#define LCD_CMD_CLEAR           0x01U
#define LCD_CMD_ENTRY_MODE      0x06U
#define LCD_CMD_DISPLAY_ON      0x0CU
#define LCD_CMD_FUNCTION_SET    0x28U

static uint8_t lcd_ready;

static void BSP_LCD1602_I2CDelay(void)
{
    volatile uint16_t i;

    for(i = 0U; i < 80U; i++)
    {
        __NOP();
    }
}

static void BSP_LCD1602_SetScl(uint8_t high)
{
    GPIO_WriteBit(LCD1602_SCL_PORT,
                  LCD1602_SCL_PIN,
                  high ? Bit_SET : Bit_RESET);
}

static void BSP_LCD1602_SetSda(uint8_t high)
{
    GPIO_WriteBit(LCD1602_SDA_PORT,
                  LCD1602_SDA_PIN,
                  high ? Bit_SET : Bit_RESET);
}

static void BSP_LCD1602_I2CStart(void)
{
    BSP_LCD1602_SetSda(1U);
    BSP_LCD1602_SetScl(1U);
    BSP_LCD1602_I2CDelay();
    BSP_LCD1602_SetSda(0U);
    BSP_LCD1602_I2CDelay();
    BSP_LCD1602_SetScl(0U);
}

static void BSP_LCD1602_I2CStop(void)
{
    BSP_LCD1602_SetSda(0U);
    BSP_LCD1602_SetScl(1U);
    BSP_LCD1602_I2CDelay();
    BSP_LCD1602_SetSda(1U);
    BSP_LCD1602_I2CDelay();
}

static void BSP_LCD1602_I2CWrite(uint8_t value)
{
    uint8_t mask;

    for(mask = 0x80U; mask != 0U; mask >>= 1)
    {
        BSP_LCD1602_SetSda((uint8_t)((value & mask) != 0U));
        BSP_LCD1602_I2CDelay();
        BSP_LCD1602_SetScl(1U);
        BSP_LCD1602_I2CDelay();
        BSP_LCD1602_SetScl(0U);
    }

    BSP_LCD1602_SetSda(1U);
    BSP_LCD1602_I2CDelay();
    BSP_LCD1602_SetScl(1U);
    BSP_LCD1602_I2CDelay();
    BSP_LCD1602_SetScl(0U);
}

static void BSP_LCD1602_WritePcf(uint8_t value)
{
    BSP_LCD1602_I2CStart();
    BSP_LCD1602_I2CWrite((uint8_t)(LCD1602_I2C_ADDRESS << 1));
    BSP_LCD1602_I2CWrite(value);
    BSP_LCD1602_I2CStop();
}

static void BSP_LCD1602_Write4(uint8_t value)
{
    BSP_LCD1602_WritePcf((uint8_t)(value | LCD_ENABLE | LCD_BACKLIGHT));
    BSP_LCD1602_I2CDelay();
    BSP_LCD1602_WritePcf((uint8_t)((value & (uint8_t)~LCD_ENABLE) |
                                   LCD_BACKLIGHT));
}

static void BSP_LCD1602_WriteByte(uint8_t value, uint8_t rs)
{
    uint8_t high;
    uint8_t low;

    high = (uint8_t)(value & 0xF0U);
    low = (uint8_t)((value << 4) & 0xF0U);
    if(rs)
    {
        high |= LCD_RS;
        low |= LCD_RS;
    }
    BSP_LCD1602_Write4(high);
    BSP_LCD1602_Write4(low);
}

static void BSP_LCD1602_Command(uint8_t command)
{
    BSP_LCD1602_WriteByte(command, 0U);
}

static void BSP_LCD1602_Data(uint8_t data)
{
    BSP_LCD1602_WriteByte(data, 1U);
}

static void BSP_LCD1602_SetCursor(uint8_t row)
{
    BSP_LCD1602_Command((uint8_t)((row == 0U) ? 0x80U : 0xC0U));
}
#endif

void BSP_LCD1602_Init(void)
{
#if NODE_ID == NODE_ID_BOARD_A
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_SetBits(LCD1602_SCL_PORT, LCD1602_SCL_PIN);
    GPIO_SetBits(LCD1602_SDA_PORT, LCD1602_SDA_PIN);

    gpio.GPIO_Pin = LCD1602_SCL_PIN | LCD1602_SDA_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &gpio);

    delay_ms(40U);
    BSP_LCD1602_Write4(0x30U);
    delay_ms(5U);
    BSP_LCD1602_Write4(0x30U);
    delay_ms(1U);
    BSP_LCD1602_Write4(0x30U);
    BSP_LCD1602_Write4(0x20U);

    BSP_LCD1602_Command(LCD_CMD_FUNCTION_SET);
    BSP_LCD1602_Command(LCD_CMD_DISPLAY_ON);
    BSP_LCD1602_Command(LCD_CMD_CLEAR);
    delay_ms(2U);
    BSP_LCD1602_Command(LCD_CMD_ENTRY_MODE);
    lcd_ready = 1U;
#endif
}

void BSP_LCD1602_WriteLine(uint8_t row, const char *text)
{
#if NODE_ID == NODE_ID_BOARD_A
    uint8_t i;
    char line[16];

    if((lcd_ready == 0U) || (row > 1U) || (text == 0))
    {
        return;
    }

    memset(line, ' ', sizeof(line));
    for(i = 0U; (i < sizeof(line)) && (text[i] != '\0'); i++)
    {
        line[i] = text[i];
    }

    BSP_LCD1602_SetCursor(row);
    for(i = 0U; i < sizeof(line); i++)
    {
        BSP_LCD1602_Data((uint8_t)line[i]);
    }
#else
    (void)row;
    (void)text;
#endif
}

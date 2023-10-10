#include "tm1650.h"

/************ START信号*******************************/
void TM1650_START()
{
    PinctrlSet(TM1650_SDA, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    PinctrlSet(TM1650_SCL, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioSetDir(TM1650_SDA, LZGPIO_DIR_OUT);
    LzGpioSetDir(TM1650_SCL, LZGPIO_DIR_OUT);

    LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_HIGH);
    LzGpioSetVal(TM1650_SDA, LZGPIO_LEVEL_HIGH);
    LzGpioSetVal(TM1650_SDA, LZGPIO_LEVEL_LOW);
    LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_LOW);
}
/******************** STOP信号************************/
void TM1650_STOP()
{
    LzGpioSetVal(TM1650_SDA, LZGPIO_LEVEL_LOW);

    LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_HIGH);

    LzGpioSetVal(TM1650_SDA, LZGPIO_LEVEL_HIGH);

    LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_LOW);
    LzGpioSetVal(TM1650_SDA, LZGPIO_LEVEL_LOW);
    
    PinctrlSet(TM1650_SCL, MUX_FUNC3, PULL_KEEP, DRIVE_KEEP);
    PinctrlSet(TM1650_SDA, MUX_FUNC3, PULL_KEEP, DRIVE_KEEP);
}
/****************写1个字节给TM1650********************/
void write_8bit(uint8_t dat)
{
    uint8_t i;
    LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_LOW);
    for (i = 0; i < 8; i++)
    {
        if (dat & 0x80)
        {
            LzGpioSetVal(TM1650_SDA, LZGPIO_LEVEL_HIGH);

            LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_HIGH);

            LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_LOW);
        }
        else
        {
            LzGpioSetVal(TM1650_SDA, LZGPIO_LEVEL_LOW);

            LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_HIGH);

            LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_LOW);
        }
        dat <<= 1;
    }
    LzGpioSetVal(TM1650_SDA, LZGPIO_LEVEL_HIGH); // ACK信号

    LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_HIGH);

    LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_LOW);
}

/**********************读8bit**************************/
uint8_t read_8bit()
{
    LzGpioValue val = LZGPIO_LEVEL_LOW;
    uint8_t dat, i;
    LzGpioSetVal(TM1650_SDA, LZGPIO_LEVEL_HIGH);
    dat = 0;
    for (i = 0; i < 8; i++)
    {
        LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_HIGH); // 时钟上沿

        dat <<= 1;
        LzGpioGetVal(TM1650_SDA, &val);
        if (val)
            dat++;
        LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_LOW);
    }
    LzGpioSetVal(TM1650_SDA, LZGPIO_LEVEL_LOW); // ACK信号

    LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_HIGH);

    LzGpioSetVal(TM1650_SCL, LZGPIO_LEVEL_LOW);

    return dat;
}
/*******************读按键命令************************/
uint8_t TM1650_read()
{
    uint8_t key;
    TM1650_START();
    write_8bit(0x49); // 读按键指令
    key = read_8bit();
    TM1650_STOP();
    return key;
}
/*****************发送命令信号***********************/
void TM1650_send(uint8_t date1, uint8_t date2)
{
    TM1650_START();
    write_8bit(date1);
    write_8bit(date2);
    TM1650_STOP();
}

void TM1650_Init()
{
    TM1650_send(CMD_DATACMD, CMD_DISPLAYON); // 开启显示模式：8段显示，8级亮度
    TM1650_SetNum(10,0);
    TM1650_SetNum(10,1);
    TM1650_SetNum(10,2);
    TM1650_SetNum(10,3);
}

void TM1650_SetNum(uint8_t dis, uint8_t location)
{
    TM1650Command location_map[] = {CMD_LOCATION_FIRST,CMD_LOCATION_SECOND,CMD_LOCATION_THIRD,CMD_LOCATION_FORTH};
    TM1650Dis dis_map[] = {DIS_NUM_0,DIS_NUM_1,DIS_NUM_2,DIS_NUM_3,DIS_NUM_4,DIS_NUM_5,DIS_NUM_6,DIS_NUM_7,DIS_NUM_8,DIS_NUM_9,DIS_CHAR_MINUS,DIS_CHAR_P,DIS_CHAR_E,DIS_CHAR_N};
    TM1650_send(location_map[location], dis_map[dis]);
}

void TM1650_SetOpen(){
    TM1650_send(CMD_LOCATION_FIRST,DIS_NUM_0);
    TM1650_send(CMD_LOCATION_SECOND,DIS_CHAR_P);
    TM1650_send(CMD_LOCATION_THIRD,DIS_CHAR_E);
    TM1650_send(CMD_LOCATION_FORTH,DIS_CHAR_N);
}
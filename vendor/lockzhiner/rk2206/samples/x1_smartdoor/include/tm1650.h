#ifndef TM1650_H
#define TM1650_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "los_task.h"
#include "ohos_init.h"
#include "lz_hardware.h"
#include "stdbool.h"

void TM1650_START();
void TM1650_STOP();
void write_8bit(uint8_t dat);
uint8_t read_8bit();
uint8_t TM1650_read();
void TM1650_send(uint8_t date1, uint8_t date2);
void TM1650_Init();
void TM1650_SetNum(uint8_t dis, uint8_t location);
void TM1650_SetOpen();

typedef enum{
  CMD_DATACMD         = 0x48,
  CMD_DISPLAYON       = 0x01,
  CMD_LOCATION_FIRST  = 0X68, // read serial number
  CMD_LOCATION_SECOND = 0X6A,
  CMD_LOCATION_THIRD  = 0X6C,
  CMD_LOCATION_FORTH  = 0X6E,
}TM1650Command;

typedef enum{
    DIS_NUM_0         = 0x3F,
    DIS_NUM_1         = 0x06,
    DIS_NUM_2         = 0x5B,
    DIS_NUM_3         = 0x4F,
    DIS_NUM_4         = 0x66,
    DIS_NUM_5         = 0x6D,
    DIS_NUM_6         = 0x7D,
    DIS_NUM_7         = 0x07,
    DIS_NUM_8         = 0x7F,
    DIS_NUM_9         = 0x6F,
    DIS_CHAR_MINUS    = 0x40,
    DIS_CHAR_P        = 0x73,
    DIS_CHAR_E        = 0x79,
    DIS_CHAR_N        = 0x37,
}TM1650Dis;

#define TM1650_SDA GPIO0_PA0
#define TM1650_SCL GPIO0_PA1

#endif
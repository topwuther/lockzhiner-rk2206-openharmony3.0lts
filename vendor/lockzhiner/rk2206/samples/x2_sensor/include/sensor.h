#ifndef SENSOR_H
#define SENSOR_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "los_task.h"
#include "ohos_init.h"
#include "lz_hardware.h"
#include "stdbool.h"
#include "lcd.h"
#include "sht3x.h"
#include "bh1750.h"

#define MQ_2_SENSOR             GPIO0_PB1
#define FLAME_SENSOR            GPIO1_PD0
#define LIGHT                   GPIO0_PB3
#define ALERT_PIN               GPIO0_PB2
#define RAIN_SENSOR             GPIO0_PB5

#define SHT30_ADDR              0x44
#define BH1750_ADDR             0x23

#define WINDOW_PIN              GPIO0_PB4

#endif
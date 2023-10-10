#ifndef SG90_H
#define SG90_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "los_task.h"
#include "ohos_init.h"
#include "lz_hardware.h"
#include "stdbool.h"

void sg90cycle(GpioID pin,uint8_t cycle);

#endif
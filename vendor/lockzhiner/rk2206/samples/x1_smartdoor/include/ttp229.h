#ifndef TTP229_H
#define TTP229_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "los_task.h"
#include "ohos_init.h"
#include "lz_hardware.h"


void ttp229_read16(GpioID ttp229_scl,GpioID ttp229_sdo,char sdo_list[]);
#endif

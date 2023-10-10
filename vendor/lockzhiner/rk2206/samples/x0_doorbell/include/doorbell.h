#ifndef DOORBELL_H
#define DOORBELL_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "los_task.h"
#include "ohos_init.h"
#include "lz_hardware.h"

#define BUTTON             GPIO0_PA3
#define DOORBELL           GPIO0_PA5

void doorbell_process();
void doorbell();

#endif

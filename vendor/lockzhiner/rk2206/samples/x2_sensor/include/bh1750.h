#ifndef BH1750_H
#define BH1750_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "los_task.h"
#include "ohos_init.h"
#include "lz_hardware.h"
#include "typedefs.h"

//-- Enumerations -------------------------------------------------------------
// Sensor Commands
typedef enum
{
    CMD_POWEROFF = 0x00,
    CMD_POWERON = 0x01,        // Waiting for measurement command.
    CMD_RESER = 0x07,        // Reset Data register value.
    CMD_MEAS_PERI_HR_M = 0x10, // Start measurement at 1lx resolution.
} bhCommands;

void BH1750_Init(u8t i2cAddress);
void BH1750_SetI2cAdr(u8t i2cAddress);
etError BH1750_PowerOn();
etError BH1750_StartPeriodicMeasurment();
etError BH1750_ReadMeasurementBuffer(ft *light);

#endif

#include "doorbell.h"

void doorbell_process()
{
    LzGpioValue value = LZGPIO_LEVEL_LOW;

    /* 初始化引脚为GPIO */
    LzGpioInit(BUTTON);
    LzGpioInit(DOORBELL);

    PinctrlSet(BUTTON, MUX_FUNC0, PULL_KEEP, DRIVE_LEVEL0);
    PinctrlSet(DOORBELL, MUX_FUNC0, PULL_KEEP, DRIVE_LEVEL0);

    LzGpioSetDir(BUTTON, LZGPIO_DIR_IN);
    LzGpioSetDir(DOORBELL, LZGPIO_DIR_OUT);
    uint8_t flag = 0;

    LOS_Msleep(5);
    while (1)
    {
        LzGpioGetVal(BUTTON, &value);
        if(value == LZGPIO_LEVEL_HIGH){
            LzGpioSetVal(DOORBELL,LZGPIO_LEVEL_LOW);
            HAL_DelayMs(1);
            LzGpioSetVal(DOORBELL,LZGPIO_LEVEL_HIGH);
            HAL_DelayMs(1);
            flag = 90;
            continue;
        }
        else{
            for(;flag!=0;--flag){
            LzGpioSetVal(DOORBELL,LZGPIO_LEVEL_LOW);
            HAL_DelayMs(3);
            LzGpioSetVal(DOORBELL,LZGPIO_LEVEL_HIGH);
            HAL_DelayMs(3);
            }
        }
        LOS_Msleep(1);
    }
}

void doorbell()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)doorbell_process;
    task.uwStackSize = 2048;
    task.pcName = "doorbell process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(doorbell);
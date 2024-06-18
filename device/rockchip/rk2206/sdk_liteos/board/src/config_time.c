#include "config_network.h"
#include "los_task.h"
#include "lz_hardware.h"
#include "config_time.h"

#define LOG_TAG "config_time"


static bool running = false;
static uint32_t timestamp;

int gettimeofday(struct timeval *tv, void *ptz)
{
    tv->tv_usec = 0;
    if (!running){
        tv->tv_sec = 0;
        return -1;
    }
    tv->tv_sec = timestamp;
    return 0;
}

int settimeofday(const struct timeval *tv, const void *tz){
    timestamp = tv->tv_sec;
}

void TaskConfigTimeEntry(){
    while(get_wifi_info() != 0){
        // LOS_Msleep(1);
        printf("wait wifi!!!\n");
    }
    running = true;
    timestamp = GetNTPTime();
    while(true){
        LOS_Msleep(1000);
        timestamp++;
    }
}
uint32_t TaskConfigTime()
{
    unsigned int threadID;
    unsigned int ret = LOS_OK;
    TSK_INIT_PARAM_S task = {0};
    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskConfigTimeEntry;
    task.uwStackSize  = 10240;
    task.pcName       = "taskConfigTimeEntry";
    task.usTaskPrio   = 8;
    ret = LOS_TaskCreate(&threadID, &task);
    if (ret != LOS_OK) {
        LZ_HARDWARE_LOGE(LOG_TAG, "LOS_TaskCreate error: %d\n", ret);
        return ret;
    }

    return LOS_OK;
}

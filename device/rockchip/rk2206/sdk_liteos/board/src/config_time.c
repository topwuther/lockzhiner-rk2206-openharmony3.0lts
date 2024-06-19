#include "config_network.h"
#include "los_task.h"
#include "lz_hardware.h"
#include "config_time.h"
#include "los_swtmr.h"

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
    return 0;
}

void timeTick(void *arg){
    timestamp++;
}

uint32_t EnableSystemTimeFeature(){
    while(get_wifi_info() != 0){
        LOS_Msleep(1000);
    }
    uint32_t timer_id;
    uint32_t ret;
    timestamp = GetNTPTime();
    ret = LOS_SwtmrCreate(1000, LOS_SWTMR_MODE_PERIOD, timeTick, &timer_id, NULL);
    running = true;
    return ret;
}

#ifndef CONFIG_TIME_H
#define CONFIG_TIME_H

#include "stdint.h"

struct timeval {
    uint32_t tv_sec, tv_usec;
};

uint32_t EnableSystemTimeFeature();
int gettimeofday(struct timeval *tv, void *ptz);
int settimeofday(const struct timeval *tv, const void *tz);
#endif

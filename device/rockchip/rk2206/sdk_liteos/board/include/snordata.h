#ifndef SNORDATA_H
#define SNORDATA_H

typedef enum {
    DATA_OK = 0x0,
    DATA_INVALID = 0xFF,
} DATA_STATUS;

struct SNORData
{
    uint8_t uuid[37];
    DATA_STATUS valid;
};


#endif

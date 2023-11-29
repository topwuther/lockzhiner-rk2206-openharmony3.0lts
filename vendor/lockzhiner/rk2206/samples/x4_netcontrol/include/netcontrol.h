#ifndef NETCONTROL_H
#define NETCONTROL_H

#include "MQTTClient.h"

#define WINDOW_PIN              GPIO0_PB4
#define GARAGE_PIN              GPIO0_PB0
#define LIGHT                   GPIO0_PB3
#define DOOR_PIN                GPIO0_PA2

typedef enum{
    DEVICE_DOOR,
    DEVICE_GARAGE,
    DEVICE_WINDOW,
    DEVICE_LED,
} Device;

void message_receive(MessageData* data);

#endif

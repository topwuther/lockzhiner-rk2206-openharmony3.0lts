#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include "MQTTClient.h"

typedef enum{
	TYPE_TIMEOUT,
	TYPE_STATUS,
	TYPE_STATUS_RET,
	TYPE_SET,
    TYPE_SET_RET,
} MQTTPacket_TYPE;


extern Network MQTTNetwork;

MQTTClient Connect();

#endif

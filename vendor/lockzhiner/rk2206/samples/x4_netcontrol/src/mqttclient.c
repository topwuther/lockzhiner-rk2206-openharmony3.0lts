#include "stdio.h"
#include "config_network.h"
#include "mqttclient.h"
#include "netcontrol.h"

#define MQTT_SERVER_IP                          "192.168.130.75"
#define MQTT_SERVER_PORT                        1883
#define MQTT_USERNAME                           "user1"
#define MQTT_PASSWORD                           "123456"

static unsigned char sendBuf[1000];
static unsigned char readBuf[1000];
Network MQTTNetwork;

MQTTClient Connect(){
    uint32_t ret;
    MQTTClient client;
    MQTTString clientId = MQTTString_initializer;
    MQTTString username = MQTTString_initializer;
    MQTTString password = MQTTString_initializer;
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    uint8_t uuid[37];
    while(get_uuid_info() != 0){
        LOS_Msleep(1000);
    }
    GetUUID(&uuid);
    // printf("NetworkInit...\n");
    NetworkInit(&MQTTNetwork);

begin:
    // printf("NetworkConnect...\n");
    NetworkConnect(&MQTTNetwork, MQTT_SERVER_IP, MQTT_SERVER_PORT);

    // printf("MQTTClientInit...\n");
    MQTTClientInit(&client, &MQTTNetwork, 2000, sendBuf, sizeof(sendBuf), readBuf, sizeof(readBuf));
    
    clientId.cstring = uuid;
    username.cstring = MQTT_USERNAME;
    password.cstring = MQTT_PASSWORD;

    data.clientID          = clientId;
    data.willFlag          = 0;
    data.MQTTVersion       = 3;
    data.keepAliveInterval = 0;
    data.cleansession      = 1;
    data.username          = username;
    data.password          = password;

    // printf("MQTTConnect...\n");
    ret = MQTTConnect(&client, &data);
    if (ret != 0) {
        printf("MQTTConnect fail:%d\n", ret);
        NetworkDisconnect(&MQTTNetwork);
        MQTTDisconnect(&client);
        LOS_Msleep(200);
        goto begin;
    }

    // printf("MQTTSubscribe...\n");
    ret = MQTTSubscribe(&client, "/smarthouse", QOS2, message_receive);
    if (ret != 0) {
        printf("MQTTSubscribe fail:%d\n", ret);
        LOS_Msleep(200);
        goto begin;
    }
    return client;
}

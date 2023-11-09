#include "stdio.h"
#include "stdlib.h"
#include "los_task.h"
#include "config_network.h"
#include "config_uuid.h"

#define LOG_TAG "config_uuid"

void GenerateUUID(char* uuid) {
    srand((unsigned int)GetNTPTime());
    int randlist[4];
    for(uint8_t i=0;i<4;++i){
        randlist[i] = rand();
        srand(randlist[i]);
    }
    // Generate a random UUID
    sprintf(uuid, "%08X-%04X-%04X-%04X-%04X%08X",
            randlist[0],
            randlist[1] & 0xFFFF,
            (randlist[1] << 4 & 0xFFFF) | 0x4000,
            randlist[2] & 0xFFFF,
            randlist[2] << 4 & 0xFFFF, randlist[3]);
}

uint8_t get_uuid_info(){
    struct SNORData snordata;
    VendorGet(VENDOR_ID_CUSTOM,&snordata,sizeof(snordata));
    if(snordata.valid != DATA_INVALID){
        return 0;
    }
    return 1;
}

void GetUUID(char* uuid){
    struct SNORData snordata;
    VendorGet(VENDOR_ID_CUSTOM,&snordata,sizeof(snordata));
    strcpy(uuid,snordata.uuid);
}

void TaskConfigUUIDEntry(){
    struct SNORData snordata;
    FlashInit();
    VendorGet(VENDOR_ID_CUSTOM,&snordata,sizeof(snordata));
    if(snordata.valid != DATA_INVALID){
        return;
    }
    while(get_wifi_info() != 0){
        LOS_Msleep(1000);
    }
    GenerateUUID(&snordata.uuid);
    snordata.valid = DATA_OK;
    VendorSet(VENDOR_ID_CUSTOM,&snordata,sizeof(snordata));
}

uint32_t TaskConfigUUID()
{
    unsigned int threadID;
    unsigned int ret = LOS_OK;
    TSK_INIT_PARAM_S task = {0};
    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskConfigUUIDEntry;
    task.uwStackSize  = 10240;
    task.pcName       = "taskConfigUUIDEntry";
    task.usTaskPrio   = 8;
    ret = LOS_TaskCreate(&threadID, &task);
    if (ret != LOS_OK) {
        LZ_HARDWARE_LOGE(LOG_TAG, "LOS_TaskCreate error: %d\n", ret);
        return ret;
    }

    return LOS_OK;
}

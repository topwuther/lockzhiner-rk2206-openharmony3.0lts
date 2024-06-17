#include "smartdoor.h"

static bool dv_signal;

static char charls[5] = {0};
static uint8_t charls_index = 0;
static uint16_t closeWaits[2] = {0};
static bool closeFlags[2] = {true,true};

/***************************************************************
 * 函数名称: gpio_isr_func
 * 说    明: gpio中断响应处理函数
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void gpio_isr_func(void *args)
{
    ++dv_signal;
}

void pwdbrd(){
    LzGpioValue sdo_value = LZGPIO_LEVEL_LOW;
    char pwd[5] = "0123";

    if(!closeFlags[0]){
        if(closeWaits[0]!=0){
            --closeWaits[0];
        }
        else{
            printf("Door close\n");
            TM1650_Init();
            sg90cycle(DOOR_PIN,0);
            closeFlags[0] = true;
        }
    }
    if (!dv_signal)
    {
        return;
    }
    char sdo_list[16];
    memset(sdo_list, 0, sizeof(sdo_list));
    ttp229_read16(KEYPAD_SCL, KEYPAD_SDO, sdo_list);
    printf("Input:[%s]\n", sdo_list);
    for (uint8_t i = 0; sdo_list[i] != '\0'; ++i)
    {
        char key = sdo_list[i];
        printf("Key:[%c]\n", sdo_list[i]);
        switch (key)
        {
        case 'C':
            charls_index = 0;
            memset(charls, 0, sizeof(charls));
            TM1650_Init();
            printf("Clear.\n");
            continue;
        case 'D':
            if(charls_index>0){
                --charls_index;
                TM1650_SetNum(10,charls_index);
                charls[charls_index] = 0x00;
            }
            // printf("Delete.\n");
            continue;
        case 'A':
            if (strcmp(charls, pwd) == 0)
            {
                // printf("Door open\n");
                TM1650_SetOpen();
                sg90cycle(DOOR_PIN,90);
                closeWaits[0] = 1000;
                closeFlags[0] = false;
            }
            else{
                TM1650_Init();
            }
            charls_index = 0;
            memset(charls, 0, sizeof(charls));
            continue;
        case '*':
        case '#':
        case 'B':
            continue;
        default:
            if (charls_index < 4)
            {
                int num = key - '0';
                // printf("set:[%d,%d]\n",num,charls_index);
                TM1650_SetNum(num,charls_index);
                charls[charls_index] = key;
                ++charls_index;
                // printf("PWD:[%s]\n", charls);
            }
        }
    }
    LOS_Msleep(2);
    dv_signal = false;
}

void detdoor(){
    LzGpioValue value = LZGPIO_LEVEL_LOW;
    LzGpioGetVal(TRACK_SENSOR,&value);
    if(value){
        // printf("Garage open\n");
        sg90cycle(GARAGE_PIN,90);
        closeFlags[1] = false;
        closeWaits[1] = 1000;
    }
    if(!closeFlags[1]){
        if(closeWaits[1]!=0){
            --closeWaits[1];
        }
        else{
            // printf("Garage close\n");
            sg90cycle(GARAGE_PIN,0);
            closeFlags[1] = true;
        }
    }
}

/***************************************************************
 * 函数名称: smartdoor_process
 * 说    明: smartdoor任务
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void smartdoor_process()
{
    unsigned int ret = 0;

    /* 初始化引脚为GPIO */
    LzGpioInit(KEYPAD_SDO);
    LzGpioInit(KEYPAD_SCL);
    LzGpioInit(DOOR_PIN);

    LzGpioInit(TM1650_SDA);
    LzGpioInit(TM1650_SCL);

    LzGpioInit(TRACK_SENSOR);
    LzGpioInit(GARAGE_PIN);

    /* 引脚复用配置为GPIO */
    PinctrlSet(KEYPAD_SDO, MUX_FUNC0, PULL_UP, DRIVE_LEVEL0);
    PinctrlSet(KEYPAD_SCL, MUX_FUNC0, PULL_UP, DRIVE_LEVEL0);
    PinctrlSet(DOOR_PIN, MUX_FUNC0, PULL_DOWN, DRIVE_LEVEL0);

    PinctrlSet(TRACK_SENSOR, MUX_FUNC0, PULL_DOWN, DRIVE_LEVEL0);
    PinctrlSet(GARAGE_PIN, MUX_FUNC0, PULL_DOWN, DRIVE_LEVEL0);

    /* 引脚配置 */
    LzGpioSetDir(KEYPAD_SDO, LZGPIO_DIR_IN);
    LzGpioSetDir(KEYPAD_SCL, LZGPIO_DIR_OUT);
    LzGpioSetDir(DOOR_PIN, LZGPIO_DIR_OUT);

    LzGpioSetDir(TRACK_SENSOR, LZGPIO_DIR_IN);
    LzGpioSetDir(GARAGE_PIN, LZGPIO_DIR_OUT);

    ret = LzGpioRegisterIsrFunc(KEYPAD_SDO, LZGPIO_INT_EDGE_FALLING, gpio_isr_func, NULL);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("LzGpioRegisterIsrFunc failed(%d)\n", ret);
        return;
    }
    LzGpioEnableIsr(KEYPAD_SDO);

    TM1650_Init();

    sg90cycle(DOOR_PIN,0);
    sg90cycle(GARAGE_PIN,0);
    while (1)
    {
        pwdbrd();
        detdoor();
        LOS_Msleep(1);
    }
}

void smartdoor()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)smartdoor_process;
    task.uwStackSize = 2048;
    task.pcName = "keypad process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(smartdoor);
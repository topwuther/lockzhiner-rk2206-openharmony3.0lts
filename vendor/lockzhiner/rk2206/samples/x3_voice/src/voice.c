/*
commands:
ledon
ledoff
windowon
windowoff
temp
humi

chan1:
01 14 1E
chan2:
02 32 17
*/

#include "voice.h"

extern void sg90cycle(GpioID pin, uint8_t cycle);
extern float temperature;
extern float humidity;
extern float light;

void voice_process()
{
    unsigned int ret;
    UartAttribute attr;
    unsigned char str[3];
    unsigned char recv_buffer[STRING_MAXSIZE];
    unsigned int recv_length = 0;
    unsigned int i;

    LzUartDeinit(UART_ID);

    attr.baudRate = 9600;
    attr.dataBits = UART_DATA_BIT_8;
    attr.pad = FLOW_CTRL_NONE;
    attr.parity = UART_PARITY_NONE;
    attr.rxBlock = UART_BLOCK_STATE_NONE_BLOCK;
    attr.stopBits = UART_STOP_BIT_1;
    attr.txBlock = UART_BLOCK_STATE_NONE_BLOCK;

    PinctrlSet(GPIO0_PB6, MUX_FUNC2, PULL_KEEP, DRIVE_LEVEL2);
    PinctrlSet(GPIO0_PB7, MUX_FUNC2, PULL_KEEP, DRIVE_LEVEL2);

    LzUartInit(UART_ID, &attr);

    while (1)
    {
        recv_length = 0;
        memset(recv_buffer, 0, sizeof(recv_buffer));
        recv_length = LzUartRead(UART_ID, recv_buffer, sizeof(recv_buffer));
        // printf("recv:[%s]\n", recv_buffer);
        if (strcmp(recv_buffer, "ledon") == 0)
        {
            LzGpioSetVal(LIGHT, LZGPIO_LEVEL_HIGH);
        }
        else if (strcmp(recv_buffer, "ledoff") == 0)
        {
            LzGpioSetVal(LIGHT, LZGPIO_LEVEL_LOW);
        }
        else if (strcmp(recv_buffer, "windowon") == 0)
        {
            sg90cycle(WINDOW_PIN, 90);
        }
        else if (strcmp(recv_buffer, "windowoff") == 0)
        {
            sg90cycle(WINDOW_PIN, 0);
        }
        else if (strcmp(recv_buffer, "temp") == 0)
        {
            uint8_t temp1 = (uint8_t)temperature;
            uint8_t temp2 = (uint8_t)((temperature - temp1) * 100);
            uint8_t send_buffer[] = {0x01, temp1, temp2};
            LzUartWrite(UART_ID, send_buffer, 3);
        }
        else if (strcmp(recv_buffer, "humi") == 0)
        {
            uint8_t humi1 = (uint8_t)humidity;
            uint8_t humi2 = (uint8_t)((humidity - humi1) * 100);
            uint8_t send_buffer[] = {0x02, humi1, humi2};
            LzUartWrite(UART_ID, send_buffer, 3);
        }
        else if (strcmp(recv_buffer, "light") == 0)
        {
            uint8_t send_buffer[] = {0x03, 0, 0, 0, 0, 0};
            int light_int = (int)light;
            send_buffer[1] = (uint8_t)(light_int);       // Least significant byte
            send_buffer[2] = (uint8_t)(light_int >> 8);  // Byte 1
            send_buffer[3] = (uint8_t)(light_int >> 16); // Byte 2
            send_buffer[4] = (uint8_t)(light_int >> 24); // Most significant byte
            send_buffer[5] = (uint8_t)((light - light_int) * 100);
            LzUartWrite(UART_ID, send_buffer, 6);
        }
        // else
        // {
        //     printf("Unknown command.\n");
        // }

        LOS_Msleep(1000);
    }

    return;
}

/***************************************************************
 * 函数名称: voice
 * 说    明: 开机自启动调用函数
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void voice()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)voice_process;
    task.uwStackSize = 2048;
    task.pcName = "voice process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(voice);

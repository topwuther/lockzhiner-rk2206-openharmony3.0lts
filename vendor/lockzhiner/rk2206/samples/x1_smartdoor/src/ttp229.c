#include "ttp229.h"

void ttp229_read16(GpioID ttp229_scl, GpioID ttp229_sdo, char sdo_list[])
{
    LzGpioValue sdo_value = LZGPIO_LEVEL_LOW;
    char keymap[] = {
        '1', '2', '3', 'A',
        '4', '5', '6', 'B',
        '7', '8', '9', 'C',
        '*', '0', '#', 'D'};
    uint8_t sdo_index = 0;
    bool values[16];
    memset(values,false,sizeof(values));

    LOS_UDelay(10);
    for (uint8_t i = 0; i < 16; ++i)
    {
        LzGpioSetVal(ttp229_scl, LZGPIO_LEVEL_LOW);
        LOS_Msleep(1);
        LzGpioGetVal(ttp229_sdo, &sdo_value);
        LzGpioSetVal(ttp229_scl, LZGPIO_LEVEL_HIGH);
        LOS_Msleep(1);
        values[i] = sdo_value;
    }
    for(uint8_t i=0;i<16;++i){
        if (!values[i])
        {
            sdo_list[sdo_index] = keymap[i];
            ++sdo_index;
        }
    }
}
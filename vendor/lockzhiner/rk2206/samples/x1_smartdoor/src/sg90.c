#include "sg90.h"

void sg90cycle(GpioID pin,uint8_t cycle){
    uint16_t highDelay = 0;
    uint16_t lowDelay = 0;
    switch (cycle)
    {
    case 0:
        highDelay = 500;
        lowDelay = 19500;
        break;
    case 45:
        highDelay = 1000;
        lowDelay = 19000;
        break;
    case 90:
        highDelay = 1500;
        lowDelay = 18500;
        break;
    case 135:
        highDelay = 2000;
        lowDelay = 18000;
        break;
    case 180:
        highDelay = 2500;
        lowDelay = 17500;
        break;
    
    default:
        break;
    }
    for(uint8_t i=0;i<30;++i){
        LzGpioSetVal(pin,LZGPIO_LEVEL_HIGH);
        HAL_DelayUs(highDelay);
        LzGpioSetVal(pin,LZGPIO_LEVEL_LOW);
        HAL_DelayUs(lowDelay);
    }
}

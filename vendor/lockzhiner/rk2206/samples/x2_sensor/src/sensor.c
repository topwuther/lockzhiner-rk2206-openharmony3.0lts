#include "sensor.h"

extern void sg90cycle(GpioID pin,uint8_t cycle);

static bool flags[5] = {true, true, true, true, true};
static uint8_t sht30Update = 0;
static uint8_t bh1750Update = 0;
float temperature = 0; // temperature [°C]
float humidity = 0;    // relative humidity [%RH]
float light = 0;

void MQ2SensorShow()
{
    // MQ-2 Sensor
    LzGpioValue value = LZGPIO_LEVEL_LOW;
    uint8_t nosmoke_chinese[] = "未检测到烟雾";
    uint8_t smoke_chinese[] = "检测到烟雾";

    LzGpioGetVal(MQ_2_SENSOR, &value);
    if (!value && !flags[0])
    {
        lcd_show_chinese(0, 0, smoke_chinese, LCD_RED, LCD_BLACK, 32, 0);
        lcd_fill(160, 0, 192, 32, LCD_BLACK);
        flags[0] = true;
    }
    else if (value && flags[0])
    {
        lcd_show_chinese(0, 0, nosmoke_chinese, LCD_WHITE, LCD_BLACK, 32, 0);
        flags[0] = false;
    }
}

void FlameSensorShow()
{
    LzGpioValue value = LZGPIO_LEVEL_LOW;
    uint8_t noframe_chinese[] = "未检测到明火";
    uint8_t frame_chinese[] = "检测到明火";

    // Flame Sensor
    LzGpioGetVal(FLAME_SENSOR, &value);
    if (!value && !flags[1])
    {
        lcd_show_chinese(0, 32, frame_chinese, LCD_RED, LCD_BLACK, 32, 0);
        lcd_fill(160, 32, 192, 64, LCD_BLACK);
        flags[1] = true;
    }
    else if (value && flags[1])
    {
        lcd_show_chinese(0, 32, noframe_chinese, LCD_WHITE, LCD_BLACK, 32, 0);
        flags[1] = false;
    }
}

void RainSensorShow()
{
    LzGpioValue value = LZGPIO_LEVEL_LOW;
    uint8_t norain_chinese[] = "未检测到水滴";
    uint8_t rain_chinese[] = "检测到水滴";

    // Rain Sensor
    LzGpioGetVal(RAIN_SENSOR, &value);
    if (!value && !flags[2])
    {
        lcd_show_chinese(0, 64, rain_chinese, LCD_RED, LCD_BLACK, 32, 0);
        lcd_fill(160, 64, 192, 96, LCD_BLACK);
        flags[2] = true;
        sg90cycle(WINDOW_PIN,0);
    }
    else if (value && flags[2])
    {
        lcd_show_chinese(0, 64, norain_chinese, LCD_WHITE, LCD_BLACK, 32, 0);
        flags[2] = false;
        sg90cycle(WINDOW_PIN,90);
    }
}

void SHT30SensorShow()
{
    etError error;
    uint8_t errsensor_chinese[] = "温湿度异常";
    uint8_t temperature_chinese[] = "温度";
    uint8_t humidity_chinese[] = "湿度";
    sht30Update = (sht30Update + 1) % 100;
    if (sht30Update == 0)
    {
        error = SHT3X_ReadMeasurementBuffer(&temperature, &humidity);
        if (error != NO_ERROR)
        {
            lcd_fill(0, 128, 160, 160, LCD_BLACK);
            lcd_show_chinese(0, 96, errsensor_chinese, LCD_RED, LCD_BLACK, 32, 0);
            return;
        }
        lcd_fill(144, 96, 160, 128, LCD_BLACK);
        lcd_show_chinese(0, 96, temperature_chinese, LCD_WHITE, LCD_BLACK, 32, 0);
        lcd_show_float_num1(64, 96, temperature, 4, LCD_WHITE, LCD_BLACK, 32);
        lcd_show_chinese(0, 128, humidity_chinese, LCD_WHITE, LCD_BLACK, 32, 0);
        lcd_show_float_num1(64, 128, humidity, 4, LCD_WHITE, LCD_BLACK, 32);
    }
}

void BH1750SensorShow()
{
    etError error;
    uint8_t recv_data[2] = {0};
    uint8_t light_chinese[] = "光照";
    uint8_t errlight_chinese[] = "光照异常";
    bh1750Update = (bh1750Update + 1) % 180;
    if (bh1750Update == 0)
    {
        error = BH1750_ReadMeasurementBuffer(&light);
        if (error != NO_ERROR)
        {
            lcd_fill(128, 160, 160, 192, LCD_BLACK);
            lcd_show_chinese(0, 160, errlight_chinese, LCD_RED, LCD_BLACK, 32, 0);
            return;
        }
        lcd_show_chinese(0, 160, light_chinese, LCD_WHITE, LCD_BLACK, 32, 0);
        lcd_show_float_num1(64, 160, light,
                            light >= 100 ? 5 : 4, LCD_WHITE, LCD_BLACK, 32);
        if (light < 100)
        {
            lcd_fill(144, 160, 160, 160, LCD_BLACK);
        }
        if (light < 30)
        {
            LzGpioSetVal(LIGHT, LZGPIO_LEVEL_HIGH);
        }
    }
}

void Alert()
{
    LzGpioValue val;
    val = flags[0] || flags[1];
    LzGpioSetVal(ALERT_PIN, val);
}


/***************************************************************
 * 函数名称: gpio_process
 * 说    明: gpio任务
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void sensor_process()
{
    /* 初始化引脚为GPIO */
    LzGpioInit(MQ_2_SENSOR);
    LzGpioInit(FLAME_SENSOR);
    LzGpioInit(LIGHT);
    LzGpioInit(ALERT_PIN);
    LzGpioInit(RAIN_SENSOR);
    LzGpioInit(WINDOW_PIN);
    /* 引脚复用配置为GPIO */
    PinctrlSet(MQ_2_SENSOR, MUX_FUNC0, PULL_KEEP, DRIVE_LEVEL0);
    PinctrlSet(FLAME_SENSOR, MUX_FUNC0, PULL_KEEP, DRIVE_LEVEL0);
    PinctrlSet(LIGHT, MUX_FUNC0, PULL_KEEP, DRIVE_LEVEL0);
    PinctrlSet(ALERT_PIN, MUX_FUNC0, PULL_UP, DRIVE_LEVEL0);
    PinctrlSet(RAIN_SENSOR, MUX_FUNC0, PULL_UP, DRIVE_LEVEL0);
    PinctrlSet(WINDOW_PIN, MUX_FUNC0, PULL_KEEP, DRIVE_LEVEL0);

    LzGpioSetDir(MQ_2_SENSOR, LZGPIO_DIR_IN);
    LzGpioSetDir(FLAME_SENSOR, LZGPIO_DIR_IN);
    LzGpioSetDir(LIGHT, LZGPIO_DIR_OUT);
    LzGpioSetDir(ALERT_PIN, LZGPIO_DIR_OUT);
    LzGpioSetDir(RAIN_SENSOR, LZGPIO_DIR_IN);
    LzGpioSetDir(WINDOW_PIN, LZGPIO_DIR_OUT);

    SHT3X_Init(SHT30_ADDR);
    BH1750_Init(BH1750_ADDR);
    BH1750_StartPeriodicMeasurment();
    SHT3X_StartPeriodicMeasurment(REPEATAB_HIGH, FREQUENCY_10HZ);
    lcd_init();
    lcd_fill(0, 0, LCD_W, LCD_H, LCD_BLACK);

    while (1)
    {
        MQ2SensorShow();
        FlameSensorShow();
        RainSensorShow();
        SHT30SensorShow();
        BH1750SensorShow();
        Alert();
        LOS_Msleep(1);
    }
}

/***************************************************************
 * 函数名称: sensor
 * 说    明: 开机自启动调用函数
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void sensor()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)sensor_process;
    task.uwStackSize = 2048;
    task.pcName = "sensor process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(sensor);
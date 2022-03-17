/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "cmsis_os2.h"
#include "config_network.h"

#include "e53_intelligent_smoke_sensation.h"
#include "eeprom.h"
#include "crc16.h"


#define  EEPROM_ADDR         0              //保存地址

#define  EEPROM_HEAD_DATA    0x4c5a0001     //Eeprom数据头
#define  DEFUALT_SN_DATA     1              //用于判断默认参数是否保存至eeprom， 当此数值改变时则保存
#define  DEFUALT_MQ_DATA     100            //烟感数据默认值
#define  DEFUALT_BEEP_DATA   333333         //蜂鸣器数据默认值
#define  DEFUALT_LED_DATA    50             //LED数据默认值

#define  ERROR_MQ_DATA_MAX   20000          //烟感异常值
#define  ERROR_MQ_DATA_MIN   10             //烟感异常值

#define  CAL_PPM             20             //校准环境中PPM值
#define  RL                  1              //RL阻值
static float m_r0;                          //元件在干净空气中的阻值

/***************************************************************
* 名		称: iss_led_dev_s
* 说    明：led设备结构体
***************************************************************/
typedef struct
{
    int                 init;               //初始化状态
    Pinctrl             led;                //io总线
} iss_led_dev_s;

/***************************************************************
* 名		称: iss_adc_dev_s
* 说    明：ADC设备结构体
***************************************************************/
typedef struct
{
    int                 init;               //初始化状态
    int                 port;               //端口
    Pinctrl             adc;                //io总线
} iss_adc_dev_s;
typedef iss_adc_dev_s iss_mq2_dev_s;

/***************************************************************
* 名		称: iss_pwm_dev_s
* 说    明：PWM设备结构体
***************************************************************/
typedef struct
{
    int                 init;               //初始化状态
    int                 port;               //端口
    uint32_t            duty;               //占空比
    uint32_t            cycle;              //周期
    uint8_t             onoff;              //开关
    PwmBusIo            pwmio;              //io总线
} iss_pwm_dev_s;
typedef iss_pwm_dev_s iss_beep_dev_s;

/***************************************************************
* 名		称: iss_eeprom_data_s
* 说    明：Eeprom数据结构体
***************************************************************/
typedef struct
{
    uint32_t            head;               //数据头   0x4c5a0001
    uint16_t            sn;                 //用于判断是否需要保存默认参数 当次数与已保存的不一致时保存默认参数
    uint16_t            mqdata;             //烟感数据
    uint32_t            beepdata;           //蜂鸣器数据
    uint16_t            leddata;            //led数据
    uint16_t            reserve[4];         //保留
    uint16_t            crc;                //crc校验
} __attribute__((packed))iss_eeprom_data_s;

/***************************************************************
* 名		称: GRF_SOC_CON29
* 说    明：GRF_SOC_CON29寄存器结构体
***************************************************************/
typedef union
{
    uint32_t value;
    struct
    {
        uint16_t grf_saradc_ana_reg_low: 4;
        uint16_t grf_saradc_vol_sel: 1;
        uint16_t grf_saradc_ana_reg_high: 11;
        struct
        {
            uint16_t grf_saradc_ana_reg_low: 4;
            uint16_t grf_saradc_vol_sel: 1;
            uint16_t grf_saradc_ana_reg_high: 11;
        } rw;
    };
} GRF_SOC_CON29;


static iss_eeprom_data_s m_iss_eeprom_data =
{
    .head     = EEPROM_HEAD_DATA,
    .sn       = DEFUALT_SN_DATA,
    .mqdata   = DEFUALT_MQ_DATA,
    .beepdata = DEFUALT_BEEP_DATA,
    .leddata  = DEFUALT_LED_DATA,
};

static iss_led_dev_s m_iss_led =
{
    .init = 0,
    .led = {.gpio = GPIO0_PA2, .func = MUX_GPIO, .type = PULL_UP, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_OUT, .val = LZGPIO_LEVEL_HIGH},
};

static iss_mq2_dev_s m_iss_mq2 =
{
    .init = 0,
    .port = 4,
    .adc = {.gpio = GPIO0_PC4, .func = MUX_FUNC1, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_IN, .val = LZGPIO_LEVEL_KEEP},
};

static iss_beep_dev_s m_iss_beep =
{
    .init = 0,
    .port = 7,
    .duty = 50,
    .cycle = 333000,
    .pwmio =
    {
        .pwm = {.gpio = GPIO1_PD0, .func = MUX_FUNC1, .type = PULL_DOWN, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
        .id  = FUNC_ID_PWM1,
        .mode = FUNC_MODE_M1,
    },
};


/***************************************************************
* 函数名称: iss_led_dev_init
* 说    明: 初始化LED设备
* 参    数: iss_led_dev_s *led
* 返 回 值: 0 成功，1 失败
***************************************************************/
static uint32_t iss_led_dev_init(iss_led_dev_s *led)
{
    if (PinctrlInit(led->led) != 0)
    {
        printf("led pin %d init failed\n", led->led.gpio);
        return 1;
    }
    
    led->init = 1;
    return 0;
}

/***************************************************************
* 函数名称: iss_adc_dev_init
* 说    明: 初始化ADC设备
* 参    数: iss_mq2_dev_s *adc
* 返 回 值: 0 成功，1 失败
***************************************************************/
static uint32_t iss_adc_dev_init(iss_mq2_dev_s *adc)
{
    if (PinctrlInit(adc->adc) != 0)
    {
        printf("adc pin %d init failed\n", adc->adc.gpio);
        return 1;
    }
    if (LzSaradcInit() != 0)
    {
        printf("saradc  %d init failed\n", adc->port);
        return 1;
    }
    
    volatile GRF_SOC_CON29 *soc = (GRF_SOC_CON29*)&GRF->SOC_CON29;
    /*配置ADC外部参考电压grf_saradc_vol_sel=0，内部参考电压grf_saradc_vol_sel=1,rw.grf_saradc_vol_sel对应位写使能*/
    soc->rw.grf_saradc_vol_sel = 1;
    soc->grf_saradc_vol_sel    = 0;
    soc->rw.grf_saradc_vol_sel = 0;
    
    adc->init = 1;
    return 0;
}

/***************************************************************
* 函数名称: iss_pwm_dev_init
* 说    明: 初始化Pwm
* 参    数: iss_pwm_dev_s *p
* 返 回 值: 0 成功，1 失败
***************************************************************/
static uint32_t iss_pwm_dev_init(iss_pwm_dev_s *p)
{
    if (PwmIoInit(p->pwmio) != 0)
    {
        printf("Pwm pin  %d init failed\n", p->pwmio.pwm.gpio);
        return 1;
    }
    if (LzPwmInit(p->port) != 0)
    {
        printf("Pwm  %d init failed\n", p->port);
        return 1;
    }
    
    p->init = 1;
    return 0;
}

/***************************************************************
* 函数名称: iss_get_eeprom_data
* 说    明: 读取eeprom数据
* 参    数: iss_eeprom_data_s *data
* 返 回 值: 0 成功，1 失败
***************************************************************/
static uint32_t iss_get_eeprom_data(iss_eeprom_data_s *data)
{
    iss_eeprom_data_s   temp = {0};
    
    if (eeprom_read(EEPROM_ADDR, (unsigned char*)&temp, sizeof(iss_eeprom_data_s)) == sizeof(iss_eeprom_data_s))
    {
        /*crc16校验*/
        if (calc_crc16((uint8_t*)&temp, sizeof(iss_eeprom_data_s) - 2) == temp.crc)
        {
            *data = temp;
            return 0;
        }
        else
        {
            printf("crc err Calc:0x%04x, read:0x%04x\n", calc_crc16((uint8_t*)&temp, sizeof(iss_eeprom_data_s) - 2), temp.crc);
        }
    }
    else
    {
        printf("read eeprom data error read\n");
    }
    
    return 1;
}

/***************************************************************
* 函数名称: iss_set_eeprom_data
* 说    明: 保存eeprom数据
* 参    数: iss_eeprom_data_s *data
* 返 回 值: 0 成功，1 失败
***************************************************************/
static uint32_t iss_set_eeprom_data(iss_eeprom_data_s *data)
{
    uint32_t wlen = 0;
    uint32_t rlen = 0;
    uint16_t crc  = 0;
    iss_eeprom_data_s temp = {0};
    /*计算crc*/
    data->crc = calc_crc16((uint8_t*)data, sizeof(iss_eeprom_data_s) - 2);
    wlen = eeprom_write(EEPROM_ADDR, (unsigned char*)data, sizeof(iss_eeprom_data_s));
    if (wlen == sizeof(iss_eeprom_data_s))
    {
        /*读出已写的数据进行校验是否写成功*/
        rlen = eeprom_read(EEPROM_ADDR, (unsigned char*)&temp, sizeof(iss_eeprom_data_s));
        if (rlen == sizeof(iss_eeprom_data_s))
        {
            crc = calc_crc16((uint8_t*)&temp, sizeof(iss_eeprom_data_s) - 2);
            if (crc == temp.crc)
            {
                return 0;
            }
            else
            {
                printf("crc err write:0x%04x, Calc:0x%04x, read:0x%04x\n", data->crc, crc, temp.crc);
            }
        }
        else
        {
            printf("read eeprom data error read len:%d, len:%d\n", rlen, sizeof(iss_eeprom_data_s));
        }
    }
    else
    {
        printf("write eeprom data error write len:%d, len:%d\n", wlen, sizeof(iss_eeprom_data_s));
    }
    
    return 1;
}
/***************************************************************
* 函数名称: eeprom_test
* 说    明: eeprom 测试
* 参    数: 无
* 返 回 值: 无
***************************************************************/
static void eeprom_test(void)
{
    int i = 0;
    uint8_t temp = 255;
    
    for (i = 0; i < 256; i++)
    {
        if (eeprom_writebyte(i, i) != 1)
        {
            printf("\n\nwrite eeprom data failed\n\n");
            return;
        }
        if (eeprom_readbyte(i, &temp) != 1)
        {
            printf("\n\nread eeprom data failed\n\n");
            return;
        }
        if (temp != i)
        {
            printf("\n\nread eeprom data error write:%02x, read:%02x\n\n", i, temp);
            return;
        }
    }
    printf("\n\neeprom data read and write success\n\n");
    
}
/***************************************************************
* 函数名称: iss_save_default_param
* 说    明: 保存默认的Iss数据
* 参    数: void
* 返 回 值: 0 成功，1 失败
***************************************************************/
static uint32_t iss_save_default_param(void)
{
    // eeprom_test();
    iss_eeprom_data_s temp = {0};
    /*读取数据*/
    if (iss_get_eeprom_data(&temp) == 0)
    {
        /*已保存的数据与默认参数一致就不重复保存*/
        if (memcmp(&temp, &m_iss_eeprom_data, sizeof(iss_eeprom_data_s)) == 0)
        {
            return 0;
        }
        /*一致不保存默认参数*/
        if (temp.sn == m_iss_eeprom_data.sn)
        {
            return 0;
        }
        /*更新sn*/
        m_iss_eeprom_data.sn   = temp.sn;
    }
    else
    {
        m_iss_eeprom_data.sn   = DEFUALT_SN_DATA;
    }
    
    m_iss_eeprom_data.head     = EEPROM_HEAD_DATA;
    m_iss_eeprom_data.mqdata   = DEFUALT_MQ_DATA;
    m_iss_eeprom_data.beepdata = DEFUALT_BEEP_DATA;
    m_iss_eeprom_data.leddata  = DEFUALT_LED_DATA;
    m_iss_eeprom_data.crc      = calc_crc16((uint8_t*)&m_iss_eeprom_data, sizeof(iss_eeprom_data_s) - 2);
    
    /*保存默认参数*/
    if (iss_set_eeprom_data(&m_iss_eeprom_data) != 0)
    {
        printf("save default err\n");
        return 1;
    }
    
    return 0;
}

/***************************************************************
* 函数名称: iss_param_init
* 说    明: 初始化Iss数据
* 参    数: void
* 返 回 值: 0 成功，1 失败
***************************************************************/
static uint32_t iss_param_init(void)
{
    /*判断是否要保存默认参数*/
    iss_save_default_param();
    /*先从eeprom读数据*/
    if (iss_get_eeprom_data(&m_iss_eeprom_data) != 0)
    {
        /*读失败时防止保存的参数是异常值*/
        if (m_iss_eeprom_data.mqdata < ERROR_MQ_DATA_MIN || m_iss_eeprom_data.mqdata > ERROR_MQ_DATA_MAX)
        {
            /*设置默认值*/
            m_iss_eeprom_data.mqdata = DEFUALT_MQ_DATA;
        }
        return 1;
    }
    printf("\n\neeprom data:%08x %04x %04x %08x %04x %04x %04x %04x %04x %04x\n\n", m_iss_eeprom_data.head, m_iss_eeprom_data.sn, m_iss_eeprom_data.mqdata,
           m_iss_eeprom_data.beepdata, m_iss_eeprom_data.leddata, m_iss_eeprom_data.reserve[0], m_iss_eeprom_data.reserve[1],
           m_iss_eeprom_data.reserve[2],  m_iss_eeprom_data.reserve[3], m_iss_eeprom_data.crc);
           
    /*读成功时防止保存的参数是异常值*/
    if (m_iss_eeprom_data.beepdata < 1000 || m_iss_eeprom_data.beepdata > 100000000)
    {
        m_iss_eeprom_data.beepdata = DEFUALT_BEEP_DATA;
    }
    if (m_iss_eeprom_data.mqdata < ERROR_MQ_DATA_MIN || m_iss_eeprom_data.mqdata > ERROR_MQ_DATA_MAX)
    {
        m_iss_eeprom_data.mqdata = DEFUALT_MQ_DATA;
    }
    /*设置beep参数*/
    m_iss_beep.cycle = m_iss_eeprom_data.beepdata;
    
    return  0;
}


/***************************************************************
* 函数名称: iss_adc_read_value
* 说    明: 读取ADC 值
* 参    数: iss_mq2_dev_s adc       ADC设备
           uint32_t *data  获取的数据
* 返 回 值: 0 成功，1 失败
***************************************************************/
static uint32_t iss_adc_read_value(iss_mq2_dev_s adc, uint32_t *data)
{
    if (adc.init == 0)
    {
        printf("ADC not init\n");
        return 1;
    }
    else if (LzSaradcReadValue(adc.port, data) != 0)
    {
        printf("ADC Read Fail\n");
        return 1;
    }
    return 0;
}

/***************************************************************
* 函数名称: iss_pwm_start
* 说    明: pwm 启动
* 参    数: iss_pwm_dev_s *pwm    pwm设备
* 返 回 值: 0 成功，1 失败
***************************************************************/
static uint32_t iss_pwm_start(iss_pwm_dev_s *pwm)
{
    if (pwm->init == 0)
    {
        printf("PWM not init\n");
        return 1;
    }
    else if (LzPwmStart(pwm->port, pwm->duty * pwm->cycle / 100, pwm->cycle) != 0)
    {
        printf("PWM Start Fail\n");
        return 1;
    }
    pwm->onoff = 1;
    return 0;
}

/***************************************************************
* 函数名称: iss_pwm_stop
* 说    明: pwm 停止
* 参    数: iss_pwm_dev_s *pwm     pwm设备
* 返 回 值: 0 成功，1 失败
***************************************************************/
static uint32_t iss_pwm_stop(iss_pwm_dev_s *pwm)
{
    if (pwm->init == 0)
    {
        printf("PWM not init\n");
        return 1;
    }
    else if (pwm->onoff == 0)
    {
        return 0;
    }
    else if (LzPwmStop(pwm->port) != 0)
    {
        printf("PWM Stop Fail\n");
        return 1;
    }
    pwm->onoff = 0;
    return 0;
}

/***************************************************************
* 函数名称: iss_get_voltage
* 说    明: 获取电压值函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
static float iss_get_voltage(void)
{
    unsigned int ret;
    unsigned int data;
    
    ret = iss_adc_read_value(m_iss_mq2, &data);
    if (ret != 0)
    {
        printf("ADC Read Fail\n");
    }
    return (float)data * 3.3  / 1024;
}

/***************************************************************
* 函数名称: e53_iss_init
* 说    明: 初始化E53_Iss扩展板
* 参    数: 无
* 返 回 值: 0 成功，反之失败
***************************************************************/
uint32_t e53_iss_init(void)
{
    uint32_t ret = 1;
    
    ret = eeprom_init();
    if (ret != 0)
    {
        return ret;
    }
    ret = iss_param_init();
    if (ret != 0)
    {
        return ret;
    }
    ret = iss_led_dev_init(&m_iss_led);
    if (ret != 0)
    {
        return ret;
    }
    ret = iss_pwm_dev_init(&m_iss_beep);
    if (ret != 0)
    {
        return ret;
    }
    ret = iss_adc_dev_init(&m_iss_mq2);
    
    return ret;
}

/***************************************************************
* 函数名称: e53_iss_get_mq2_ppm
* 说    明: 获取PPM函数
* 参    数: 无
* 返 回 值: ppm
***************************************************************/
float e53_iss_get_mq2_ppm(void)
{
    float voltage, rs, ppm;
    
    voltage = iss_get_voltage();
    rs = (5 - voltage) / voltage * RL; //计算rs
    ppm = 613.9f * pow(rs / m_r0, -2.074f); //计算ppm
    return ppm;
}

/***************************************************************
* 函数名称: e53_iss_mq2_ppm_calibration
* 说    明: 传感器校准函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_iss_mq2_ppm_calibration(void)
{
    float voltage = iss_get_voltage();
    float rs = (5 - voltage) / voltage * RL;
    
    m_r0 = rs / pow(CAL_PPM / 613.9f, 1 / -2.074f);
}

/***************************************************************
* 函数名称: e53_iss_get_mq2_alarm_value
* 说    明: 获取ppm报警阈值函数
* 参    数: 无
* 返 回 值: 报警阈值
***************************************************************/
uint16_t e53_iss_get_mq2_alarm_value(void)
{
    return m_iss_eeprom_data.mqdata;
}

/***************************************************************
* 函数名称: e53_iss_beep_status_set
* 说    明: 蜂鸣器报警与否
* 参    数: status,ENUM枚举的数据
*		   OFF,关蜂鸣器
*		   ON,开蜂鸣器
* 返 回 值: 无
***************************************************************/
void e53_iss_beep_status_set(e53_iss_status_e status)
{
    if (status == ON)
    {
        iss_pwm_start(&m_iss_beep);
    }
    if (status == OFF)
    {
        iss_pwm_stop(&m_iss_beep);
    }
}

/***************************************************************
* 函数名称: e53_iss_led_status_set
* 说    明: LED报警与否
* 参    数: status,ENUM枚举的数据
*		   OFF,关led
*		   ON,开led
* 返 回 值: 无
***************************************************************/
void e53_iss_led_status_set(e53_iss_status_e status)
{
    if (status == ON)
    {
        LzGpioSetVal(m_iss_led.led.gpio, LZGPIO_LEVEL_LOW);//设置GPIO输出低电平打开led
    }
    if (status == OFF)
    {
        LzGpioSetVal(m_iss_led.led.gpio, LZGPIO_LEVEL_HIGH);//设置GPIO输出高电平关闭led
    }
}



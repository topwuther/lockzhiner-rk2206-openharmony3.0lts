/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "sensor_config_controller.h"
#include <securec.h>
#include "osal_mem.h"
#include "osal_time.h"
#include "sensor_platform_if.h"

#define HDF_LOG_TAG    sensor_config_controller_c

static int32_t SensorOpsNop(struct SensorBusCfg *busCfg, struct SensorRegCfg *cfgItem)
{
    (void)busCfg;
    (void)cfgItem;
    return HDF_SUCCESS;
}

static int32_t SensorOpsRead(struct SensorBusCfg *busCfg, struct SensorRegCfg *cfgItem)
{
    uint16_t value = 0;
    int32_t ret;

    ret = ReadSensor(busCfg, cfgItem->regAddr, (uint8_t *)&value, sizeof(value));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read i2c reg");

    return value;
}

static uint32_t GetSensorRegRealValueMask(struct SensorRegCfg *cfgItem, uint32_t *value, uint32_t busMask)
{
    uint32_t mask = cfgItem->mask & busMask;
    *value = cfgItem->value & busMask;

    if (cfgItem->shiftNum != 0) {
        if (cfgItem->calType == SENSOR_CFG_CALC_TYPE_RIGHT_SHIFT) {
            *value = *value >> cfgItem->shiftNum;
            mask = mask >> cfgItem->shiftNum;
        } else {
            *value = *value << cfgItem->shiftNum;
            mask = mask << cfgItem->shiftNum;
        }
    }

    return mask;
}

static int32_t SensorOpsWrite(struct SensorBusCfg *busCfg, struct SensorRegCfg *cfgItem)
{
    uint8_t calType = cfgItem->calType;
    uint8_t value[SENSOR_VALUE_BUTT];
    int32_t ret = HDF_FAILURE;

    value[SENSOR_ADDR_INDEX] = cfgItem->regAddr;
    value[SENSOR_VALUE_INDEX] = cfgItem->value;

    switch (calType) {
        case SENSOR_CFG_CALC_TYPE_NONE:
            ret = WriteSensor(busCfg, value, sizeof(value));
            break;
        case SENSOR_CFG_CALC_TYPE_SET:
        case SENSOR_CFG_CALC_TYPE_REVERT:
        case SENSOR_CFG_CALC_TYPE_XOR:
        case SENSOR_CFG_CALC_TYPE_LEFT_SHIFT:
        case SENSOR_CFG_CALC_TYPE_RIGHT_SHIFT:
        default:
            break;
    }

    return ret;
}

static int32_t SensorOpsReadCheck(struct SensorBusCfg *busCfg, struct SensorRegCfg *cfgItem)
{
    uint32_t value = 0;
    uint32_t originValue;
    uint32_t mask;
    uint32_t busMask = 0xffff;
    int32_t ret;

    CHECK_NULL_PTR_RETURN_VALUE(busCfg, HDF_FAILURE);

    if (busCfg->busType == SENSOR_BUS_I2C) {
        ret = ReadSensor(busCfg, cfgItem->regAddr, (uint8_t *)&value, sizeof(value));
        CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read i2c reg");
        busMask = (busCfg->i2cCfg.regWidth == SENSOR_ADDR_WIDTH_1_BYTE) ? 0x00ff : 0xffff;
    }

    mask = GetSensorRegRealValueMask(cfgItem, &originValue, busMask);
    if ((value & mask) != (originValue & mask)) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t SensorBitwiseCalculate(struct SensorRegCfg *cfgItem, uint32_t *value, uint32_t valueMask)
{
    uint32_t originValue;
    uint32_t mask;
    uint32_t tmp;

    mask = GetSensorRegRealValueMask(cfgItem, &originValue, valueMask);
    switch ((enum SensorCalculateType)cfgItem->calType)
    {
        case SENSOR_CFG_CALC_TYPE_SET:
            *value &= ~mask;
            *value |= (originValue & mask);
            break;
        case SENSOR_CFG_CALC_TYPE_REVERT:
            tmp = *value & (~mask);
            *value = ~(*value & mask);
            *value = tmp | (*value & mask);
            break;
        default:
            HDF_LOGE("%s: unsupported cal type", __func__);
            break;
    }
    return 0;
}

static int32_t SensorOpsUpdateBitwise(struct SensorBusCfg *busCfg, struct SensorRegCfg *cfgItem)
{
    uint32_t value = 0;
    uint32_t busMask = 0x000000ff;
    int32_t ret;
    uint8_t valueArray[SENSOR_VALUE_BUTT];

    CHECK_NULL_PTR_RETURN_VALUE(busCfg, HDF_FAILURE);

    if (busCfg->busType == SENSOR_BUS_I2C) {
        ret = ReadSensor(busCfg, cfgItem->regAddr, (uint8_t *)&value, busCfg->i2cCfg.regWidth);
        CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read i2c reg");
        busMask = (busCfg->i2cCfg.regWidth == SENSOR_ADDR_WIDTH_1_BYTE) ? 0x000000ff : 0x0000ffff;
    }

    ret = SensorBitwiseCalculate(cfgItem, &value, busMask);
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "update bitwise failed");

    valueArray[SENSOR_ADDR_INDEX] = cfgItem->regAddr;
    valueArray[SENSOR_VALUE_INDEX] = (uint8_t)value;

    ret = WriteSensor(busCfg, valueArray, sizeof(valueArray));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "update bitewise write failed");

    return HDF_SUCCESS;
}

static struct SensorOpsCall g_doOpsCall[] = {
    { SENSOR_OPS_TYPE_NOP,                         SensorOpsNop },
    { SENSOR_OPS_TYPE_READ,                        SensorOpsRead },
    { SENSOR_OPS_TYPE_WRITE,                       SensorOpsWrite },
    { SENSOR_OPS_TYPE_READ_CHECK,                  SensorOpsReadCheck },
    { SENSOR_OPS_TYPE_UPDATE_BITWISE,              SensorOpsUpdateBitwise },
};

int32_t SetSensorRegCfgArray(struct SensorBusCfg *busCfg, const struct SensorRegCfgGroupNode *group)
{
    int32_t num = 0;
    uint32_t count;
    struct SensorRegCfg *cfgItem = NULL;

    CHECK_NULL_PTR_RETURN_VALUE(busCfg, HDF_FAILURE);
    CHECK_NULL_PTR_RETURN_VALUE(group, HDF_FAILURE);
    CHECK_NULL_PTR_RETURN_VALUE(group->regCfgItem, HDF_FAILURE);

    count = sizeof(g_doOpsCall) / sizeof(g_doOpsCall[0]);

    while (num < group->itemNum) {
        cfgItem = (group->regCfgItem + num);
        if (cfgItem->opsType >= count) {
            HDF_LOGE("%s: cfg item para invalid", __func__);
            break;
        }
        if (g_doOpsCall[cfgItem->opsType].ops != NULL) {
            if (g_doOpsCall[cfgItem->opsType].ops(busCfg, cfgItem) != HDF_SUCCESS) {
                HDF_LOGE("%s: malloc sensor reg config item data failed", __func__);
                return HDF_FAILURE;
            }
        }
        if (cfgItem->delay != 0) {
            OsalMDelay(cfgItem->delay);
        }
        num++;
    }

    return HDF_SUCCESS;
}

/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "parameter_hal.h"

#include <cstring>
#include <fstream>
#include <securec.h>

#include "parameters.h"
#include "sysparam_errno.h"
#include "string_ex.h"

static const char *g_emptyStr = "";

static const char OHOS_MANUFACTURE[] = {"****"};
static const char OHOS_BRAND[] = {"****"};
static const char OHOS_PRODUCT_SERIES[] = {"****"};
static const char OHOS_SOFTWARE_MODEL[] = {"****"};
static const char OHOS_HARDWARE_MODEL[] = {"****"};
static const char OHOS_HARDWARE_PROFILE[] = {"****"};
static const char DEF_OHOS_SERIAL[] = {"1234567890"};
static const char SN_FILE[] = {"/sys/block/mmcblk0/device/cid"};
static const char OHOS_ABI_LIST[] = {"****"};
static const char OHOS_BOOTLOADER_VERSION[] = {"bootloader"};
static const int OHOS_FIRST_API_LEVEL = 1;
static const char OHOS_DISPLAY_VERSION[] = {"OpenHarmony 3.0 LTS"};
static const char OHOS_INCREMENTAL_VERSION[] = {"****"};
static const char OHOS_BUILD_TYPE[] = {"****"};
static const char OHOS_BUILD_USER[] = {"****"};
static const char OHOS_BUILD_HOST[] = {"****"};
static const char OHOS_BUILD_TIME[] = {"****"};

static bool IsValidValue(const char *value, unsigned int len)
{
    if (value == nullptr) {
        return false;
    }
    const std::string strValue(value);
    if (strValue.length() + 1 > len) {
        return false;
    }
    return true;
}

int HalGetParameter(const char *key, const char *def, char *value, unsigned int len)
{
    if ((key == nullptr) || (value == nullptr)) {
        return EC_INVALID;
    }
    const std::string strKey(key);
    std::string res = OHOS::system::GetParameter(strKey, "");
    if (res == "") {
        if (!IsValidValue(def, len)) {
            return EC_INVALID;
        }
        if (sprintf_s(value, len, "%s", def) < 0) {
            return EC_FAILURE;
        }
        return EC_SUCCESS;
    }

    const char *result = res.c_str();
    if (!IsValidValue(result, len)) {
        return EC_INVALID;
    }
    if (sprintf_s(value, len, "%s", result) <= 0) {
        return EC_FAILURE;
    }

    return EC_SUCCESS;
}

int HalGetIntParameter(const char *key, int def)
{
    const std::string strKey(key);
    return OHOS::system::GetIntParameter(strKey, def);
}

int HalSetParameter(const char *key, const char *value)
{
    if ((key == nullptr) || (value == nullptr)) {
        return EC_INVALID;
    }
    const std::string strKey(key);
    const std::string strVal(value);
    bool ret = OHOS::system::SetParameter(strKey, strVal);
    return ret ? EC_SUCCESS : EC_FAILURE;
}

const char *GetProperty(const std::string &key, const char **paramHolder)
{
    if (*paramHolder != nullptr) {
        return *paramHolder;
    }
    std::string result = OHOS::system::GetParameter(key, "");
    const char *res = strdup(result.c_str());
    if (res == nullptr) {
        return g_emptyStr;
    }
    *paramHolder = res;
    return *paramHolder;
}

const char *HalGetDeviceType()
{
    static const char *productType = nullptr;
    return GetProperty("ro.build.characteristics", &productType);
}

const char *HalGetProductModel()
{
    static const char *productModel = nullptr;
    return GetProperty("ro.product.model", &productModel);
}

const char *HalGetManufacture()
{
    return OHOS_MANUFACTURE;
}

const char *HalGetBrand()
{
    return OHOS_BRAND;
}

const char *HalGetMarketName()
{
    static const char *marketName = nullptr;
    return GetProperty("ro.product.name", &marketName);
}

const char *HalGetProductSeries()
{
    return OHOS_PRODUCT_SERIES;
}

const char *HalGetSoftwareModel()
{
    return OHOS_SOFTWARE_MODEL;
}

const char *HalGetHardwareModel()
{
    return OHOS_HARDWARE_MODEL;
}

const char *HalGetHardwareProfile()
{
    return OHOS_HARDWARE_PROFILE;
}

const char *HalGetSerial()
{
    static const char* ohos_serial = nullptr;
    if (ohos_serial != nullptr) {
        return ohos_serial;
    }
    std::ifstream infile;
    infile.open(SN_FILE);
    std::string value;
    infile >> value;
    if (value.empty()) {
        ohos_serial = DEF_OHOS_SERIAL;
    } else {
        std::string sn = OHOS::ReplaceStr(value, ":", "");
        const char* res = strdup(sn.c_str());
        if (res == nullptr) {
            return DEF_OHOS_SERIAL;
        }
        ohos_serial = res;
    }
    return ohos_serial;
}

const char *HalGetAbiList()
{
    return OHOS_ABI_LIST;
}

const char *HalGetBootloaderVersion()
{
    return OHOS_BOOTLOADER_VERSION;
}

int HalGetFirstApiVersion()
{
    return OHOS_FIRST_API_LEVEL;
}

const char *HalGetDisplayVersion()
{
    return OHOS_DISPLAY_VERSION;
}

const char *HalGetIncrementalVersion()
{
    return OHOS_INCREMENTAL_VERSION;
}

const char *HalGetBuildType()
{
    return OHOS_BUILD_TYPE;
}

const char *HalGetBuildUser()
{
    return OHOS_BUILD_USER;
}

const char *HalGetBuildHost()
{
    return OHOS_BUILD_HOST;
}

const char *HalGetBuildTime()
{
    return OHOS_BUILD_TIME;
}

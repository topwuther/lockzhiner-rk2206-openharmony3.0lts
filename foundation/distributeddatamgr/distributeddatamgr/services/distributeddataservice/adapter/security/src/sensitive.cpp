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

#include "sensitive.h"
#include <utility>
#include "iprocess_system_api_adapter.h"
#include "log_print.h"
#include "serializable.h"
#include "1.0/dev_slinfo_mgr.h"
#undef LOG_TAG
#define LOG_TAG "Sensitive"

namespace OHOS::DistributedKv {
Sensitive::Sensitive(std::string deviceId, uint32_t type)
    : deviceId(std::move(deviceId)), securityLevel(DATA_SEC_LEVEL1), deviceType(type)
{
}

Sensitive::Sensitive(const std::vector<uint8_t> &value)
    : securityLevel(DATA_SEC_LEVEL1), deviceType(0)
{
    Unmarshal(value);
}

std::vector<uint8_t> Sensitive::Marshal() const
{
    Json::Value root;
    root[GET_NAME(securityLevel)] = securityLevel;
    root[GET_NAME(deviceId)] = deviceId;
    root[GET_NAME(dataBase64)] = dataBase64;
    root[GET_NAME(deviceType)] = deviceType;

    Json::FastWriter writer;
    auto jsonStr = writer.write(root);
    ZLOGD("len:%d, value:%.20s!", int32_t(jsonStr.size()), jsonStr.c_str());
    return {jsonStr.begin(), jsonStr.end()};
}

void Sensitive::Unmarshal(const std::vector<uint8_t> &value)
{
    std::string input(reinterpret_cast<const char *>(value.data()), value.size());
    Json::Reader reader;
    Json::Value root;
    ZLOGD("len:%d, value:%.20s!", int32_t(value.size()), input.c_str());
    bool success = reader.parse(input, root);
    if (!success) {
        ZLOGE("reader.parse failed!");
    }

    securityLevel = Serializable::GetVal(root[GET_NAME(securityLevel)], securityLevel);
    deviceId = Serializable::GetVal(root[GET_NAME(deviceId)], deviceId);
    dataBase64 = Serializable::GetVal(root[GET_NAME(dataBase64)], dataBase64);
    deviceType = Serializable::GetVal(root[GET_NAME(deviceType)], deviceType);
}

uint32_t Sensitive::GetSensitiveLevel()
{
    DEVSLQueryParams query;
    DEVSL_INIT_PARAMS(&query);
    query.udid = reinterpret_cast<const uint8_t *>(deviceId.c_str());
    query.sensitiveData = reinterpret_cast<const uint8_t *>(dataBase64.c_str());
    query.idLen = uint32_t(deviceId.size());
    query.sensitiveDataLen = uint32_t(dataBase64.size());
    if (dataBase64.empty()) {
        query.devType = GetDevslDeviceType();
    }

    uint32_t level = DATA_SEC_LEVEL2;
    uint32_t result = DEVSL_GetHighestSecLevel(&query, &level);
    if (result != DEVSL_SUCCESS) {
        ZLOGE("get highest level failed(%.10s)! level: %d, error: %d, cert (%.10s)",
              deviceId.c_str(), securityLevel, result, dataBase64.c_str());
        return securityLevel;
    }
    securityLevel = level;
    ZLOGD("get highest level success(%.10s)! level: %d cert (%.10s)",
          deviceId.c_str(), securityLevel, dataBase64.c_str());
    return securityLevel;
}

bool Sensitive::operator >= (const DistributedDB::SecurityOption &option)
{
    return (option.securityLabel == DistributedDB::NOT_SET) ||
           (GetSensitiveLevel() >= static_cast<uint32_t>(option.securityLabel - 1));
}

bool Sensitive::LoadData()
{
    uint8_t data[Sensitive::MAX_DATA_LEN + 1];
    uint32_t length = Sensitive::MAX_DATA_LEN;
    int32_t result = DEVSL_GetLocalCertData(data, Sensitive::MAX_DATA_LEN, &length);
    if (result != DEVSL_SUCCESS) {
        ZLOGE("DEVSL_GetLocalCertData failed %d", result);
        return false;
    }
    data[length] = 0;
    dataBase64 = reinterpret_cast<char *>(data);
    DEVSLQueryParams query;
    DEVSL_INIT_PARAMS(&query);
    query.udid = reinterpret_cast<const uint8_t *>(deviceId.c_str());
    query.sensitiveData = data;
    query.idLen = uint32_t(deviceId.size());
    query.sensitiveDataLen = length;

    if (DEVSL_GetHighestSecLevel(&query, &securityLevel) != DEVSL_SUCCESS) {
        securityLevel = DATA_SEC_LEVEL1;
    }
    return true;
}

Sensitive::Sensitive(Sensitive &&sensitive) noexcept
{
    this->operator=(std::move(sensitive));
}

Sensitive &Sensitive::operator=(Sensitive &&sensitive) noexcept
{
    if (this == &sensitive) {
        return *this;
    }
    deviceId = std::move(sensitive.deviceId);
    dataBase64 = std::move(sensitive.dataBase64);
    securityLevel = sensitive.securityLevel;
    deviceType = sensitive.deviceType;
    return *this;
}

Sensitive::Sensitive(const Sensitive &sensitive)
{
    this->operator=(sensitive);
}

Sensitive &Sensitive::operator=(const Sensitive &sensitive)
{
    if (this == &sensitive) {
        return *this;
    }
    deviceId = sensitive.deviceId;
    dataBase64 = sensitive.dataBase64;
    securityLevel = sensitive.securityLevel;
    deviceType = sensitive.deviceType;
    return *this;
}

uint32_t Sensitive::GetDevslDeviceType() const
{
    return deviceType;
}
}

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
#ifndef HI_APP_EVENT_CONFIG_H
#define HI_APP_EVENT_CONFIG_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
class HiAppEventConfig {
public:
    static HiAppEventConfig& GetInstance();
    void SetStorageDir(const std::string& dir);
    bool SetConfigurationItem(std::string name, std::string value);
    bool GetDisable();
    uint64_t GetMaxStorageSize();
    std::string GetStorageDir();

private:
    HiAppEventConfig() {}
    ~HiAppEventConfig() {}
    HiAppEventConfig(const HiAppEventConfig&);
    HiAppEventConfig& operator=(const HiAppEventConfig&);
    bool SetDisableItem(const std::string& value);
    bool SetMaxStorageSizeItem(const std::string& value);
    void SetDisable(bool disable);
    void SetMaxStorageSize(uint64_t size);

private:
    bool disable = false;
    uint64_t maxStorageSize = 10 * 1024 * 1024; // max storage size is 10M, 10 * 1024 * 1024 Byte
    std::string storageDir = "";
};
} // HiviewDFX
} // OHOS
#endif // HI_APP_EVENT_CONFIG_H
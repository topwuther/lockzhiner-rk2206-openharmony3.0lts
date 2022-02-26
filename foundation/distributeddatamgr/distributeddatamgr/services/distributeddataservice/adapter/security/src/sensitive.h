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

#ifndef OHOS_SENSITIVE_H
#define OHOS_SENSITIVE_H

#include <string>
#include <vector>
#include <iprocess_system_api_adapter.h>

namespace OHOS::DistributedKv {
class Sensitive final {
public:
    static constexpr uint32_t MAX_DATA_LEN = 16 * 1024;
    Sensitive(std::string deviceId, uint32_t type);
    explicit Sensitive(const std::vector<uint8_t> &value = {});
    Sensitive(const Sensitive &sensitive);
    Sensitive &operator=(const Sensitive &sensitive);
    Sensitive(Sensitive &&sensitive) noexcept;
    Sensitive &operator=(Sensitive &&sensitive) noexcept;
    ~Sensitive() = default;

    std::vector<uint8_t> Marshal() const;
    void Unmarshal(const std::vector<uint8_t> &value);

    bool operator >= (const DistributedDB::SecurityOption &option);

    bool LoadData();
private:
    uint32_t GetSensitiveLevel();
    uint32_t GetDevslDeviceType() const;

    std::string deviceId {};
    std::string dataBase64 {};
    uint32_t securityLevel = 0;
    uint32_t deviceType = 0;
};
}

#endif // OHOS_SENSITIVE_H

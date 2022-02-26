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

#ifndef PROCESS_SYSTEM_API_ADAPTER_H
#define PROCESS_SYSTEM_API_ADAPTER_H

#include <string>
#include <map>
#include <mutex>

#include "iprocess_system_api_adapter.h"
#include "types.h"

namespace DistributedDB {
class ProcessSystemApiAdapterImpl : public IProcessSystemApiAdapter {
public:
    ProcessSystemApiAdapterImpl();
    ~ProcessSystemApiAdapterImpl() override;
    DBStatus RegOnAccessControlledEvent(const OnAccessControlledEvent &callback) override;
    bool IsAccessControlled() const override;
    DBStatus SetSecurityOption(const std::string &filePath, const SecurityOption &option) override;
    DBStatus GetSecurityOption(const std::string &filePath, SecurityOption &option) const override;
    bool CheckDeviceSecurityAbility(const std::string &devId, const SecurityOption &option) const override;
    void SetLockStatus(bool isLock);
    void ResetSecOptDic();
    void ResetAdapter();

private:
    mutable std::mutex adapterlock_;
    OnAccessControlledEvent callback_;
    std::map<const std::string, SecurityOption> pathSecOptDic_;
    bool isLocked_;
};
} // namespace DistributedDB

#endif

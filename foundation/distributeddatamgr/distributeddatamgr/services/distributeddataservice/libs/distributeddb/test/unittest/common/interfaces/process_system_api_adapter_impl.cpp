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

#include "process_system_api_adapter_impl.h"

#include <sys/types.h>
#include <dirent.h>

#include "log_print.h"
#include "platform_specific.h"
#include "distributeddb_tools_unit_test.h"

namespace DistributedDB {
ProcessSystemApiAdapterImpl::ProcessSystemApiAdapterImpl()
    : callback_(nullptr),
      isLocked_(false)
{
}

ProcessSystemApiAdapterImpl::~ProcessSystemApiAdapterImpl()
{
    callback_ = nullptr;
}

DBStatus ProcessSystemApiAdapterImpl::RegOnAccessControlledEvent(const OnAccessControlledEvent &callback)
{
    callback_ = callback;
    return OK;
}

bool ProcessSystemApiAdapterImpl::IsAccessControlled() const
{
    return isLocked_;
}

DBStatus ProcessSystemApiAdapterImpl::SetSecurityOption(const std::string &filePath, const SecurityOption &option)
{
    bool isExisted = OS::CheckPathExistence(filePath);
    if (!isExisted) {
        LOGE("SetSecurityOption to unexistence dir![%s]", filePath.c_str());
        return NOT_FOUND;
    }

    std::string dirName;
    struct dirent *direntPtr = nullptr;
    DIR *dirPtr = opendir(filePath.c_str());
    if (dirPtr == nullptr) {
        LOGD("set path secOpt![%s] [%d] [%d]", filePath.c_str(), option.securityFlag, option.securityLabel);
        pathSecOptDic_[filePath] = option;
        return OK;
    }

    while (true) {
        direntPtr = readdir(dirPtr);
        // condition to exit the loop
        if (direntPtr == nullptr) {
            break;
        }
        // only remove all *.db files
        std::string str(direntPtr->d_name);
        if (str == "." || str == "..") {
            continue;
        }
        dirName.clear();
        dirName.append(filePath).append("/").append(str);
        if (direntPtr->d_type == DT_DIR) {
            SetSecurityOption(dirName, option);
            std::lock_guard<std::mutex> lock(adapterlock_);
            pathSecOptDic_[dirName] = option;
            LOGD("set path secOpt![%s] [%d] [%d]", dirName.c_str(), option.securityFlag, option.securityLabel);
        } else {
            std::lock_guard<std::mutex> lock(adapterlock_);
            pathSecOptDic_[dirName] = option;
            LOGD("set path secOpt![%s] [%d] [%d]", dirName.c_str(), option.securityFlag, option.securityLabel);
            continue;
        }
    }
    closedir(dirPtr);
    pathSecOptDic_[filePath] = option;
    return OK;
}

DBStatus ProcessSystemApiAdapterImpl::GetSecurityOption(const std::string &filePath, SecurityOption &option) const
{
    std::map<const std::string, SecurityOption> temp = pathSecOptDic_; // For const interface only for test
    if (temp.find(filePath) == temp.end()) {
        LOGE("[ProcessSystemApiAdapterImpl]::[GetSecurityOption] path [%s] not set secOpt!", filePath.c_str());
        option.securityLabel = NOT_SET;
        option.securityFlag = 0;
        return OK;
    }
    LOGD("[AdapterImpl] Get path secOpt![%s] [%d] [%d]", filePath.c_str(), option.securityFlag, option.securityLabel);
    option = temp[filePath];
    return OK;
}

bool ProcessSystemApiAdapterImpl::CheckDeviceSecurityAbility(const std::string &devId,
    const SecurityOption &option) const
{
    return true;
}

void ProcessSystemApiAdapterImpl::SetLockStatus(bool isLock)
{
    std::lock_guard<std::mutex> lock(adapterlock_);
    if (callback_) {
        callback_(isLock);
    }
    isLocked_ = isLock;
}

void ProcessSystemApiAdapterImpl::ResetSecOptDic()
{
    pathSecOptDic_.clear();
}

void ProcessSystemApiAdapterImpl::ResetAdapter()
{
    ResetSecOptDic();
    SetLockStatus(false);
}
};

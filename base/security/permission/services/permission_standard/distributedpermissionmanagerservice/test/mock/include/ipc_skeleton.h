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

#ifndef OHOS_IPC_IPC_SKELETON_H
#define OHOS_IPC_IPC_SKELETON_H

#include "iremote_object.h"

namespace OHOS {
class IPCSkeleton {
public:
    IPCSkeleton() = default;
    ~IPCSkeleton() = default;

    static bool SetContextObject(sptr<IRemoteObject> &object)
    {
        return false;
    }

    static sptr<IRemoteObject> GetContextObject()
    {
        return nullptr;
    }

    static bool SetMaxWorkThreadNum(int maxThreadNum)
    {
        return false;
    }

    static pid_t GetCallingPid()
    {
        return pid_;
    }

    static pid_t GetCallingUid()
    {
        return uid_;
    }

    static std::string GetLocalDeviceID()
    {
        return localDeviceId_;
    }

    static std::string GetCallingDeviceID()
    {
        return deviceId_;
    }

    static IPCSkeleton GetInstance()
    {
        static IPCSkeleton skeleton;
        return skeleton;
    }

    static bool IsLocalCalling()
    {
        return true;
    }

    static int FlushCommands(IRemoteObject *object)
    {
        return 1;
    }

    static std::string ResetCallingIdentity()
    {
        return "";
    }

    static bool SetCallingIdentity(std::string &identity)
    {
        return true;
    }

public:
    static pid_t pid_;

    static pid_t uid_;

    static std::string localDeviceId_;

    static std::string deviceId_;
};
}  // namespace OHOS
#endif  // OHOS_IPC_IPC_SKELETON_H
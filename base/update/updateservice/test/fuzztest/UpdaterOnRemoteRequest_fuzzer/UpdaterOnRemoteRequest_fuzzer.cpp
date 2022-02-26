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

#include "UpdaterOnRemoteRequest_fuzzer.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include "iremote_stub.h"
#include "iupdate_service.h"
#include "message_parcel.h"
#include "parcel.h"
#include "update_service.h"
#include "update_service_stub.h"

using namespace OHOS;
using namespace OHOS::update_engine;

class UpdaterOnRemoteRequestFuzzer : public UpdateServiceStub {
public:
    virtual int32_t RegisterUpdateCallback(const UpdateContext &ctx, const sptr<IUpdateCallback>& updateCallback)
    {
        return 0;
    }

    virtual int32_t UnregisterUpdateCallback()
    {
        return 0;
    }

    virtual int32_t CheckNewVersion()
    {
        return 0;
    }

    virtual int32_t DownloadVersion()
    {
        return 0;
    }

    virtual int32_t DoUpdate()
    {
        return 0;
    }

    virtual int32_t GetNewVersion(VersionInfo &versionInfo)
    {
        return 0;
    }

    virtual int32_t GetUpgradeStatus (UpgradeInfo &info)
    {
        return 0;
    }

    virtual int32_t SetUpdatePolicy(const UpdatePolicy &policy)
    {
        return 0;
    }

    virtual int32_t GetUpdatePolicy(UpdatePolicy &policy)
    {
        return 0;
    }

    virtual int32_t Cancel(int32_t service)
    {
        return 0;
    }

    virtual int32_t RebootAndClean(const std::string &miscFile, const std::string &cmd)
    {
        return 0;
    }

    virtual int32_t RebootAndInstall(const std::string &miscFile, const std::string &packageName)
    {
        return 0;
    }
};

static uint32_t U32_AT(const uint8_t *ptr)
{
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

static int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    MessageParcel reply;
    MessageOption option;
    UpdaterOnRemoteRequestFuzzer updaterser;
    auto ret = updaterser.OnRemoteRequest(code, data, reply, option);

    return ret;
}

static void FuzzAccountService(const uint8_t* data, size_t size)
{
    MessageParcel dataMessageParcel;

    uint32_t code = U32_AT(data);

    data = data + 4;  /* Intercept the first 4 bytes of data for variable code */
    size = size - 4;  /* size of data need to reduce 4 bytes */
    dataMessageParcel.WriteInterfaceToken(UpdateServiceStub::GetDescriptor());
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    OnRemoteRequest(code, dataMessageParcel);
}

namespace OHOS {
    bool FuzzOnRemoteRequest(const uint8_t* data, size_t size)
    {
        FuzzAccountService(data, size);
        return 0;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzOnRemoteRequest(data, size);
    return 0;
}


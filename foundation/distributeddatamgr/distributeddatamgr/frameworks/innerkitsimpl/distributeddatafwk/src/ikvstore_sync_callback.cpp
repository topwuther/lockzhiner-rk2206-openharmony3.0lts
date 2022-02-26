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

#define LOG_TAG "KvStoreSyncCallbackProxy"

#include "ikvstore_sync_callback.h"
#include <chrono>
#include <ctime>
#include "log_print.h"
#include "message_parcel.h"

namespace OHOS {
namespace DistributedKv {
enum {
    SYNCCOMPLETED,
};
constexpr int32_t MAX_DEVICES = 4096;
KvStoreSyncCallbackProxy::KvStoreSyncCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IKvStoreSyncCallback>(impl)
{}

void KvStoreSyncCallbackProxy::SyncCompleted(const std::map<std::string, Status> &results)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreSyncCallbackProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return;
    }
    if (!data.WriteInt32(static_cast<int>(results.size()))) {
        ZLOGW("write results size error.");
        return;
    }
    for (auto const &[k, v] : results) {
        if (!data.WriteString(k) ||
            !data.WriteInt32(static_cast<int>(v))) {
            ZLOGW("write results error.");
            return;
        }
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int error = Remote()->SendRequest(SYNCCOMPLETED, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest failed, error %d", error);
    }
}

int32_t KvStoreSyncCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
                                                 MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = KvStoreSyncCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        ZLOGE("local descriptor is not equal to remote");
        return -1;
    }
    switch (code) {
        case SYNCCOMPLETED: {
            std::map<std::string, Status> results;
            int32_t size = data.ReadInt32();
            if (size < 0 || size > MAX_DEVICES) {
                ZLOGW("size < 0(%d)", size);
                return 0;
            }
            for (int32_t i = 0; i < size; i++) {
                results.insert(std::pair<std::string, Status>(data.ReadString(),
                    static_cast<Status>(data.ReadInt32())));
            }
            SyncCompleted(results);
            return 0;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}
}  // namespace DistributedKv
}  // namespace OHOS

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

#ifndef SYNCER_H
#define SYNCER_H

#include "generic_syncer.h"

namespace DistributedDB {
class SingleVerSyncer final : public GenericSyncer {
public:
    SingleVerSyncer();
    ~SingleVerSyncer() override;

    // Enable auto sync function
    void EnableAutoSync(bool enable) override;

    // delete specified device's watermark
    int EraseDeviceWaterMark(const std::string &deviceId, bool isNeedHash) override;

    // Local data changed callback
    void LocalDataChanged(int notifyEvent) override;

    // Remote data changed callback
    void RemoteDataChanged(const std::string &device) override;

    // Set stale data wipe policy
    int SetStaleDataWipePolicy(WipePolicy policy) override;

protected:
    // Create a sync engine, if has memory error, will return nullptr.
    ISyncEngine *CreateSyncEngine() override;

private:
    bool autoSyncEnable_;
};
} // namespace DistributedDB

#endif  // SYNCER_H

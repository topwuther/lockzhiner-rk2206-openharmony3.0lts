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

#ifndef SYNC_UID_STATE_COMMAND_H
#define SYNC_UID_STATE_COMMAND_H

namespace OHOS {
namespace Security {
namespace Permission {
/**
 * SyncUidStateCommand
 */
class SyncUidStateCommand : public BaseRemoteCommand {
public:
    void Prepare() override;

    void Execute() override;

    void Finish() override;

    std::string ToJsonPayload() override;

    SyncUidStateCommand(const std::string &json);
    SyncUidStateCommand(const int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId);
    virtual ~SyncUidStateCommand() = default;

private:
    /**
     * The command name. Should be equal to class name.
     */
    const std::string COMMAND_NAME = "SyncUidStateCommand";
    int32_t uid_;
    int32_t uidState_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif
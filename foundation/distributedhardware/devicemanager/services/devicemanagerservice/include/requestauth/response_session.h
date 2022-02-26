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
#ifndef OHOS_RESPONSE_SESSION_H
#define OHOS_RESPONSE_SESSION_H

#include <string>
#include <vector>
#include <mutex>
#include <semaphore.h>

#include "single_instance.h"
#include "dm_device_info.h"
#include "msg_response_auth.h"
#include "msg_request_auth.h"
#include "hichain_connector.h"
#include "dm_timer.h"

namespace OHOS {
namespace DistributedHardware {
enum ResponseSessionStatus : int32_t {
    SESSION_INIT = 0,
    SESSION_WAITTING_USER_CONFIRM = 1,
    SESSION_WAITTING_GROUP_CREATED = 2,
    SESSION_WAITTING_PIN_CODE = 3
};

class ResponseSession : public HichainConnectorCallback, public std::enable_shared_from_this<ResponseSession> {
public:
    ResponseSession();
    virtual ~ResponseSession() {};
    int32_t SendResponseMessage(int32_t reply);
    void OnUserOperate(int32_t action);
    void OnGroupCreated(int64_t requestId, const std::string &groupId) override;
    void OnMemberJoin(int64_t requestId, int32_t status) override;
    void BuildAuthenticationInfo(DmAuthParam &authParam);
    void OnReceiveMsg(long long channelId, std::string &message);
    int64_t GetRequestId();
    int32_t GetPinCodeByReqId(int64_t requestId);
    bool IsMyChannelId(long long channelId);
    void Release();
    void CancelDisplay();
    int32_t GetStatus();

private:
    int32_t DecodeReqMsg(std::string &message);
    int32_t StartFaService();
    std::string GenerateGroupName();
    void OnUserConfirm();
    void OnUserReject(int32_t errorCode);
    int32_t GeneratePincode();

private:
    int64_t mRequestId_;
    std::string mGroupId_;
    std::string mGroupName_;
    std::string mReqDeviceId_;
    sem_t mSem_;
    std::shared_ptr<MsgRequestAuth> mMsgRequestAuthPtr_;
    int64_t mChannelId_;
    int32_t mPincode_;
    int32_t mSessionStatus_;
    std::shared_ptr<DmTimer> mReceiveTimerPtr_;
    std::shared_ptr<DmTimer> mMemberJoinTimerPtr_;
};
} // namespace DistributedHardware
} // namespace OHOS


#endif

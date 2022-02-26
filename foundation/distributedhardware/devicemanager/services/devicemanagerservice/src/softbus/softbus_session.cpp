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

#include "softbus_session.h"
#include "softbus_adapter.h"
#include "anonymous_string.h"
#include "device_manager_log.h"
#include "device_manager_errno.h"
#include "auth_manager.h"
#include "encrypt_utils.h"
#include "constants.h"
#include "session.h"
#include "inner_session.h"

namespace OHOS {
namespace DistributedHardware {
static int32_t SessionOpened(int32_t sessionId, int32_t result)
{
    return SoftbusSession::GetInstance().OnSessionOpened(sessionId, result);
}

static void SessionClosed(int32_t sessionId)
{
    SoftbusSession::GetInstance().OnSessionClosed(sessionId);
}

static void BytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    SoftbusSession::GetInstance().GetInstance().OnBytesReceived(sessionId, data, dataLen);
}

static void MessageReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    (void)sessionId;
    (void)data;
    (void)dataLen;
    DMLOG(DM_LOG_INFO, "sessionId:%d, dataLen:%d", sessionId, dataLen);
}

static void StreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext, const FrameInfo *param)
{
    (void)sessionId;
    (void)data;
    (void)ext;
    (void)param;
    DMLOG(DM_LOG_INFO, "sessionId:%d", sessionId);
}

IMPLEMENT_SINGLE_INSTANCE(SoftbusSession);

SoftbusSession::~SoftbusSession()
{
    (void)RemoveSessionServer(PKG_NAME, SESSION_NAME);
}

int32_t SoftbusSession::Start()
{
    DMLOG(DM_LOG_INFO, "SoftbusSession start");
    ISessionListener iSessionListener = {
        .OnSessionOpened = SessionOpened,
        .OnSessionClosed = SessionClosed,
        .OnBytesReceived = BytesReceived,
        .OnMessageReceived = MessageReceived,
        .OnStreamReceived = StreamReceived
    };
    int32_t ret = CreateSessionServer(PKG_NAME, SESSION_NAME, &iSessionListener);
    if (ret != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_DEBUG, "CreateSessionServer failed");
        return DEVICEMANAGER_CREATE_SESSION_SERVER_FAILED;
    }
    return DEVICEMANAGER_OK;
}

int32_t SoftbusSession::OnSessionOpened(int32_t sessionId, int32_t result)
{
    if (result != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_INFO, "session open failed, sessionId %d", sessionId);
        if (sessionIdSet_.count(sessionId) > 0) {
            sessionIdSet_.erase(sessionId);
            messages_.clear();
        }
        return DEVICEMANAGER_OK;
    }

    int32_t sessionSide = GetSessionSide(sessionId);
    DMLOG(DM_LOG_INFO, "session open succeed, sessionId %d, sessionSide %d", sessionId, sessionSide);
    if (sessionSide == AUTH_SESSION_SIDE_CLIENT) {
        for (auto msg : messages_) {
            int32_t ret = SendMsg(sessionId, msg);
            if (ret != DEVICEMANAGER_OK) {
                DMLOG(DM_LOG_INFO, "send message failed");
                return ret;
            }
        }
    } else {
        sessionIdSet_.insert(sessionId);
    }
    return DEVICEMANAGER_OK;
}

void SoftbusSession::OnSessionClosed(int32_t sessionId)
{
    DMLOG(DM_LOG_INFO, "OnSessionClosed, sessionId:%d", sessionId);
    if (sessionIdSet_.count(sessionId) > 0) {
        sessionIdSet_.erase(sessionId);
    }
}

void SoftbusSession::OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DMLOG(DM_LOG_INFO, "OnBytesReceived, sessionId:%d, dataLen:%d", sessionId, dataLen);
    if (sessionId < 0 || data == nullptr || dataLen <= 0) {
        DMLOG(DM_LOG_INFO, "OnBytesReceived param check failed");
        return;
    }

    uint8_t *buf = (uint8_t *)calloc(sizeof(uint8_t), dataLen + 1);
    if (buf == nullptr) {
        DMLOG(DM_LOG_ERROR, "SendMsg: malloc memory failed");
        return;
    }

    int32_t outLen = 0;
    int32_t ret = EncryptUtils::MbedtlsDecrypt((const uint8_t*)data, dataLen, buf, dataLen, &outLen);
    if (ret != DEVICEMANAGER_OK || outLen > (int32_t)dataLen) {
        DMLOG(DM_LOG_ERROR, "MbedtlsDecrypt data failed");
        free(buf);
        return;
    }

    std::string message = (char *)buf;
    AuthManager::GetInstance().OnReceiveMsg(sessionId, message);
    free(buf);
    DMLOG(DM_LOG_INFO, "OnBytesReceived completed");
    return;
}

void SoftbusSession::CloseSession(int32_t sessionId)
{
    DMLOG(DM_LOG_INFO, "CloseSession in");
    ::CloseSession(sessionId);
}

int32_t SoftbusSession::SendData(int32_t sessionId, const void *data, int32_t len)
{
    DMLOG(DM_LOG_INFO, "in, datalen:%d", len);
    int32_t ret = DEVICEMANAGER_FAILED;
    if (sessionIdSet_.count(sessionId) > 0) {
        ret = SendBytes(sessionId, data, len);
        if (ret != DEVICEMANAGER_OK) {
            return DEVICEMANAGER_FAILED;
        }
    } else {
        DMLOG(DM_LOG_INFO, "in, datalen:%d", len);
    }
    return ret;
}

// send message by sessionId (channel opened)
int32_t SoftbusSession::SendMsg(int32_t sessionId, std::string &message)
{
    DMLOG(DM_LOG_ERROR, "start SendMsg");
    uint8_t *buf = (uint8_t *)calloc(sizeof(uint8_t), (MSG_MAX_SIZE + ENCRYPT_TAG_LEN));
    if (buf == nullptr) {
        DMLOG(DM_LOG_ERROR, "SendMsg: malloc memory failed");
        return DEVICEMANAGER_MALLOC_ERROR;
    }
    int32_t outLen = 0;
    int32_t ret = EncryptUtils::MbedtlsEncrypt((const uint8_t *)message.c_str(), message.size(), buf, MSG_MAX_SIZE,
        &outLen);
    if (ret != DEVICEMANAGER_OK || outLen > MSG_MAX_SIZE) {
        DMLOG(DM_LOG_ERROR, "MbedtlsEncrypt data failed");
        free(buf);
        return ENCRYPT_UTILS_AES_GCM_ENCRYPT_FAILED;
    }
    ret = SendData(sessionId, buf, outLen);
    free(buf);
    return ret;
}

// send message while the channel is not opend
int32_t SoftbusSession::SendMessages(const char *deviceId, std::vector<std::string> &message)
{
    DMLOG(DM_LOG_ERROR, "open channel and start SendMsg");
    int32_t sessionId = -1;
    messages_ = message;
    ConnectionAddr *addrInfo = SoftbusAdapter::GetConnectAddr(deviceId);
    if (addrInfo == nullptr) {
        DMLOG(DM_LOG_ERROR, "GetConnectAddr error");
        return sessionId;
    }

    sessionId = ::OpenAuthSession(SESSION_NAME, addrInfo, 1, nullptr);
    if (sessionId < 0) {
        DMLOG(DM_LOG_ERROR, "open session error, ret:%d", sessionId);
        return sessionId;
    }
    sessionIdSet_.insert(sessionId);
    DMLOG(DM_LOG_INFO, "opened auth session is:%d", sessionId);
    return sessionId;
}

void SoftbusSession::GetPeerDeviceId(int32_t sessionId, std::string &peerDevId)
{
    char peerDeviceId[DEVICE_UUID_LENGTH] = {0};
    int32_t ret = ::GetPeerDeviceId(sessionId, &peerDeviceId[0], DEVICE_UUID_LENGTH);
    if (ret == 0) {
        peerDevId = peerDeviceId;
        DMLOG(DM_LOG_INFO, "GetPeerDeviceId success for session:%d, peerDeviceId:%s", sessionId,
            GetAnonyString(peerDevId).c_str());
        return;
    }

    DMLOG(DM_LOG_ERROR, "GetPeerDeviceId failed for session:%d", sessionId);
    peerDevId = "";
}
} // namespace DistributedHardware
} // namespace OHOS

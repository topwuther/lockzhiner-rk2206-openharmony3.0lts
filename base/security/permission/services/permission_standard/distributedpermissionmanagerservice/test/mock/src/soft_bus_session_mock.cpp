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

#include "session.h"
#include "constant.h"
#include "soft_bus_manager.h"
#include "permission_log.h"

using namespace OHOS::Security::Permission;
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SoftBusSenssionMock"};
static const int SESSION_COUNT_LIMIT = 20;
static const int SERVER_COUNT_LIMIT = 10;
}  // namespace

#define MIN_(x, y) ((x) < (y)) ? (x) : (y)

static int serverCount_ = -1;
bool IsServerCountOK()
{
    return serverCount_ >= 0 && serverCount_ < SERVER_COUNT_LIMIT;
}

static ISessionListener *listener_ = nullptr;
int CreateSessionServer(const char *pkgName, const char *sessionName, const ISessionListener *listener)
{
    PERMISSION_LOG_DEBUG(LABEL, "pkg name: %{public}s", pkgName);
    serverCount_++;
    if (IsServerCountOK()) {
        listener_ = const_cast<ISessionListener *>(listener);
        PERMISSION_LOG_DEBUG(LABEL, "success, server count: %{public}d", serverCount_);
        return Constant::SUCCESS;
    }

    PERMISSION_LOG_DEBUG(LABEL, "failure, server count: %{public}d", serverCount_);
    return Constant::FAILURE;
}
int RemoveSessionServer(const char *pkgName, const char *sessionName)
{
    PERMISSION_LOG_DEBUG(LABEL, "pkg name: %{public}s", pkgName);
    if (IsServerCountOK()) {
        serverCount_--;
        listener_ = nullptr;
        PERMISSION_LOG_DEBUG(LABEL, "success, server count: %{public}d", serverCount_);
        return Constant::SUCCESS;
    }

    if (serverCount_ >= 0) {
        serverCount_--;
    }
    PERMISSION_LOG_DEBUG(LABEL, "failure, server count: %{public}d", serverCount_);
    return Constant::FAILURE;
}

static int sessionCount_ = -1;
bool IsSessionCountOK()
{
    return sessionCount_ >= 0 && sessionCount_ < SESSION_COUNT_LIMIT;
}

int OpenSession(const char *mySessionName, const char *peerSessionName, const char *peerDeviceId, const char *groupId,
    const SessionAttribute *attr)
{
    PERMISSION_LOG_DEBUG(LABEL, "mySessionName: %{public}s", mySessionName);
    PERMISSION_LOG_DEBUG(LABEL, "peerSessionName: %{public}s", peerSessionName);
    PERMISSION_LOG_DEBUG(LABEL, "peerDeviceId: %{public}s", peerDeviceId);
    PERMISSION_LOG_DEBUG(LABEL, "groupId: %{public}s", groupId);
    if (serverCount_ == -1) {
        PERMISSION_LOG_DEBUG(LABEL, "failure, server count: %{public}d", serverCount_);
        return Constant::INVALID_SESSION;
    }

    sessionCount_++;
    if (IsSessionCountOK()) {
        PERMISSION_LOG_DEBUG(LABEL, "success, session count: %{public}d", sessionCount_);
        return 1;
    }

    PERMISSION_LOG_DEBUG(LABEL, "failure, session count: %{public}d", sessionCount_);
    return Constant::INVALID_SESSION;
}
void CloseSession(int sessionId)
{
    PERMISSION_LOG_DEBUG(LABEL, "sessionId: %{public}d", sessionId);
    if (sessionCount_ >= 0) {
        sessionCount_--;
        PERMISSION_LOG_DEBUG(LABEL, "success, session count: %{public}d", sessionCount_);
    }
}

int SendBytes(int sessionId, const void *data, unsigned int len)
{
    PERMISSION_LOG_DEBUG(LABEL, "len: %{public}d", len);
    if (sessionId == Constant::INVALID_SESSION) {
        return Constant::FAILURE;
    }
    printf("\n[soft_bus_session] send data:: ");
    std::string s((char *)data);
    printf("%s", s.c_str());
    printf("\n\n");
    PERMISSION_LOG_DEBUG(LABEL, "success, session id: %{public}d", sessionId);
    return Constant::SUCCESS;
}

int GetPeerSessionName(int sessionId, char *sessionName, unsigned int len)
{
    if (sessionId == Constant::INVALID_SESSION) {
        PERMISSION_LOG_DEBUG(LABEL, "failure, session id: %{public}d", sessionId);
        return Constant::FAILURE;
    }
    std::string x;
    if (sessionId < SERVER_COUNT_LIMIT) {
        x = SoftBusManager::SESSION_NAME;
    } else {
        x = "sessionid-" + std::to_string(sessionId);
    }
    if (len < x.length()) {
        // PERMISSION_LOG_DEBUG(LABEL, "failure, buffer size too small, need: %{public}d", x.length());
        return Constant::FAILURE;
    }
    memcpy(sessionName, x.c_str(), x.length());
    sessionName[x.length()] = '\0';
    PERMISSION_LOG_DEBUG(LABEL, "success, session name: %{public}s", sessionName);

    return 0;
}

int GetPeerDeviceId(int sessionId, char *devId, unsigned int len)
{
    if (sessionId == Constant::INVALID_SESSION) {
        PERMISSION_LOG_DEBUG(LABEL, "failure, session id: %{public}d", sessionId);
        return Constant::FAILURE;
    }

    std::string x = "deviceid-" + std::to_string(sessionId);
    if (len < x.length()) {
        // PERMISSION_LOG_DEBUG(LABEL, "failure, buffer size too small, need: %{public}d", x.length());
        return Constant::FAILURE;
    }
    memcpy(devId, x.c_str(), x.length());
    devId[x.length()] = '\0';
    PERMISSION_LOG_DEBUG(LABEL, "success, device id: %{public}s", devId);
    return 0;
}
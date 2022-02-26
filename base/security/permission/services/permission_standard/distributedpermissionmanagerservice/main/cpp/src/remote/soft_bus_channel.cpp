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

#include "soft_bus_channel.h"
#include "soft_bus_manager.h"
#include "device_info_manager.h"

#include "distributed_permission_event_handler.h"
#include "distributed_permission_manager_service.h"
#include "singleton.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SoftBusChannel"};
}
namespace {
static const std::string REQUEST_TYPE = "request";
static const std::string RESPONSE_TYPE = "response";
static const std::string TASK_NAME_CLOSE_SESSION = "dpms_soft_bus_channel_close_session";
static const long EXECUTE_COMMAND_TIME_OUT = 3000;
static const long WAIT_SESSION_CLOSE_MILLISECONDS = 5 * 1000;
// send buf size for header
static const int RPC_TRANSFER_HEAD_BYTES_LENGTH = 1024 * 256;
// decompress buf size
static const int RPC_TRANSFER_BYTES_MAX_LENGTH = 1024 * 1024;
}  // namespace
SoftBusChannel::SoftBusChannel(const std::string &deviceId)
    : deviceId_(deviceId), mutex_(), callbacks_(), responseResult_(""), loadedCond_()
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel(deviceId)");
    isDelayClosing_ = false;
    session_ = Constant::INVALID_SESSION;
    isSessionUsing_ = false;
}

SoftBusChannel::~SoftBusChannel()
{
    PERMISSION_LOG_DEBUG(LABEL, "~SoftBusChannel()");
}

int SoftBusChannel::BuildConnection()
{
    CancelCloseConnectionIfNeeded();
    if (session_ != Constant::INVALID_SESSION) {
        PERMISSION_LOG_INFO(LABEL, "session is exist, no need open again.");
        return Constant::SUCCESS;
    }

    std::unique_lock<std::mutex> lock(sessionMutex_);
    if (session_ == Constant::INVALID_SESSION) {
        PERMISSION_LOG_INFO(LABEL, "open session with device: %{public}s", (deviceId_.c_str()));
        int session = SoftBusManager::GetInstance().OpenSession(deviceId_);
        if (session == Constant::INVALID_SESSION) {
            PERMISSION_LOG_ERROR(LABEL, "open session failed.");
            return Constant::FAILURE;
        }
        session_ = session;
    }
    return Constant::SUCCESS;
}

void SoftBusChannel::CloseConnection()
{
    PERMISSION_LOG_DEBUG(LABEL, "close connection");
    std::unique_lock<std::mutex> lock(mutex_);
    if (isDelayClosing_) {
        return;
    }

    std::shared_ptr<DistributedPermissionEventHandler> handler =
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->GetEventHandler();
    if (handler == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "fail to get EventHandler");
        return;
    }
    auto thisPtr = shared_from_this();
    std::function<void()> delayed = ([thisPtr]() {
        std::unique_lock<std::mutex> lock(thisPtr->sessionMutex_);
        if (thisPtr->isSessionUsing_) {
            PERMISSION_LOG_DEBUG(LABEL, "session is in using, cancel close session");
        } else {
            SoftBusManager::GetInstance().CloseSession(thisPtr->session_);
            thisPtr->session_ = Constant::INVALID_SESSION;
            PERMISSION_LOG_INFO(LABEL, "close session for device: %{public}s", thisPtr->deviceId_.c_str());
        }
        thisPtr->isDelayClosing_ = false;
    });

    PERMISSION_LOG_DEBUG(LABEL, "close session after %{public}ld ms", WAIT_SESSION_CLOSE_MILLISECONDS);
    handler->ProxyPostTask(delayed, TASK_NAME_CLOSE_SESSION, WAIT_SESSION_CLOSE_MILLISECONDS);

    isDelayClosing_ = true;
}

void SoftBusChannel::Release()
{
    std::shared_ptr<DistributedPermissionEventHandler> handler =
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->GetEventHandler();
    if (handler == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "fail to get EventHandler");
        return;
    }
    handler->ProxyRemoveTask(TASK_NAME_CLOSE_SESSION);
}

std::string SoftBusChannel::ExecuteCommand(const std::string &commandName, const std::string &jsonPayload)
{
    if (commandName.empty() || jsonPayload.empty()) {
        PERMISSION_LOG_ERROR(LABEL,
            "invalid params, commandName: %{public}s, jsonPayload: %{public}s",
            commandName.c_str(),
            jsonPayload.c_str());
        return "";
    }

    // to use a lib like libuuid
    char uuidbuf[37];  // 32+4+1
    random_uuid(uuidbuf);
    std::string uuid(uuidbuf);
    PERMISSION_LOG_DEBUG(LABEL, "generated message uuid: %{public}s", uuid.c_str());

    int len = RPC_TRANSFER_HEAD_BYTES_LENGTH + jsonPayload.length();
    unsigned char *buf = (unsigned char *)malloc(len + 1);
    memset(buf, 0, len + 1);
    if (buf == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "no enough memory: %{public}d", len);
        return "";
    }
    int result = PrepareBytes(REQUEST_TYPE, uuid, commandName, jsonPayload, buf, len);
    if (result != Constant::SUCCESS) {
        free(buf);
        return "";
    }

    std::unique_lock<std::mutex> lock(sessionMutex_);
    std::function<void(const std::string &)> callback = [&](const std::string &result) {
        PERMISSION_LOG_INFO(LABEL, "onResponse called, data: %{public}s", result.c_str());
        responseResult_ = std::string(result);
        loadedCond_.notify_all();
        PERMISSION_LOG_DEBUG(LABEL, "onResponse called end");
    };
    callbacks_.insert(std::pair<std::string, std::function<void(std::string)>>(uuid, callback));

    isSessionUsing_ = true;
    lock.unlock();

    PERMISSION_LOG_DEBUG(LABEL, "send command begin, len: %{public}d", len);
    int retCode = SendRequestBytes(buf, len);
    PERMISSION_LOG_DEBUG(LABEL, "send command end, len: %{public}d", len);
    free(buf);

    std::unique_lock<std::mutex> lock2(sessionMutex_);
    if (retCode != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL, "send request data failed: %{public}d ", retCode);
        callbacks_.erase(uuid);
        isSessionUsing_ = false;
        return "";
    }

    PERMISSION_LOG_DEBUG(LABEL, "wait command response");
    if (loadedCond_.wait_for(lock2, std::chrono::milliseconds(EXECUTE_COMMAND_TIME_OUT)) == std::cv_status::timeout) {
        PERMISSION_LOG_WARN(LABEL, "time out to wait response.");
        callbacks_.erase(uuid);
        isSessionUsing_ = false;
        return "";
    }

    isSessionUsing_ = false;
    return responseResult_;
}

void SoftBusChannel::HandleDataReceived(int session, const char unsigned *bytes, const int length)
{
    PERMISSION_LOG_DEBUG(LABEL, "HandleDataReceived");

    if (session <= 0 || length <= 0) {
        PERMISSION_LOG_ERROR(LABEL, "invalid params: session: %{public}d, data length: %{public}d", session, length);
        return;
    }
    std::string receiveData = Decompress(bytes, length);
    if (receiveData.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "invalid parameter bytes");
        return;
    }
    std::shared_ptr<SoftBusMessage> message = SoftBusMessage::FromJson(receiveData);
    if (message == nullptr) {
        PERMISSION_LOG_DEBUG(LABEL, "invalid json string: %{public}s", receiveData.c_str());
        return;
    }
    if (!message->IsValid()) {
        PERMISSION_LOG_DEBUG(LABEL, "invalid data, has empty field: %{public}s", receiveData.c_str());
        return;
    }

    std::string type = message->GetType();
    if (REQUEST_TYPE == (type)) {
        HandleRequest(session, message->GetId(), message->GetCommandName(), message->GetJsonPayload());
    } else if (RESPONSE_TYPE == (type)) {
        HandleResponse(message->GetId(), message->GetJsonPayload());
    } else {
        PERMISSION_LOG_ERROR(LABEL, "invalid type: %{public}s ", type.c_str());
    }
}

int SoftBusChannel::PrepareBytes(const std::string &type, const std::string &id, const std::string &commandName,
    const std::string &jsonPayload, const unsigned char *bytes, int &bytesLength)
{
    SoftBusMessage messageEntity(type, id, commandName, jsonPayload);
    std::string json = messageEntity.ToJson();
    PERMISSION_LOG_DEBUG(LABEL, "softbus message json: %{public}s", json.c_str());
    return Compress(json, bytes, bytesLength);
}

int SoftBusChannel::Compress(const std::string &json, const unsigned char *compressedBytes, int &compressedLength)
{
    uLong len = compressBound(json.size());
    // length will not so that long
    if (compressedLength > 0 && (int)len > compressedLength) {
        PERMISSION_LOG_ERROR(LABEL,
            "compress error. data length overflow, bound length: %{public}d, buffer length: %{public}d",
            (int)len,
            compressedLength);
        return Constant::FAILURE;
    }

    int result = compress((Byte *)compressedBytes, &len, (unsigned char *)json.c_str(), json.size() + 1);
    if (result != Z_OK) {
        PERMISSION_LOG_ERROR(LABEL, "compress failed! error code: %{public}d", result);
        return result;
    }
    PERMISSION_LOG_DEBUG(
        LABEL, "compress complete. compress %{public}d bytes to %{public}d", compressedLength, (int)len);
    compressedLength = len;
    return Constant::SUCCESS;
}

std::string SoftBusChannel::Decompress(const unsigned char *bytes, const int length)
{
    PERMISSION_LOG_DEBUG(LABEL, "input length: %{public}d", length);
    uLong len = RPC_TRANSFER_BYTES_MAX_LENGTH;
    unsigned char *buf = (unsigned char *)malloc(len + 1);
    memset(buf, 0, len + 1);
    if (buf == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "no enough memory!");
        return "";
    }
    int result = uncompress(buf, &len, (unsigned char *)bytes, length);
    if (result != Z_OK) {
        PERMISSION_LOG_ERROR(LABEL,
            "uncompress failed, error code: %{public}d, bound length: %{public}d, buffer length: %{public}d",
            result,
            (int)len,
            length);
        return "";
    }
    buf[len] = '\0';
    std::string str((char *)buf);
    free(buf);
    PERMISSION_LOG_DEBUG(LABEL, "done, output: %{public}s", str.c_str());
    return str;
}

int SoftBusChannel::SendRequestBytes(const unsigned char *bytes, const int bytesLength)
{
    if (bytesLength == 0) {
        PERMISSION_LOG_ERROR(LABEL, "bytes data is invalid.");
        return Constant::FAILURE;
    }

    std::unique_lock<std::mutex> lock(sessionMutex_);
    if (CheckSessionMayReopenLocked() != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL, "session invalid and reopen failed!");
        return Constant::FAILURE;
    }

    PERMISSION_LOG_DEBUG(LABEL, "send len (after compress len)= %{public}d", bytesLength);
    int result = ::SendBytes(session_, bytes, bytesLength);
    if (result != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL, "fail to send! result= %{public}d", result);
        return Constant::FAILURE;
    }
    PERMISSION_LOG_DEBUG(LABEL, "send successfully.");
    return Constant::SUCCESS;
}

int SoftBusChannel::CheckSessionMayReopenLocked()
{
    // when session is opened, we got a valid sessionid, when session closed, we will reset sessionid.
    if (IsSessionAvailable()) {
        return Constant::SUCCESS;
    }
    int session = SoftBusManager::GetInstance().OpenSession(deviceId_);
    if (session != Constant::INVALID_SESSION) {
        session_ = session;
        return Constant::SUCCESS;
    }
    return Constant::FAILURE;
}

bool SoftBusChannel::IsSessionAvailable()
{
    if (session_ == Constant::INVALID_SESSION) {
        return false;
    }
    return session_ > Constant::INVALID_SESSION;
}

void SoftBusChannel::CancelCloseConnectionIfNeeded()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (!isDelayClosing_) {
        return;
    }
    PERMISSION_LOG_DEBUG(LABEL, "cancel close connection");

    Release();
    isDelayClosing_ = false;
}

void SoftBusChannel::HandleRequest(
    int session, const std::string &id, const std::string &commandName, const std::string &jsonPayload)
{
    std::shared_ptr<BaseRemoteCommand> command =
        RemoteCommandFactory::GetInstance().NewRemoteCommandFromJson(commandName, jsonPayload);
    if (command == nullptr) {
        // send result back directly
        PERMISSION_LOG_WARN(
            LABEL, "command %{public}s cannot get from json %{public}s", commandName.c_str(), jsonPayload.c_str());

        int sendlen = RPC_TRANSFER_HEAD_BYTES_LENGTH + jsonPayload.length();
        unsigned char *sendbuf = (unsigned char *)malloc(sendlen + 1);
        memset(sendbuf, 0, sendlen + 1);
        if (sendbuf == nullptr) {
            PERMISSION_LOG_ERROR(LABEL, "no enough memory: %{public}d", sendlen);
            return;
        }
        int sendResult = PrepareBytes(RESPONSE_TYPE, id, commandName, jsonPayload, sendbuf, sendlen);
        if (sendResult != Constant::SUCCESS) {
            free(sendbuf);
            return;
        }
        int sendResultCode = SendResponseBytes(session, sendbuf, sendlen);
        free(sendbuf);
        PERMISSION_LOG_DEBUG(LABEL, "send response result= %{public}d ", sendResultCode);
        return;
    }

    // execute command
    command->Execute();
    PERMISSION_LOG_DEBUG(LABEL,
        "command uniqueId: %{public}s, finish with status: %{public}d, message: %{public}s",
        command->remoteProtocol_.uniqueId.c_str(),
        command->remoteProtocol_.statusCode,
        command->remoteProtocol_.message.c_str());

    // send result back
    std::string resultJsonPayload = command->ToJsonPayload();
    int len = RPC_TRANSFER_HEAD_BYTES_LENGTH + resultJsonPayload.length();
    unsigned char *buf = (unsigned char *)malloc(len + 1);
    memset(buf, 0, len + 1);
    if (buf == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "no enough memory: %{public}d", len);
        return;
    }
    int result = PrepareBytes(RESPONSE_TYPE, id, commandName, resultJsonPayload, buf, len);
    if (result != Constant::SUCCESS) {
        free(buf);
        return;
    }
    int retCode = SendResponseBytes(session, buf, len);
    free(buf);
    PERMISSION_LOG_DEBUG(LABEL, "send response result= %{public}d", retCode);
}

void SoftBusChannel::HandleResponse(const std::string &id, const std::string &jsonPayload)
{
    // PERMISSION_LOG_DEBUG(LABEL,
    //     "notify response data back for message id: %{public}s, callback size: %{public}d",
    //     id.c_str(),
    //     callbacks_.size());

    std::unique_lock<std::mutex> lock(sessionMutex_);
    auto callback = callbacks_.find(id);
    if (callback != callbacks_.end()) {
        (callback->second)(jsonPayload);
        callbacks_.erase(callback);
    }
}

int SoftBusChannel::SendResponseBytes(int session, const unsigned char *bytes, const int bytesLength)
{
    PERMISSION_LOG_DEBUG(LABEL, "send len (after compress len)= %{public}d", bytesLength);
    int result = ::SendBytes(session, bytes, bytesLength);
    if (result != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL, "fail to send! result= %{public}d", result);
        return Constant::FAILURE;
    }
    PERMISSION_LOG_DEBUG(LABEL, "send successfully.");
    return Constant::SUCCESS;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

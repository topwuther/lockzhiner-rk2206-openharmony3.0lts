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

#include "update_client.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <vector>

#include "iupdate_service.h"
#include "misc_info/misc_info.h"
#include "node_api.h"
#include "node_api_types.h"
#include "package/package.h"
#include "securec.h"
#include "update_helper.h"
#include "update_service_kits.h"
#include "update_session.h"

using namespace std;
using namespace OHOS::update_engine;

namespace updateClient {
const int32_t MAX_ARGC = 3;
const int32_t DEVICE_ID_INDEX = 1;
const int32_t MID_ARGC = 2;
const int32_t CLIENT_STRING_MAX_LENGTH = 200;
constexpr int PROGRESS_DOWNLOAD_FINISH = 100;
const std::string MISC_FILE = "/dev/block/platform/soc/10100000.himci.eMMC/by-name/misc";
const std::string CMD_WIPE_DATA = "--user_wipe_data";
const std::string UPDATER_PKG_NAME = "/data/updater/updater.zip";

UpdateClient::UpdateClient(napi_env env, napi_value thisVar)
{
    thisReference_ = nullptr;
    env_ = env;
    napi_create_reference(env, thisVar, 1, &thisReference_);
    context_.type = "OTA";
    context_.upgradeDevId = "local";
    context_.controlDevId = "local";
    context_.upgradeApp = "updateclient";
    context_.upgradeFile = UPDATER_PKG_NAME;
}

UpdateClient::~UpdateClient()
{
    sessions_.clear();
    if (thisReference_ != nullptr) {
        napi_delete_reference(env_, thisReference_);
    }
}

UpdateSession *UpdateClient::RemoveSession(uint32_t sessionId)
{
    CLIENT_LOGI("RemoveSession sess");
#ifndef UPDATER_API_TEST
    std::lock_guard<std::mutex> guard(sessionMutex_);
#endif
    UpdateSession *sess = nullptr;
    auto iter = sessions_.find(sessionId);
    if (iter != sessions_.end()) {
        sess = iter->second.get();
        sessions_.erase(iter);
    }
    return sess;
}

void UpdateClient::AddSession(std::shared_ptr<UpdateSession> session)
{
    CLIENT_CHECK(session != nullptr, return, "Invalid param");
#ifndef UPDATER_API_TEST
    std::lock_guard<std::mutex> guard(sessionMutex_);
#endif
    sessions_.insert(make_pair(session->GetSessionId(), session));
}

napi_value UpdateClient::GetUpdaterForOther(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARGC;
    napi_value args[MAX_ARGC] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Error get cb info");
    CLIENT_CHECK_NAPI_CALL(env, argc >= MID_ARGC, return nullptr, "Invalid param");

    // Get the devid.
    int ret = GetStringValue(env, args[1], context_.upgradeDevId);
    CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok, return nullptr, "Error get type");
    return GetUpdater(env, info, DEVICE_ID_INDEX + 1);
}

napi_value UpdateClient::GetUpdaterFromOther(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARGC;
    napi_value args[MAX_ARGC] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Error get cb info");
    CLIENT_CHECK_NAPI_CALL(env, argc >= MID_ARGC, return nullptr, "Invalid param");

    // Get the devid.
    int ret = GetStringValue(env, args[1], context_.controlDevId);
    CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok, return nullptr, "Error get type");
    return GetUpdater(env, info, DEVICE_ID_INDEX + 1);
}

bool UpdateClient::CheckUpgradeType(const std::string &type)
{
    std::vector<std::string> upgradeTypes = {
        "ota", "patch"
    };
    std::string upgradeType = type;
    upgradeType.erase(0, upgradeType.find_first_not_of(" "));
    upgradeType.erase(upgradeType.find_last_not_of(" ") + 1);
    std::transform(upgradeType.begin(), upgradeType.end(), upgradeType.begin(), ::tolower);
    for (auto inter = upgradeTypes.cbegin(); inter != upgradeTypes.cend(); inter++) {
        if ((*inter).compare(upgradeType) == 0) {
            return true;
        }
    }
    return false;
}

bool UpdateClient::CheckUpgradeFile(const std::string &upgradeFile)
{
    if (upgradeFile.empty()) {
        return false;
    }
    std::string file = upgradeFile;
    file.erase(0, file.find_first_not_of(" "));
    file.erase(file.find_last_not_of(" ") + 1);
    int32_t pos = file.find_first_of('/');
    if (pos != 0) {
        return false;
    }
    pos = file.find_last_of('.');
    if (pos < 0) {
        return false;
    }
    std::string postfix = file.substr(pos + 1, -1);
    std::transform(postfix.begin(), postfix.end(), postfix.begin(), ::tolower);
    if (postfix.compare("bin") == 0) {
        return true;
    } else if (postfix.compare("zip") == 0) {
        return true;
    } else if (postfix.compare("lz4") == 0) {
        return true;
    } else if (postfix.compare("gz") == 0) {
        return true;
    }
    return false;
}

napi_value UpdateClient::GetUpdater(napi_env env, napi_callback_info info, int32_t typeIndex)
{
    napi_value result;
    napi_create_int32(env, 0, &result);
    size_t argc = MAX_ARGC;
    napi_value args[MAX_ARGC] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Error get cb info");
    CLIENT_CHECK_NAPI_CALL(env, argc >= 1, return nullptr, "Invalid param");
    CLIENT_CHECK_NAPI_CALL(env, !isInit, return result, "Has beed init");

    int ret = GetStringValue(env, args[0], context_.upgradeFile);
    CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok && CheckUpgradeFile(context_.upgradeFile),
        return nullptr, "Invalid upgradeFile");
    ret = GetStringValue(env, args[typeIndex], context_.type);
    if (ret == napi_ok) {
        CLIENT_CHECK_NAPI_CALL(env, CheckUpgradeType(context_.type), return nullptr, "Error get upgradeType");
    } else {
        context_.type = "OTA";
    }
    CLIENT_LOGE("GetUpdater argc %s", context_.type.c_str());
    UpdateCallbackInfo callback {
        [&](const VersionInfo &info) {
            this->NotifyCheckVersionDone(info);
        },
        [&](const Progress &info) {
            this->NotifyDownloadProgress(info);
        },
        [&](const Progress &info) {
            this->NotifyUpgradeProgresss(info);
        },
    };
    UpdateServiceKits::GetInstance().RegisterUpdateCallback(context_, callback);
    isInit = true;
    return result;
}

napi_value UpdateClient::StartSession(napi_env env,
    napi_callback_info info, int32_t type, size_t callbackStartIndex, DoWorkFunction function)
{
    size_t argc = MAX_ARGC;
    napi_value args[MAX_ARGC] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Error get cb info");

    CLIENT_LOGE("StartSession type %d argc %zu callbackStartIndex %d", type, argc,
        static_cast<int>(callbackStartIndex));
    std::shared_ptr<UpdateSession> sess = nullptr;
    if (argc > callbackStartIndex) {
        sess = std::make_shared<UpdateAsyncession>(this, type, argc, 1);
    } else {
        sess = std::make_shared<UpdatePromiseSession>(this, type, argc, 0);
    }
    CLIENT_CHECK_NAPI_CALL(env, sess != nullptr, return nullptr, "Failed to create update session");
    AddSession(sess);
    napi_value retValue = sess->StartWork(env, callbackStartIndex, args, function, nullptr);
    CLIENT_CHECK(retValue != nullptr, RemoveSession(sess->GetSessionId()); return nullptr, "Failed to start worker.");

    return retValue;
}

napi_value UpdateClient::CheckNewVersion(napi_env env, napi_callback_info info)
{
    versionInfo_.status = SYSTEM_ERROR;
    napi_value ret = StartSession(env, info, SESSION_CHECK_VERSION, 0,
        [&](int32_t type, void *context) -> int {
            return UpdateServiceKits::GetInstance().CheckNewVersion();
        });
    CLIENT_CHECK(ret != nullptr, return nullptr, "Failed to start worker.");
    return ret;
}

napi_value UpdateClient::CancelUpgrade(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARGC;
    napi_value args[MAX_ARGC] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok && argc == 0, return nullptr, "Error get cb info");
    CLIENT_LOGE("CancelUpgrade");
    std::shared_ptr<UpdateSession> sess = nullptr;
    sess = std::make_shared<UpdateAsyncessionNoCallback>(this, SESSION_CANCEL_UPGRADE, argc, 0);
    CLIENT_CHECK_NAPI_CALL(env, sess != nullptr, return nullptr, "Failed to create update session");
    AddSession(sess);
    napi_value retValue = sess->StartWork(env, 0, args,
        [&](int32_t type, void *context) -> int {
            return UpdateServiceKits::GetInstance().Cancel(IUpdateService::DOWNLOAD);
        }, nullptr);
    CLIENT_CHECK(retValue != nullptr, RemoveSession(sess->GetSessionId()); return nullptr, "Failed to start worker.");
    return retValue;
}

napi_value UpdateClient::DownloadVersion(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARGC;
    napi_value args[MAX_ARGC] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Error get cb info");
    CLIENT_LOGE("DownloadVersion");
    std::shared_ptr<UpdateSession> sess = nullptr;
    sess = std::make_shared<UpdateAsyncessionNoCallback>(this, SESSION_DOWNLOAD, argc, 0);
    CLIENT_CHECK_NAPI_CALL(env, sess != nullptr, return nullptr, "Failed to create update session");
    AddSession(sess);
    napi_value retValue = sess->StartWork(env, 0, args,
        [&](int32_t type, void *context) -> int {
            return UpdateServiceKits::GetInstance().DownloadVersion();
        }, nullptr);
    CLIENT_CHECK(retValue != nullptr, RemoveSession(sess->GetSessionId()); return nullptr, "Failed to start worker.");
    return retValue;
}

napi_value UpdateClient::UpgradeVersion(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARGC;
    napi_value args[MAX_ARGC] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Error get cb info");
    std::shared_ptr<UpdateSession> sess = nullptr;
    sess = std::make_shared<UpdateAsyncessionNoCallback>(this, SESSION_UPGRADE, argc, 0);
    CLIENT_CHECK_NAPI_CALL(env, sess != nullptr, return nullptr, "Failed to create update session");
    AddSession(sess);
    napi_value retValue = sess->StartWork(env, 0, args,
        [&](int32_t type, void *context) -> int {
#ifndef UPDATER_API_TEST
            return UpdateServiceKits::GetInstance().DoUpdate();
#else
            return 0;
#endif
        }, nullptr);
    CLIENT_CHECK(retValue != nullptr, RemoveSession(sess->GetSessionId()); return nullptr, "Failed to start worker.");
    return retValue;
}

napi_value UpdateClient::SetUpdatePolicy(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARGC;
    napi_value args[MAX_ARGC] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Error get cb info");

    int ret = GetUpdatePolicyFromArg(env, args[0], updatePolicy_);
    CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok, return nullptr, "Failed to get policy para");

    std::shared_ptr<UpdateSession> sess = nullptr;
    if (argc >= MID_ARGC) {
        sess = std::make_shared<UpdateAsyncession>(this, SESSION_SET_POLICY, argc, 1);
    } else {
        sess = std::make_shared<UpdatePromiseSession>(this, SESSION_SET_POLICY, argc, 0);
    }
    CLIENT_CHECK_NAPI_CALL(env, sess != nullptr, return nullptr, "Failed to create update session");
    AddSession(sess);
    napi_value retValue = sess->StartWork(env, 1, args,
        [&](int32_t type, void *context) -> int {
            result_ = UpdateServiceKits::GetInstance().SetUpdatePolicy(updatePolicy_);
            return result_;
        }, nullptr);
    CLIENT_CHECK(retValue != nullptr, RemoveSession(sess->GetSessionId());
        return nullptr, "Failed to SetUpdatePolicy.");
    return retValue;
}

napi_value UpdateClient::GetUpdatePolicy(napi_env env, napi_callback_info info)
{
    napi_value retValue = StartSession(env, info, SESSION_GET_POLICY, 0,
        [&](int32_t type, void *context) -> int {
            return UpdateServiceKits::GetInstance().GetUpdatePolicy(updatePolicy_);
        });
    CLIENT_CHECK(retValue != nullptr, return nullptr, "Failed to UpgradeVersion.");
    return retValue;
}

napi_value UpdateClient::GetNewVersionInfo(napi_env env, napi_callback_info info)
{
    napi_value retValue = StartSession(env, info, SESSION_GET_NEW_VERSION, 0,
        [&](int32_t type, void *context) -> int {
            return UpdateServiceKits::GetInstance().GetNewVersion(versionInfo_);
        });
    CLIENT_CHECK(retValue != nullptr, return nullptr, "Failed to GetNewVersionInfo.");
    return retValue;
}

napi_value UpdateClient::GetUpgradeStatus(napi_env env, napi_callback_info info)
{
    napi_value retValue = StartSession(env, info, SESSION_GET_STATUS, 0,
        [&](int32_t type, void *context) -> int {
            return UpdateServiceKits::GetInstance().GetUpgradeStatus(upgradeInfo_);
        });
    CLIENT_CHECK(retValue != nullptr, return nullptr, "Failed to GetUpgradeStatus.");
    return retValue;
}

napi_value UpdateClient::ApplyNewVersion(napi_env env, napi_callback_info info)
{
    napi_value retValue = StartSession(env, info, SESSION_APPLY_NEW_VERSION, 0,
        [&](int32_t type, void *context) -> int {
#ifndef UPDATER_API_TEST
            result_ = UpdateServiceKits::GetInstance().RebootAndInstall(MISC_FILE, UPDATER_PKG_NAME);
#endif
            return result_;
        });
    CLIENT_CHECK(retValue != nullptr, return nullptr, "Failed to GetNewVersionInfo.");
    return retValue;
}

napi_value UpdateClient::RebootAndClean(napi_env env, napi_callback_info info)
{
    napi_value retValue = StartSession(env, info, SESSION_REBOOT_AND_CLEAN, 0,
        [&](int32_t type, void *context) -> int {
#ifndef UPDATER_API_TEST
            result_ = UpdateServiceKits::GetInstance().RebootAndClean(MISC_FILE, CMD_WIPE_DATA);
#endif
            return result_;
        });
    CLIENT_CHECK(retValue != nullptr, return nullptr, "Failed to GetNewVersionInfo.");
    return retValue;
}

napi_value UpdateClient::VerifyUpdatePackage(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARGC;
    napi_value args[MAX_ARGC] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Error get cb info");
    CLIENT_CHECK_NAPI_CALL(env, argc >= MID_ARGC, return nullptr, "Error get cb info");

    int ret = GetStringValue(env, args[0], upgradeFile_);
    CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok, return nullptr, "Error get upgradeType");
    ret = GetStringValue(env, args[1], certsFile_);
    CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok, return nullptr, "Error get certsFile");

    CLIENT_LOGE("VerifyUpdatePackage");
    std::shared_ptr<UpdateSession> sess = nullptr;
    sess = std::make_shared<UpdateAsyncessionNoCallback>(this, SESSION_VERIFY_PACKAGE, argc, 0);
    CLIENT_CHECK_NAPI_CALL(env, sess != nullptr, return nullptr, "Fail to create update session");
    AddSession(sess);
    size_t startIndex = 2;
    napi_value retValue = sess->StartWork(env, startIndex, args,
        [&](int32_t type, void *context) -> int {
            CLIENT_LOGE("StartWork VerifyUpdatePackage");
            result_ = VerifyPackageWithCallback(upgradeFile_, certsFile_,
                [&](int32_t result, uint32_t percent) { NotifyVerifyProgresss(result, percent); });
            return result_;
        },
        nullptr);
    CLIENT_CHECK(retValue != nullptr, RemoveSession(sess->GetSessionId()); return nullptr, "Failed to start worker.");
    return retValue;
}

napi_value UpdateClient::SubscribeEvent(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARGC;
    napi_value args[MAX_ARGC] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Error get cb info");
    // Two arguments are required: type + callback.
    CLIENT_CHECK_NAPI_CALL(env, argc >= MID_ARGC, return nullptr, "Invalid param");

    std::string eventType;
    int ret = UpdateClient::GetStringValue(env, args[0], eventType);
    CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok, return nullptr, "Failed to get event type");
    CLIENT_CHECK(FindSessionByHandle(env, eventType, args[1]) == nullptr, return nullptr, "Handle has been sub");

    std::shared_ptr<UpdateSession> sess = std::make_shared<UpdateListener>(this, SESSION_SUBSCRIBE, argc, 1, false);
    CLIENT_CHECK_NAPI_CALL(env, sess != nullptr, return nullptr, "Failed to create listener");
    AddSession(sess);
    napi_value retValue = sess->StartWork(env, 1, args,
        [&](int32_t type, void *context) -> int {
            return 0;
        }, nullptr);
    CLIENT_CHECK(retValue != nullptr, RemoveSession(sess->GetSessionId()); return nullptr, "Failed to SubscribeEvent.");
    return retValue;
}

napi_value UpdateClient::UnsubscribeEvent(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_ARGC;
    napi_value args[MAX_ARGC] = {0};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Error get cb info");

    std::string eventType;
    int ret = UpdateClient::GetStringValue(env, args[0], eventType);
    CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok, return nullptr, "Failed to get event type");

    CLIENT_LOGI("UnsubscribeEvent %s argc %d", eventType.c_str(), static_cast<int>(argc));
    if (argc >= MID_ARGC) {
        napi_valuetype valuetype;
        napi_status status = napi_typeof(env, args[1], &valuetype);
        CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Failed to napi_typeof");
        CLIENT_CHECK_NAPI_CALL(env, valuetype == napi_function, return nullptr, "Invalid callback type");
    }
    ret = ProcessUnsubscribe(eventType, argc, args[1]);
    napi_value result;
    napi_create_int32(env, ret, &result);
    return result;
}

int32_t UpdateClient::ProcessUnsubscribe(const std::string &eventType, size_t argc, napi_value arg)
{
    napi_handle_scope scope;
    napi_status status = napi_open_handle_scope(env_, &scope);
    CLIENT_CHECK(status == napi_ok, return -1, "Error open handle");

    uint32_t nextSessId = 0;
    bool hasNext = GetFirstSessionId(nextSessId);
    while (hasNext) {
        uint32_t currSessId = nextSessId;
        auto iter = sessions_.find(currSessId);
        if (iter == sessions_.end()) {
            break;
        }
        hasNext = GetNextSessionId(nextSessId);

        UpdateListener *listener = static_cast<UpdateListener *>(iter->second.get());
        if (listener->GetType() != SESSION_SUBSCRIBE
            || eventType.compare(listener->GetEventType()) != 0) {
            continue;
        }
        CLIENT_LOGI("ProcessUnsubscribe remove session");
        if (argc == 1) {
            listener->RemoveHandlerRef(env_);
            RemoveSession(currSessId);
        } else if (listener->CheckEqual(env_, arg, eventType)) {
            listener->RemoveHandlerRef(env_);
            RemoveSession(currSessId);
            break;
        }
    }
    napi_close_handle_scope(env_, scope);
    return 0;
}

UpdateSession *UpdateClient::FindSessionByHandle(napi_env env, const std::string &eventType, napi_value arg)
{
    uint32_t nextSessId = 0;
    bool hasNext = GetFirstSessionId(nextSessId);
    while (hasNext) {
        uint32_t currSessId = nextSessId;
        auto iter = sessions_.find(currSessId);
        if (iter == sessions_.end()) {
            break;
        }
        hasNext = GetNextSessionId(nextSessId);

        UpdateListener *listener = static_cast<UpdateListener *>(iter->second.get());
        if (listener->GetType() != SESSION_SUBSCRIBE) {
            continue;
        }
        if ((eventType.compare(listener->GetEventType()) == 0) && listener->CheckEqual(env_, arg, eventType)) {
            return listener;
        }
    }
    return nullptr;
}

bool UpdateClient::GetNextSessionId(uint32_t &sessionId)
{
#ifndef UPDATER_API_TEST
    std::lock_guard<std::mutex> guard(sessionMutex_);
#endif    
    {
        auto iter = sessions_.find(sessionId);
        if (iter == sessions_.end()) {
            return false;
        }
        iter++;
        if (iter == sessions_.end()) {
            return false;
        }
        sessionId = iter->second->GetSessionId();
    }
    return true;
}

bool UpdateClient::GetFirstSessionId(uint32_t &sessionId)
{
#ifndef UPDATER_API_TEST
    std::lock_guard<std::mutex> guard(sessionMutex_);
#endif
    {
        if (sessions_.empty()) {
            return false;
        }
        sessionId = sessions_.begin()->second->GetSessionId();
        return true;
    }
}

void UpdateClient::Emit(const std::string &type, int32_t retcode, const UpdateResult &result)
{
    napi_handle_scope scope;
    napi_status status = napi_open_handle_scope(env_, &scope);
    CLIENT_CHECK_NAPI_CALL(env_, status == napi_ok, return, "Error open_handle_scope");
    napi_value thisVar = nullptr;
    status = napi_get_reference_value(env_, thisReference_, &thisVar);
    CLIENT_CHECK_NAPI_CALL(env_, status == napi_ok, return, "Error get_reference");

    uint32_t nextSessId = 0;
    bool hasNext = GetFirstSessionId(nextSessId);
    while (hasNext) {
        uint32_t currSessId = nextSessId;
        auto iter = sessions_.find(currSessId);
        if (iter == sessions_.end()) {
            break;
        }
        hasNext = GetNextSessionId(nextSessId);
        UpdateListener *listener = static_cast<UpdateListener *>((iter->second).get());
        if ((listener->GetType() != SESSION_SUBSCRIBE) || (type.compare(listener->GetEventType()) != 0)) {
            continue;
        }
        listener->NotifyJS(env_, thisVar, retcode, result);
        iter = sessions_.find(currSessId);
        if (iter == sessions_.end()) {
            continue;
        }
        listener = static_cast<UpdateListener *>((iter->second).get());
        if (listener->IsOnce()) {
            listener->RemoveHandlerRef(env_);
            RemoveSession(currSessId);
        }
    }
    napi_close_handle_scope(env_, scope);
}

void UpdateClient::NotifyDownloadProgress(const Progress &progress)
{
    CLIENT_LOGI("NotifyDownloadProgress status %d  %d", progress.status, progress.percent);
    if (progress.percent == PROGRESS_DOWNLOAD_FINISH && progress.status == UPDATE_STATE_DOWNLOAD_ON) {
        return;
    }
    progress_.percent = progress.percent;
    progress_.status = progress.status;
    progress_.endReason = progress.endReason;
    UpdateResult result;
    result.type = SESSION_DOWNLOAD;
    result.result.progress = &progress_;
    result.buildJSObject = BuildProgress;
    int32_t fail = (progress_.status == UPDATE_STATE_DOWNLOAD_FAIL || progress_.status == UPDATE_STATE_VERIFY_FAIL) ?
        progress_.status : 0;
    Emit("downloadProgress", fail, result);
}

void UpdateClient::NotifyUpgradeProgresss(const Progress &progress)
{
    CLIENT_LOGI("NotifyUpgradeProgresss status %d  %d", progress.status, progress.percent);
    progress_.percent = progress.percent;
    progress_.status = progress.status;

    UpdateResult result;
    result.type = SESSION_UPGRADE;
    result.result.progress = &progress_;
    result.buildJSObject = BuildProgress;
    int32_t fail = (progress_.status == UPDATE_STATE_DOWNLOAD_FAIL) ? progress_.status : 0;
    Emit("upgradeProgress", fail, result);
}

void UpdateClient::NotifyVerifyProgresss(int32_t retCode, uint32_t percent)
{
    verifyProgress_.status = (retCode == 0) ? UPDATE_STATE_VERIFY_SUCCESS : UPDATE_STATE_VERIFY_FAIL;
    verifyProgress_.percent = percent;

    UpdateResult result;
    result.type = SESSION_VERIFY_PACKAGE;
    result.result.progress = &verifyProgress_;
    result.buildJSObject = BuildProgress;
    Emit("verifyProgress", retCode, result);
}

void UpdateClient::NotifyCheckVersionDone(const VersionInfo &info)
{
    CLIENT_LOGE("NotifyCheckVersionDone status %d", info.status);
    CLIENT_LOGE("NotifyCheckVersionDone errMsg %s", info.errMsg.c_str());
    CLIENT_LOGE("NotifyCheckVersionDone versionName : %s", info.result[0].versionName.c_str());
    CLIENT_LOGE("NotifyCheckVersionDone versionCode : %s", info.result[0].versionCode.c_str());
    CLIENT_LOGE("NotifyCheckVersionDone verifyInfo : %s", info.result[0].verifyInfo.c_str());
    CLIENT_LOGE("NotifyCheckVersionDone size : %zu", info.result[0].size);
    CLIENT_LOGE("NotifyCheckVersionDone content : %s", info.descriptInfo[0].content.c_str());

    UpdateHelper::CopyVersionInfo(info, versionInfo_);
}

int32_t UpdateClient::GetInt32(napi_env env, napi_value arg, const std::string &attrName, int32_t &intValue)
{
    bool result = false;
    napi_status status = napi_has_named_property(env, arg, attrName.c_str(), &result);
    if (result && (status == napi_ok)) {
        napi_value value = nullptr;
        napi_get_named_property(env, arg, attrName.c_str(), &value);
        napi_get_value_int32(env, value, &intValue);
    }
    return CLIENT_SUCCESS;
}

int32_t UpdateClient::GetBool(napi_env env, napi_value arg, const std::string &attrName, bool &value)
{
    bool result = false;
    napi_status status = napi_has_named_property(env, arg, attrName.c_str(), &result);
    if (result && (status == napi_ok)) {
        napi_value obj = nullptr;
        napi_get_named_property(env, arg, attrName.c_str(), &obj);
        napi_get_value_bool(env, obj, &value);
    }
    return CLIENT_SUCCESS;
}

int32_t UpdateClient::GetStringValue(napi_env env, napi_value arg, std::string &strValue)
{
    napi_valuetype valuetype;
    napi_status status = napi_typeof(env, arg, &valuetype);
    CLIENT_CHECK(status == napi_ok, return status, "Failed to napi_typeof");
    CLIENT_CHECK(valuetype == napi_string, return CLIENT_INVALID_TYPE, "Invalid type");
    std::vector<char> buff(CLIENT_STRING_MAX_LENGTH);
    size_t copied;
    status = napi_get_value_string_utf8(env, arg, (char*)buff.data(), CLIENT_STRING_MAX_LENGTH, &copied);
    CLIENT_CHECK(status == napi_ok, return CLIENT_INVALID_TYPE, "Error get string");
    strValue.assign(buff.data(), copied);
    return napi_ok;
}

int32_t UpdateClient::SetString(napi_env env, napi_value arg, const std::string &attrName, const std::string &string)
{
    napi_value value = nullptr;
    napi_create_string_utf8(env, string.c_str(), string.length(), &value);
    napi_set_named_property(env, arg, attrName.c_str(), value);
    return CLIENT_SUCCESS;
}

int32_t UpdateClient::SetInt32(napi_env env, napi_value arg, const std::string &attrName, int32_t intValue)
{
    napi_value infoStatus = nullptr;
    napi_create_int32(env, intValue, &infoStatus);
    napi_set_named_property(env, arg, attrName.c_str(), infoStatus);
    return CLIENT_SUCCESS;
}

int32_t UpdateClient::SetBool(napi_env env, napi_value arg, const std::string &attrName, bool value)
{
    napi_value infoStatus = nullptr;
    napi_create_int32(env, value, &infoStatus);
    napi_set_named_property(env, arg, attrName.c_str(), infoStatus);
    return CLIENT_SUCCESS;
}

int32_t UpdateClient::SetInt64(napi_env env, napi_value arg, const std::string &attrName, int64_t intValue)
{
    napi_value infoStatus = nullptr;
    napi_create_int64(env, intValue, &infoStatus);
    napi_set_named_property(env, arg, attrName.c_str(), infoStatus);
    return CLIENT_SUCCESS;
}

int32_t UpdateClient::GetUpdatePolicyFromArg(napi_env env,
    const napi_value arg, UpdatePolicy &updatePolicy) const
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, arg, &type);
    CLIENT_CHECK(status == napi_ok, return CLIENT_INVALID_TYPE, "Invlid argc %d", static_cast<int32_t>(status));
    CLIENT_CHECK(type == napi_object, return CLIENT_INVALID_TYPE, "Invlid argc %d", static_cast<int32_t>(type));

    // updatePolicy
    int32_t tmpValue = 0;
    int32_t ret = GetBool(env, arg, "autoDownload", updatePolicy.autoDownload);
    ret |= GetBool(env, arg, "autoDownloadNet", updatePolicy.autoDownloadNet);
    ret |= GetInt32(env, arg, "mode", tmpValue);
    updatePolicy.mode = static_cast<InstallMode>(tmpValue);
    CLIENT_CHECK(ret == 0, return CLIENT_INVALID_TYPE, "Failed to get attr ");

    // Get the array.
    bool result = false;
    status = napi_has_named_property(env, arg, "autoUpgradeInterval", &result);
    if (result && (status == napi_ok)) {
        napi_value value;
        status = napi_get_named_property(env, arg, "autoUpgradeInterval", &value);
        CLIENT_CHECK(status == napi_ok, return CLIENT_FAIL, "Failed to get attr autoUpgradeInterval");
        status = napi_is_array(env, value, &result);
        CLIENT_CHECK(status == napi_ok, return CLIENT_FAIL, "napi_is_array failed");
        uint32_t count = 0;
        status = napi_get_array_length(env, value, &count);
        CLIENT_CHECK(status == napi_ok, return CLIENT_FAIL, "napi_get_array_length failed");
        uint32_t i = 0;
        do {
            napi_value element;
            ret = napi_get_element(env, value, i, &element);
            ret = napi_get_value_uint32(env, element, &updatePolicy.autoUpgradeInterval[i]);
            CLIENT_LOGI("updatePolicy autoUpgradeInterval：%u ", updatePolicy.autoUpgradeInterval[i]);
            if (i >= sizeof(updatePolicy.autoUpgradeInterval) / sizeof(updatePolicy.autoUpgradeInterval[0])) {
                break;
            }
            i++;
        } while (i < count);
    }
    ret |= GetInt32(env, arg, "autoUpgradeCondition", tmpValue);
    CLIENT_CHECK(ret == 0, return CLIENT_INVALID_TYPE, "Failed to get attr autoUpgradeCondition");
    updatePolicy.autoUpgradeCondition = static_cast<AutoUpgradeCondition>(tmpValue);
    CLIENT_LOGI("updatePolicy autoDownload：%d autoDownloadNet:%d mode:%d autoUpgradeCondition:%d",
        static_cast<int32_t>(updatePolicy.autoDownload),
        static_cast<int32_t>(updatePolicy.autoDownloadNet),
        static_cast<int32_t>(updatePolicy.mode),
        static_cast<int32_t>(updatePolicy.autoUpgradeCondition));
    return CLIENT_SUCCESS;
}

int32_t UpdateClient::BuildCheckVersionResult(napi_env env, napi_value &obj, const UpdateResult &result)
{
    CLIENT_CHECK(result.type == SESSION_CHECK_VERSION || result.type == SESSION_GET_NEW_VERSION,
        return CLIENT_INVALID_TYPE, "invalid type %d", result.type);
    napi_status status = napi_create_object(env, &obj);
    CLIENT_CHECK(status == napi_ok, return CLIENT_INVALID_TYPE,
        "Failed to create napi_create_object %d", static_cast<int32_t>(status));
    VersionInfo *info = result.result.versionInfo;

    // Add the result.
    int32_t ret = SetInt32(env, obj, "status", info->status);
    if (info->status == SERVER_BUSY || info->status == SYSTEM_ERROR) {
        ret = SetString(env, obj, "errMsg", info->errMsg);
        return ret;
    }
    napi_value checkResults;
    napi_create_array_with_length(env, sizeof(info->result) / sizeof(info->result[0]), &checkResults);
    for (size_t i = 0; i < sizeof(info->result) / sizeof(info->result[0]); i++) {
        napi_value result;
        status = napi_create_object(env, &result);

        ret |= SetString(env, result, "versionName", info->result[i].versionName);
        ret |= SetString(env, result, "versionCode", info->result[i].versionCode);
        ret |= SetString(env, result, "verifyInfo", info->result[i].verifyInfo);
        ret |= SetString(env, result, "descriptionId", info->result[i].descriptPackageId);
        ret |= SetInt64(env, result, "size", info->result[i].size);
        ret |= SetInt32(env, result, "packageType", info->result[i].packageType);
        napi_set_element(env, checkResults, i, result);
    }
    napi_set_named_property(env, obj, "checkResults", checkResults);

    napi_value descriptInfos;
    napi_create_array_with_length(env, sizeof(info->descriptInfo) / sizeof(info->descriptInfo[0]), &descriptInfos);
    for (size_t i = 0; i < sizeof(info->descriptInfo) / sizeof(info->descriptInfo[0]); i++) {
        napi_value descriptInfo;
        status = napi_create_object(env, &descriptInfo);
        ret |= SetString(env, descriptInfo, "descriptionId", info->descriptInfo[i].descriptPackageId);
        ret |= SetString(env, descriptInfo, "content", info->descriptInfo[i].content);
        napi_set_element(env, descriptInfos, i, descriptInfo);
    }
    napi_set_named_property(env, obj, "descriptionInfo", descriptInfos);
    return CLIENT_SUCCESS;
}

int32_t UpdateClient::BuildProgress(napi_env env, napi_value &obj, const UpdateResult &result)
{
    napi_status status = napi_create_object(env, &obj);
    CLIENT_CHECK(status == napi_ok, return CLIENT_INVALID_TYPE,
        "Failed to create napi_create_object %d", static_cast<int32_t>(status));
    int32_t ret = SetInt32(env, obj, "status", result.result.progress->status);
    ret |= SetInt32(env, obj, "percent", result.result.progress->percent);
    ret |= SetString(env, obj, "endReason", result.result.progress->endReason);
    return CLIENT_SUCCESS;
}

int32_t UpdateClient::BuildErrorResult(napi_env env, napi_value &obj, int32_t result)
{
    napi_status status = napi_create_object(env, &obj);
    CLIENT_CHECK(status == napi_ok, return CLIENT_INVALID_TYPE,
        "Failed to create napi_create_object %d", static_cast<int32_t>(status));
    return SetInt32(env, obj, "code", result);
}

int32_t UpdateClient::BuildInt32Status(napi_env env, napi_value &obj, const UpdateResult &result)
{
    return napi_create_int32(env, result.result.status, &obj);
}

int32_t UpdateClient::BuildUpdatePolicy(napi_env env, napi_value &obj, const UpdateResult &result)
{
    CLIENT_CHECK(result.type == SESSION_GET_POLICY || result.type == SESSION_SET_POLICY,
        return CLIENT_INVALID_TYPE, "invalid type %d", result.type);
    napi_status status = napi_create_object(env, &obj);
    CLIENT_CHECK(status == napi_ok, return status, "Failed to create napi_create_object %d", status);
    UpdatePolicy &updatePolicy = *result.result.updatePolicy;

    // Add the result.
    int32_t ret = SetBool(env, obj, "autoDownload", updatePolicy.autoDownload);
    ret |= SetBool(env, obj, "autoDownloadNet", updatePolicy.autoDownloadNet);
    ret |= SetInt32(env, obj, "mode", static_cast<int32_t>(updatePolicy.mode));
    CLIENT_CHECK(ret == napi_ok, return ret, "Failed to add value %d", ret);

    napi_value autoUpgradeInterval;
    size_t count = sizeof(updatePolicy.autoUpgradeInterval) / sizeof(updatePolicy.autoUpgradeInterval[0]);
    status = napi_create_array_with_length(env, count, &autoUpgradeInterval);
    CLIENT_CHECK(status == napi_ok, return status, "Failed to create array for interval %d", status);
    for (size_t i = 0; i < count; i++) {
        napi_value interval;
        status = napi_create_uint32(env, updatePolicy.autoUpgradeInterval[i], &interval);
        status = napi_set_element(env, autoUpgradeInterval, i, interval);
        CLIENT_CHECK(status == napi_ok, return status, "Failed to add interval to array %d", status);
    }
    status = napi_set_named_property(env, obj, "autoUpgradeInterval", autoUpgradeInterval);
    CLIENT_CHECK(status == napi_ok, return status, "Failed to add autoUpgradeInterval %d", status);
    ret |= SetInt32(env, obj, "autoUpgradeCondition", static_cast<int32_t>(updatePolicy.autoUpgradeCondition));
    CLIENT_CHECK(ret == napi_ok, return ret, "Failed to add autoUpgradeCondition %d", ret);
    return napi_ok;
}

int32_t UpdateClient::GetUpdateResult(int type, UpdateResult &result, int32_t &fail)
{
    fail = 0;
    result.type = type;
    switch (type) {
        case SESSION_CHECK_VERSION:
        case SESSION_GET_NEW_VERSION: {
            fail = (versionInfo_.status == SYSTEM_ERROR) ? versionInfo_.status : 0;
            result.result.versionInfo = &versionInfo_;
            result.buildJSObject = BuildCheckVersionResult;
            break;
        }
        case SESSION_DOWNLOAD: {
            fail = (progress_.status == UPDATE_STATE_DOWNLOAD_FAIL || progress_.status == UPDATE_STATE_VERIFY_FAIL) ?
                progress_.status : 0;
            result.result.progress = &progress_;
            result.buildJSObject = BuildProgress;
            break;
        }
        case SESSION_UPGRADE: {
            fail = (progress_.status == UPDATE_STATE_DOWNLOAD_FAIL) ? progress_.status : 0;
            result.result.progress = &progress_;
            result.buildJSObject = BuildProgress;
            break;
        }
        case SESSION_VERIFY_PACKAGE: {
            fail = (verifyProgress_.status == UPDATE_STATE_VERIFY_FAIL) ? verifyProgress_.status : 0;
            result.result.progress = &verifyProgress_;
            result.buildJSObject = BuildProgress;
            break;
        }
        case SESSION_GET_POLICY: {
            result.result.updatePolicy = &updatePolicy_;
            result.buildJSObject = BuildUpdatePolicy;
            break;
        }
        case SESSION_GET_STATUS: {
            result.result.status = upgradeInfo_.status;
            result.buildJSObject = BuildInt32Status;
            break;
        }
        default:{
            fail = result_;
            result.result.status = result_;
            result.buildJSObject = BuildInt32Status;
            break;
        }
    }
    return napi_ok;
}
} // namespace updateClient

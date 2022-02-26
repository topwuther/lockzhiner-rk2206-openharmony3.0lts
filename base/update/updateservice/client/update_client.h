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

#ifndef UPDATE_CLIENT_H
#define UPDATE_CLIENT_H

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "node_api.h"
#include "node_api_types.h"
#include "update_helper.h"
#include "update_service_kits.h"

static constexpr OHOS::HiviewDFX::HiLogLabel UPDATE_CLIENT = {LOG_CORE, 0, "UPDATE_CLIENT"};
#define CLIENT_LOGE(format, ...)  \
    OHOS::update_engine::UpdateHelper::Logger(__FILE__,  (__LINE__), format, ##__VA_ARGS__); \
    OHOS::HiviewDFX::HiLog::Error(UPDATE_CLIENT, "[%{public}s(%{public}d)] " format, \
        OHOS::update_engine::UpdateHelper::GetBriefFileName(std::string(__FILE__)).c_str(), __LINE__, ##__VA_ARGS__)

#define CLIENT_LOGI(format, ...)  \
    OHOS::update_engine::UpdateHelper::Logger(__FILE__,  (__LINE__), format, ##__VA_ARGS__); \
    OHOS::HiviewDFX::HiLog::Info(UPDATE_CLIENT, "[%{public}s(%{public}d)] " format, \
        OHOS::update_engine::UpdateHelper::GetBriefFileName(std::string(__FILE__)).c_str(), __LINE__, ##__VA_ARGS__)

#define CLIENT_CHECK(retCode, exper, ...) \
    if (!(retCode)) {                     \
        CLIENT_LOGE(__VA_ARGS__);         \
        exper;                            \
    }

#define CLIENT_CHECK_NAPI_CALL(env, assertion, exper, message) \
    if (!(assertion)) {                     \
        CLIENT_LOGE(message);               \
        exper;                            \
    }

namespace updateClient {
// Update status
enum ClientStatus {
    CLIENT_SUCCESS = 0,
    CLIENT_INVALID_PARAM = 1000,
    CLIENT_INVALID_TYPE,
    CLIENT_REPEAT_REQ,
    CLIENT_FAIL,
    CLIENT_CHECK_NEW_FIRST,
};

enum SessionType {
    SESSION_CHECK_VERSION = 0,
    SESSION_DOWNLOAD,
    SESSION_UPGRADE,
    SESSION_SET_POLICY,
    SESSION_GET_POLICY,
    SESSION_GET_NEW_VERSION,
    SESSION_GET_STATUS,
    SESSION_SUBSCRIBE,
    SESSION_UNSUBSCRIBE,
    SESSION_GET_UPDATER,
    SESSION_APPLY_NEW_VERSION,
    SESSION_REBOOT_AND_CLEAN,
    SESSION_VERIFY_PACKAGE,
    SESSION_CANCEL_UPGRADE,
    SESSION_MAX
};

struct UpdateResult {
    using BuildJSObject = std::function<int(napi_env env, napi_value &obj, const UpdateResult &result)>;
    int32_t type;
    union {
        OHOS::update_engine::UpdatePolicy *updatePolicy;
        OHOS::update_engine::Progress *progress;
        OHOS::update_engine::VersionInfo *versionInfo;
        int32_t status;
    } result;

    BuildJSObject buildJSObject;
};


class UpdateSession;
class UpdateClient {
public:
    using DoWorkFunction = std::function<int(int32_t type, void *context)>;

    UpdateClient(napi_env env, napi_value thisVar);
    ~UpdateClient();

    // Obtain the updater engine and return it through the sync API.
    napi_value GetUpdater(napi_env env, napi_callback_info info, int32_t index);
    napi_value GetUpdaterForOther(napi_env env, napi_callback_info info);
    napi_value GetUpdaterFromOther(napi_env env, napi_callback_info info);

    // Asynchronous API
    napi_value CheckNewVersion(napi_env env, napi_callback_info info);
    napi_value SetUpdatePolicy(napi_env env, napi_callback_info info);
    napi_value GetUpdatePolicy(napi_env env, napi_callback_info info);
    napi_value GetNewVersionInfo(napi_env env, napi_callback_info info);
    napi_value GetUpgradeStatus(napi_env env, napi_callback_info info);
    napi_value ApplyNewVersion(napi_env env, napi_callback_info info);
    napi_value RebootAndClean(napi_env env, napi_callback_info info);

    // Event mode, which is used to send the result to the JS.
    napi_value CancelUpgrade(napi_env env, napi_callback_info info);
    napi_value DownloadVersion(napi_env env, napi_callback_info info);
    napi_value UpgradeVersion(napi_env env, napi_callback_info info);
    napi_value VerifyUpdatePackage(napi_env env, napi_callback_info info);

    // Subscription
    napi_value SubscribeEvent(napi_env env, napi_callback_info info);
    napi_value UnsubscribeEvent(napi_env env, napi_callback_info info);

    int32_t GetUpdateResult(int type, UpdateResult &result, int32_t &isFail);

    UpdateSession *RemoveSession(uint32_t sessionId);
    void AddSession(std::shared_ptr<UpdateSession> session);

    void Emit(const std::string &type, int32_t retCode, const UpdateResult &result);

    static int32_t GetStringValue(napi_env env, napi_value arg, std::string &strValue);
    static int32_t BuildErrorResult(napi_env env, napi_value &obj, int32_t result);

    // Notify the session.
    void NotifyCheckVersionDone(const OHOS::update_engine::VersionInfo &info);
    void NotifyDownloadProgress(const OHOS::update_engine::Progress &progress);
    void NotifyUpgradeProgresss(const OHOS::update_engine::Progress &progress);
    void NotifyVerifyProgresss(int32_t result, uint32_t percent);

    #ifdef UPDATER_UT
    UpdateSession *GetUpdateSession(uint32_t sessId)
    {
    #ifndef UPDATER_API_TEST
        std::lock_guard<std::mutex> guard(sessionMutex_);
    #endif
        auto iter = sessions_.find(sessId);
        if (iter != sessions_.end()) {
            return iter->second.get();
        }
        return nullptr;
    }
    #endif
private:
    napi_value StartSession(napi_env env,
        napi_callback_info info, int32_t type, size_t startIndex, DoWorkFunction function);
    int32_t GetNewVersion(bool isCheck);
    bool GetNextSessionId(uint32_t &sessionId);
    bool GetFirstSessionId(uint32_t &sessionId);
    bool CheckUpgradeType(const std::string &type);
    bool CheckUpgradeFile(const std::string &upgradeFile);
    int32_t ProcessUnsubscribe(const std::string &eventType, size_t argc, napi_value arg);
    UpdateSession *FindSessionByHandle(napi_env env, const std::string &eventType, napi_value arg);

    // Verify the version and parameters.
    std::vector<uint8_t> HexToDegist(const std::string &str) const;
    bool VerifyDownloadPkg();
    std::vector<std::string> SplitString(const std::string &str, const std::string &delimiter = ".") const;
    int32_t CompareVersion(const std::string &version1, const std::string &version2) const;

    int32_t GetUpdatePolicyFromArg(napi_env env,
        const napi_value arg, OHOS::update_engine::UpdatePolicy &updatePolicy) const;

    void GetEnginContext(OHOS::update_engine::UpdateContext &context) const;
    void GetEnginPkgInfo(OHOS::update_engine::CheckResult &info) const;

    // Parse input parameters.
    static int32_t GetInt32(napi_env env, napi_value arg, const std::string &attrName, int32_t &intValue);
    static int32_t GetInt64(napi_env env, napi_value arg, const std::string &attrName, int64_t &intValue);
    static int32_t GetBool(napi_env env, napi_value arg, const std::string &attrName, bool &value);

    static int32_t SetString(napi_env env, napi_value arg, const std::string &attrName, const std::string &string);
    static int32_t SetInt32(napi_env env, napi_value arg, const std::string &attrName, int32_t intValue);
    static int32_t SetInt64(napi_env env, napi_value arg, const std::string &attrName, int64_t intValue);
    static int32_t SetBool(napi_env env, napi_value arg, const std::string &attrName, bool value);

    // Construct output parameters.
    static int32_t BuildCheckVersionResult(napi_env env, napi_value &obj, const UpdateResult &result);
    static int32_t BuildProgress(napi_env env, napi_value &obj, const UpdateResult &result);
    static int32_t BuildUpdatePolicy(napi_env env, napi_value &obj, const UpdateResult &result);
    static int32_t BuildInt32Status(napi_env env, napi_value &obj, const UpdateResult &result);
private:
    napi_env env_ {};
    napi_ref thisReference_ {};
    std::map<uint32_t, std::shared_ptr<UpdateSession>> sessions_ {};
#ifndef UPDATER_API_TEST
    std::mutex sessionMutex_;
#endif
    bool isInit = false;
    int32_t result_ = 0;
    std::string upgradeFile_;
    std::string certsFile_;
    OHOS::update_engine::UpdateContext context_ {};
    OHOS::update_engine::UpdatePolicy updatePolicy_ {};
    OHOS::update_engine::Progress progress_ {};
    OHOS::update_engine::Progress verifyProgress_ {};
    OHOS::update_engine::VersionInfo versionInfo_ {};
    OHOS::update_engine::UpgradeInfo upgradeInfo_ {};
};

// Obtain the updater engine and return it through the synchronous API.
napi_value GetUpdater(napi_env env, napi_callback_info info);
napi_value GetUpdaterForOther(napi_env env, napi_callback_info info);
napi_value GetUpdaterFromOther(napi_env env, napi_callback_info info);

// Asynchronous API
napi_value CheckNewVersion(napi_env env, napi_callback_info info);
napi_value SetUpdatePolicy(napi_env env, napi_callback_info info);
napi_value GetUpdatePolicy(napi_env env, napi_callback_info info);
napi_value GetNewVersionInfo(napi_env env, napi_callback_info info);
napi_value GetUpgradeStatus(napi_env env, napi_callback_info info);
napi_value ApplyNewVersion(napi_env env, napi_callback_info info);
napi_value RebootAndClean(napi_env env, napi_callback_info info);

// Event mode, which is used to send the result to the JS.
napi_value CancelUpgrade(napi_env env, napi_callback_info info);
napi_value DownloadVersion(napi_env env, napi_callback_info info);
napi_value UpgradeVersion(napi_env env, napi_callback_info info);
napi_value VerifyUpdatePackage(napi_env env, napi_callback_info info);

napi_value SubscribeEvent(napi_env env, napi_callback_info info);
napi_value UnsubscribeEvent(napi_env env, napi_callback_info info);

#ifdef UPDATER_UT
napi_value UpdateClientInit(napi_env env, napi_value exports);
#endif
} // namespace updateClient
#endif // UPDATE_CLIENT_H
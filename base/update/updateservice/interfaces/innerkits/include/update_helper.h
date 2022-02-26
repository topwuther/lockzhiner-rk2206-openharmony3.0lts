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

#ifndef UPDATER_HELPER_H
#define UPDATER_HELPER_H

#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <ipc_types.h>
#include <string_ex.h>
#include "parcel.h"
#include "message_parcel.h"
#include "hilog/log.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace update_engine {
// 搜索状态
enum SearchStatus {
    SYSTEM_ERROR = -1,
    HAS_NEW_VERSION,
    NO_NEW_VERSION,
    SERVER_BUSY,
};

enum UpgradeStatus {
    UPDATE_STATE_INIT = 0,
    UPDATE_STATE_CHECK_VERSION_ON = 10,
    UPDATE_STATE_CHECK_VERSION_FAIL,
    UPDATE_STATE_CHECK_VERSION_SUCCESS,
    UPDATE_STATE_DOWNLOAD_ON = 20,
    UPDATE_STATE_DOWNLOAD_PAUSE,
    UPDATE_STATE_DOWNLOAD_CANCEL,
    UPDATE_STATE_DOWNLOAD_FAIL,
    UPDATE_STATE_DOWNLOAD_SUCCESS,
    UPDATE_STATE_VERIFY_ON = 30,
    UPDATE_STATE_VERIFY_FAIL,
    UPDATE_STATE_VERIFY_SUCCESS,
    UPDATE_STATE_PACKAGE_TRANS_ON = 70,
    UPDATE_STATE_PACKAGE_TRANS_FAIL,
    UPDATE_STATE_PACKAGE_TRANS_SUCCESS,
    UPDATE_STATE_INSTALL_ON = 80,
    UPDATE_STATE_INSTALL_FAIL,
    UPDATE_STATE_INSTALL_SUCCESS,
    UPDATE_STATE_UPDATE_ON = 90,
    UPDATE_STATE_UPDATE_FAIL,
    UPDATE_STATE_UPDATE_SUCCESS
};

enum PackageType {
    PACKAGE_TYPE_NORMAL = 1,
    PACKAGE_TYPE_BASE = 2,
    PACKAGE_TYPE_CUST = 3,
    PACKAGE_TYPE_PRELOAD = 4,
    PACKAGE_TYPE_COTA = 5,
    PACKAGE_TYPE_VERSION = 6,
    PACKAGE_TYPE_PATCH = 7
};

struct UpdateContext {
    std::string upgradeDevId;
    std::string controlDevId;
    std::string upgradeApp;
    std::string upgradeFile;
    std::string type;
};

struct DescriptInfo {
    std::string descriptPackageId;
    std::string content;
};

struct CheckResult {
    size_t size;
    PackageType packageType;
    std::string versionName;
    std::string versionCode;
    std::string verifyInfo;
    std::string descriptPackageId;
};

struct VersionInfo {
    SearchStatus status;
    std::string errMsg;
    CheckResult result[2];
    DescriptInfo descriptInfo[2];
};

struct Progress {
    uint32_t percent;
    UpgradeStatus status;
    std::string endReason;
};

struct UpgradeInfo {
    UpgradeStatus status;
};

enum InstallMode {
    INSTALLMODE_NORMAL = 0,
    INSTALLMODE_NIGHT,
    INSTALLMODE_AUTO
};

enum AutoUpgradeCondition {
    AUTOUPGRADECONDITION_IDLE = 0,
};

struct UpdatePolicy {
    bool autoDownload;
    bool autoDownloadNet;
    InstallMode mode;
    AutoUpgradeCondition autoUpgradeCondition;
    uint32_t autoUpgradeInterval[2];
};

using CheckNewVersionDone = std::function<void(const VersionInfo &info)>;
using DownloadProgress = std::function<void(const Progress &progress)>;
using UpgradeProgress = std::function<void(const Progress &progress)>;

// 回调函数
struct UpdateCallbackInfo {
    CheckNewVersionDone checkNewVersionDone;
    DownloadProgress downloadProgress;
    UpgradeProgress upgradeProgress;
};

#ifdef UPDATE_SERVICE
static constexpr OHOS::HiviewDFX::HiLogLabel UPDATE_LABEL = {LOG_CORE, 0, "UPDATE_SA"};
#else
static constexpr OHOS::HiviewDFX::HiLogLabel UPDATE_LABEL = {LOG_CORE, 0, "UPDATE_KITS"};
#endif

enum class UpdateLogLevel {
    UPDATE_DEBUG = 0,
    UPDATE_INFO,
    UPDATE_WARN,
    UPDATE_ERROR,
    UPDATE_FATAL
};

class UpdateHelper {
public:
    static int32_t WriteUpdateContext(MessageParcel &data, const UpdateContext &info);
    static int32_t ReadUpdateContext(MessageParcel &reply, UpdateContext &info);

    static int32_t ReadVersionInfo(MessageParcel &reply, VersionInfo &info);
    static int32_t WriteVersionInfo(MessageParcel &data, const VersionInfo &info);

    static int32_t WriteUpdatePolicy(MessageParcel &data, const UpdatePolicy &policy);
    static int32_t ReadUpdatePolicy(MessageParcel &reply, UpdatePolicy &policy);

    static int32_t ReadUpgradeInfo(MessageParcel &reply, UpgradeInfo &info);
    static int32_t WriteUpgradeInfo(MessageParcel &reply, const UpgradeInfo &info);

    static int32_t ReadUpdateProgress(MessageParcel &reply, Progress &info);
    static int32_t WriteUpdateProgress(MessageParcel &data, const Progress &info);

    static int32_t CopyVersionInfo(const VersionInfo &srcInfo, VersionInfo &dstInfo);
    static int32_t CopyUpdatePolicy(const UpdatePolicy &srcInfo, UpdatePolicy &dstInfo);

    static std::vector<uint8_t> HexToDegist(const std::string &str);
    static int32_t CompareVersion(const std::string &version1, const std::string &version2);
    static std::vector<std::string> SplitString(const std::string &str, const std::string &delimiter);

    static void Logger(const std::string &fileName, int32_t line, const char *format, ...);

    static bool JudgeLevel(const UpdateLogLevel& level);

    static void SetLogLevel(const UpdateLogLevel& level)
    {
        level_ = level;
    }

    static const UpdateLogLevel& GetLogLevel()
    {
        return level_;
    }

    static std::string GetBriefFileName(const std::string &file);

private:
    static UpdateLogLevel level_;
};

// 暂时记录两边日志
#define PRINT_LOG(LEVEL, Level, fmt, ...) \
    UpdateHelper::Logger(__FILE__,  (__LINE__), fmt, ##__VA_ARGS__); \
    if (UpdateHelper::JudgeLevel(UpdateLogLevel::LEVEL)) \
        OHOS::HiviewDFX::HiLog::Level(UPDATE_LABEL, "[%{public}s(%{public}d)] " fmt, \
        UpdateHelper::GetBriefFileName(std::string(__FILE__)).c_str(), __LINE__, ##__VA_ARGS__)

#define ENGINE_LOGI(fmt, ...) PRINT_LOG(UPDATE_INFO, Info, fmt, ##__VA_ARGS__)
#define ENGINE_LOGE(fmt, ...) PRINT_LOG(UPDATE_ERROR, Error, fmt, ##__VA_ARGS__)

#define ENGINE_CHECK(retCode, exper, ...) \
    do { \
        if (!(retCode)) {                     \
            ENGINE_LOGE(__VA_ARGS__);         \
            exper;                            \
        } \
    } while (0)
}
} // namespace OHOS
#endif // UPDATER_HELPER_H
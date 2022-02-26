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

#include "update_service.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <sys/reboot.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <vector>

#include "cJSON.h"
#include "iservice_registry.h"
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "misc_info/misc_info.h"
#include "openssl/err.h"
#include "openssl/ssl.h"
#include "package/package.h"
#include "parameters.h"
#include "progress_thread.h"
#include "securec.h"
#include "system_ability_definition.h"
#include "update_helper.h"
#include "updaterkits/updaterkits.h"
#include "utils.h"

namespace OHOS {
namespace update_engine {
REGISTER_SYSTEM_ABILITY_BY_ID(UpdateService, UPDATE_DISTRIBUTED_SERVICE_ID, true)

constexpr int32_t PORT_NUMBER = 5022;
constexpr int32_t JSON_MAX_SIZE = 4096;
const mode_t MKDIR_MODE = 0777;

const std::string MISC_FILE = "/dev/block/platform/soc/10100000.himci.eMMC/by-name/misc";
const std::string BASE_PATH = "/data/updater";
#ifndef UPDATER_UT
const std::string SIGNING_CERT_NAME = "/data/update_sa/signing_cert.crt";
#else
const std::string SIGNING_CERT_NAME = "/data/updater/src/signing_cert.crt";
#endif
const std::string PARAM_NAME_FOR_VERSION = "hw_sc.build.os.version";
const std::string DEFAULT_VERSION = "2.2.0";
const std::string PARAM_NAME_FOR_SEARCH = "update.serverip.search";
const std::string PARAM_NAME_FOR_DOWNLOAD = "update.serverip.download";
const std::string DEFAULT_SERVER_IP = "127.0.0.1";

UpdateService::UpdateService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate)
{
    InitVersionInfo(versionInfo_);
}

UpdateService::~UpdateService()
{
    ENGINE_LOGE("UpdateServerTest free %p", this);
    if (downloadThread_ != nullptr) {
        downloadThread_->StopDownload();
        delete downloadThread_;
        downloadThread_ = nullptr;
    }
}

int32_t UpdateService::RegisterUpdateCallback(const UpdateContext &ctx,
    const sptr<IUpdateCallback> &updateCallback)
{
    ENGINE_CHECK(updateCallback != nullptr, return -1, "Invalid callback");
    updateCallback_ = updateCallback;
    updateContext_.upgradeDevId = ctx.upgradeDevId;
    updateContext_.controlDevId = ctx.controlDevId;
    updateContext_.upgradeApp = ctx.upgradeApp;
    updateContext_.type = ctx.type;
    updateContext_.upgradeFile = ctx.upgradeFile;
    return 0;
}

int32_t UpdateService::UnregisterUpdateCallback()
{
    updateCallback_ = nullptr;
    return 0;
}

int32_t UpdateService::GetNewVersion(VersionInfo &versionInfo)
{
    InitVersionInfo(versionInfo);
    return 0;
}

int32_t UpdateService::GetUpgradeStatus(UpgradeInfo &info)
{
    info.status = upgradeStatus_;
    return 0;
}

int32_t UpdateService::SetUpdatePolicy(const UpdatePolicy &policy)
{
    return UpdateHelper::CopyUpdatePolicy(policy, policy_);
}

int32_t UpdateService::GetUpdatePolicy(UpdatePolicy &policy)
{
    return UpdateHelper::CopyUpdatePolicy(policy_, policy);
}

int32_t UpdateService::CheckNewVersion()
{
    upgradeStatus_ = UPDATE_STATE_CHECK_VERSION_ON;
    int32_t engineSocket = socket(AF_INET, SOCK_STREAM, 0);
    ENGINE_CHECK(engineSocket >= 0, SearchCallback("socket error !", SERVER_BUSY); return 1, "socket error !");

    std::string serverIp = OHOS::system::GetParameter(PARAM_NAME_FOR_SEARCH, DEFAULT_SERVER_IP);
    ENGINE_LOGI("CheckNewVersion serverIp: %s ", serverIp.c_str());

    sockaddr_in engineSin {};
    engineSin.sin_family = AF_INET;
    engineSin.sin_port = htons(PORT_NUMBER);
    int32_t ret = inet_pton(AF_INET, serverIp.c_str(), &engineSin.sin_addr);
    ENGINE_CHECK(ret > 0, close(engineSocket); SearchCallback("Invalid ip!", SERVER_BUSY); return 1, "socket error");

    struct timeval tv = {TIMEOUT_FOR_CONNECT, 0};
    setsockopt(engineSocket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
    ret = connect(engineSocket, reinterpret_cast<sockaddr*>(&engineSin), sizeof(engineSin));
    ENGINE_CHECK(ret == 0,
#ifndef UPDATER_UT
        SearchCallback("Connect error !", SERVER_BUSY);
        close(engineSocket);
#else
        versionInfo_.result[0].verifyInfo = "updater/test.txt";
        SearchCallback("update service test connect error !", HAS_NEW_VERSION);
        close(engineSocket);
#endif
        return 1, "connect error");
    ReadDataFromSSL(engineSocket);
    return 0;
}

int32_t UpdateService::DownloadVersion()
{
    if (access(BASE_PATH.c_str(), 0) == -1) {
        mkdir(BASE_PATH.c_str(), MKDIR_MODE);
    }

    Progress progress0 = {0, UPDATE_STATE_DOWNLOAD_ON, ""};
    ENGINE_CHECK(upgradeStatus_ >= UPDATE_STATE_CHECK_VERSION_SUCCESS,
        progress0.status = UPDATE_STATE_DOWNLOAD_FAIL;
        progress0.endReason = "Invalid status";
        DownloadCallback("", progress0); return -1, "Invalid status %d", upgradeStatus_);

    ENGINE_CHECK(!versionInfo_.result[0].verifyInfo.empty(),
        progress0.status = UPDATE_STATE_DOWNLOAD_FAIL;
        progress0.endReason = "Invalid verify info";
        DownloadCallback("", progress0); return -1, "Invalid verify info");
    std::string downloadFileName = BASE_PATH + "/" + versionInfo_.result[0].verifyInfo;
    size_t localFileLength = DownloadThread::GetLocalFileLength(downloadFileName);
    ENGINE_LOGI("Download %zu %s", localFileLength, downloadFileName.c_str());
    if (localFileLength == versionInfo_.result[0].size && versionInfo_.result[0].size != 0) {
        progress0.percent = DOWNLOAD_FINISH_PERCENT;
        progress0.status = UPDATE_STATE_DOWNLOAD_SUCCESS;
        DownloadCallback(downloadFileName, progress0);
        return 0;
    }

    upgradeStatus_ = UPDATE_STATE_DOWNLOAD_ON;
    if (downloadThread_ == nullptr) {
        downloadThread_ = new DownloadThread([&](const std::string &fileName, const Progress &progress) -> int {
            DownloadCallback(fileName, progress);
            return 0;
        });
        ENGINE_CHECK(downloadThread_ != nullptr,
            progress0.status = UPDATE_STATE_DOWNLOAD_FAIL;
            progress0.endReason = "Failed to start thread";
            DownloadCallback(downloadFileName, progress0); return -1, "Failed to start thread");
    }
    return downloadThread_->StartDownload(downloadFileName, GetDownloadServerUrl());
}

int32_t UpdateService::DoUpdate()
{
    Progress progress;
    progress.percent = 1;
    progress.status = UPDATE_STATE_INSTALL_ON;
    ENGINE_CHECK(upgradeStatus_ >= UPDATE_STATE_DOWNLOAD_SUCCESS,
        progress.endReason = "Invalid status";
        progress.status = UPDATE_STATE_INSTALL_FAIL;
        UpgradeCallback(progress);
        return -1, "Invalid status %d", upgradeStatus_);

    progress.status = UPDATE_STATE_INSTALL_SUCCESS;
    bool ret = RebootAndInstallUpgradePackage(MISC_FILE, updateContext_.upgradeFile);
    ENGINE_CHECK(ret, return -1, "UpdateService::DoUpdate SetParameter failed");
    progress.percent = DOWNLOAD_FINISH_PERCENT;
    UpgradeCallback(progress);
    return 0;
}

void UpdateService::SearchCallback(const std::string &msg, SearchStatus status)
{
    ENGINE_LOGI("SearchCallback %s ", msg.c_str());
    versionInfo_.status = status;
    versionInfo_.errMsg = msg;
    if (status == HAS_NEW_VERSION || status == NO_NEW_VERSION) {
        upgradeStatus_ = UPDATE_STATE_CHECK_VERSION_SUCCESS;

        // Compare the downloaded version with the local version.
        std::string loadVersion = OHOS::system::GetParameter(PARAM_NAME_FOR_VERSION, DEFAULT_VERSION);
        int32_t ret = UpdateHelper::CompareVersion(versionInfo_.result[0].versionCode, loadVersion);
        if (ret <= 0) {
            versionInfo_.status = NO_NEW_VERSION;
        }
    } else {
        upgradeStatus_ = UPDATE_STATE_CHECK_VERSION_FAIL;
    }
    if (updateCallback_ != nullptr) {
        updateCallback_->OnCheckVersionDone(versionInfo_);
    }
}

void UpdateService::DownloadCallback(const std::string &fileName, const Progress &progress)
{
    Progress downloadProgress {};
    upgradeStatus_ = UPDATE_STATE_DOWNLOAD_ON;
    if (progress.status == UPDATE_STATE_DOWNLOAD_FAIL ||
        progress.status == UPDATE_STATE_DOWNLOAD_SUCCESS) {
        upgradeStatus_ = progress.status;
    }
    downloadProgress.percent = progress.percent;
    downloadProgress.status = progress.status;
    downloadProgress.endReason = progress.endReason;

#ifdef UPDATER_UT
    upgradeStatus_ = UPDATE_STATE_DOWNLOAD_SUCCESS;
#endif
    ENGINE_LOGI("DownloadCallback status %d  %d", progress.status, progress.percent);
    if (upgradeStatus_ == UPDATE_STATE_DOWNLOAD_SUCCESS) {
        ENGINE_LOGI("DownloadCallback fileName %s %s", fileName.c_str(), updateContext_.upgradeFile.c_str());
        if (rename(fileName.c_str(), updateContext_.upgradeFile.c_str())) {
            ENGINE_LOGE("Rename file fail %s", fileName.c_str());
            remove(updateContext_.upgradeFile.c_str());
            downloadProgress.status = UPDATE_STATE_DOWNLOAD_FAIL;
        } else if (!VerifyDownloadPkg(updateContext_.upgradeFile, downloadProgress)) {
            // If the verification fails, delete the corresponding package.
            remove(updateContext_.upgradeFile.c_str());
            downloadProgress.status = UPDATE_STATE_VERIFY_FAIL;
        }
    }

    if (updateCallback_ != nullptr) {
        updateCallback_->OnDownloadProgress(downloadProgress);
    }
}

void UpdateService::UpgradeCallback(const Progress &progress)
{
    upgradeStatus_ = progress.status;
    ENGINE_LOGE("UpgradeCallback status %d  %d", progress.status, progress.percent);
    if (updateCallback_ != nullptr) {
        updateCallback_->OnUpgradeProgress(progress);
    }
}

void UpdateService::ReadDataFromSSL(int32_t engineSocket)
{
    SearchStatus result = SERVER_BUSY;
    std::string errMsg = "Couldn't connect to server";
    std::vector<char> buffer(JSON_MAX_SIZE);

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSL_CTX* sslCtx = SSL_CTX_new(SSLv23_client_method());
    ENGINE_CHECK(sslCtx != nullptr, return, "sslCtx is nullptr");
    SSL *ssl = SSL_new(sslCtx);
    ENGINE_CHECK(ssl != nullptr, SSL_CTX_free(sslCtx); return, "ssl is nullptr");
    SSL_set_fd(ssl, engineSocket);
    int32_t ret = SSL_connect(ssl);
    if (ret != -1) {
        int32_t len = SSL_read(ssl, buffer.data(), JSON_MAX_SIZE);
        if (len > 0 && ParseJsonFile(buffer, versionInfo_) == 0) {
            result = HAS_NEW_VERSION;
            errMsg = "";
        } else {
            result = SYSTEM_ERROR;
            errMsg = "Couldn't read data";
        }
    } else {
        result = SYSTEM_ERROR;
        errMsg = "Couldn't connect to server";
    }
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(engineSocket);
    SSL_CTX_free(sslCtx);

    SearchCallback(errMsg, result);
    return;
}

int32_t UpdateService::ParseJsonFile(const std::vector<char> &buffer, VersionInfo &info)
{
    ENGINE_CHECK(buffer.size() > 0, return -1, "JsonFile length must > 0");
    cJSON *root = cJSON_Parse(buffer.data());
    ENGINE_CHECK(root != nullptr, return -1, "Error get root");

    cJSON *item = cJSON_GetObjectItem(root, "searchStatus");
    ENGINE_CHECK(item != nullptr, cJSON_Delete(root); return -1, "Error get searchStatus");
    info.status = static_cast<SearchStatus>(item->valueint);

    item = cJSON_GetObjectItem(root, "errMsg");
    ENGINE_CHECK(item != nullptr, cJSON_Delete(root); return -1, "Error get errMsg");
    info.errMsg = item->valuestring;

    cJSON *results = cJSON_GetObjectItem(root, "checkResults");
    ENGINE_CHECK(results != nullptr, cJSON_Delete(root); return -1, "Error get checkResults");
    int32_t ret = ReadCheckVersionResult(results, info);
    ENGINE_CHECK(ret == 0, cJSON_Delete(root); return -1, "Error get checkResults");

    cJSON *descriptInfo = cJSON_GetObjectItem(root, "descriptInfo");
    ENGINE_CHECK(descriptInfo != nullptr, cJSON_Delete(root); return -1, "Error get descriptInfo");
    ret = ReadCheckVersiondescriptInfo(descriptInfo, info);
    ENGINE_CHECK(ret == 0, cJSON_Delete(root); return -1, "Error get descriptInfo");

    cJSON_Delete(root);
    if (info.status == HAS_NEW_VERSION) {
        ENGINE_CHECK(!info.result[0].verifyInfo.empty() &&
            !info.result[0].versionName.empty() &&
            info.result[0].size > 0, return -1, "Error get descriptInfo");
    }
    return 0;
}

int32_t UpdateService::ReadCheckVersionResult(const cJSON* results, VersionInfo &info)
{
    size_t number = cJSON_GetArraySize(results);
    for (size_t i = 0; i < number && i < sizeof(info.result) / sizeof(info.result[0]); i++) {
        cJSON *result = cJSON_GetArrayItem(results, i);
        ENGINE_CHECK(result != nullptr, return -1, "Error get result");

        cJSON *item = cJSON_GetObjectItem(result, "versionName");
        ENGINE_CHECK(item != nullptr, return -1, "Error get versionName");
        info.result[i].versionName = item->valuestring;

        item = cJSON_GetObjectItem(result, "versionCode");
        ENGINE_CHECK(item != nullptr, return -1, "Error get versionCode");
        info.result[i].versionCode = item->valuestring;

        item = cJSON_GetObjectItem(result, "verifyInfo");
        ENGINE_CHECK(item != nullptr, return -1, "Error get verifyInfo");
        info.result[i].verifyInfo = item->valuestring;

        item = cJSON_GetObjectItem(result, "size");
        ENGINE_CHECK(item != nullptr,  return -1, "Error get size");
        info.result[i].size = item->valueint;

        item = cJSON_GetObjectItem(result, "packageType");
        ENGINE_CHECK(item != nullptr, return -1, "Error get packageType");
        info.result[i].packageType = (PackageType)(item->valueint);

        item = cJSON_GetObjectItem(result, "descriptPackageId");
        ENGINE_CHECK(item != nullptr, return -1, "Error get descriptPackageId");
        info.result[i].descriptPackageId = item->valuestring;
    }
    return 0;
}

int32_t UpdateService::ReadCheckVersiondescriptInfo(const cJSON *descriptInfo, VersionInfo &info)
{
    size_t number = cJSON_GetArraySize(descriptInfo);
    for (size_t i = 0; i < number && i < sizeof(info.result) / sizeof(info.result[0]); i++) {
        cJSON* descript = cJSON_GetArrayItem(descriptInfo, i);
        ENGINE_CHECK(descript != nullptr, return -1, "Error get descriptInfo");

        cJSON *item = cJSON_GetObjectItem(descript, "descriptPackageId");
        if (item != nullptr) {
            info.descriptInfo[i].descriptPackageId = item->valuestring;
        }
        item = cJSON_GetObjectItem(descript, "content");
        if (item != nullptr) {
            info.descriptInfo[i].content = item->valuestring;
            ENGINE_LOGI(" descriptInfo content %s", info.descriptInfo[i].content.c_str());
        }
    }
    return 0;
}

bool UpdateService::VerifyDownloadPkg(const std::string &pkgName, Progress &progress)
{
    // Compare the downloaded version with the local version. Only update is supported.
    std::string loadVersion = OHOS::system::GetParameter(PARAM_NAME_FOR_VERSION, DEFAULT_VERSION);
    int32_t ret = UpdateHelper::CompareVersion(versionInfo_.result[0].versionCode, loadVersion);
    if (ret <= 0) {
        progress.endReason = "Update package version earlier than the local version";
        ENGINE_LOGE("Version compare Failed local '%s' server '%s'",
            loadVersion.c_str(), versionInfo_.result[0].versionCode.c_str());
        return false;
    }
    ENGINE_LOGI("versionInfo_.result[0].verifyInfo %s ", versionInfo_.result[0].verifyInfo.c_str());
    std::vector<uint8_t> digest = UpdateHelper::HexToDegist(versionInfo_.result[0].verifyInfo);
    ret = VerifyPackage(pkgName.c_str(),
        SIGNING_CERT_NAME.c_str(), versionInfo_.result[0].versionCode.c_str(), digest.data(), digest.size());
    if (ret != 0) {
        progress.endReason = "Upgrade package verify Failed";
        ENGINE_LOGE("Package %s verification Failed", pkgName.c_str());
        return false;
    }
    ENGINE_LOGE("Package verify success");
    return true;
}

std::string UpdateService::GetDownloadServerUrl() const
{
    std::string serverIp = OHOS::system::GetParameter(PARAM_NAME_FOR_DOWNLOAD, DEFAULT_SERVER_IP);
    ENGINE_LOGI("GetDownloadServerUrl serverIp: %s ", serverIp.c_str());
    std::string url = "http://";
    url += serverIp;
    url += "/";
    url += versionInfo_.result[0].descriptPackageId;
    return url;
}

int32_t UpdateService::Cancel(int32_t service)
{
    ENGINE_LOGI("Cancel %d", service);
    if (downloadThread_ != nullptr && service == DOWNLOAD) {
        downloadThread_->StopDownload();
    }
    return 0;
}

int32_t UpdateService::RebootAndClean(const std::string &miscFile, const std::string &cmd)
{
#ifndef UPDATER_UT
    return RebootAndCleanUserData(miscFile, cmd) ? 0 : -1;
#else
    return 0;
#endif
}

int32_t UpdateService::RebootAndInstall(const std::string &miscFile, const std::string &packageName)
{
#ifndef UPDATER_UT
    return RebootAndInstallUpgradePackage(miscFile, packageName) ? 0 : -1;
#else
    return 0;
#endif
}

void UpdateService::InitVersionInfo(VersionInfo &versionInfo) const
{
    versionInfo.status = HAS_NEW_VERSION;
    std::string versionName = OHOS::system::GetParameter(PARAM_NAME_FOR_VERSION, DEFAULT_VERSION);
    if (versionName.empty()) {
        versionInfo.status = SYSTEM_ERROR;
        versionInfo.errMsg = "Can not get local version";
    }

    for (size_t i = 0; i < sizeof(versionInfo.result) / sizeof(versionInfo.result[0]); i++) {
        versionInfo.result[i].size = 0;
        versionInfo.result[i].packageType = PACKAGE_TYPE_NORMAL;
        versionInfo.result[i].versionName = versionName;
        versionInfo.result[i].versionCode = "";
        versionInfo.result[i].verifyInfo = "";
        versionInfo.result[i].descriptPackageId = "";
    }
    for (size_t i = 0; i < sizeof(versionInfo.descriptInfo) / sizeof(versionInfo.descriptInfo[0]); i++) {
        versionInfo.descriptInfo[i].content = "";
        versionInfo.descriptInfo[i].descriptPackageId = "";
    }
}

void UpdateService::OnStart()
{
    ENGINE_LOGI("UpdaterService OnStart");
    bool res = Publish(this);
    if (res == false) {
        ENGINE_LOGI("UpdaterService OnStart failed");
    }
    return;
}

void UpdateService::OnStop()
{
    ENGINE_LOGI("UpdaterService OnStop");
}
}
} // namespace OHOS

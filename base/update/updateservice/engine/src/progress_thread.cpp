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

#include "progress_thread.h"
#include <unistd.h>
#include "curl/curl.h"
#include "curl/easy.h"
#include "securec.h"
#include "update_helper.h"

namespace OHOS {
namespace update_engine {
ProgressThread::~ProgressThread() {}

void ProgressThread::ExitThread()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        isWake_ = true;
        isExitThread_ = true;
        condition_.notify_one();
    }
    if (pDealThread_ != nullptr) {
        pDealThread_->join();
        delete pDealThread_;
        pDealThread_ = nullptr;
    }
}

int32_t ProgressThread::StartProgress()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (pDealThread_ == nullptr) {
        pDealThread_ = new (std::nothrow)std::thread(&ProgressThread::ExecuteThreadFunc, this);
        ENGINE_CHECK(pDealThread_ != nullptr, return -1, "Failed to create thread");
    }
    ENGINE_LOGI("StartProgress");
    isWake_ = true;
    condition_.notify_one();
    return 0;
}

void ProgressThread::StopProgress()
{
    std::unique_lock<std::mutex> lock(mutex_);
    isWake_ = true;
    isExitThread_ = false;
    condition_.notify_one();
}

void ProgressThread::ExecuteThreadFunc()
{
    while (1) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (!isWake_) {
                ENGINE_LOGI("ExecuteThreadFunc wait");
                condition_.wait(lock);
            }
            if (isExitThread_) {
                break;
            }
            isWake_ = false;
        }
        ProcessThreadExecute();
    }
    // thread exit and release resource
    ProcessThreadExit();
}

int32_t DownloadThread::StartDownload(const std::string &fileName, const std::string &url)
{
    downloadFileName_ = fileName;
    serverUrl_ = url;
    exitDownload_ = false;
    curl_global_init(CURL_GLOBAL_ALL);
    return StartProgress();
}

void DownloadThread::StopDownload()
{
    ENGINE_LOGI("StopDownload ");
    exitDownload_ = true;
    StopProgress();
    curl_global_cleanup();
}

bool DownloadThread::ProcessThreadExecute()
{
    packageSize_ = GetLocalFileLength(downloadFileName_);
    ENGINE_LOGI("download  packageSize_: %zu ", packageSize_);

    downloadFile_ = fopen(downloadFileName_.c_str(), "ab+");
    ENGINE_CHECK(downloadFile_ != nullptr,
        DownloadCallback(0, UPDATE_STATE_DOWNLOAD_FAIL, "Failed ot open file");
        return true, "Failed to open file %s", downloadFileName_.c_str());

    downloadHandle_ = curl_easy_init();
    ENGINE_CHECK(downloadHandle_ != nullptr,
        ProcessThreadExit();
        DownloadCallback(0, UPDATE_STATE_DOWNLOAD_FAIL, "Failed to init curl");
        return true, "Failed to init curl");

    curl_easy_setopt(downloadHandle_, CURLOPT_TIMEOUT, TIMEOUT_FOR_DOWNLOAD);
    curl_easy_setopt(downloadHandle_, CURLOPT_CONNECTTIMEOUT, TIMEOUT_FOR_CONNECT);
    curl_easy_setopt(downloadHandle_, CURLOPT_URL, serverUrl_.c_str());
    curl_easy_setopt(downloadHandle_, CURLOPT_WRITEDATA, downloadFile_);
    curl_easy_setopt(downloadHandle_, CURLOPT_WRITEFUNCTION, WriteFunc);
    if (packageSize_ > 0) {
        curl_easy_setopt(downloadHandle_, CURLOPT_RESUME_FROM_LARGE, static_cast<curl_off_t>(packageSize_));
    }
    curl_easy_setopt(downloadHandle_, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(downloadHandle_, CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt(downloadHandle_, CURLOPT_PROGRESSFUNCTION, DownloadProgress);
    CURLcode res = curl_easy_perform(downloadHandle_);
    if (res != CURLE_OK) {
        ProcessThreadExit();
        ENGINE_LOGI("Failed to download %s res %s", serverUrl_.c_str(), curl_easy_strerror(res));
        if (res != CURLE_ABORTED_BY_CALLBACK) { // cancel by user, do not callback
            DownloadCallback(0, UPDATE_STATE_DOWNLOAD_FAIL, curl_easy_strerror(res));
        }
    } else {
        ProcessThreadExit();
        ENGINE_LOGI("Success to download %s", serverUrl_.c_str());
        DownloadCallback(DOWNLOAD_FINISH_PERCENT, UPDATE_STATE_DOWNLOAD_SUCCESS, "");
    }
    // clear up
    downloadProgress_.endReason = "";
    downloadProgress_.percent = 0;
    downloadProgress_.status = UPDATE_STATE_DOWNLOAD_ON;
    return false;
}

void DownloadThread::ProcessThreadExit()
{
    ENGINE_LOGI("ProcessThreadExit");
    if (downloadHandle_ != nullptr) {
        curl_easy_cleanup(downloadHandle_);
    }
    downloadHandle_ = nullptr;
    if (downloadFile_ != nullptr) {
        fclose(downloadFile_);
    }
    downloadFile_ = nullptr;
}

int32_t DownloadThread::DownloadCallback(uint32_t percent, UpgradeStatus status, const std::string &error)
{
    if (exitDownload_) {
        ENGINE_LOGI("StopDownlDownloadCallbackoad");
        return -1;
    }
    if (status == UPDATE_STATE_DOWNLOAD_FAIL) {
        if (access(downloadFileName_.c_str(), 0) == 0) {
            unlink(downloadFileName_.c_str());
        }
    } else if (percent != DOWNLOAD_FINISH_PERCENT
        && (percent < (downloadProgress_.percent + DOWNLOAD_PERIOD_PERCENT))) {
        return 0;
    }

    // wait until the download is complete, and then make a notification
    if (percent == DOWNLOAD_FINISH_PERCENT
        && status == UPDATE_STATE_DOWNLOAD_ON) {
        return 0;
    }
    downloadProgress_.endReason = error;
    downloadProgress_.percent = percent;
    downloadProgress_.status = status;
    if (callback_ != nullptr) {
        callback_(downloadFileName_, downloadProgress_);
    }
    return 0;
}

int32_t DownloadThread::DownloadProgress(const void *localData,
    double dlTotal, double dlNow, double ulTotal, double ulNow)
{
    ENGINE_CHECK_NO_LOG(dlTotal > 0, return 0);
    auto engine = reinterpret_cast<DownloadThread*>(const_cast<void*>(localData));
    ENGINE_CHECK(engine != nullptr, return -1, "Can not find engine");
    double curr = engine->GetPackageSize();
    unsigned int percent = (dlNow + curr) / (curr + dlTotal) * DOWNLOAD_FINISH_PERCENT;
    return engine->DownloadCallback(percent, UPDATE_STATE_DOWNLOAD_ON, "");
}

size_t DownloadThread::WriteFunc(void *ptr, size_t size, size_t nmemb, const void *stream)
{
    return fwrite(ptr, size, nmemb, reinterpret_cast<FILE*>(const_cast<void*>(stream)));
}

size_t DownloadThread::GetLocalFileLength(const std::string &fileName)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    ENGINE_CHECK_NO_LOG(fp != nullptr, return 0);
    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    fclose(fp);
    return length;
}
}
} // namespace OHOS

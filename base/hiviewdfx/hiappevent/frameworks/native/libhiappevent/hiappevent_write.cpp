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

#include "hiappevent_write.h"

#include <algorithm>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hiappevent_read.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
using namespace std;
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_write" };
const int DATE_SIZE = 9;
const string APP_EVENT_DIR = "/hiappevent/";
mutex g_mutex;

bool IsFileExists(const string& fileName)
{
    if (access(fileName.c_str(), F_OK) != 0) {
        return false;
    }

    return true;
}

bool ForceCreateDirectory(const string& path)
{
    string::size_type index = 0;
    do {
        string subPath;
        index = path.find('/', index + 1);
        if (index == string::npos) {
            subPath = path;
        } else {
            subPath = path.substr(0, index);
        }

        if (access(subPath.c_str(), F_OK) != 0) {
            if (mkdir(subPath.c_str(), S_IRWXU) != 0) {
                HiLog::Error(LABEL, "failed to create hiappevent dir, errno=%{public}d.", errno);
                return false;
            }
        }
    } while (index != string::npos);

    return access(path.c_str(), F_OK) == 0;
}

string GetStorageDirPath()
{
    string filesDir = HiAppEventConfig::GetInstance().GetStorageDir();
    return (filesDir == "") ? "" : (filesDir + APP_EVENT_DIR);
}

uint64_t GetMaxStorageSize()
{
    return HiAppEventConfig::GetInstance().GetMaxStorageSize();
}

string GetStorageFilePath()
{
    time_t nowTime = time(nullptr);
    char dateChs[DATE_SIZE] = {0};
    struct tm *ptm = localtime(&nowTime);
    if (ptm == nullptr) {
        HiLog::Error(LABEL, "failed to get localtime.");
        return GetStorageDirPath() + "app_event_20210101.log";
    }
    strftime(dateChs, sizeof(dateChs), "%Y%m%d", localtime(&nowTime));
    string dateStr = dateChs;
    return GetStorageDirPath() + "app_event_" + dateStr + ".log";
}

void GetStorageDirFiles(const string& dirPath, vector<string>& files)
{
    DIR *dir = opendir(dirPath.c_str());
    if (dir == nullptr) {
        HiLog::Error(LABEL, "failed to opendir hiappevent dir.");
        return;
    }

    struct dirent *ent = nullptr;
    while ((ent = readdir(dir)) != nullptr) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
            continue;
        }
        files.push_back(dirPath + ent->d_name);
    }
    closedir(dir);
}

uint64_t GetStorageDirSize(const string& dirPath)
{
    vector<string> files;
    GetStorageDirFiles(dirPath, files);

    uint64_t totalSize = 0;
    struct stat statbuf {};
    for (auto& file : files) {
        if (stat(file.c_str(), &statbuf) == 0) {
            totalSize += statbuf.st_size;
        }
    }

    return totalSize;
}

bool IsNeedCleanDir(const string& dirPath)
{
    if (!IsFileExists(dirPath)) {
        HiLog::Info(LABEL, "the hiappevent dir does not exist and does not need to be cleaned.");
        return false;
    }

    struct stat st {};
    if (stat(dirPath.c_str(), &st)) {
        HiLog::Error(LABEL, "failed to execute the stat function, hiappevent dir.");
        return false;
    }

    return GetStorageDirSize(dirPath) > GetMaxStorageSize();
}

void CleanDirSpace(const string& dirPath)
{
    vector<string> files;
    GetStorageDirFiles(dirPath, files);
    sort(files.begin(), files.end());

    uint64_t currSize = GetStorageDirSize(dirPath);
    uint64_t maxSize = GetMaxStorageSize();
    struct stat statbuf {};
    while (!files.empty() && currSize > maxSize) {
        string delFile = files[0];
        if (access(delFile.c_str(), F_OK) == 0 && stat(delFile.c_str(), &statbuf) == 0
            && remove(delFile.c_str()) == 0) {
            currSize -= statbuf.st_size;
        } else {
            HiLog::Error(LABEL, "failed to access or remove log file.");
        }

        files.erase(files.begin());
    }
}

bool WriteEventToFile(const string& filePath, const string& event, bool truncated = false)
{
    if (event.empty()) {
        return true;
    }

    ofstream file;
    if (truncated) {
        file.open(filePath.c_str(), ios::out | ios::trunc);
    } else {
        file.open(filePath.c_str(), ios::out | ios::app);
    }

    if (!file.is_open()) {
        HiLog::Error(LABEL, "failed to open the log file.");
        return false;
    }

    file.write(event.c_str(), event.length());
    if (file.fail()) {
        HiLog::Error(LABEL, "failed to write the event to the log file.");
        return false;
    }
    LogAssistant::Instance().RealTimeAppLogUpdate(event);
    return true;
}
}

void WriterEvent(shared_ptr<AppEventPack> appEventPack)
{
    if (HiAppEventConfig::GetInstance().GetDisable()) {
        HiLog::Warn(LABEL, "the HiAppEvent function is disabled.");
        return;
    }

    if (appEventPack == nullptr) {
        HiLog::Error(LABEL, "appEventPack is null.");
        return;
    }
    HiLog::Debug(LABEL, "WriteEvent eventInfo=%{public}s.", appEventPack->GetJsonString().c_str());

    string dirPath = GetStorageDirPath();
    if (dirPath == "") {
        HiLog::Error(LABEL, "dirPath is null, stop writing the event.");
        return;
    }

    {
        lock_guard<mutex> lockGuard(g_mutex);

        if (!IsFileExists(dirPath) && !ForceCreateDirectory(dirPath)) {
            HiLog::Error(LABEL, "failed to create hiappevent dir, errno=%{public}d.", errno);
            return;
        }

        if (IsNeedCleanDir(dirPath)) {
            HiLog::Info(LABEL, "the hiappevent dir space is full.");
            CleanDirSpace(dirPath);
        }

        string filePath = GetStorageFilePath();
        if (!WriteEventToFile(filePath, appEventPack->GetJsonString())) {
            HiLog::Error(LABEL, "failed to write event to log file.");
        }
    }
}
} // HiviewDFX
} // OHOS
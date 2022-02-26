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
#include "../include/storage_ability.h"

#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stack>
#include <string>
#include <unistd.h>
#include <vector>

#include "../include/fd_holder.h"
#include "../include/file_info.h"
#include "../include/fms_utils.h"
#include "../include/log_util.h"
#include "../include/root_info.h"

namespace OHOS {
namespace FileManager {
using namespace std;
using Uri = OHOS::Uri;
unique_ptr<StorageAbility> StorageAbility::mInstance = nullptr;
StorageAbility *StorageAbility::Instance()
{
    if (mInstance.get() == nullptr) {
        mInstance.reset(new StorageAbility());
    }
    return mInstance.get();
}
StorageAbility::StorageAbility() {}
int StorageAbility::CheckUri(string &uriStr, const string &scheme, string &absoluteUri) const
{
    if (uriStr.empty()) {
        return static_cast<int>(STATUS_NUM::PARAMETER_EXCEPTION);
    } else if (scheme != "dataability") {
        return static_cast<int>(STATUS_NUM::URI_EXCEPTION);
    } else if (strstr(uriStr.c_str(), ":") != nullptr || strstr(uriStr.c_str(), "*") != nullptr ||
               strstr(uriStr.c_str(), "?") != nullptr || strstr(uriStr.c_str(), "<") != nullptr ||
               strstr(uriStr.c_str(), ">") != nullptr || strstr(uriStr.c_str(), "|") != nullptr) {
        return static_cast<int>(STATUS_NUM::IO_EXCEPTION);
    }
    string pattern = "/";
    stack<string> uriResult;
    vector<string> uriSplite;
    size_t pos = uriStr.find(pattern);
    while (pos != uriStr.npos) {
        string temp = uriStr.substr(static_cast<int>(COMMON_NUM::ZERO), pos);
        uriSplite.push_back(temp);
        uriStr = uriStr.substr(pos + 1, uriStr.size());
        pos = uriStr.find(pattern);
    }
    for (auto urisp : uriSplite) {
        if (urisp == "." || urisp == "") {
            continue;
        } else if (urisp == ".." && !uriResult.empty()) {
            uriResult.pop();
        } else if (urisp == ".." && uriResult.empty()) {
            continue;
        } else {
            uriResult.push(urisp);
        }
    }
    while (!uriResult.empty()) {
        absoluteUri = pattern + uriResult.top() + absoluteUri;
        uriResult.pop();
    }
    return static_cast<int>(STATUS_NUM::SUCCESS);
}
int StorageAbility::QueryFiles(const Uri &uri, vector<FileInfo> &result)
{
    FmsUtils *mFmsUtils = FmsUtils::Instance();
    string mPath = "";
    string uriStr = mFmsUtils->GetCurrentPath(uri);
    int code = CheckUri(uriStr, const_cast<Uri &>(uri).GetScheme(), mPath);
    if (code != static_cast<int>(STATUS_NUM::SUCCESS)) {
        return code;
    }
    if (mFmsUtils->GetCurrentDirFileInfoList(mPath, result) == static_cast<int>(STATUS_NUM::FAIL)) {
        return static_cast<int>(STATUS_NUM::IO_EXCEPTION);
    }
    return static_cast<int>(STATUS_NUM::SUCCESS);
}
int StorageAbility::QueryDeviceInfo(vector<RootInfo> &result)
{
    RootInfo rootInfo;
    rootInfo.path = "/storage/d4ab5596-a98b-4753-8e8f-7cbf342ab15d";
    rootInfo.deviceId = "d4ab5596-a98b-4753-8e8f-7cbf342ab15d";
    rootInfo.id = "public:197.24";
    rootInfo.mountFlags = 2;
    rootInfo.diskId = "public";
    result.push_back(rootInfo);
    return static_cast<int>(STATUS_NUM::SUCCESS);
}
int StorageAbility::SearchFiles(const Uri &uri, const string &fileName, vector<FileInfo> &result)
{
    FmsUtils *mFmsUtils = FmsUtils::Instance();
    string mPath = "";
    string uriStr = mFmsUtils->GetCurrentPath(uri);
    int code = CheckUri(uriStr, const_cast<Uri &>(uri).GetScheme(), mPath);
    if (code != static_cast<int>(STATUS_NUM::SUCCESS)) {
        return code;
    } else if (fileName.empty()) {
        return static_cast<int>(STATUS_NUM::PARAMETER_EXCEPTION);
    }
    mFmsUtils->GetSearchFileInfoList(mPath, fileName, result);
    return static_cast<int>(STATUS_NUM::SUCCESS);
}
int StorageAbility::SaveFiles(const Uri &uri, const vector<string> &srcPath, vector<SaveFileInfo> &result)
{
    FmsUtils *mFmsUtils = FmsUtils::Instance();
    string mPath = "";
    string uriStr = mFmsUtils->GetCurrentPath(uri);
    int code = CheckUri(uriStr, const_cast<Uri &>(uri).GetScheme(), mPath);
    if (code != static_cast<int>(STATUS_NUM::SUCCESS)) {
        return code;
    } else if (access(mPath.c_str(), 0) != 0 && mFmsUtils->Mkdirs(mPath) != 0) {
        return static_cast<int>(STATUS_NUM::IO_EXCEPTION);
    }
    for (auto curSrcUriStr : srcPath) {
        Uri curSrcUri = Uri(curSrcUriStr);
        string curSrcPath = mFmsUtils->GetCurrentPath(curSrcUri);
        string realSrcPath = "";
        int SrcPathCode = CheckUri(curSrcPath, const_cast<Uri &>(uri).GetScheme(), realSrcPath);
        if (SrcPathCode != static_cast<int>(STATUS_NUM::SUCCESS)) {
            return SrcPathCode;
        }
        FDHolder srcFHolder;
        FDHolder tarFHolder;
        struct stat srcStat;
        srcFHolder.SetFD(open(realSrcPath.c_str(), O_RDONLY));
        if (fstat(srcFHolder.GetFD(), &srcStat) == -1) {
            result.push_back({ .srcUri = curSrcUriStr });
            continue;
        }
        int index = realSrcPath.find_last_of("/");
        string fileName = realSrcPath.substr(index + 1);
        string tarPath = mPath + "/" + fileName;
        while (access(tarPath.c_str(), 0) == 0) {
            int dotIndex = (fileName.find_last_of(".") == fileName.npos) ? fileName.length() :
                                                                           fileName.find_last_of(".");
            fileName.insert(dotIndex, "(1)");
            tarPath = mPath + "/" + fileName;
        }
        tarFHolder.SetFD(open(tarPath.c_str(), O_WRONLY | O_CREAT, srcStat.st_mode));
        if (tarFHolder.GetFD() == -1) {
            result.push_back({ .srcUri = curSrcUriStr });
            continue;
        }
        if (sendfile(tarFHolder.GetFD(), srcFHolder.GetFD(), nullptr, srcStat.st_size) == -1) {
            result.push_back({ .srcUri = curSrcUriStr });
            continue;
        }
        result.push_back({ .status = true, .srcUri = curSrcUriStr, .dstUri = mFmsUtils->RealPathToUri(tarPath) });
    }
    return static_cast<int>(STATUS_NUM::SUCCESS);
}
int StorageAbility::CreateDir(const Uri &uri)
{
    FmsUtils *mFmsUtils = FmsUtils::Instance();
    int32_t i = -1;
    string mPath = "";
    string uriStr = mFmsUtils->GetCurrentPath(uri);
    int code = CheckUri(uriStr, const_cast<Uri &>(uri).GetScheme(), mPath);
    if (code != static_cast<int>(STATUS_NUM::SUCCESS)) {
        return code;
    } else if (mFmsUtils->Mkdirs(mPath) == i) {
        return static_cast<int>(STATUS_NUM::IO_EXCEPTION);
    }
    return static_cast<int>(STATUS_NUM::SUCCESS);
}
} // namespace FileManager
} // namespace OHOS
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
#include "../include/shared_path_strategy.h"

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <list>
#include <unistd.h>

#include "../include/log_util.h"

namespace fs = std::filesystem;
namespace OHOS {
namespace FileManager {
constexpr int STORAGE_LEN = 9;
SharedPathStrategy::SharedPathStrategy() {}
SharedPathStrategy::SharedPathStrategy(const std::string &authority)
{
    mAuthority = authority;
}
std::string SharedPathStrategy::GetUriStr(std::string &file, const std::string &deviceId)
{
    std::string getName = "";
    std::string getPath = "";
    std::string uriStr = "";
    std::map<std::string, std::string>::iterator iter;
    for (iter = mRoots.begin(); iter != mRoots.end(); iter++) {
        std::string rootPath = iter->second;
        if (file.find(rootPath) == 0) {
            if (getPath.empty() || rootPath.size() > getPath.size()) {
                getName = iter->first;
                getPath = iter->second;
            }
        }
    }
    if (getPath.empty()) {
        return uriStr;
    }
    file = file.substr(getPath.find_last_of("/") + 1);
    uriStr = "dataability://" + deviceId + "/" + mAuthority;
    int lStorage = getPath.find("/storage/");
    if (lStorage >= 0) {
        std::string storageStr = "";
        int rStorage = getPath.find("/", lStorage + STORAGE_LEN + 1);
        storageStr = getPath.substr(lStorage + STORAGE_LEN, rStorage - lStorage - STORAGE_LEN);
        uriStr = uriStr + "/" + storageStr;
    } else {
        uriStr.append("/primary");
    }
    if (getPath.find("/files/") == getPath.npos && getPath.find("/cache/") == getPath.npos) {
        uriStr = uriStr + "/root/" + getName + "/";
    } else {
        if (getPath.find("/files/") != getPath.npos) {
            uriStr = uriStr + "/document/files/" + getName;
        } else if (getPath.find("/cache/") != getPath.npos) {
            uriStr = uriStr + "/document/cache/" + getName;
        }
        if (file.empty()) {
            uriStr = uriStr + "/leaf";
        } else if (!file.empty()) {
            uriStr = uriStr + "/" + file + "/leaf";
        }
    }
    return uriStr;
}
Uri SharedPathStrategy::FuzzyFileToUri(std::string &file, std::string &deviceId)
{
    std::string uriStr = "";
    if (file.empty()) {
        return Uri(uriStr);
    } else if (mAuthority == "ERROR_JSON_CONFIG") {
        return Uri("ERROR_JSON_CONFIG");
    } else if (PILING_AUTHORITY != mAuthority) {
        return Uri("ERROR_AUTHORITY");
    }
    uriStr = GetUriStr(file, deviceId);
    Uri uri(uriStr);
    return uri;
}
std::string GetFileName(std::string fileMark, std::string tag, std::string path)
{
    std::string fileName = "";
    int fileMarkIndex = -1;
    int tagIndex = -1;
    int tagLen = tag.length();
    fileMarkIndex = path.rfind(fileMark);
    tagIndex = path.find(tag);
    int begin = tagIndex + tagLen + 1;
    int fileNameLen = fileMarkIndex - begin;
    fileName = path.substr(begin, fileNameLen);
    if (fileNameLen < 0) {
        return "";
    }
    return fileName;
}
std::string GetTag(std::string pathType, std::string path)
{
    int pathTypeLen = pathType.length();
    int typeIndex = path.find(pathType);
    int tagEndIndex = path.find("/", typeIndex + pathTypeLen + 1);
    std::string tag = path.substr(typeIndex + pathTypeLen + 1, tagEndIndex - typeIndex - pathTypeLen - 1);
    return tag;
}
std::string ProcessUri(const Uri &uri)
{
    std::string afterPath = "";
    std::string path = const_cast<Uri &>(uri).ToString();
    if (path.empty()) {
        return afterPath;
    }
    std::vector<std::string> pathVector;
    const_cast<Uri &>(uri).GetPathSegments(pathVector);
    std::string pathScheme = pathVector[0];
    if (const_cast<Uri &>(uri).GetScheme() != "dataability" || pathVector[0] != "ohos.acts.test") {
        return "302";
    }
    unsigned int lastSeparator = path.find_last_of("/");
    if (lastSeparator != path.length() - 1) {
        afterPath = path;
    } else {
        afterPath = path.substr(0, path.length() - 1);
    }
    return afterPath;
}
std::string SharedPathStrategy::ParseFileInUri(const Uri &uri)
{
    std::string file = "";
    std::string path = ProcessUri(uri);
    if (path == "302") {
        return "302";
    }
    std::string tag = "";
    std::map<std::string, std::string>::iterator iter;
    if (path.find("/files") != path.npos) {
        tag = GetTag("/files", path);
    } else {
        return file;
    }
    iter = mRoots.find(tag);
    if (iter == mRoots.end()) {
        return file;
    }
    std::string root = iter->second;
    std::string fileName = "";
    if (path.rfind("/leaf") != path.npos) {
        fileName = GetFileName("/leaf", tag, path);
    } else if (path.rfind("?displayName") != path.npos) {
        fileName = GetFileName("?displayName", tag, path);
    } else {
        int pathLastDot = path.find_last_of(".");
        int pathLastSeparator = path.find_last_of("/");
        if (pathLastDot > 0 && pathLastDot > pathLastSeparator) {
            fileName = path.substr(pathLastSeparator + 1);
        } else {
            fileName = "";
        }
    }
    std::string rootFileName = "";
    int rootLastDot = root.find_last_of(".");
    int rootLastSeparator = root.find_last_of("/");
    if (rootLastDot > 0 && rootLastDot > rootLastSeparator) {
        rootFileName = root.substr(rootLastSeparator + 1);
    }
    if (fileName == rootFileName) {
        file = root;
    }
    return file;
}
void SharedPathStrategy::AddRoot(const std::string &name, const std::string &root)
{
    if (!name.empty() && !root.empty()) {
        mRoots.insert(std::map<std::string, std::string>::value_type(name, root));
    }
}
} // namespace FileManager
} // namespace OHOS

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
#include "../include/file_share_ability.h"

#include <cstdio>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <unistd.h>

#include "../include/fms_utils.h"
#include "../include/log_util.h"
#include "../include/path_strategy.h"
#include "../include/shared_path_strategy.h"

namespace OHOS {
namespace FileManager {
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::NativeRdb;
using namespace std;
using json = nlohmann::json;
constexpr int DISPLAYNAME_LEN = 12;
unique_ptr<FileShareAbility> FileShareAbility::mInstance = nullptr;
FileShareAbility *FileShareAbility::Instance()
{
    if (mInstance.get() == nullptr) {
        mInstance.reset(new FileShareAbility());
    }
    return mInstance.get();
}

FileShareAbility::FileShareAbility() {}
void FileShareAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
                            const std::shared_ptr<OHOSApplication> &application,
                            std::shared_ptr<AbilityHandler> &handler,
                            const sptr<IRemoteObject> &token)
{
    mAbilityInfo = abilityInfo;
    Ability::Init(abilityInfo, application, handler, token);
}
void FileShareAbility::OnStart(const Want &want)
{
    // permission check
    if (mAbilityInfo == nullptr) {
        return;
    } else if (mAbilityInfo->visible == true) {
        return;
    } else {
        originWant_ = want;
        Ability::OnStart(want);
    }
}

std::string GetRealPath(std::string path)
{
    unique_ptr<char[]> absPath = make_unique<char[]>(PATH_MAX + 1);
    if (realpath(path.c_str(), absPath.get()) == nullptr) {
        return "";
    }
    return absPath.get();
}
bool CheckUri(std::string filePath)
{
    bool isDirUri = false;
    if (filePath.empty()) {
        isDirUri = true;
    } else if (GetRealPath(filePath).empty()) {
        isDirUri = true;
    } else {
        struct stat tmp;
        int r = stat(filePath.c_str(), &tmp);
        if (r == 0 && S_ISDIR(tmp.st_mode)) {
            isDirUri = true;
        }
    }
    return isDirUri;
}
std::shared_ptr<NativeRdb::AbsSharedResultSet> FileShareAbility::Query(
    const Uri &uri,
    const std::vector<std::string> &columns,
    const NativeRdb::DataAbilityPredicates &predicates)
{
    printf("===============fms FileShareAbility::Query================== \n");
    FmsUtils *fm = FmsUtils::Instance();
    SharedPathStrategy rootStrat = ParsePathStrategy();
    std::string filePath = rootStrat.ParseFileInUri(const_cast<Uri &>(uri));
    int32_t uriException = static_cast<int32_t>(STATUS_NUM::URI_EXCEPTION);
    int32_t iOException = static_cast<int32_t>(STATUS_NUM::IO_EXCEPTION);
    if (filePath == "302") {
        return fm->Int32ToResultset(uriException);
    } else if (CheckUri(filePath)) {
        return fm->Int32ToResultset(iOException);
    }
    int punctuation = filePath.find_last_of("/");
    std::string fileName = filePath.substr(punctuation + 1);
    std::string displayName = "";
    Uri uriCopy = uri;
    std::string path = uriCopy.GetQuery();
    int wenhao = path.find("?");
    int findPath = path.find("displayName=");
    if (wenhao >= 0 && findPath >= 0) {
        displayName = path.substr(findPath + DISPLAYNAME_LEN);
    } else {
        punctuation = filePath.find_last_of("/");
        displayName = filePath.substr(punctuation + 1);
    }
    FileInfo fi = fm->GetFileInfo(filePath, fileName);
    std::vector<std::string> queryResult;
    std::vector<std::string> listing = const_cast<vector<std::string> &>(columns);
    if (listing.size() == 0) {
        queryResult.push_back(displayName);
        queryResult.push_back(to_string(fi.fileSize));
    }
    for (size_t i = 0; i < listing.size(); i++) {
        if (listing[i] == "fileName") {
            queryResult.push_back(displayName);
        } else if (listing[i] == "fileSize") {
            queryResult.push_back(to_string(fi.fileSize));
        }
    }
    if (queryResult.size() == 0) {
        return nullptr;
    }
    return fm->VectorToResultset1(queryResult);
}
int FileShareAbility::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    SharedPathStrategy rootStrat = ParsePathStrategy();
    int deleteSymbol = 0;
    std::string file = rootStrat.ParseFileInUri(const_cast<Uri &>(uri));
    if (CheckUri(file)) {
        printf("=================result test Delete start=================== \n");
        printf("Delete result = %d \n", deleteSymbol);
        printf("=================result test Delete end=================== \n");
        return deleteSymbol;
    }
    const char *path = file.data();
    deleteSymbol = remove(path) ? 0 : 1;
    printf("=================result test Delete start=================== \n");
    printf("Delete result = %d \n", deleteSymbol);
    printf("=================result test Delete end=================== \n");
    return deleteSymbol;
}
std::string FileShareAbility::GetType(const Uri &uri)
{
    SharedPathStrategy rootStrat = ParsePathStrategy();
    std::string filePath = rootStrat.ParseFileInUri(const_cast<Uri &>(uri));
    if (CheckUri(filePath)) {
        printf("=================result test GetType start=================== \n");
        printf("GetType result  type = \n");
        printf("=================result test GetType end=================== \n");
        return "";
    }
    int lastDot = filePath.find_last_of(".");
    int lastSeparator = filePath.find_last_of("/");
    if (lastDot > 0 && lastDot > lastSeparator) {
        std::string extension = filePath.substr(lastDot + 1);
        printf("=================result test GetType start=================== \n");
        printf("GetType result  type = %s \n", extension.c_str());
        printf("=================result test GetType end=================== \n");
        return extension;
    }
    printf("=================result test GetType start=================== \n");
    printf("GetType result  application/octet-stream \n");
    printf("=================result test GetType end=================== \n");
    return "application/octet-stream";
}
int ModeToMode(std::string mode)
{
    int modeBits = -1;
    if (mode.compare("r") == 0) {
        modeBits = O_RDONLY;
    } else if (mode.compare("w") == 0 || mode.compare("wt") == 0) {
        modeBits = O_WRONLY | O_TRUNC;
    } else if (mode.compare("wa") == 0) {
        modeBits = O_WRONLY | O_APPEND;
    } else if (mode.compare("rw") == 0) {
        modeBits = O_RDWR;
    } else if (mode.compare("rwt") == 0) {
        modeBits = O_RDWR | O_TRUNC;
    }
    return modeBits;
}
int FileShareAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    int fp = -1;
    SharedPathStrategy rootStrat = ParsePathStrategy();
    std::string file = rootStrat.ParseFileInUri(const_cast<Uri &>(uri));
    if (CheckUri(file)) {
        printf("=================result test OpenFile start=================== \n");
        printf("OpenFile result fp = : %d \n", fp);
        printf("=================result test OpenFile end=================== \n");
        return fp;
    }
    const char *filePath = file.data();
    int fileMode = ModeToMode(mode);
    fp = open(filePath, fileMode);
    printf("=================result test OpenFile start=================== \n");
    printf("OpenFile result fp = : %d \n", fp);
    printf("=================result test OpenFile end=================== \n");
    return fp;
}
Uri FileShareAbility::FuzzyFileToUri(Ability *context,
                                     std::string &deviceId,
                                     std::string &authority,
                                     std::string &file,
                                     std::string &displayName)
{
    Uri uri("");
    if (authority.empty()) {
        return uri;
    }
    if (file.empty()) {
        return uri;
    }
    SharedPathStrategy strategy = ParsePathStrategy(context, authority);
    uri = strategy.FuzzyFileToUri(file, deviceId);
    if (uri.ToString() == "ERROR_AUTHORITY" || uri.ToString() == "ERROR_JSON_CONFIG") {
        return uri;
    }
    if (uri.ToString().empty()) {
        return uri;
    }
    if (!displayName.empty()) {
        std::string path = uri.ToString();
        path = path + "?displayName=" + displayName + "#";
        Uri rUri(path);
        return rUri;
    }
    return uri;
}
json OpenJsonFile(void)
{
    json jsonData;
    std::string jsonString = "";
    std::string temp = "";
    ifstream in("/data/accounts/account_0/appdata/ohos.acts.test/files/test.json", ios::in);
    if (!in.is_open()) {
        return jsonData;
    }
    while (getline(in, temp)) {
        jsonString = jsonString + temp;
    }
    jsonData = json::parse(jsonString, nullptr, false);
    in.close();
    return jsonData;
}
SharedPathStrategy FileShareAbility::ParsePathStrategy(void)
{
    std::string authroiry = "ohos.acts.test.ability";
    SharedPathStrategy rootStrat(authroiry);
    json jsonPath = OpenJsonFile();
    if (!jsonPath.is_array()) {
        return rootStrat;
    }
    for (size_t i = 0; i < jsonPath.size(); i++) {
        string pathType, name, path;
        if (jsonPath[i][ATTR_TYPE].is_string()) {
            pathType = jsonPath[i][ATTR_TYPE];
        }
        std::string target = "";
        if (jsonPath[i][ATTR_NAME].is_string()) {
            name = jsonPath[i][ATTR_NAME];
        }
        if (jsonPath[i][ATTR_PATH].is_string()) {
            path = jsonPath[i][ATTR_PATH];
        }
        if (pathType == ROOT_PATH) {
            std::string target = "/";
        } else if (pathType == FILES_PATH) {
            target = "/data/accounts/account_0/appdata/ohos.acts.test/files/";
        } else if (pathType == CACHE_PATH) {
            target = "/data/accounts/account_0/appdata/ohos.acts.test/cache/";
        } else if (pathType == EXTERNAL_FILES_PATH) {
            target = "/storage/440E-0906/files/";
        } else if (pathType == EXTERNAL_CACHE_PATH) {
            HILOGI("fms FileShareAbility::ParsePathStrategy external_cache_path");
        } else {
            HILOGI("fms FileShareAbility::ParsePathStrategy external_path");
        }
        if (!target.empty()) {
            target = target + "/" + path;
            target = GetRealPath(target);
            rootStrat.AddRoot(name, target);
        } else {
        }
    }
    return rootStrat;
}
SharedPathStrategy FileShareAbility::ParsePathStrategy(const Ability *context, const std::string &authority)
{
    json jsonPath = OpenJsonFile();
    if (!jsonPath.is_array()) {
        return SharedPathStrategy("ERROR_JSON_CONFIG");
    }
    SharedPathStrategy strat(authority);
    for (size_t i = 0; i < jsonPath.size(); i++) {
        string pathType, name, path;
        if (jsonPath[i][ATTR_TYPE].is_string()) {
            pathType = jsonPath[i][ATTR_TYPE];
        }
        std::string target = "";
        if (jsonPath[i][ATTR_NAME].is_string()) {
            name = jsonPath[i][ATTR_NAME];
        }
        if (jsonPath[i][ATTR_PATH].is_string()) {
            path = jsonPath[i][ATTR_PATH];
        }
        if (pathType == ROOT_PATH) {
            std::string target = "/";
        } else if (pathType == FILES_PATH) {
            target = const_cast<Ability *>(context)->GetDataDir();
        } else if (pathType == CACHE_PATH) {
            target = const_cast<Ability *>(context)->GetCacheDir();
        } else if (pathType == EXTERNAL_FILES_PATH) {
            target = "/storage/440E-0906/files/";
        } else if (pathType == EXTERNAL_CACHE_PATH) {
        } else {
        }
        if (!target.empty()) {
            target = target + "/" + path;
            target = GetRealPath(target);
            strat.AddRoot(name, target);
        }
    }
    return strat;
}
} // namespace FileManager
} // namespace OHOS

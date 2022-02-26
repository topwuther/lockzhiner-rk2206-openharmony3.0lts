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

#include "update_helper.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include "hilog/log.h"
#include "parcel.h"
#include "securec.h"

namespace OHOS {
namespace update_engine {
#ifdef UPDATE_SERVICE
const std::string LOG_LABEL = "update_engine";
const std::string LOG_NAME = "/data/update_service_log.txt";
#else
const std::string LOG_LABEL = "update_engine";
const std::string LOG_NAME = "/data/update_client.log.txt";
#endif
constexpr int VECTOR_MAX_BUF_SIZE = 1024;
constexpr int MAX_TIME_SIZE = 20;
constexpr int HEX_DIGEST_NUM = 2;
constexpr int HEX_DIGEST_BASE = 16;

UpdateLogLevel UpdateHelper::level_ = UpdateLogLevel::UPDATE_INFO;

int32_t UpdateHelper::WriteUpdateContext(MessageParcel &data, const UpdateContext &info)
{
    data.WriteString16(Str8ToStr16(info.upgradeDevId));
    data.WriteString16(Str8ToStr16(info.controlDevId));
    data.WriteString16(Str8ToStr16(info.upgradeApp));
    data.WriteString16(Str8ToStr16(info.upgradeFile));
    data.WriteString16(Str8ToStr16(info.type));
    return 0;
}

int32_t UpdateHelper::ReadUpdateContext(MessageParcel &reply, UpdateContext &info)
{
    info.upgradeDevId = Str16ToStr8(reply.ReadString16());
    info.controlDevId = Str16ToStr8(reply.ReadString16());
    info.upgradeApp = Str16ToStr8(reply.ReadString16());
    info.upgradeFile = Str16ToStr8(reply.ReadString16());
    info.type = Str16ToStr8(reply.ReadString16());
    return 0;
}

int32_t UpdateHelper::WriteVersionInfo(MessageParcel &data, const VersionInfo &info)
{
    data.WriteInt32(static_cast<int32_t>(info.status));
    data.WriteString16(Str8ToStr16(info.errMsg));
    data.WriteInt32(static_cast<int32_t>(sizeof(info.result) / sizeof(info.result[0])));
    for (size_t i = 0; i < sizeof(info.result) / sizeof(info.result[0]); i++) {
        data.WriteUint64(static_cast<uint64_t>(info.result[i].size));
        data.WriteInt32(static_cast<int32_t>(info.result[i].packageType));

        data.WriteString16(Str8ToStr16(info.result[i].versionName));
        data.WriteString16(Str8ToStr16(info.result[i].versionCode));
        data.WriteString16(Str8ToStr16(info.result[i].verifyInfo));
        data.WriteString16(Str8ToStr16(info.result[i].descriptPackageId));
    }
    data.WriteInt32(static_cast<int32_t>(sizeof(info.descriptInfo) / sizeof(info.descriptInfo[0])));
    for (size_t i = 0; i < sizeof(info.descriptInfo) / sizeof(info.descriptInfo[0]); i++) {
        data.WriteString16(Str8ToStr16(info.descriptInfo[i].descriptPackageId));
        data.WriteString16(Str8ToStr16(info.descriptInfo[i].content));
    }
    return 0;
}

int32_t UpdateHelper::ReadVersionInfo(MessageParcel &reply, VersionInfo &info)
{
    info.status = static_cast<SearchStatus>(reply.ReadInt32());
    info.errMsg = Str16ToStr8(reply.ReadString16());

    int32_t count = reply.ReadInt32();
    for (size_t i = 0; (i < static_cast<size_t>(count)) && (i < sizeof(info.result) / sizeof(info.result[0])); i++) {
        uint64_t uint64 = reply.ReadUint64();
        info.result[i].size = static_cast<size_t>(uint64);
        info.result[i].packageType = static_cast<PackageType>(reply.ReadInt32());

        info.result[i].versionName = Str16ToStr8(reply.ReadString16());
        info.result[i].versionCode = Str16ToStr8(reply.ReadString16());
        info.result[i].verifyInfo = Str16ToStr8(reply.ReadString16());
        info.result[i].descriptPackageId = Str16ToStr8(reply.ReadString16());
    }
    count = reply.ReadInt32();
    for (size_t i = 0; (i < static_cast<size_t>(count))
        && (i < sizeof(info.descriptInfo) / sizeof(info.descriptInfo[0])); i++) {
        info.descriptInfo[i].descriptPackageId = Str16ToStr8(reply.ReadString16());
        info.descriptInfo[i].content = Str16ToStr8(reply.ReadString16());
    }
    return 0;
}

int32_t UpdateHelper::WriteUpdatePolicy(MessageParcel &data, const UpdatePolicy &policy)
{
    data.WriteBool(policy.autoDownload);
    data.WriteBool(policy.autoDownloadNet);
    data.WriteInt32(static_cast<int32_t>(policy.mode));
    data.WriteInt32(static_cast<int32_t>(policy.autoUpgradeCondition));
    data.WriteInt32(static_cast<int32_t>(sizeof(policy.autoUpgradeInterval) / sizeof(policy.autoUpgradeInterval[0])));
    for (size_t i = 0; i < sizeof(policy.autoUpgradeInterval) / sizeof(policy.autoUpgradeInterval[0]); i++) {
        data.WriteUint32(policy.autoUpgradeInterval[i]);
    }
    return 0;
}

int32_t UpdateHelper::ReadUpdatePolicy(MessageParcel &data, UpdatePolicy &policy)
{
    policy.autoDownload = static_cast<bool>(data.ReadBool());
    policy.autoDownloadNet = static_cast<bool>(data.ReadBool());
    policy.mode = static_cast<InstallMode>(data.ReadInt32());
    policy.autoUpgradeCondition = static_cast<AutoUpgradeCondition>(data.ReadInt32());
    int32_t count = data.ReadInt32();
    for (size_t i = 0; (i < static_cast<size_t>(count)) && (i <
        sizeof(policy.autoUpgradeInterval) / sizeof(policy.autoUpgradeInterval[0])); i++) {
        policy.autoUpgradeInterval[i] = data.ReadUint32();
    }
    return 0;
}

int32_t UpdateHelper::ReadUpgradeInfo(MessageParcel &reply, UpgradeInfo &info)
{
    info.status = static_cast<UpgradeStatus>(reply.ReadInt32());
    return 0;
}

int32_t UpdateHelper::WriteUpgradeInfo(MessageParcel &data, const UpgradeInfo &info)
{
    data.WriteInt32(info.status);
    return 0;
}

int32_t UpdateHelper::ReadUpdateProgress(MessageParcel &reply, Progress &info)
{
    info.percent = static_cast<uint32_t>(reply.ReadUint32());
    info.status = static_cast<UpgradeStatus>(reply.ReadInt32());
    info.endReason = Str16ToStr8(reply.ReadString16());
    return 0;
}

int32_t UpdateHelper::WriteUpdateProgress(MessageParcel &data, const Progress &info)
{
    data.WriteUint32(info.percent);
    data.WriteInt32(static_cast<int32_t>(info.status));
    data.WriteString16(Str8ToStr16(info.endReason));
    return 0;
}

int32_t UpdateHelper::CopyVersionInfo(const VersionInfo &srcInfo, VersionInfo &dstInfo)
{
    dstInfo.status = srcInfo.status;
    dstInfo.errMsg = srcInfo.errMsg;
    for (size_t i = 0; i < sizeof(dstInfo.result) / sizeof(dstInfo.result[0]); i++) {
        dstInfo.result[i].size = srcInfo.result[i].size;
        dstInfo.result[i].packageType = srcInfo.result[i].packageType;
        dstInfo.result[i].versionName = srcInfo.result[i].versionName;
        dstInfo.result[i].versionCode = srcInfo.result[i].versionCode;
        dstInfo.result[i].verifyInfo = srcInfo.result[i].verifyInfo;
        dstInfo.result[i].descriptPackageId = srcInfo.result[i].descriptPackageId;
    }
    for (size_t i = 0; i < sizeof(dstInfo.descriptInfo) / sizeof(dstInfo.descriptInfo[0]); i++) {
        dstInfo.descriptInfo[i].content = srcInfo.descriptInfo[i].content;
        dstInfo.descriptInfo[i].descriptPackageId = srcInfo.descriptInfo[i].descriptPackageId;
    }
    return 0;
}

int32_t UpdateHelper::CopyUpdatePolicy(const UpdatePolicy &srcInfo, UpdatePolicy &dstInfo)
{
    dstInfo.autoDownload = srcInfo.autoDownload;
    dstInfo.autoDownloadNet = srcInfo.autoDownloadNet;
    dstInfo.mode = srcInfo.mode;
    dstInfo.autoUpgradeCondition = srcInfo.autoUpgradeCondition;
    for (size_t i = 0; i < sizeof(dstInfo.autoUpgradeInterval) / sizeof(dstInfo.autoUpgradeInterval[0]); i++) {
        dstInfo.autoUpgradeInterval[i] = srcInfo.autoUpgradeInterval[i];
    }
    return 0;
}

void UpdateHelper::Logger(const std::string &fileName, int32_t line, const char *format, ...)
{
    std::string name = UpdateHelper::GetBriefFileName(fileName);
    std::ofstream logStream(LOG_NAME, std::ios::app | std::ios::out);
    static std::vector<char> buff(VECTOR_MAX_BUF_SIZE);
    va_list list;
    va_start(list, format);
    int size = vsnprintf_s(reinterpret_cast<char*>(buff.data()), buff.capacity(), buff.capacity() - 1, format, list);
    ENGINE_CHECK(size != -1, return, "");
    va_end(list);
    std::string str(buff.data(), size);
    char realTime[MAX_TIME_SIZE] = {0};
    auto sysTime = std::chrono::system_clock::now();
    auto currentTime = std::chrono::system_clock::to_time_t(sysTime);
    struct tm *localTime = std::localtime(&currentTime);
    if (localTime != nullptr) {
        std::strftime(realTime, sizeof(realTime), "%Y-%m-%d %H:%M:%S", localTime);
    }
    logStream << realTime <<  "[" << LOG_LABEL << "]" << name << " " << line << " : " << str << std::endl;
    std::cout << realTime <<  "[" << LOG_LABEL << "]" << name << " " << line << " : " << str << std::endl;
    logStream.close();
}

bool UpdateHelper::JudgeLevel(const UpdateLogLevel& level)
{
    const UpdateLogLevel& curLevel = UpdateHelper::GetLogLevel();
    if (level <= curLevel) {
        return true;
    }
    return true;
}

std::string UpdateHelper::GetBriefFileName(const std::string &file)
{
    auto pos = file.find_last_of("/");
    if (pos != std::string::npos) {
        return file.substr(pos + 1);
    }

    pos = file.find_last_of("\\");
    if (pos != std::string::npos) {
        return file.substr(pos + 1);
    }

    return file;
}

std::vector<std::string> UpdateHelper::SplitString(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> result;
    ENGINE_CHECK(!str.empty(), return result, "string is empty");

    size_t found = std::string::npos;
    size_t start = 0;
    while (true) {
        found = str.find_first_of(delimiter, start);
        result.push_back(str.substr(start, found - start));
        if (found == std::string::npos) {
            break;
        }
        start = found + 1;
    }
    return result;
}

int32_t UpdateHelper::CompareVersion(const std::string &version1, const std::string &version2)
{
    std::vector<std::string> result1 = SplitString(version1, ".");
    std::vector<std::string> result2 = SplitString(version2, ".");
    if (result1.size() != result2.size()) {
        return ((result1.size() > result2.size()) ? -1 : 1);
    }

    for (size_t i = 1; i < result1.size(); i++) {
        long long ver1 = std::stoll(result1[i]);
        long long ver2 = std::stoll(result2[i]);
        if (ver1 == ver2) {
            continue;
        }
        return ((ver1 > ver2) ? 1 : -1);
    }
    return 0;
}

std::vector<uint8_t> UpdateHelper::HexToDegist(const std::string &str)
{
    std::vector<uint8_t> result;
    for (size_t i = 0; i < str.length(); i += HEX_DIGEST_NUM) {
        std::string byte = str.substr(i, HEX_DIGEST_NUM);
        auto chr = static_cast<uint8_t>(static_cast<int>(strtol(byte.c_str(), nullptr, HEX_DIGEST_BASE)));
        result.push_back(chr);
    }
    return result;
}
}
} // namespace OHOS

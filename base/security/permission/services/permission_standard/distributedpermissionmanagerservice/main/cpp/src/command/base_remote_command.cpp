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
#include "base_remote_command.h"
#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "BaseRemoteCommand"};
}
/**
 * Check input object is valid or not.
 *
 * @param uidBundleBo UidBundleBo
 * @return True if valid.
 */
bool BaseRemoteCommand::IsValid(const UidBundleBo &uidBundleBo)
{
    for (auto bundle : uidBundleBo.bundles) {
        if (bundle.permissions.size() <= 0) {
            return true;
        }
    }
    return false;
}
/**
 * Check permission is sensitive resource permission.
 *
 * @param permission The input permission
 * @return True if it is.
 */
bool BaseRemoteCommand::IsSensitiveResource(const std::string &permission)
{
    return (permission == Constant::LOCATION || permission == Constant::CAMERA || permission == Constant::MICROPHONE ||
            permission == Constant::READ_HEALTH_DATA);
}
/**
 * Grant a sensitive permission to RUID.
 *
 * @param uidBundleBo UidBundleBo
 * @param permission A sensitive permission.
 */
void BaseRemoteCommand::GrantSensitivePermissionToRemoteApp(UidBundleBo &uidBundleBo, const std::string &permission)
{
    if (!IsSensitiveResource(permission)) {
        PERMISSION_LOG_ERROR(LABEL, "The permission is not sensitiveResource. %s ", permission.c_str());
        return;
    }
    uidBundleBo.remoteSensitivePermission.insert(permission);
}
/**
 * Check whether the sensitive permission is granted.
 *
 * @param uidBundleBo UidBundleBo
 * @param permission the sensitive permission to be checked.
 * @return true if the sensitive permission is granted.
 */
bool BaseRemoteCommand::IsGrantedSensitivePermission(const UidBundleBo &uidBundleBo, const std::string &permission)
{
    if (!IsSensitiveResource(permission)) {
        PERMISSION_LOG_ERROR(LABEL, "The permission is not sensitiveResource. %s ", permission.c_str());
        return false;
    }

    auto it = std::find_if(uidBundleBo.remoteSensitivePermission.begin(),
        uidBundleBo.remoteSensitivePermission.end(),
        [&permission](const auto &sensitivePermission) { return sensitivePermission == permission; });
    if (it != uidBundleBo.remoteSensitivePermission.end()) {
        return true;
    }
    return false;
}
/**
 * Json to RemoteProtocol
 * @param jsonObject
 */
void BaseRemoteCommand::FromRemoteProtocolJson(nlohmann::json jsonObject)
{
    if (jsonObject.find("commandName") != jsonObject.end() && jsonObject.at("commandName").is_string()) {
        remoteProtocol_.commandName = jsonObject.at("commandName").get<std::string>();
    }
    if (jsonObject.find("uniqueId") != jsonObject.end() && jsonObject.at("uniqueId").is_string()) {
        remoteProtocol_.uniqueId = jsonObject.at("uniqueId").get<std::string>();
    }
    if (jsonObject.find("requestVersion") != jsonObject.end() && jsonObject.at("requestVersion").is_number()) {
        remoteProtocol_.requestVersion = jsonObject.at("requestVersion").get<int32_t>();
    }
    if (jsonObject.find("srcDeviceId") != jsonObject.end() && jsonObject.at("srcDeviceId").is_string()) {
        remoteProtocol_.srcDeviceId = jsonObject.at("srcDeviceId").get<std::string>();
    }
    if (jsonObject.find("srcDeviceLevel") != jsonObject.end() && jsonObject.at("srcDeviceLevel").is_string()) {
        remoteProtocol_.srcDeviceLevel = jsonObject.at("srcDeviceLevel").get<std::string>();
    }
    if (jsonObject.find("dstDeviceId") != jsonObject.end() && jsonObject.at("dstDeviceId").is_string()) {
        remoteProtocol_.dstDeviceId = jsonObject.at("dstDeviceId").get<std::string>();
    }
    if (jsonObject.find("dstDeviceLevel") != jsonObject.end() && jsonObject.at("dstDeviceLevel").is_string()) {
        remoteProtocol_.dstDeviceLevel = jsonObject.at("dstDeviceLevel").get<std::string>();
    }
    if (jsonObject.find("statusCode") != jsonObject.end() && jsonObject.at("statusCode").is_number()) {
        remoteProtocol_.statusCode = jsonObject.at("statusCode").get<int32_t>();
    }
    if (jsonObject.find("message") != jsonObject.end() && jsonObject.at("message").is_string()) {
        remoteProtocol_.message = jsonObject.at("message").get<std::string>();
    }
    if (jsonObject.find("responseVersion") != jsonObject.end() && jsonObject.at("responseVersion").is_number()) {
        remoteProtocol_.responseVersion = jsonObject.at("responseVersion").get<int32_t>();
    }
    if (jsonObject.find("responseDeviceId") != jsonObject.end() && jsonObject.at("responseDeviceId").is_string()) {
        remoteProtocol_.responseDeviceId = jsonObject.at("responseDeviceId").get<std::string>();
    }
}
/**
 * Json to UidBundlePermissions
 * @param bundlePermissionsJson
 * @param uidBundlePermissions
 */
void BaseRemoteCommand::FromUidBundlePermissionsJson(const nlohmann::json &uidBundleBoJson, UidBundleBo &uidBundleBo)
{
    uidBundleBoJson.at("DEFAULT_SIZE").get_to(uidBundleBo.DEFAULT_SIZE);
    uidBundleBoJson.at("MIN_UID_PACKAGES_NUM").get_to(uidBundleBo.MIN_UID_PACKAGES_NUM);
    uidBundleBoJson.at("uid").get_to(uidBundleBo.uid);
    uidBundleBoJson.at("appAttribute").get_to(uidBundleBo.appAttribute);
    uidBundleBoJson.at("uidState").get_to(uidBundleBo.uidState);

    if ((uidBundleBoJson.find("remoteSensitivePermission") != uidBundleBoJson.end()) &&
        (uidBundleBoJson.at("remoteSensitivePermission").is_string())) {
        uidBundleBoJson.at("remoteSensitivePermission")
            .get_to<std::set<std::string>>(uidBundleBo.remoteSensitivePermission);
    }

    if (uidBundleBoJson.find("bundles") != uidBundleBoJson.end()) {
        std::vector<BundlePermissionsDto> bundles;
        nlohmann::json bundlesJson = uidBundleBoJson.at("bundles").get<nlohmann::json>();
        for (auto bundleJson : bundlesJson) {
            BundlePermissionsDto bundle;
            bundleJson.at("name").get_to(bundle.name);
            bundleJson.at("bundleLabel").get_to(bundle.bundleLabel);
            bundleJson.at("appId").get_to(bundle.appId);
            if (bundleJson.find("sign") != bundleJson.end()) {
                std::vector<SignDto> signs;
                nlohmann::json signsJson = bundleJson.at("sign").get<nlohmann::json>();
                for (auto signJson : signsJson) {
                    SignDto signDto;
                    signJson.at("sha256").get_to(signDto.sha256);
                    signs.emplace_back(signDto);
                }
                bundle.sign = signs;
            }
            if (bundleJson.find("permissions") != bundleJson.end()) {
                std::vector<PermissionDto> permissions;
                nlohmann::json permissionsJson = bundleJson.at("permissions").get<nlohmann::json>();
                for (auto permissionJson : permissionsJson) {
                    PermissionDto permission;
                    permissionJson.at("name").get_to(permission.name);
                    permissionJson.at("type").get_to(permission.type);
                    permissionJson.at("defInfo").get_to(permission.defInfo);
                    permissionJson.at("level").get_to(permission.level);
                    permissionJson.at("status").get_to(permission.status);
                    permissionJson.at("grantMode").get_to(permission.grantMode);
                    permissions.emplace_back(permission);
                }
                bundle.permissions = permissions;
            }
            bundles.push_back(bundle);
        }
        uidBundleBo.bundles = bundles;
    }
}
/**
 * RemoteProtocol to Json
 */
nlohmann::json BaseRemoteCommand::ToRemoteProtocolJson()
{
    nlohmann::json j;
    j["commandName"] = remoteProtocol_.commandName;
    j["uniqueId"] = remoteProtocol_.uniqueId;
    j["requestVersion"] = remoteProtocol_.requestVersion;
    j["srcDeviceId"] = remoteProtocol_.srcDeviceId;
    j["srcDeviceLevel"] = remoteProtocol_.srcDeviceLevel;
    j["dstDeviceId"] = remoteProtocol_.dstDeviceId;
    j["dstDeviceLevel"] = remoteProtocol_.dstDeviceLevel;
    j["statusCode"] = remoteProtocol_.statusCode;
    j["message"] = remoteProtocol_.message;
    j["responseVersion"] = remoteProtocol_.responseVersion;
    j["responseDeviceId"] = remoteProtocol_.responseDeviceId;
    return j;
}
/**
 * UidBundlePermissions to Json
 * @param bundle UidBundleBo
 * @return json
 */
nlohmann::json BaseRemoteCommand::ToUidBundlePermissionsJson(const UidBundleBo &uidBundleBo)
{
    nlohmann::json bundlesJson;
    for (auto bundle : uidBundleBo.bundles) {
        nlohmann::json permissionsJson;
        for (auto permission : bundle.permissions) {
            nlohmann::json permissionJson = nlohmann::json{
                {"name", permission.name},
                {"type", permission.type},
                {"defInfo", permission.defInfo},
                {"level", permission.level},
                {"status", permission.status},
                {"grantMode", permission.grantMode},
            };
            permissionsJson.emplace_back(permissionJson);
        }

        nlohmann::json signsJson;
        for (auto sign : bundle.sign) {
            nlohmann::json signJson = nlohmann::json{
                {"sha256", sign.sha256},
            };
            signsJson.emplace_back(signJson);
        }

        nlohmann::json bundleJson = nlohmann::json{
            {"name", bundle.name},
            {"bundleLabel", bundle.bundleLabel},
            {"appId", bundle.appId},
            {"sign", signsJson},
            {"permissions", permissionsJson},
        };
        bundlesJson.emplace_back(bundleJson);
    }

    nlohmann::json remoteSensitivePermissionJson;
    for (auto record : uidBundleBo.remoteSensitivePermission) {
        remoteSensitivePermissionJson.emplace_back(record);
    }

    nlohmann::json uidBundleBoJson = nlohmann::json{
        {"DEFAULT_SIZE", uidBundleBo.DEFAULT_SIZE},
        {"MIN_UID_PACKAGES_NUM", uidBundleBo.MIN_UID_PACKAGES_NUM},
        {"uid", uidBundleBo.uid},
        {"appAttribute", uidBundleBo.appAttribute},
        {"uidState", uidBundleBo.uidState},
        {"remoteSensitivePermission", remoteSensitivePermissionJson},
        {"bundles", bundlesJson},
    };
    return uidBundleBoJson;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

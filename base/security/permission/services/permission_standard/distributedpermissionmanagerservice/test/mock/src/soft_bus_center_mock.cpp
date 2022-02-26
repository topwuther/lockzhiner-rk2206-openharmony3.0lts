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

#include "softbus_bus_center.h"
#include <string>
#include <cstring>
#include "constant.h"
#include "permission_log.h"

using namespace OHOS::Security::Permission;
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SoftBusCenterMock"};
static const int REG_COUNT_LIMIT = 10;
}  // namespace
static int regCount_ = -1;
static INodeStateCb *callback_ = nullptr;

bool IsRegCountOK()
{
    return regCount_ >= 0 && regCount_ < REG_COUNT_LIMIT;
}

int32_t RegNodeDeviceStateCb(const char *pkgName, INodeStateCb *callback)
{
    regCount_++;
    // reg:0-9
    if (IsRegCountOK()) {
        callback_ = const_cast<INodeStateCb *>(callback);
        PERMISSION_LOG_DEBUG(LABEL, "success, pkg:%{public}s, count: %{public}d", pkgName, regCount_);
        return Constant::SUCCESS;
    }

    // count 10 above alway return failure for retry.
    PERMISSION_LOG_DEBUG(LABEL, "failure, count: %{public}d", regCount_);
    return Constant::FAILURE;
}

int32_t UnregNodeDeviceStateCb(INodeStateCb *callback)
{
    // unreg: 0-9
    if (IsRegCountOK()) {
        regCount_--;
        callback_ = nullptr;
        PERMISSION_LOG_DEBUG(LABEL, "success, count: %{public}d", regCount_);
        return Constant::SUCCESS;
    }

    if (regCount_ >= 0) {
        regCount_--;
    }
    PERMISSION_LOG_DEBUG(LABEL, "failure, count: %{public}d", regCount_);
    return Constant::SUCCESS;
}

int32_t GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info)
{
    if (IsRegCountOK()) {
        strcpy(info->deviceName, "test-device-id-001");
        strcpy(info->networkId, "test-device-id-001:network-id-001");
        info->deviceTypeId = 1;
        PERMISSION_LOG_DEBUG(LABEL, "success, count: %{public}d", regCount_);
        return Constant::SUCCESS;
    }
    PERMISSION_LOG_DEBUG(LABEL, "failure, pkg: %{public}s, count: %{public}d", pkgName, regCount_);
    return Constant::FAILURE;
}

int32_t GetNodeKeyInfo(
    const char *pkgName, const char *networkId, NodeDeivceInfoKey key, uint8_t *info, int32_t infoLen)
{
    if (networkId == nullptr || networkId[0] == '\0') {
        PERMISSION_LOG_DEBUG(LABEL, "failure, invalid networkId, pkg name: %{public}s", pkgName);
        return Constant::FAILURE;
    }

    if (IsRegCountOK()) {
        if (key == NodeDeivceInfoKey::NODE_KEY_UDID) {
            std::string temp = networkId;
            temp += ":udid-001";
            strncpy((char *)info, temp.c_str(), temp.length());
            infoLen = temp.length();
        }
        if (key == NodeDeivceInfoKey::NODE_KEY_UUID) {
            std::string temp = networkId;
            temp += ":uuid-001";
            strncpy((char *)info, temp.c_str(), temp.length());
        }
        PERMISSION_LOG_DEBUG(LABEL, "success, count: %{public}d, id: %{public}s", regCount_, info);
        return Constant::SUCCESS;
    }
    PERMISSION_LOG_DEBUG(LABEL, "failure, count: %{public}d", regCount_);
    return Constant::FAILURE;
}
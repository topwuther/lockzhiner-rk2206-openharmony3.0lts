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

#include "update_callback.h"
#include <cstring>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "iservice_registry.h"
#include "parameters.h"
#include "securec.h"
#include "system_ability_definition.h"
#include "update_helper.h"

namespace OHOS {
namespace update_engine {
void UpdateCallback::OnCheckVersionDone(const VersionInfo &info)
{
    ENGINE_LOGI("OnCheckVersionDone VersionInfo status %d", info.status);
    ENGINE_LOGI("OnCheckVersionDone VersionInfo errMsg %s", info.errMsg.c_str());
    ENGINE_LOGI("OnCheckVersionDone VersionInfo versionName : %s", info.result[0].versionName.c_str());
    ENGINE_LOGI("OnCheckVersionDone VersionInfo versionCode : %s", info.result[0].versionCode.c_str());
    ENGINE_LOGI("OnCheckVersionDone VersionInfo verifyInfo : %s", info.result[0].verifyInfo.c_str());
    ENGINE_LOGI("OnCheckVersionDone VersionInfo size : %zu", info.result[0].size);
}

void UpdateCallback::OnDownloadProgress(const Progress &progress)
{
    ENGINE_LOGI("OnDownloadProgress progress %u %d", progress.percent, progress.status);
}

void UpdateCallback::OnUpgradeProgress(const Progress &progress)
{
    ENGINE_LOGI("OnUpgradeProgress progress %u %d", progress.percent, progress.status);
}
}
} // namespace OHOS

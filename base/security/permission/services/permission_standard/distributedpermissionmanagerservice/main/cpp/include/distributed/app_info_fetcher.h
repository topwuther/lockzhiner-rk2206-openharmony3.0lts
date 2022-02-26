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

#ifndef APP_INFO_FETCHER_H
#define APP_INFO_FETCHER_H

#include <map>

#include "bundle_info.h"

namespace OHOS {
namespace Security {
namespace Permission {

enum class AppAttribute {
    // Attribute indicates the application is installed in the device's system image.
    APP_ATTRIBUTE_PRESET = 1 << 0,
    // Attribute indicates the application is permitted to hold privileged permissions.
    APP_ATTRIBUTE_PRIVILEGED = 1 << 1,
    // Attribute indicates the application is signed with the platform key.
    APP_ATTRIBUTE_SIGNED_WITH_PLATFORM_KEY = 1 << 2,
};

/**
 * Fetcher which used to get application information and then post-process.
 *
 */
class AppInfoFetcher {
public:
    static AppInfoFetcher &GetInstance();
    int32_t GetAppAttribute(int32_t uid, AppExecFwk::BundleInfo &bundleInfo);
    void RemoveAttributeBuffer(int32_t uid);

private:
    std::map<int32_t, int32_t> attributeBuffer_;
};

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif
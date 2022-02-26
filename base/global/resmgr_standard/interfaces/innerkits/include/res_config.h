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
#ifndef OHOS_RESOURCE_MANAGER_RESCONFIG_H
#define OHOS_RESOURCE_MANAGER_RESCONFIG_H

#include <unicode/locid.h>
#include <vector>
#include "res_common.h"
#include "rstate.h"

using icu::Locale;
namespace OHOS {
namespace Global {
namespace Resource {
class ResConfig {
public:
    virtual RState SetLocaleInfo(const char *language, const char *script, const char *region) = 0;

    virtual RState SetLocaleInfo(Locale &localeInfo) = 0;

    virtual void SetDeviceType(DeviceType deviceType) = 0;

    virtual void SetDirection(Direction direction) = 0;

    virtual void SetScreenDensity(ScreenDensity screenDensity) = 0;

    virtual const Locale *GetLocaleInfo() const = 0;

    virtual Direction GetDirection() const = 0;

    virtual ScreenDensity GetScreenDensity() const = 0;

    virtual DeviceType GetDeviceType() const = 0;

    virtual bool Copy(ResConfig &other) = 0;

    virtual ~ResConfig() {}
};

ResConfig *CreateResConfig();

const Locale *GetSysDefault();

void UpdateSysDefault(const Locale &localeInfo, bool needNotify);

Locale *BuildFromString(const char *str, char sep, RState &rState);

Locale *BuildFromParts(const char *language, const char *script, const char *region, RState &rState);

void FindAndSort(std::string localeStr, std::vector<std::string> &candidateLocale, std::vector<std::string> &outValue);
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif
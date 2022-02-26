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
#ifndef OHOS_RESOURCE_MANAGER_RESCONFIG_IMPL_H
#define OHOS_RESOURCE_MANAGER_RESCONFIG_IMPL_H

#include <stdint.h>
#include <unicode/locid.h>
#include "res_locale.h"
#include "res_common.h"
#include "res_config.h"

using icu::Locale;
namespace OHOS {
namespace Global {
namespace Resource {
class ResConfigImpl : public ResConfig {
public:
    ResConfigImpl();

    bool IsMoreSuitable(const ResConfigImpl *other, const ResConfigImpl *request) const;

    RState SetLocaleInfo(const char *language, const char *script, const char *region);

    RState SetLocaleInfo(Locale &localeInfo);

    void SetDeviceType(DeviceType deviceType);

    void SetDirection(Direction direction);

    void SetScreenDensity(ScreenDensity screenDensity);

    const Locale *GetLocaleInfo() const;

    const ResLocale *GetResLocale() const;

    Direction GetDirection() const;

    ScreenDensity GetScreenDensity() const;

    DeviceType GetDeviceType() const;

    bool Match(const ResConfigImpl *other) const;

    bool Copy(ResConfig &other);

    void CompleteScript();

    bool IsCompletedScript() const;

    virtual ~ResConfigImpl();

private:
    bool IsMoreSpecificThan(const ResConfigImpl *other) const;

    bool CopyLocale(ResConfig &other);

private:
    ResLocale *resLocale_;
    Direction direction_;
    ScreenDensity screenDensity_;
    DeviceType deviceType_;
    bool isCompletedScript_;
    Locale *localeInfo_;
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif
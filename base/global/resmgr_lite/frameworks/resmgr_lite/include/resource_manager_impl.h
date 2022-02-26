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
#ifndef OHOS_RESOURCE_MANAGER_RESOURCEMANAGERIMPL_H
#define OHOS_RESOURCE_MANAGER_RESOURCEMANAGERIMPL_H

#include "hap_manager.h"
#include "resource_manager.h"
#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace Global {
namespace Resource {
class ResourceManagerImpl : public ResourceManager {
public:
    ResourceManagerImpl();

    ~ResourceManagerImpl();

    bool Init();

    virtual bool AddResource(const char *path);

    virtual RState UpdateResConfig(ResConfig &resConfig);

    virtual void GetResConfig(ResConfig &resConfig);

    virtual RState GetStringById(uint32_t id, std::string &outValue);

    virtual RState GetStringByName(const char *name, std::string &outValue);

    virtual RState GetStringFormatById(std::string &outValue, uint32_t id, ...);

    virtual RState GetStringFormatByName(std::string &outValue, const char *name, ...);

    virtual RState GetStringArrayById(uint32_t id, std::vector<std::string> &outValue);

    virtual RState GetStringArrayByName(const char *name, std::vector<std::string> &outValue);

    virtual RState GetPatternById(uint32_t id, std::map<std::string, std::string> &outValue);

    virtual RState GetPatternByName(const char *name, std::map<std::string, std::string> &outValue);

    virtual RState GetPluralStringById(uint32_t id, int quantity, std::string &outValue);

    virtual RState GetPluralStringByName(const char *name, int quantity, std::string &outValue);

    virtual RState GetPluralStringByIdFormat(std::string &outValue, uint32_t id, int quantity, ...);

    virtual RState GetPluralStringByNameFormat(std::string &outValue, const char *name, int quantity, ...);

    virtual RState GetThemeById(uint32_t id, std::map<std::string, std::string> &outValue);

    virtual RState GetThemeByName(const char *name, std::map<std::string, std::string> &outValue);

    virtual RState GetBooleanById(uint32_t id, bool &outValue);

    virtual RState GetBooleanByName(const char *name, bool &outValue);

    virtual RState GetIntegerById(uint32_t id, int &outValue);

    virtual RState GetIntegerByName(const char *name, int &outValue);

    virtual RState GetFloatById(uint32_t id, float &outValue);

    virtual RState GetFloatByName(const char *name, float &outValue);

    virtual RState GetIntArrayById(uint32_t id, std::vector<int> &outValue);

    virtual RState GetIntArrayByName(const char *name, std::vector<int> &outValue);

    virtual RState GetColorById(uint32_t id, uint32_t &outValue);

    virtual RState GetColorByName(const char *name, uint32_t &outValue);

    virtual RState GetProfileById(uint32_t id, std::string &outValue);

    virtual RState GetProfileByName(const char *name, std::string &outValue);

    virtual RState GetMediaById(uint32_t id, std::string &outValue);

    virtual RState GetMediaByName(const char *name, std::string &outValue);

private:
    RState GetString(const IdItem *idItem, std::string &outValue);

    RState GetStringArray(const IdItem *idItem, std::vector<std::string> &outValue);

    RState GetPattern(const IdItem *idItem, std::map<std::string, std::string> &outValue);

    RState GetTheme(const IdItem *idItem, std::map<std::string, std::string> &outValue);

    RState GetPluralString(const HapResource::ValueUnderQualifierDir *vuqd, int quantity, std::string &outValue);

    RState ResolveReference(const std::string value, std::string &outValue);

    RState GetBoolean(const IdItem *idItem, bool &outValue);

    RState GetFloat(const IdItem *idItem, float &outValue);

    RState GetInteger(const IdItem *idItem, int &outValue);

    RState GetColor(const IdItem *idItem, uint32_t &outValue);

    RState GetIntArray(const IdItem *idItem, std::vector<int> &outValue);

    RState GetRawFile(const HapResource::ValueUnderQualifierDir *vuqd, const ResType resType, std::string &outValue);

    RState ResolveParentReference(const IdItem *idItem, std::map<std::string, std::string> &outValue);

    HapManager *hapManager_;
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif
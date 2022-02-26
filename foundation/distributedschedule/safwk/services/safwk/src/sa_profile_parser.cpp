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

#include "sa_profile_parser.h"

#include <dlfcn.h>
#include <fstream>
#include <sstream>

#include "safwk_log.h"
#include "string_ex.h"

namespace OHOS {
using std::string;

namespace {
const string TAG = "SaProfileParser";

const auto XML_TAG_PROFILE = "profile";
const auto XML_TAG_INFO = "info";
const auto XML_TAG_SYSTEM_ABILITY = "systemability";
const auto XML_TAG_LIB_PATH = "libpath";
const auto XML_TAG_NAME = "name";
const auto XML_TAG_DEPEND = "depend";
const auto XML_TAG_DEPEND_TIMEOUT = "depend-time-out";
const auto XML_TAG_RUN_ON_CREATE = "run-on-create";
const auto XML_TAG_DISTRIBUTED = "distributed";
const auto XML_TAG_DUMP_LEVEL = "dump-level";
const auto XML_TAG_CAPABILITY = "capability";
const auto XML_TAG_DEF_PERMISSION = "def-permission";
const auto XML_TAG_PERMISSION = "permission";
const auto XML_TAG_BOOT_PHASE = "bootphase";
}

SaProfileParser::~SaProfileParser()
{
    ClearResource();
}

void SaProfileParser::CloseHandle(SaProfile& saProfile)
{
    if (saProfile.handle == nullptr) {
        return;
    }
    int32_t ret = dlclose(saProfile.handle);
    if (ret) {
        HILOGW(TAG, "close handle failed with errno:%{public}d!", errno);
    }
    saProfile.handle = nullptr;
}

void SaProfileParser::CloseSo()
{
    for (auto& saProfile : saProfiles_) {
        CloseHandle(saProfile);
    }
}

void SaProfileParser::CloseSo(int32_t systemAbilityId)
{
    for (auto& saProfile : saProfiles_) {
        if (saProfile.saId == systemAbilityId) {
            CloseHandle(saProfile);
            break;
        }
    }
}

void SaProfileParser::ClearResource()
{
    CloseSo();
    saProfiles_.clear();
}

void SaProfileParser::OpenSo()
{
    for (auto& saProfile : saProfiles_) {
        if (saProfile.runOnCreate) {
            OpenSo(saProfile);
        }
    }
}

void SaProfileParser::OpenSo(SaProfile& saProfile)
{
    if (saProfile.handle == nullptr) {
        DlHandle handle = dlopen(Str16ToStr8(saProfile.libPath).c_str(), RTLD_NOW);
        if (handle == nullptr) {
            std::vector<string> libPathVec;
            string fileName = "";
            SplitStr(Str16ToStr8(saProfile.libPath), "/", libPathVec);
            if ((libPathVec.size() > 0)) {
                fileName = libPathVec[libPathVec.size() - 1];
            }
            HILOGE(TAG, "dlopen %{public}s failed with errno:%{public}d!", fileName.c_str(), errno);
            return;
        }
        saProfile.handle = handle;
    } else {
        HILOGI(TAG, "SA:%{public}d handle is not null", saProfile.saId);
    }
}

bool SaProfileParser::LoadSaLib(int32_t systemAbilityId)
{
    for (auto& saProfile : saProfiles_) {
        if (saProfile.saId == systemAbilityId) {
            OpenSo(saProfile);
            return true;
        }
    }
    return false;
}

const std::list<SaProfile>& SaProfileParser::GetAllSaProfiles() const
{
    return saProfiles_;
}

void SaProfileParser::ParseSAProp(const string& nodeName, const string& nodeContent, SaProfile& saProfile)
{
    if (nodeName == XML_TAG_NAME) {
        saProfile.saId = atoi(nodeContent.c_str());
    } else if (nodeName == XML_TAG_LIB_PATH) {
        saProfile.libPath = Str8ToStr16(nodeContent);
    } else if (nodeName == XML_TAG_DEPEND) {
        saProfile.dependSa.emplace_back(Str8ToStr16(nodeContent));
    } else if (nodeName == XML_TAG_DEPEND_TIMEOUT) {
        saProfile.dependTimeout = atoi(nodeContent.c_str());
    } else if (nodeName == XML_TAG_RUN_ON_CREATE) {
        std::istringstream(nodeContent) >> std::boolalpha >> saProfile.runOnCreate;
    } else if (nodeName == XML_TAG_DISTRIBUTED) {
        std::istringstream(nodeContent) >> std::boolalpha >> saProfile.distributed;
    } else if (nodeName == XML_TAG_DUMP_LEVEL) {
        std::stringstream ss(nodeContent);
        ss >> saProfile.dumpLevel;
    } else if (nodeName == XML_TAG_CAPABILITY) {
        saProfile.capability = Str8ToStr16(nodeContent);
    } else if ((nodeName == XML_TAG_DEF_PERMISSION) || (nodeName == XML_TAG_PERMISSION)) {
        saProfile.permission = Str8ToStr16(nodeContent);
    } else if (nodeName == XML_TAG_BOOT_PHASE) {
        saProfile.bootPhase = Str8ToStr16(nodeContent);
    }
}

bool SaProfileParser::ParseSystemAbility(const xmlNode& rootNode)
{
    auto currNodePtr = rootNode.xmlChildrenNode;
    if (currNodePtr == nullptr) {
        return false;
    }
    SaProfile saProfile;
    for (; currNodePtr != nullptr; currNodePtr = currNodePtr->next) {
        if (currNodePtr->name == nullptr || currNodePtr->type == XML_COMMENT_NODE) {
            continue;
        }
        auto contentPtr = xmlNodeGetContent(currNodePtr);
        if (contentPtr == nullptr) {
            continue;
        }
        string nodeName(reinterpret_cast<const char*>(currNodePtr->name));
        string nodeContent(reinterpret_cast<char*>(contentPtr));
        ParseSAProp(nodeName, nodeContent, saProfile);
        xmlFree(contentPtr);
    }
    saProfiles_.emplace_back(saProfile);
    return true;
}

bool SaProfileParser::ParseSaProfiles(const string& profilePath)
{
    HILOGI(TAG, "xmlFile:%{private}s", profilePath.c_str());
    std::ifstream profileStream(profilePath.c_str());
    if (!profileStream.good()) {
        HILOGE(TAG, "bad profile path!");
        return false;
    }
    xmlDocPtr ptrDoc = xmlReadFile(profilePath.c_str(), nullptr, XML_PARSE_NOBLANKS);
    if (ptrDoc == nullptr) {
        HILOGE(TAG, "xmlReadFile error!");
        return false;
    }
    xmlNodePtr rootNodePtr = xmlDocGetRootElement(ptrDoc);
    if (rootNodePtr == nullptr || rootNodePtr->name == nullptr ||
        (xmlStrcmp(rootNodePtr->name, reinterpret_cast<const xmlChar*>(XML_TAG_PROFILE)) != 0 &&
        xmlStrcmp(rootNodePtr->name, reinterpret_cast<const xmlChar*>(XML_TAG_INFO)) != 0)) {
        HILOGW(TAG, "wrong root element tag!");
        xmlFreeDoc(ptrDoc);
        return false;
    }

    bool isParseCorrect = false;
    xmlNodePtr currNodePtr = rootNodePtr->xmlChildrenNode;
    for (; currNodePtr != nullptr; currNodePtr = currNodePtr->next) {
        if (currNodePtr->name == nullptr || currNodePtr->type == XML_COMMENT_NODE) {
            continue;
        }

        string nodeName(reinterpret_cast<const char*>(currNodePtr->name));
        HILOGI(TAG, "profile nodeName:%{public}s", nodeName.c_str());
        if (nodeName == XML_TAG_SYSTEM_ABILITY) {
            bool ret = ParseSystemAbility(*currNodePtr);
            if (!ret) {
                HILOGW(TAG, "profile %{public}s wrong tag!", currNodePtr->name);
                xmlFreeDoc(ptrDoc);
                return false;
            }
            isParseCorrect = true;
        }
    }
    xmlFreeDoc(ptrDoc);
    return isParseCorrect;
}
} // namespace OHOS

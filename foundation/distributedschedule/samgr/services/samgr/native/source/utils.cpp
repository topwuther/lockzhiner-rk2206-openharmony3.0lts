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

#include "utils.h"
#include <fstream>
#include <sstream>
#include "libxml/parser.h"
#include "nlohmann/json.hpp"
#include "sam_log.h"

using namespace std;
namespace OHOS {
namespace {
    const string XML_TAG_ROOT = "systemAbility";
    const string XML_TAG_NAME = "name";
    const string SYSTEM_CAP_KEY = "systemCapability";
    const string SYS_CAP_NAME = "name";
    const string REGISTER_ON_STARTUP = "register-on-startup";
    constexpr int32_t MAX_SYSCAP_SIZE = 512;
    constexpr int32_t MAX_SYSCAP_NAME_LEN = 64;
    template<typename T>
    inline bool CheckTagName(const T curNodePtr, const string& tagName)
    {
        if (curNodePtr == nullptr || curNodePtr->name == nullptr) {
            return false;
        }
        return xmlStrcmp(curNodePtr->name, reinterpret_cast<const xmlChar*>(tagName.c_str())) == 0;
    }
}

uint64_t Utils::StrToUint64(const string& str)
{
    istringstream buf(str);
    uint64_t num = MAX_U64_VAL;
    if ((buf >> num) && (buf.eof())) {
        return num;
    }
    return MAX_U64_VAL;
}

bool Utils::ParseCoreSaList(const string& saPath, set<int32_t>& saList)
{
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(saPath.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        HILOGE("Utils::coreSa::ParseCoreSaList xmlReadFile error!");
        return false;
    }
    xmlNodePtr rootNodePtr = xmlDocGetRootElement(docPtr.get());
    if ((rootNodePtr == nullptr) || (!CheckTagName(rootNodePtr, XML_TAG_ROOT))) {
        HILOGW("Utils::coreSa::ParseCoreSaList root element tag wrong!");
        return false;
    }
    return ParseSystemAbility(*rootNodePtr, saList);
}

bool Utils::ParseSystemAbility(const xmlNode& rootNode, set<int32_t>& saList)
{
    auto currNodePtr = rootNode.xmlChildrenNode;
    if (currNodePtr == nullptr) {
        return false;
    }
    for (; currNodePtr != nullptr; currNodePtr = currNodePtr->next) {
        if (currNodePtr->name == nullptr || currNodePtr->type == XML_COMMENT_NODE) {
            continue;
        }
        auto contentPtr = xmlNodeGetContent(currNodePtr);
        if (contentPtr == nullptr) {
            continue;
        }
        std::string nodeName(reinterpret_cast<const char*>(currNodePtr->name));
        std::string nodeContent(reinterpret_cast<char*>(contentPtr));
        HILOGI("Utils::coreSa::ParseSystemAbility nodeName:%{public}s nodeContent:%{public}s",
            nodeName.c_str(), nodeContent.c_str());
        if (nodeName == XML_TAG_NAME) {
            int32_t systemAbilityId = atoi(nodeContent.c_str());
            saList.emplace(systemAbilityId);
        }
        xmlFree(contentPtr);
    }
    return true;
}

bool Utils::ParseSysCapMap(const string& sysCapPath, map<string, bool>& sysCapMap)
{
    ifstream sysCapStream(sysCapPath.c_str());
    if (!sysCapStream.good()) {
        HILOGI("ParseSysCapMap ifstream bad!");
        return false;
    }
    auto json = nlohmann::json::parse(sysCapStream, nullptr, false);
    if (json.is_discarded()) {
        HILOGW("Utils::ParseSysCapMap sysCapStream exception");
        return false;
    }
    if (json.find(SYSTEM_CAP_KEY) == json.end()) {
        return false;
    }
    auto sysCap = json.at(SYSTEM_CAP_KEY);
    if (!sysCap.is_array()) {
        return false;
    }
    size_t sysCapSize = sysCap.size();
    int sysCapNum = 0;
    for (size_t i = 0; i < sysCapSize; i++) {
        if (sysCapNum >= MAX_SYSCAP_SIZE) {
            HILOGE("ParseSysCapMap system capability exceed!");
            break;
        }
        if (!sysCap[i].is_object()) {
            continue;
        }
        auto item = sysCap[i];
        if (item.find(SYS_CAP_NAME) == item.end() || item.find(REGISTER_ON_STARTUP) == item.end()) {
            continue;
        }
        auto nameJson = item.at(SYS_CAP_NAME);
        auto isRegJson = item.at(REGISTER_ON_STARTUP);
        if (!nameJson.is_string() || !isRegJson.is_boolean()) {
            continue;
        }
        auto name = nameJson.get<string>();
        if (name.empty() || name.length() > MAX_SYSCAP_NAME_LEN) {
            HILOGW("ParseSysCapMap system capability invalid");
            continue;
        }
        auto isRegister = isRegJson.get<bool>();
        if (sysCapMap.count(name) != 0) {
            HILOGW("ParseSysCapMap duplicate system capability %s", name.c_str());
            continue;
        }
        sysCapMap[name] = isRegister;
        sysCapNum++;
    }
    return true;
}
} // namespace OHOS
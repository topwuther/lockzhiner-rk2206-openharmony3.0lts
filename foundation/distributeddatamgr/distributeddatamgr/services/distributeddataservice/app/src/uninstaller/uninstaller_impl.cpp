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

#define LOG_TAG "UninstallerImpl"

#include "uninstaller_impl.h"
#include <thread>
#include <unistd.h>
#include "common_event_manager.h"
#include "common_event_support.h"
#include "kvstore_meta_manager.h"
#include "log_print.h"
#include "ohos/aafwk/base/array_wrapper.h"
#include "ohos/aafwk/base/string_wrapper.h"

namespace OHOS::DistributedKv {
using namespace OHOS::EventFwk;
using namespace OHOS::AAFwk;
const std::string UninstallEventSubscriber::USER_ID = "userId";
const std::string PACKAGE_SCHEME = "package";
const std::string SCHEME_SPLIT = ":";
const std::string EXTRA_REPLACING = "intent.extra.REPLACING";

UninstallEventSubscriber::UninstallEventSubscriber(const CommonEventSubscribeInfo &info,
    UninstallEventCallback callback)
    : CommonEventSubscriber(info), callback_(callback)
{}

void UninstallEventSubscriber::OnReceiveEvent(const CommonEventData &event)
{
    ZLOGI("Intent Action Rec");
    Want want = event.GetWant();
    std::string action = want.GetAction();
    if (action != CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        return;
    }

    auto params = want.GetParams();
    auto &paramMap = params.GetParams();
    for (auto &[key, value] : paramMap) {
        size_t pos = key.rfind(EXTRA_REPLACING);
        if (pos != std::string::npos) {
            if (want.GetBoolParam(key, false)) {
                ZLOGI("is update package!");
                return;
            }
            break;
        }
    }

    auto uri = want.GetUri().ToString();
    if (uri.size() < (PACKAGE_SCHEME.size() + SCHEME_SPLIT.size())) {
        ZLOGW("invalid intent Uri!");
        return;
    }
    std::string bundleName = uri.substr(PACKAGE_SCHEME.size() + SCHEME_SPLIT.size());
    ZLOGI("bundleName is %s", bundleName.c_str());

    int userId = want.GetIntParam(USER_ID, -1);
    callback_(bundleName, userId);
}

UninstallerImpl::~UninstallerImpl()
{
    auto code = CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    if (!code) {
        ZLOGW("unsubscribe failed code=%d", code);
    }
}

Status UninstallerImpl::Init(KvStoreDataService *kvStoreDataService)
{
    if (kvStoreDataService == nullptr) {
        ZLOGW("kvStoreDataService is null.");
        return Status::INVALID_ARGUMENT;
    }
    MatchingSkills matchingSkills;
    matchingSkills.AddScheme(PACKAGE_SCHEME);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    CommonEventSubscribeInfo info(matchingSkills);
    auto callback = [kvStoreDataService](const std::string &bundleName, int userId) {
        ZLOGI("uninstalled bundleName:%s, userId:%d", bundleName.c_str(), userId);
        KvStoreMetaData kvStoreMetaData;
        if (!KvStoreMetaManager::GetInstance().GetKvStoreMetaDataByBundleName(bundleName, kvStoreMetaData)) {
            return;
        }
        if (!kvStoreMetaData.appId.empty() && !kvStoreMetaData.storeId.empty()) {
            ZLOGI("Has been uninstalled bundleName:%s", bundleName.c_str());
            AppId appid = {kvStoreMetaData.bundleName};
            StoreId storeId = {kvStoreMetaData.storeId};
            kvStoreDataService->DeleteKvStore(appid, storeId, kvStoreMetaData.appId);
        }
    };
    subscriber_ = std::make_shared<UninstallEventSubscriber>(info, callback);
    std::thread th = std::thread([this] {
        int tryTimes = 0;
        constexpr int MAX_RETRY_TIME = 300;
        constexpr int RETRY_WAIT_TIME_S = 1;

        // we use this method to make sure regist success
        while (tryTimes < MAX_RETRY_TIME) {
            auto result = CommonEventManager::SubscribeCommonEvent(subscriber_);
            if (result) {
                ZLOGI("EventManager: Success");
                break;
            } else {
                ZLOGE("EventManager: Fail to Register Subscriber, error:%d", result);
                sleep(RETRY_WAIT_TIME_S);
            }
            tryTimes++;
        }
        if (MAX_RETRY_TIME == tryTimes) {
            ZLOGE("EventManager: Fail to Register Subscriber!!!");
        }
        ZLOGI("Register listener End!!");
    });
    th.detach();
    return Status::SUCCESS;
}
}

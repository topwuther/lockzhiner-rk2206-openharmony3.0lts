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

#ifndef DISTRIBUTEDDATAMGR_APP_KVSTORE_CORRUPTION_OBSERVER_H
#define DISTRIBUTEDDATAMGR_APP_KVSTORE_CORRUPTION_OBSERVER_H
namespace OHOS {
namespace AppDistributedKv {
class AppKvStoreCorruptionObserver {
public:
    KVSTORE_API virtual ~AppKvStoreCorruptionObserver() {};
    KVSTORE_API
    virtual void OnCorruption(const std::string &appId, const std::string &userId, const std::string &storeId) = 0;
};
}  // namespace AppDistributedKv
}  // namespace OHOS
#endif // DISTRIBUTEDDATAMGR_APP_KVSTORE_CORRUPTION_OBSERVER_H

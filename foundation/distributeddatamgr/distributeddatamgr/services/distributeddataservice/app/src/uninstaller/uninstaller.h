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

#ifndef DISTRIBUTEDDATAMGR_UNINSTALLER_H
#define DISTRIBUTEDDATAMGR_UNINSTALLER_H

#include <memory.h>
#include <mutex>

#include "kvstore_data_service.h"
#include "visibility.h"

namespace OHOS::DistributedKv {
class Uninstaller {
public:
    KVSTORE_API virtual Status Init(KvStoreDataService *kvStoreDataService) = 0;
    KVSTORE_API virtual ~Uninstaller() {};
    KVSTORE_API static Uninstaller &GetInstance();
};
}
#endif // DISTRIBUTEDDATAMGR_UNINSTALLER_H

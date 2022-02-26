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

#ifndef SYNC_TYPES_H
#define SYNC_TYPES_H

namespace DistributedDB {
enum MessageId {
    TIME_SYNC_MESSAGE = 1,
    DATA_SYNC_MESSAGE,
    COMMIT_HISTORY_SYNC_MESSAGE,
    MULTI_VER_DATA_SYNC_MESSAGE,
    VALUE_SLICE_SYNC_MESSAGE,
    LOCAL_DATA_CHANGED,
    ABILITY_SYNC_MESSAGE,
};

const static uint32_t SEND_TIME_OUT = 3000; // 3s
const int NOT_SURPPORT_SEC_CLASSIFICATION = 0xff;
}

#endif
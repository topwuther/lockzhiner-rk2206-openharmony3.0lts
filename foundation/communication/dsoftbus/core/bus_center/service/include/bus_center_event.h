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

#ifndef BUS_CENTER_EVENT_H
#define BUS_CENTER_EVENT_H

#include <stdbool.h>
#include <stdint.h>

#include "softbus_bus_center.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*LnnOnTimeSyncResult)(const char *pkgName, const TimeSyncResultInfo *info, int32_t retCode);

void LnnNotifyJoinResult(ConnectionAddr *addr,
    const char *networkId, int32_t retCode);
void LnnNotifyLeaveResult(const char *networkId, int32_t retCode);

void LnnNotifyOnlineState(bool isOnline, NodeBasicInfo *info);
void LnnNotifyBasicInfoChanged(NodeBasicInfo *info, NodeBasicInfoType type);

void LnnNotifyTimeSyncResult(const char *pkgName, const TimeSyncResultInfo *info, int32_t retCode);

#ifdef __cplusplus
}
#endif
#endif

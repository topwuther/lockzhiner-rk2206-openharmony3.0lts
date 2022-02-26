/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTEDSCHEDULE_MSGPARSER_H
#define OHOS_DISTRIBUTEDSCHEDULE_MSGPARSER_H

#include "dmslite_inner_common.h"
#include "dmslite_tlv_common.h"

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
* @brief The entry of processing commucation or xts testing
* @param commuMessage incoming message from remote
* @param dmsFeatureCallback callbacks for notification
* @return DmsLiteCommonErrorCode
*/
int32_t ProcessCommuMsg(const CommuMessage *commuMessage, const IDmsFeatureCallback *dmsFeatureCallback);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* OHOS_DISTRIBUTEDSCHEDULE_MSGPARSER_H */

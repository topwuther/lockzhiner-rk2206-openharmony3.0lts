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

#ifndef OHOS_DMSLITE_MSG_HANDLER_H
#define OHOS_DMSLITE_MSG_HANDLER_H

#include "dmslite_inner_common.h"
#include "dmslite_tlv_common.h"

int32_t StartAbilityFromRemoteHandler(const TlvNode *tlvHead, StartAbilityCallback onStartAbilityDone);
int32_t ReplyMsgHandler(const TlvNode *tlvHead);

#endif // OHOS_DMSLITE_MSG_HANDLER_H
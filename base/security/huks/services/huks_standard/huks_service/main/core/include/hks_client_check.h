/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HKS_CLIENT_CHECK_H
#define HKS_CLIENT_CHECK_H

#include "hks_type_inner.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t HksCheckProcessNameAndKeyAlias(const struct HksBlob *processName, const struct HksBlob *keyAlias);

int32_t HksCheckGenAndImportKeyParams(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSetIn, const struct HksBlob *key);

int32_t HksCheckAllParams(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *data1, const struct HksBlob *data2);

int32_t HksCheckGetKeyParamSetParams(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet);

int32_t HksCheckGenerateRandomParams(const struct HksBlob *processName, const struct HksBlob *random);

int32_t HksCheckExportPublicKeyParams(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksBlob *key);

int32_t HksCheckDeriveKeyParams(const struct HksBlob *processName, const struct HksParamSet *paramSet,
    const struct HksBlob *mainKey, const struct HksBlob *derivedKey);

int32_t HksCheckGetKeyInfoListParams(const struct HksBlob *processName, const struct HksKeyInfo *keyInfoList,
    const uint32_t *listCount);

int32_t HksCheckInitParams(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const uint64_t *operationHandle);

#ifdef __cplusplus
}
#endif

#endif

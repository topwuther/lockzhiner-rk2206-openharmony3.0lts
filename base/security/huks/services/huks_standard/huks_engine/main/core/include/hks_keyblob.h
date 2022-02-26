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

#ifndef HKS_KEYBLOB_H
#define HKS_KEYBLOB_H

#include "hks_type_inner.h"

#define HKS_KEY_BLOB_DERIVE_SALT_SIZE 16
#define HKS_KEY_BLOB_DERIVE_CNT 1000
#define HKS_KEY_BLOB_TAG_SIZE 16
#define HKS_KEY_BLOB_NONCE_SIZE 12
#define HKS_KEY_BLOB_MAIN_KEY_SIZE 32

enum HksKeyNodeStatus {
    HKS_KEYNODE_INACTIVE = 0x0,
    HKS_KEYNODE_ACTIVE = 0x2f2f,
};

struct HksKeyNode {
    struct HksParamSet *paramSet;
    uint32_t refCnt;
    uint32_t status;
    uint64_t handle;
};

#ifdef __cplusplus
extern "C" {
#endif

struct HksKeyNode *HksGenerateKeyNode(const struct HksBlob *key);

void HksFreeKeyNode(struct HksKeyNode **keyNode);

int32_t HksBuildKeyBlob(const struct HksBlob *keyAlias, uint8_t keyFlag, const struct HksBlob *key,
    const struct HksParamSet *paramSet, struct HksBlob *keyOut);

int32_t HksGetEncryptKey(struct HksBlob *mainKey);

int32_t HksGetRawKey(const struct HksParamSet *paramSet, struct HksBlob *rawKey);

#ifdef __cplusplus
}
#endif

#endif

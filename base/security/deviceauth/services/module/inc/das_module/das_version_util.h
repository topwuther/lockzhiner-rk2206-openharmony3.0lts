/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef DAS_VERSION_UTIL_H
#define DAS_VERSION_UTIL_H

#include "version_util.h"

typedef enum {
    INITIAL,
    VERSION_CONFIRM,
    VERSION_DECIDED,
} VersionAgreementStatus;

typedef struct VersionInfoT {
    int opCode;
    VersionAgreementStatus versionStatus;
    VersionStruct minVersion;
    VersionStruct curVersion;
} VersionInfo;

int32_t GetVersionFromJson(const CJson *jsonObj, VersionStruct *min, VersionStruct *max);
int32_t AddVersionToJson(CJson *jsonObj, const VersionStruct *min, const VersionStruct *max);

bool IsVersionEqual(VersionStruct *src, VersionStruct *des);
int32_t NegotiateVersion(VersionStruct *minVersionPeer, VersionStruct *curVersionPeer,
    VersionStruct *curVersionSelf);

ProtocolType GetPrototolType(VersionStruct *curVersion, OperationCode opCode);

AlgType GetSupportedPakeAlg(VersionStruct *curVersion);

#endif
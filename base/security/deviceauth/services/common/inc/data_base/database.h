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

#ifndef DATABASE_H
#define DATABASE_H

#include "hc_string.h"
#include "hc_tlv_parser.h"
#include "hc_vector.h"

#define MAX_EXPIRE_TIME 90

DECLARE_HC_VECTOR(StringVector, HcString)
DECLARE_HC_VECTOR(Int64Vector, int64_t)

typedef struct {
    HcString name; /* group name */
    HcString id; /* group id */
    int32_t type; /* including identical account group(1), peer to peer group(256), across account group(1282) */
    int32_t visibility; /* visibility of the group */
    int32_t expireTime; /* the time of group expired, unit day, user config */
    int64_t userId; /* user account id */
    Int64Vector sharedUserIdVec; /* shared user account id vector */
    StringVector managers; /* group manager vector, group manager can add and delete members, index 0 is the owner */
    StringVector friends; /* group friend vector, group friend can query group information */
} TrustedGroupEntry;
DECLARE_HC_VECTOR(TrustedGroupTable, TrustedGroupEntry*)

typedef struct {
    TrustedGroupEntry* groupEntry;
    HcString udid; /* unique device id */
    HcString authId; /* id by service defined for authentication */
    HcString serviceType; /* compatible with previous versions, the value is the same as groupId */
    HcParcel ext; /* for caching extern data, user data */
    uint8_t credential; /* 1 - asymmetrical, 2 - symmetrical */
    uint8_t devType; /* 0 - accessory, 1 - controller, 2 - proxy */
    int64_t userId; /* user account id */
    uint64_t lastTm; /* accessed time of the device of the auth information, absolute time */
} TrustedDeviceEntry;
DECLARE_HC_VECTOR(TrustedDeviceTable, TrustedDeviceEntry)

typedef struct {
    DECLARE_TLV_STRUCT(9)
    TlvString name;
    TlvString id;
    TlvUint32 type;
    TlvInt32 visibility;
    TlvInt32 expireTime;
    TlvInt64 userId;
    TlvBuffer sharedUserIdVec;
    TlvBuffer managers;
    TlvBuffer friends;
} TlvGroupElement;
DECLEAR_INIT_FUNC(TlvGroupElement)
DECLARE_TLV_VECTOR(TlvGroupVec, TlvGroupElement)

typedef struct {
    uint8_t credential;
    uint8_t devType;
    int64_t userId;
    uint64_t lastTm;
} DevAuthFixedLenInfo;
DECLARE_TLV_FIX_LENGTH_TYPE(TlvDevAuthFixedLenInfo, DevAuthFixedLenInfo)
DECLEAR_INIT_FUNC(TlvDevAuthFixedLenInfo)

typedef struct {
    DECLARE_TLV_STRUCT(6)
    TlvString groupId;
    TlvString udid;
    TlvString authId;
    TlvString serviceType;
    TlvBuffer ext;
    TlvDevAuthFixedLenInfo info;
} TlvDevAuthElement;
DECLEAR_INIT_FUNC(TlvDevAuthElement)
DECLARE_TLV_VECTOR(TlvDevAuthVec, TlvDevAuthElement)

typedef struct {
    DECLARE_TLV_STRUCT(3)
    TlvInt32 version;
    TlvGroupVec groups;
    TlvDevAuthVec deviceAuthInfos;
} HCDataBaseV1;
DECLEAR_INIT_FUNC(HCDataBaseV1)

typedef struct {
    HcString name; /* group name */
    HcString id; /* group id */
    HcString ownerName; /* group owner name */
    int32_t type; /* group type, 0 - invalid, 1 - the same account, 2 - different account, 3 - none account(p2p) */
    int32_t visibility; /* visibility of the group */
    int32_t expireTime; /* the time of group expired, unit day, user config */
    int64_t userId; /* user account id */
    int64_t sharedUserId; /* shared user account id */
} GroupInfo;
DECLARE_HC_VECTOR(GroupInfoVec, void *)

typedef struct {
    HcString udid; /* unique device id */
    HcString authId; /* id by service defined for authentication */
    uint8_t credential; /* 1 - asymmetrical, 2 - symmetrical */
    uint8_t devType; /* 0 - accessory, 1 - controller, 2 - proxy */
    int64_t userId; /* user account id */
    HcString groupId; /* map the device authentication data for group */
    HcString serviceType; /* compatible with previous versions, the value is the same as groupId */
} DeviceInfo;
DECLARE_HC_VECTOR(DeviceInfoVec, void *)

#endif

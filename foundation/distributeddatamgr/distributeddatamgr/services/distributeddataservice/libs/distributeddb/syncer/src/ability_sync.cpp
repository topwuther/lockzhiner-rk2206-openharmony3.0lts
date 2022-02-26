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

#include "ability_sync.h"

#include "message_transform.h"
#include "version.h"
#include "db_errno.h"
#include "log_print.h"
#include "sync_types.h"
#include "single_ver_kvdb_sync_interface.h"
#include "single_ver_sync_task_context.h"

namespace DistributedDB {
AbilitySyncRequestPacket::AbilitySyncRequestPacket()
    : protocolVersion_(ABILITY_SYNC_VERSION_V1),
      sendCode_(E_OK),
      softwareVersion_(SOFTWARE_VERSION_CURRENT),
      secLabel_(0),
      secFlag_(0),
      schemaType_(0)
{
}

AbilitySyncRequestPacket::~AbilitySyncRequestPacket()
{
}

void AbilitySyncRequestPacket::SetProtocolVersion(uint32_t protocolVersion)
{
    protocolVersion_ = protocolVersion;
}

uint32_t AbilitySyncRequestPacket::GetProtocolVersion() const
{
    return protocolVersion_;
}

void AbilitySyncRequestPacket::SetSendCode(int32_t sendCode)
{
    sendCode_ = sendCode;
}

int32_t AbilitySyncRequestPacket::GetSendCode() const
{
    return sendCode_;
}

void AbilitySyncRequestPacket::SetSoftwareVersion(uint32_t swVersion)
{
    softwareVersion_ = swVersion;
}

uint32_t AbilitySyncRequestPacket::GetSoftwareVersion() const
{
    return softwareVersion_;
}

void AbilitySyncRequestPacket::SetSchema(const std::string &schema)
{
    schema_ = schema;
}

void AbilitySyncRequestPacket::GetSchema(std::string &schema) const
{
    schema = schema_;
}

void AbilitySyncRequestPacket::SetSchemaType(uint32_t schemaType)
{
    schemaType_ = schemaType;
}

uint32_t AbilitySyncRequestPacket::GetSchemaType() const
{
    return schemaType_;
}

void AbilitySyncRequestPacket::SetSecLabel(int32_t secLabel)
{
    secLabel_ = secLabel;
}

int32_t AbilitySyncRequestPacket::GetSecLabel() const
{
    return secLabel_;
}

void AbilitySyncRequestPacket::SetSecFlag(int32_t secFlag)
{
    secFlag_ = secFlag;
}

int32_t AbilitySyncRequestPacket::GetSecFlag() const
{
    return secFlag_;
}

uint32_t AbilitySyncRequestPacket::CalculateLen() const
{
    uint64_t len = 0;
    len += Parcel::GetUInt32Len(); // protocolVersion_
    len += Parcel::GetIntLen(); // sendCode_
    len += Parcel::GetUInt32Len(); // softwareVersion_
    uint32_t schemLen = Parcel::GetStringLen(schema_);
    if (schemLen == 0) {
        LOGE("[AbilitySyncRequestPacket][CalculateLen] schemLen err!");
        return 0;
    }
    len += schemLen;
    len += Parcel::GetIntLen(); // secLabel_
    len += Parcel::GetIntLen(); // secFlag_
    len += Parcel::GetUInt32Len(); // schemaType_
    // the reason why not 8-byte align is that old version is not 8-byte align
    // so it is not possible to set 8-byte align for high version.
    if (len > INT32_MAX) {
        LOGE("[AbilitySyncRequestPacket][CalculateLen]  err len:%llu", len);
        return 0;
    }
    return len;
}

AbilitySyncAckPacket::AbilitySyncAckPacket()
    : protocolVersion_(ABILITY_SYNC_VERSION_V1),
      softwareVersion_(SOFTWARE_VERSION_CURRENT),
      ackCode_(E_OK),
      secLabel_(0),
      secFlag_(0),
      schemaType_(0),
      permitSync_(0),
      requirePeerConvert_(0)
{
}

AbilitySyncAckPacket::~AbilitySyncAckPacket()
{
}

void AbilitySyncAckPacket::SetProtocolVersion(uint32_t protocolVersion)
{
    protocolVersion_ = protocolVersion;
}

void AbilitySyncAckPacket::SetSoftwareVersion(uint32_t swVersion)
{
    softwareVersion_ = swVersion;
}

uint32_t AbilitySyncAckPacket::GetSoftwareVersion() const
{
    return softwareVersion_;
}

uint32_t AbilitySyncAckPacket::GetProtocolVersion() const
{
    return protocolVersion_;
}

void AbilitySyncAckPacket::SetAckCode(int32_t ackCode)
{
    ackCode_ = ackCode;
}

int32_t AbilitySyncAckPacket::GetAckCode() const
{
    return ackCode_;
}

void AbilitySyncAckPacket::SetSchema(const std::string &schema)
{
    schema_ = schema;
}

void AbilitySyncAckPacket::GetSchema(std::string &schema) const
{
    schema = schema_;
}

void AbilitySyncAckPacket::SetSchemaType(uint32_t schemaType)
{
    schemaType_ = schemaType;
}

uint32_t AbilitySyncAckPacket::GetSchemaType() const
{
    return schemaType_;
}

void AbilitySyncAckPacket::SetSecLabel(int32_t secLabel)
{
    secLabel_ = secLabel;
}

int32_t AbilitySyncAckPacket::GetSecLabel() const
{
    return secLabel_;
}

void AbilitySyncAckPacket::SetSecFlag(int32_t secFlag)
{
    secFlag_ = secFlag;
}

int32_t AbilitySyncAckPacket::GetSecFlag() const
{
    return secFlag_;
}

void AbilitySyncAckPacket::SetPermitSync(uint32_t permitSync)
{
    permitSync_ = permitSync;
}

uint32_t AbilitySyncAckPacket::GetPermitSync() const
{
    return permitSync_;
}

void AbilitySyncAckPacket::SetRequirePeerConvert(uint32_t requirePeerConvert)
{
    requirePeerConvert_ = requirePeerConvert;
}

uint32_t AbilitySyncAckPacket::GetRequirePeerConvert() const
{
    return requirePeerConvert_;
}

uint32_t AbilitySyncAckPacket::CalculateLen() const
{
    uint64_t len = 0;
    len += Parcel::GetUInt32Len();
    len += Parcel::GetUInt32Len();
    len += Parcel::GetIntLen();
    uint32_t schemLen = Parcel::GetStringLen(schema_);
    if (schemLen == 0) {
        LOGE("[AbilitySyncAckPacket][CalculateLen] schemLen err!");
        return 0;
    }
    len += schemLen;
    len += Parcel::GetIntLen(); // secLabel_
    len += Parcel::GetIntLen(); // secFlag_
    len += Parcel::GetUInt32Len(); // schemaType_
    len += Parcel::GetUInt32Len(); // permitSync_
    len += Parcel::GetUInt32Len(); // requirePeerConvert_
    if (len > INT32_MAX) {
        LOGE("[AbilitySyncAckPacket][CalculateLen]  err len:%llu", len);
        return 0;
    }
    return len;
}

AbilitySync::AbilitySync()
    : communicator_(nullptr),
      storageInterface_(nullptr),
      syncFinished_(false)
{
}

AbilitySync::~AbilitySync()
{
    communicator_ = nullptr;
    storageInterface_ = nullptr;
}

int AbilitySync::Initialize(ICommunicator *inCommunicator, IKvDBSyncInterface *inStorage, const std::string &deviceId)
{
    if (inCommunicator == nullptr || inStorage == nullptr || deviceId.empty()) {
        return -E_INVALID_ARGS;
    }
    communicator_ = inCommunicator;
    storageInterface_ = inStorage;
    deviceId_ = deviceId;
    return E_OK;
}

int AbilitySync::SyncStart(uint32_t sessionId, uint32_t sequenceId, uint16_t remoteCommunicatorVersion,
    const CommErrHandler &handler)
{
    Message *message = new (std::nothrow) Message(ABILITY_SYNC_MESSAGE);
    if (message == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    AbilitySyncRequestPacket packet;
    packet.SetProtocolVersion(ABILITY_SYNC_VERSION_V1);
    packet.SetSoftwareVersion(SOFTWARE_VERSION_CURRENT);
    SchemaObject schemaObj = (static_cast<SingleVerKvDBSyncInterface *>(storageInterface_))->GetSchemaInfo();
    // 102 version is forbidden to sync with 103 json-schema or flatbuffer-schema
    // so schema should put null string while remote is 102 version to avoid this bug.
    if (remoteCommunicatorVersion == 1) {
        packet.SetSchema("");
        packet.SetSchemaType(0);
    } else {
        packet.SetSchema(schemaObj.ToSchemaString());
        packet.SetSchemaType(static_cast<uint32_t>(schemaObj.GetSchemaType()));
    }
    SecurityOption option;
    GetPacketSecOption(option);
    packet.SetSecLabel(option.securityLabel);
    packet.SetSecFlag(option.securityFlag);

    message->SetMessageType(TYPE_REQUEST);
    int errCode = message->SetCopiedObject<>(packet);
    if (errCode != E_OK) {
        LOGE("[AbilitySync][SyncStart] SetCopiedObject failed, err %d", errCode);
        delete message;
        message = nullptr;
        return errCode;
    }
    message->SetVersion(MSG_VERSION_EXT);
    message->SetSessionId(sessionId);
    message->SetSequenceId(sequenceId);
    LOGI("[AbilitySync][SyncStart] software version = %u, Label = %d, Flag = %d", SOFTWARE_VERSION_CURRENT,
        option.securityLabel, option.securityFlag);
    errCode = communicator_->SendMessage(deviceId_, message, false, SEND_TIME_OUT, handler);
    if (errCode != E_OK) {
        LOGE("[AbilitySync][SyncStart] SendPacket failed, err %d", errCode);
        delete message;
        message = nullptr;
    }
    return errCode;
}

int AbilitySync::AckRecv(const Message *message, ISyncTaskContext *context)
{
    if (message == nullptr || context == nullptr) {
        return -E_INVALID_ARGS;
    }
    const AbilitySyncAckPacket *packet = message->GetObject<AbilitySyncAckPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    int errCode = CheckAckCode(message, context, packet->GetAckCode());
    if (errCode != E_OK) {
        return errCode;
    }
    uint32_t remoteSoftwareVersion = packet->GetSoftwareVersion();
    context->SetRemoteSoftwareVersion(remoteSoftwareVersion);
    std::string schema;
    packet->GetSchema(schema);
    SingleVerKvDBSyncInterface *storage = static_cast<SingleVerKvDBSyncInterface *>(storageInterface_);
    if (remoteSoftwareVersion > SOFTWARE_VERSION_RELEASE_2_0) {
        HandleVersionV3AckSecOptionParam(packet, context);
        SyncOpinion localSyncOpinion = HandleVersionV3AckSchemaParam(packet, schema, context);
        bool permitSync = ((static_cast<SingleVerSyncTaskContext *>(context))->GetSyncStrategy()).permitSync;
        if (!permitSync) {
            (static_cast<SingleVerSyncTaskContext *>(context))->SetTaskErrCode(-E_SCHEMA_MISMATCH);
            LOGE("[AbilitySync][AckRecv] scheme check failed");
            return -E_SCHEMA_MISMATCH;
        }
        (void)SendAck(message, storage->GetSchemaInfo(), AbilitySync::CHECK_SUCCESS, localSyncOpinion, true);
        (static_cast<SingleVerSyncTaskContext *>(context))->SetIsSchemaSync(true);
    } else {
        bool isCompatible = storage->CheckCompatible(schema);
        if (!isCompatible) {
            (static_cast<SingleVerSyncTaskContext *>(context))->SetTaskErrCode(-E_SCHEMA_MISMATCH);
            LOGE("[AbilitySync][AckRecv] scheme check failed");
            return -E_SCHEMA_MISMATCH;
        }
        LOGI("[AbilitySync][AckRecv]remoteSoftwareVersion = %u, isCompatible = %d,", remoteSoftwareVersion,
            isCompatible);
    }
    return E_OK;
}

int AbilitySync::RequestRecv(const Message *message, ISyncTaskContext *context)
{
    if (message == nullptr || context == nullptr) {
        return -E_INVALID_ARGS;
    }
    const AbilitySyncRequestPacket *packet = message->GetObject<AbilitySyncRequestPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    SyncOpinion localSyncOpinion;
    SingleVerKvDBSyncInterface *storage = static_cast<SingleVerKvDBSyncInterface *>(storageInterface_);
    if (packet->GetSendCode() == -E_VERSION_NOT_SUPPORT) {
        (void)SendAck(message, storage->GetSchemaInfo(), -E_VERSION_NOT_SUPPORT, localSyncOpinion, false);
        LOGI("[AbilitySync][RequestRecv] version can not support, remote version is %u", packet->GetProtocolVersion());
        return -E_VERSION_NOT_SUPPORT;
    }

    std::string schema;
    packet->GetSchema(schema);
    bool isCompatible = storage->CheckCompatible(schema);
    if (!isCompatible) {
        (static_cast<SingleVerSyncTaskContext *>(context))->SetTaskErrCode(-E_SCHEMA_MISMATCH);
    }
    uint32_t remoteSoftwareVersion = packet->GetSoftwareVersion();
    context->SetRemoteSoftwareVersion(remoteSoftwareVersion);
    int ackCode;
    if (remoteSoftwareVersion > SOFTWARE_VERSION_RELEASE_2_0) {
        localSyncOpinion = HandleVersionV3RequestParam(packet, context, schema);
        if (SecLabelCheck(packet)) {
            ackCode = E_OK;
        } else {
            ackCode = -E_SECURITY_OPTION_CHECK_ERROR;
        }
    } else {
        LOGI("[AbilitySync][RequestRecv] remote version = %u, CheckSchemaCompatible = %d",
            remoteSoftwareVersion, isCompatible);
        return SendAck(message, SchemaObject(), E_OK, localSyncOpinion, false);
    }
    LOGI("[AbilitySync][RequestRecv] remote dev = %s{private}, version = %u, CheckSchemaCompatible = %d",
        deviceId_.c_str(), remoteSoftwareVersion, isCompatible);
    return SendAck(message, storage->GetSchemaInfo(), ackCode, localSyncOpinion, false);
}

int AbilitySync::AckNotifyRecv(const Message *message, ISyncTaskContext *context)
{
    if (message == nullptr || context == nullptr) {
        return -E_INVALID_ARGS;
    }

    if (message->GetErrorNo() == E_FEEDBACK_UNKNOWN_MESSAGE) {
        LOGE("[AbilitySync][AckNotifyRecv] Remote device dose not support this message id");
        context->SetRemoteSoftwareVersion(SOFTWARE_VERSION_EARLIEST);
        return -E_FEEDBACK_UNKNOWN_MESSAGE;
    }
    const AbilitySyncAckPacket *packet = message->GetObject<AbilitySyncAckPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    int errCode = packet->GetAckCode();
    if (errCode != E_OK) {
        LOGE("[AbilitySync][AckNotifyRecv] received a errCode %d", errCode);
        return errCode;
    }
    std::string schema;
    packet->GetSchema(schema);
    uint32_t remoteSoftwareVersion = packet->GetSoftwareVersion();
    context->SetRemoteSoftwareVersion(remoteSoftwareVersion);
    SyncOpinion localSyncOpinion = HandleVersionV3AckSchemaParam(packet, schema, context);
    LOGI("[AckNotifyRecv] receive dev = %s{private} ack notify, remoteSoftwareVersion = %u, ackCode = %d",
        deviceId_.c_str(), remoteSoftwareVersion, errCode);
    (static_cast<SingleVerSyncTaskContext *>(context))->SetIsSchemaSync(true);
    (void)SendAck(message, SchemaObject(), AbilitySync::LAST_NOTIFY, localSyncOpinion, true);
    return E_OK;
}

bool AbilitySync::GetAbilitySyncFinishedStatus() const
{
    return syncFinished_;
}

void AbilitySync::SetAbilitySyncFinishedStatus(bool syncFinished)
{
    syncFinished_ = syncFinished;
}

bool AbilitySync::SecLabelCheck(const AbilitySyncRequestPacket *packet) const
{
    int32_t remoteSecLabel = packet->GetSecLabel();
    int32_t remoteSecFlag = packet->GetSecFlag();
    if (remoteSecLabel == NOT_SURPPORT_SEC_CLASSIFICATION || remoteSecLabel == SecurityLabel::NOT_SET) {
        return true;
    }
    SecurityOption option;
    int errCode = (static_cast<SingleVerKvDBSyncInterface *>(storageInterface_))->GetSecurityOption(option);
    LOGI("[AbilitySync][RequestRecv] local l:%d, f:%d, errCode:%d", option.securityLabel, option.securityFlag, errCode);
    if (errCode == -E_NOT_SUPPORT || option.securityLabel == SecurityLabel::NOT_SET) {
        return true;
    }
    if (remoteSecLabel == FAILED_GET_SEC_CLASSIFICATION || errCode != E_OK) {
        LOGE("[AbilitySync][RequestRecv] check error remoteL:%d, errCode:%d", remoteSecLabel, errCode);
        return false;
    }
    if (remoteSecLabel == option.securityLabel) {
        return true;
    } else {
        LOGE("[AbilitySync][RequestRecv] check error remote:%d , %d local:%d , %d",
            remoteSecLabel, remoteSecFlag, option.securityLabel, option.securityFlag);
        return false;
    }
}

SyncOpinion AbilitySync::HandleVersionV3RequestParam(const AbilitySyncRequestPacket *packet, ISyncTaskContext *context,
    const std::string &remoteSchema) const
{
    int32_t remoteSecLabel = packet->GetSecLabel();
    int32_t remoteSecFlag = packet->GetSecFlag();
    SecurityOption secOption = {remoteSecLabel, remoteSecFlag};
    (static_cast<SingleVerSyncTaskContext *>(context))->SetRemoteSeccurityOption(secOption);
    (static_cast<SingleVerSyncTaskContext *>(context))->SetReceivcPermitCheck(false);
    uint8_t remoteSchemaType = packet->GetSchemaType();
    SchemaObject localSchema = (static_cast<SingleVerKvDBSyncInterface *>(storageInterface_))->GetSchemaInfo();
    SyncOpinion localSyncOpinion = SchemaObject::MakeLocalSyncOpinion(localSchema, remoteSchema, remoteSchemaType);
    LOGI("[AbilitySync][HandleVersionV3RequestParam] remoteSecLabel = %d, remoteSecFlag = %d, remoteSchemaType = %u",
        remoteSecLabel, remoteSecFlag, remoteSchemaType);
    return localSyncOpinion;
}

void AbilitySync::HandleVersionV3AckSecOptionParam(const AbilitySyncAckPacket *packet,
    ISyncTaskContext *context) const
{
    int32_t remoteSecLabel = packet->GetSecLabel();
    int32_t remoteSecFlag = packet->GetSecFlag();
    SecurityOption secOption = {remoteSecLabel, remoteSecFlag};
    (static_cast<SingleVerSyncTaskContext *>(context))->SetRemoteSeccurityOption(secOption);
    (static_cast<SingleVerSyncTaskContext *>(context))->SetSendPermitCheck(false);
    LOGI("[AbilitySync][AckRecv] remoteSecLabel = %d, remoteSecFlag = %d", remoteSecLabel, remoteSecFlag);
}

SyncOpinion AbilitySync::HandleVersionV3AckSchemaParam(const AbilitySyncAckPacket *packet,
    const std::string &remoteSchema, ISyncTaskContext *context) const
{
    bool permitSync = static_cast<bool>(packet->GetPermitSync());
    bool requirePeerConvert = static_cast<bool>(packet->GetRequirePeerConvert());
    SyncOpinion remoteOpinion = {permitSync, requirePeerConvert, true};
    uint8_t remoteSchemaType = packet->GetSchemaType();
    SchemaObject localSchema = (static_cast<SingleVerKvDBSyncInterface *>(storageInterface_))->GetSchemaInfo();
    SyncOpinion localOpinion = SchemaObject::MakeLocalSyncOpinion(localSchema, remoteSchema, remoteSchemaType);
    SyncStrategy localStrategy = SchemaObject::ConcludeSyncStrategy(localOpinion, remoteOpinion);
    (static_cast<SingleVerSyncTaskContext *>(context))->SetSyncStrategy(localStrategy);
    return localOpinion;
}

void AbilitySync::GetPacketSecOption(SecurityOption &option)
{
    int errCode = (static_cast<SingleVerKvDBSyncInterface *>(storageInterface_))->GetSecurityOption(option);
    if (errCode == -E_NOT_SUPPORT) {
        LOGE("[AbilitySync][SyncStart] GetSecOpt not surpport sec classification");
        option.securityLabel = NOT_SURPPORT_SEC_CLASSIFICATION;
    } else if (errCode != E_OK) {
        LOGE("[AbilitySync][SyncStart] GetSecOpt errCode:%d", errCode);
        option.securityLabel = FAILED_GET_SEC_CLASSIFICATION;
    }
}

int AbilitySync::RegisterTransformFunc()
{
    TransformFunc func;
    func.computeFunc = std::bind(&AbilitySync::CalculateLen, std::placeholders::_1);
    func.serializeFunc = std::bind(&AbilitySync::Serialization, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3);
    func.deserializeFunc = std::bind(&AbilitySync::DeSerialization, std::placeholders::_1,
                                     std::placeholders::_2, std::placeholders::_3);
    return MessageTransform::RegTransformFunction(ABILITY_SYNC_MESSAGE, func);
}

uint32_t AbilitySync::CalculateLen(const Message *inMsg)
{
    if ((inMsg == nullptr) || (inMsg->GetMessageId() != ABILITY_SYNC_MESSAGE)) {
        return 0;
    }
    int errCode;
    uint32_t len = 0;
    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            errCode = RequestPacketCalculateLen(inMsg, len);
            if (errCode != E_OK) {
                LOGE("[AbilitySync][CalculateLen] request packet calc length err %d", errCode);
            }
            break;
        case TYPE_RESPONSE:
            errCode = AckPacketCalculateLen(inMsg, len);
            if (errCode != E_OK) {
                LOGE("[AbilitySync][CalculateLen] ack packet calc length err %d", errCode);
            }
            break;
        case TYPE_NOTIFY:
            errCode = AckPacketCalculateLen(inMsg, len);
            if (errCode != E_OK) {
                LOGE("[AbilitySync][CalculateLen] ack packet calc length err %d", errCode);
            }
            break;
        default:
            LOGE("[AbilitySync][CalculateLen] message type not support, type %d", inMsg->GetMessageType());
            break;
    }
    return len;
}

int AbilitySync::Serialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    if ((buffer == nullptr) || (inMsg == nullptr)) {
        return -E_INVALID_ARGS;
    }

    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            return RequestPacketSerialization(buffer, length, inMsg);
        case TYPE_RESPONSE:
            return AckPacketSerialization(buffer, length, inMsg);
        case TYPE_NOTIFY:
            return AckPacketSerialization(buffer, length, inMsg);
        default:
            return -E_MESSAGE_TYPE_ERROR;
    }
}

int AbilitySync::DeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    if ((buffer == nullptr) || (inMsg == nullptr)) {
        return -E_INVALID_ARGS;
    }

    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            return RequestPacketDeSerialization(buffer, length, inMsg);
        case TYPE_RESPONSE:
            return AckPacketDeSerialization(buffer, length, inMsg);
        case TYPE_NOTIFY:
            return AckPacketDeSerialization(buffer, length, inMsg);
        default:
            return -E_MESSAGE_TYPE_ERROR;
    }
}

int AbilitySync::SendAck(const Message *inMsg, const SchemaObject &schemaObj, int ackCode, SyncOpinion localOpinion,
    bool isAckNotify)
{
    Message *ackMessage = new (std::nothrow) Message(ABILITY_SYNC_MESSAGE);
    if (ackMessage == nullptr) {
        LOGE("[AbilitySync][SendAck] message create failed, may be memleak!");
        return -E_OUT_OF_MEMORY;
    }

    AbilitySyncAckPacket ackPacket;
    ackPacket.SetProtocolVersion(ABILITY_SYNC_VERSION_V1);
    ackPacket.SetSoftwareVersion(SOFTWARE_VERSION_CURRENT);
    ackPacket.SetSchema(schemaObj.ToSchemaString());
    ackPacket.SetSchemaType(static_cast<uint32_t>(schemaObj.GetSchemaType()));
    ackPacket.SetAckCode(ackCode);
    if (!isAckNotify) {
        SecurityOption option;
        GetPacketSecOption(option);
        ackPacket.SetSecLabel(option.securityLabel);
        ackPacket.SetSecFlag(option.securityFlag);
    }
    ackPacket.SetPermitSync(localOpinion.permitSync);
    ackPacket.SetRequirePeerConvert(localOpinion.requirePeerConvert);
    int errCode = ackMessage->SetCopiedObject<>(ackPacket);
    if (errCode != E_OK) {
        LOGE("[AbilitySync][SendAck] SetCopiedObject failed, err %d", errCode);
        delete ackMessage;
        ackMessage = nullptr;
        return errCode;
    }
    (!isAckNotify) ? ackMessage->SetMessageType(TYPE_RESPONSE) : ackMessage->SetMessageType(TYPE_NOTIFY);
    ackMessage->SetTarget(deviceId_);
    ackMessage->SetSessionId(inMsg->GetSessionId());
    ackMessage->SetSequenceId(inMsg->GetSequenceId());

    errCode = communicator_->SendMessage(deviceId_, ackMessage, false, SEND_TIME_OUT);
    if (errCode != E_OK) {
        LOGE("[AbilitySync][SendAck] SendPacket failed, err %d", errCode);
        delete ackMessage;
        ackMessage = nullptr;
    }
    return errCode;
}

int AbilitySync::RequestPacketCalculateLen(const Message *inMsg, uint32_t &len)
{
    const AbilitySyncRequestPacket *packet = inMsg->GetObject<AbilitySyncRequestPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }

    len = packet->CalculateLen();
    return E_OK;
}

int AbilitySync::AckPacketCalculateLen(const Message *inMsg, uint32_t &len)
{
    const AbilitySyncAckPacket *packet = inMsg->GetObject<AbilitySyncAckPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }

    len = packet->CalculateLen();
    return E_OK;
}

int AbilitySync::RequestPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    const AbilitySyncRequestPacket *packet = inMsg->GetObject<AbilitySyncRequestPacket>();
    if ((packet == nullptr) || (length != packet->CalculateLen())) {
        return -E_INVALID_ARGS;
    }

    Parcel parcel(buffer, length);
    int errCode = parcel.WriteUInt32(packet->GetProtocolVersion());
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteInt(packet->GetSendCode());
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt32(packet->GetSoftwareVersion());
    if (errCode != E_OK) {
        return errCode;
    }
    std::string schema;
    packet->GetSchema(schema);
    errCode = parcel.WriteString(schema);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteInt(packet->GetSecLabel());
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteInt(packet->GetSecFlag());
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt32(packet->GetSchemaType());
    if (errCode != E_OK) {
        return errCode;
    }
    return E_OK;
}

int AbilitySync::AckPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    const AbilitySyncAckPacket *packet = inMsg->GetObject<AbilitySyncAckPacket>();
    if ((packet == nullptr) || (length != packet->CalculateLen())) {
        return -E_INVALID_ARGS;
    }

    Parcel parcel(buffer, length);
    int errCode = parcel.WriteUInt32(ABILITY_SYNC_VERSION_V1);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt32(SOFTWARE_VERSION_CURRENT);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteInt(packet->GetAckCode());
    if (errCode != E_OK) {
        return errCode;
    }
    std::string schema;
    packet->GetSchema(schema);
    errCode = parcel.WriteString(schema);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteInt(packet->GetSecLabel());
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteInt(packet->GetSecFlag());
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt32(packet->GetSchemaType());
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt32(packet->GetPermitSync());
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt32(packet->GetRequirePeerConvert());
    if (errCode != E_OK) {
        return errCode;
    }
    return E_OK;
}

int AbilitySync::RequestPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    auto *packet = new (std::nothrow) AbilitySyncRequestPacket();
    if (packet == nullptr) {
        return -E_OUT_OF_MEMORY;
    }

    Parcel parcel(const_cast<uint8_t *>(buffer), length);
    uint32_t version = 0;
    uint32_t softwareVersion = 0;
    std::string schema;
    int32_t sendCode = 0;
    int errCode = -E_PARSE_FAIL;

    parcel.ReadUInt32(version);
    if (parcel.IsError()) {
        goto ERROR_OUT;
    }
    packet->SetProtocolVersion(version);
    if (version > ABILITY_SYNC_VERSION_V1) {
        packet->SetSendCode(-E_VERSION_NOT_SUPPORT);
        errCode = inMsg->SetExternalObject<>(packet);
        if (errCode != E_OK) {
            goto ERROR_OUT;
        }
        return errCode;
    }
    parcel.ReadInt(sendCode);
    parcel.ReadUInt32(softwareVersion);
    parcel.ReadString(schema);
    if (!parcel.IsError() && softwareVersion > SOFTWARE_VERSION_RELEASE_2_0) {
        RequestPacketDeSerializationTailPart(parcel, packet);
    }
    if (parcel.IsError()) {
        goto ERROR_OUT;
    }
    packet->SetSendCode(sendCode);
    packet->SetSoftwareVersion(softwareVersion);
    packet->SetSchema(schema);

    errCode = inMsg->SetExternalObject<>(packet);
    if (errCode == E_OK) {
        return E_OK;
    }

ERROR_OUT:
    delete packet;
    packet = nullptr;
    return errCode;
}

void AbilitySync::RequestPacketDeSerializationTailPart(Parcel &parcel, AbilitySyncRequestPacket *packet)
{
    int32_t secLabel = 0;
    int32_t secFlag = 0;
    uint32_t schemaType = 0;
    parcel.ReadInt(secLabel);
    parcel.ReadInt(secFlag);
    parcel.ReadUInt32(schemaType);
    packet->SetSecLabel(secLabel);
    packet->SetSecFlag(secFlag);
    packet->SetSchemaType(schemaType);
}

void AbilitySync::AckPacketDeSerializationTailPart(Parcel &parcel, AbilitySyncAckPacket *packet)
{
    int32_t secLabel = 0;
    int32_t secFlag = 0;
    uint32_t schemaType = 0;
    uint32_t permitSync = 0;
    uint32_t requirePeerConvert = 0;
    parcel.ReadInt(secLabel);
    parcel.ReadInt(secFlag);
    parcel.ReadUInt32(schemaType);
    parcel.ReadUInt32(permitSync);
    parcel.ReadUInt32(requirePeerConvert);
    packet->SetSecLabel(secLabel);
    packet->SetSecFlag(secFlag);
    packet->SetSchemaType(schemaType);
    packet->SetPermitSync(permitSync);
    packet->SetRequirePeerConvert(requirePeerConvert);
}

int AbilitySync::AckPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    auto *packet = new (std::nothrow) AbilitySyncAckPacket();
    if (packet == nullptr) {
        return -E_OUT_OF_MEMORY;
    }

    Parcel parcel(const_cast<uint8_t *>(buffer), length);
    uint32_t version = 0;
    uint32_t softwareVersion = 0;
    int32_t ackCode = E_OK;
    std::string schema;
    int errCode;
    parcel.ReadUInt32(version);
    if (parcel.IsError()) {
        LOGE("[AbilitySync][RequestDeSerialization] read version failed!");
        errCode = -E_PARSE_FAIL;
        goto ERROR_OUT;
    }
    packet->SetProtocolVersion(version);
    if (version > ABILITY_SYNC_VERSION_V1) {
        packet->SetAckCode(-E_VERSION_NOT_SUPPORT);
        errCode = inMsg->SetExternalObject<>(packet);
        if (errCode != E_OK) {
            goto ERROR_OUT;
        }
        return errCode;
    }
    parcel.ReadUInt32(softwareVersion);
    parcel.ReadInt(ackCode);
    parcel.ReadString(schema);
    if (!parcel.IsError() && softwareVersion > SOFTWARE_VERSION_RELEASE_2_0) {
        AckPacketDeSerializationTailPart(parcel, packet);
    }
    if (parcel.IsError()) {
        LOGE("[AbilitySync][RequestDeSerialization] DeSerialization failed!");
        errCode = -E_PARSE_FAIL;
        goto ERROR_OUT;
    }
    packet->SetSoftwareVersion(softwareVersion);
    packet->SetAckCode(ackCode);
    packet->SetSchema(schema);
    errCode = inMsg->SetExternalObject<>(packet);
    if (errCode == E_OK) {
        return E_OK;
    }

ERROR_OUT:
    delete packet;
    packet = nullptr;
    return errCode;
}

int AbilitySync::CheckAckCode(const Message *message, ISyncTaskContext *context, int errCode)
{
    if (message->GetErrorNo() == E_FEEDBACK_UNKNOWN_MESSAGE) {
        LOGE("[AbilitySync][AckRecv] Remote device dose not support this message id");
        context->SetRemoteSoftwareVersion(SOFTWARE_VERSION_EARLIEST);
        return -E_FEEDBACK_UNKNOWN_MESSAGE;
    }

    if (errCode != E_OK) {
        LOGE("[AbilitySync][AckRecv] received a errCode %d", errCode);
        if (errCode == -E_SECURITY_OPTION_CHECK_ERROR) {
            context->SetTaskErrCode(-E_SECURITY_OPTION_CHECK_ERROR);
        }
        return errCode;
    }
    return E_OK;
}
} // namespace DistributedDB
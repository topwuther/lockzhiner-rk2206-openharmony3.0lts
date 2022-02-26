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

#include "header_converter.h"
#include "endian_convert.h"
#include "communicator_type_define.h"

namespace DistributedDB {
void HeaderConverter::ConvertHostToNet(const CommPhyHeader &headerOriginal, CommPhyHeader &headerConverted)
{
    headerConverted.magic = HostToNet(headerOriginal.magic);
    headerConverted.version = HostToNet(headerOriginal.version);
    headerConverted.packetLen = HostToNet(headerOriginal.packetLen);
    headerConverted.checkSum = HostToNet(headerOriginal.checkSum);
    headerConverted.sourceId = HostToNet(headerOriginal.sourceId);
    headerConverted.frameId = HostToNet(headerOriginal.frameId);
    headerConverted.packetType = HostToNet(headerOriginal.packetType);
    headerConverted.paddingLen = HostToNet(headerOriginal.paddingLen);
    headerConverted.dbIntVer = HostToNet(headerOriginal.dbIntVer);
}

void HeaderConverter::ConvertHostToNet(const CommPhyOptHeader &headerOriginal, CommPhyOptHeader &headerConverted)
{
    headerConverted.frameLen = HostToNet(headerOriginal.frameLen);
    headerConverted.fragCount = HostToNet(headerOriginal.fragCount);
    headerConverted.fragNo = HostToNet(headerOriginal.fragNo);
}

void HeaderConverter::ConvertHostToNet(const CommDivergeHeader &headerOriginal, CommDivergeHeader &headerConverted)
{
    ConvertNetToHost(headerOriginal, headerConverted);
}

void HeaderConverter::ConvertHostToNet(const MessageHeader &headerOriginal, MessageHeader &headerConverted)
{
    ConvertNetToHost(headerOriginal, headerConverted);
}

void HeaderConverter::ConvertNetToHost(const CommPhyHeader &headerOriginal, CommPhyHeader &headerConverted)
{
    ConvertHostToNet(headerOriginal, headerConverted);
}

void HeaderConverter::ConvertNetToHost(const CommPhyOptHeader &headerOriginal, CommPhyOptHeader &headerConverted)
{
    ConvertHostToNet(headerOriginal, headerConverted);
}

void HeaderConverter::ConvertNetToHost(const CommDivergeHeader &headerOriginal, CommDivergeHeader &headerConverted)
{
    headerConverted.version = NetToHost(headerOriginal.version);
    headerConverted.reserved = NetToHost(headerOriginal.reserved);
    headerConverted.payLoadLen = NetToHost(headerOriginal.payLoadLen);
    // commLabel now is array of uint8_t, so no need to do endian convert, but we need to copy it here
    for (unsigned int i = 0; i < COMM_LABEL_LENGTH; i++) {
        headerConverted.commLabel[i] = headerOriginal.commLabel[i];
    }
}

void HeaderConverter::ConvertNetToHost(const MessageHeader &headerOriginal, MessageHeader &headerConverted)
{
    headerConverted.version = NetToHost(headerOriginal.version);
    headerConverted.messageType = NetToHost(headerOriginal.messageType);
    headerConverted.messageId = NetToHost(headerOriginal.messageId);
    headerConverted.sessionId = NetToHost(headerOriginal.sessionId);
    headerConverted.sequenceId = NetToHost(headerOriginal.sequenceId);
    headerConverted.errorNo = NetToHost(headerOriginal.errorNo);
    headerConverted.dataLen = NetToHost(headerOriginal.dataLen);
}
} // namespace DistributedDB
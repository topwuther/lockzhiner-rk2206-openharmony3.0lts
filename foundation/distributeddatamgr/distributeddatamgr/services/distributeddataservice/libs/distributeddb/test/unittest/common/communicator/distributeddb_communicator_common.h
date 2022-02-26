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

#ifndef DISTRIBUTEDDB_COMMUNICATOR_COMMON_H
#define DISTRIBUTEDDB_COMMUNICATOR_COMMON_H

#include <set>
#include <string>
#include <vector>
#include "message.h"
#include "adapter_stub.h"
#include "frame_header.h"
#include "communicator_aggregator.h"

struct EnvHandle {
    DistributedDB::AdapterStub *adapterHandle = nullptr;
    DistributedDB::CommunicatorAggregator *commAggrHandle = nullptr;
};

struct OnOfflineDevice {
    std::set<std::string> onlineDevices;
    std::string latestOnlineDevice;
    std::string latestOfflineDevice;
};

bool SetUpEnv(EnvHandle &inEnv, const std::string &inName);
void TearDownEnv(EnvHandle &inEnv);

struct RegedTinyObject {
    uint32_t placeHolder_ = 0;
};

struct RegedHugeObject {
    uint32_t placeHolder_ = 0;
};

struct RegedGiantObject {
    std::vector<uint8_t> rawData_;
    static bool CheckEqual(const RegedGiantObject &inLeft, const RegedGiantObject &inRight);
};

struct RegedOverSizeObject {
    uint32_t placeHolder_ = 0;
};

struct UnRegedTinyObject {
    uint32_t placeHolder_ = 0;
};

const std::string DEVICE_NAME_A = "DeviceA";
const std::string DEVICE_NAME_B = "DeviceB";
const std::string DEVICE_NAME_C = "DeviceC";
constexpr uint64_t LABEL_A = 1234;
constexpr uint64_t LABEL_B = 2345;
constexpr uint64_t LABEL_C = 3456;
constexpr uint32_t REGED_TINY_MSG_ID = 1111;
constexpr uint32_t REGED_HUGE_MSG_ID = 2222;
constexpr uint32_t REGED_GIANT_MSG_ID = 3333;
constexpr uint32_t REGED_OVERSIZE_MSG_ID = 4444;
constexpr uint32_t UNREGED_TINY_MSG_ID = 5555;
constexpr uint32_t FIXED_SESSIONID = 98765;
constexpr uint32_t FIXED_SEQUENCEID = 87654;
constexpr uint32_t TINY_SIZE = 100; // 100 Bytes
constexpr uint32_t HUGE_SIZE = 4 * 1024 * 1024; // 4 MBytes, 1024 is scale
constexpr uint32_t OVER_SIZE = 100 * 1024 * 1024; // 100 MBytes, 1024 is scale
constexpr uint32_t HEADER_SIZE = sizeof(DistributedDB::CommPhyHeader) + sizeof(DistributedDB::CommDivergeHeader) +
    sizeof(DistributedDB::MessageHeader); // 96 Bytes For Header, 32 phyHeader, 40 divergeHeader, 24 msgHeader
constexpr uint32_t MAX_CAPACITY = 64 * 1024 * 1024; // 64 MBytes, 1024 is scale

void DoRegTransformFunction();

DistributedDB::Message *BuildRegedTinyMessage();
DistributedDB::Message *BuildRegedHugeMessage();
DistributedDB::Message *BuildRegedGiantMessage(uint32_t length);
DistributedDB::Message *BuildRegedOverSizeMessage();
DistributedDB::Message *BuildUnRegedTinyMessage();

#define ASSERT_NOT_NULL_AND_ACTIVATE(communicator) \
{ \
    ASSERT_NE(communicator, nullptr); \
    communicator->Activate(); \
}

#endif // DISTRIBUTEDDB_COMMUNICATOR_COMMON_H
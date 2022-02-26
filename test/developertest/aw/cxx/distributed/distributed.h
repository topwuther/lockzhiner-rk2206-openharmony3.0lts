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

#ifndef _DISTRIBUTED_H_
#define _DISTRIBUTED_H_

#include <iostream>
#include "securec.h"

namespace OHOS {
namespace DistributeSystemTest {
static const int MAX_BUFF_LEN = 1024;
static const int DST_COMMAND_NOTIFY = 0;
static const int DST_COMMAND_CALL = 1;
static const int DST_COMMAND_MSG = 2;
static const int DST_COMMAND_END = 3;
static const int DEFAULT_AGENT_PORT = 6789;

struct DistributedCmd {
    int no;  // record command no, as return no.
    int cmdTestType;
    int len; // len + userszcmdinfo + len + cmd args + len + rexpectreturn. type of len is int
    union {
        char alignmentCmd[1]; // int Byte alignment
        int returnValue; // record ret value;
    };
};
using DistributedMsg = DistributedCmd;

#define DST_COMMAND_HEAD_LEN (sizeof(DistributedCmd)- sizeof(int))

struct DistDevInfo {
    int devNo;
    std::string ipAddr;
    int fd;
};
using DistDeviceInfo = DistDevInfo;
} // namespace DistributeSystemTest
} // namespace OHOS

#endif // _DISTRIBUTED_H_

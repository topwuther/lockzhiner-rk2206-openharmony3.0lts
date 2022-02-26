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
#ifndef DISTRIBUTED_TEST_SYSINFO_H
#define DISTRIBUTED_TEST_SYSINFO_H

#include <string.h>
#include "platform_specific.h"
#include <linux/limits.h>

const uint64_t SYSTEM_INFO_BUFFER_SIZE = 20;
const uint64_t PROC_BUFFER_LENGTH = 4096;
const uint64_t DEFAULT_INTEVAL = 250000;
const uint64_t DEFAULT_COUNT = 5;

const std::string SYS_MEM_FILE = "/proc/meminfo";
const std::string SYS_CPU_FILE = "/proc/stat";
const std::string POWER_FOLLOW_FILE = "/sys/class/power_supply/Battery/current_now";
const std::string VOLTAGE_FILE = "/sys/class/power_supply/Battery/voltage_now";
const std::string FILE_READ_PERMISSION = "r";

// struct of mem occupy
struct MemOccupy {
    char memTotalName_[SYSTEM_INFO_BUFFER_SIZE];
    uint64_t memTotal_;
    char memFreeName_[SYSTEM_INFO_BUFFER_SIZE];
    uint64_t memFree_;
    char buffersName_[SYSTEM_INFO_BUFFER_SIZE];
    uint64_t buffers_;
    char cachedName_[SYSTEM_INFO_BUFFER_SIZE];
    uint64_t cached_;
    char swapCachedName_[SYSTEM_INFO_BUFFER_SIZE];
    uint64_t swapCached_;
};

// struct of cpu occupy
struct CpuOccupy {
    char name_[SYSTEM_INFO_BUFFER_SIZE];
    uint64_t user_;
    uint64_t nice_;
    uint64_t system_;
    uint64_t idle_;
    uint64_t iowait_;
    uint64_t irq_;
    uint64_t softirq_;
};

enum SeqNo {
    FIRST = 1,
    SECOND
};

// this class get system info, such as system or cpu storage and power at the moment
class DistributedTestSysInfo final {
public:

    DistributedTestSysInfo();
    ~DistributedTestSysInfo() {}

    // Delete the copy and assign constructors
    DistributedTestSysInfo(const DistributedTestSysInfo &distributeTestSysInfo) = delete;
    DistributedTestSysInfo& operator=(const DistributedTestSysInfo &distributeTestSysInfo) = delete;
    DistributedTestSysInfo(DistributedTestSysInfo &&distributeTestSysInfo) = delete;
    DistributedTestSysInfo& operator=(DistributedTestSysInfo &&distributeTestSysInfo) = delete;

    // read memory information from system file
    void GetSysMemOccpy(SeqNo seqNo);
    // read cpu information from system file sleep microSeconds between first and second read.
    void GetSysCpuUsage(SeqNo seqNo, uint64_t microSeconds);
    // read value from specific file
    float ReadSysValFromFile(const std::string &filePath);
    // read value from specific file and take the average within totalCount
    float GetSysMeanCurrentVal(const std::string &filePath, int totalCount, uint64_t microSeconds);
    // read power from specific file
    void GetSysCurrentPower(SeqNo seqNo, int totalCount, uint64_t microSeconds);

    uint64_t GetFirstMemFree() const;
    uint64_t GetSecondMemFree() const;
    float GetFirstCpuUsage() const;
    float GetSecondCpuUsage() const;
    float GetFirstPower() const;
    float GetSecondPower() const;

    void SaveSecondToFirst();

private:
    MemOccupy memStatFirst_, memStatSecond_;
    CpuOccupy cpuStatFirst_, cpuStatSecond_;
    float cpuStatFirstUsage_, cpuStatSecondUsage_;
    float powerStatFirst_, powerStatSecond_;
    float val_;
};
#endif // DISTRIBUTED_TEST_SYSINFO_H
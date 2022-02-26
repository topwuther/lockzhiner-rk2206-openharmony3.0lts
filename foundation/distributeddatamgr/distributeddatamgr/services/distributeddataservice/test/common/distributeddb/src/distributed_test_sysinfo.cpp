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
#include <thread>
#include "distributed_test_sysinfo.h"
#include "distributeddb_data_generator.h"
#include "securec.h"

const uint64_t PERCENTAGE_FACTOR = 100;
const float FLOAT_RET_ERROR = -1.0f;

DistributedTestSysInfo::DistributedTestSysInfo()
{
    if ((memset_s(&memStatFirst_, sizeof(memStatFirst_), 0, sizeof(memStatFirst_)) != EOK) &&
        (memset_s(&memStatSecond_, sizeof(memStatSecond_), 0, sizeof(memStatSecond_)) != EOK) &&
        (memset_s(&cpuStatFirst_, sizeof(cpuStatFirst_), 0, sizeof(cpuStatFirst_)) != EOK) &&
        (memset_s(&cpuStatSecond_, sizeof(cpuStatSecond_), 0, sizeof(cpuStatSecond_)) != EOK)) {
        MST_LOG("distribute info set 0 failed!");
    }

    cpuStatFirstUsage_ = 0.0f;
    cpuStatSecondUsage_ = 0.0f;
    powerStatFirst_ = 0.0f;
    powerStatSecond_ = 0.0f;
    val_ = 0.0f;
}

void DistributedTestSysInfo::GetSysMemOccpy(SeqNo seqNo)
{
    MemOccupy* memOccupy = nullptr;
    if (seqNo == FIRST) {
        memOccupy = &memStatFirst_;
    } else {
        memOccupy = &memStatSecond_;
    }

    FILE *fp = nullptr;
    char buff[PROC_BUFFER_LENGTH] = { 0 };

    fp = fopen(SYS_MEM_FILE.c_str(), FILE_READ_PERMISSION.c_str());
    if (fp == nullptr) {
        perror("fopen:");
        return;
    }

    fgets(buff, sizeof(buff), fp);
    int result1 = sscanf_s(buff, "%s %llu ",
        memOccupy->memTotalName_, sizeof(memOccupy->memTotalName_), &memOccupy->memTotal_);
    fgets(buff, sizeof(buff), fp);
    int result2 = sscanf_s(buff, "%s %llu ",
        memOccupy->memFreeName_, sizeof(memOccupy->memFreeName_), &memOccupy->memFree_);
    fgets(buff, sizeof(buff), fp);
    int result3 = sscanf_s(buff, "%s %llu ",
        memOccupy->buffersName_, sizeof(memOccupy->buffersName_), &memOccupy->buffers_);
    fgets(buff, sizeof(buff), fp);
    int result4 = sscanf_s(buff, "%s %llu ",
        memOccupy->cachedName_, sizeof(memOccupy->cachedName_), &memOccupy->cached_);
    fgets(buff, sizeof(buff), fp);
    int result5 = sscanf_s(buff, "%s %llu",
        memOccupy->swapCachedName_, sizeof(memOccupy->swapCachedName_), &memOccupy->swapCached_);
    if (result1 != 2 || result2 != 2) { // there are 2 incoming param.
        MST_LOG("get mem1 info failed.");
    }
    if (result3 != 2 || result4 != 2 || result5 != 2) { // there are 2 incoming param.
        MST_LOG("get mem2 info failed.");
    }
    fclose(fp);
    fp = nullptr;
}

void GetSysCpuInfo(CpuOccupy &cpuStatFirst_, CpuOccupy &cpuStatSecond_, uint64_t microSeconds, float *&cpuStatUsage)
{
    FILE *fp = nullptr;
    char buff[PROC_BUFFER_LENGTH] = { 0 };
    uint64_t allFirst, allSecond, idleFirst, idleSecond;
    fp = fopen(SYS_CPU_FILE.c_str(), FILE_READ_PERMISSION.c_str());
    if (fp == nullptr) {
        perror("fopen:");
        exit(0);
    }
    fgets(buff, sizeof(buff), fp);
    int result = sscanf_s(buff, "%s %llu %llu %llu %llu %llu %llu %llu",
        cpuStatFirst_.name_, sizeof(cpuStatFirst_.name_),
        &cpuStatFirst_.user_, &cpuStatFirst_.nice_,
        &cpuStatFirst_.system_, &cpuStatFirst_.idle_, &cpuStatFirst_.iowait_,
        &cpuStatFirst_.irq_, &cpuStatFirst_.softirq_);
    if (result != 8) { // there are 8 incoming param.
        fclose(fp);
        return;
    }
    allFirst = cpuStatFirst_.user_ + cpuStatFirst_.nice_ + cpuStatFirst_.system_ + cpuStatFirst_.idle_ \
        + cpuStatFirst_.iowait_ + cpuStatFirst_.irq_ + cpuStatFirst_.softirq_;
    idleFirst = cpuStatFirst_.idle_;
    rewind(fp);
    std::this_thread::sleep_for(std::chrono::microseconds(microSeconds));
    if (memset_s(buff, sizeof(buff), 0, sizeof(buff)) != EOK) {
        MST_LOG("set buffer to 0 failed!");
        fclose(fp);
        return;
    }
    fgets(buff, sizeof(buff), fp);
    result = sscanf_s(buff, "%s %llu %llu %llu %llu %llu %llu %llu",
        cpuStatSecond_.name_, sizeof(cpuStatSecond_.name_),
        &cpuStatSecond_.user_, &cpuStatSecond_.nice_,
        &cpuStatSecond_.system_, &cpuStatSecond_.idle_, &cpuStatSecond_.iowait_,
        &cpuStatSecond_.irq_, &cpuStatSecond_.softirq_);
    if (result < 0) {
        fclose(fp);
        return;
    }

    allSecond = cpuStatSecond_.user_ + cpuStatSecond_.nice_ + cpuStatSecond_.system_ + cpuStatSecond_.idle_ \
        + cpuStatSecond_.iowait_ + cpuStatSecond_.irq_ + cpuStatSecond_.softirq_;
    idleSecond = cpuStatSecond_.idle_;
    *cpuStatUsage = (static_cast<float>(allSecond - allFirst - (idleSecond - idleFirst)))
        / (allSecond - allFirst) * PERCENTAGE_FACTOR;
    MST_LOG(" [CpuUsage] = %.2f%%", *cpuStatUsage);
    fclose(fp);
    fp = nullptr;
    return;
}
void DistributedTestSysInfo::GetSysCpuUsage(SeqNo seqNo, uint64_t microSeconds)
{
    float *cpuStatUsage = nullptr;
    if (seqNo == FIRST) {
        cpuStatUsage = &cpuStatFirstUsage_;
    } else {
        cpuStatUsage = &cpuStatSecondUsage_;
    }
    GetSysCpuInfo(cpuStatFirst_, cpuStatSecond_, microSeconds, cpuStatUsage);
    if ((memset_s(&cpuStatFirst_, sizeof(cpuStatFirst_), 0, sizeof(cpuStatFirst_)) != EOK) &&
        (memset_s(&cpuStatSecond_, sizeof(cpuStatSecond_), 0, sizeof(cpuStatSecond_)) != EOK)) {
        MST_LOG("set cpu to 0 failed!");
    }
}

float DistributedTestSysInfo::ReadSysValFromFile(const std::string &filePath)
{
    FILE *fp = nullptr;
    char buff[SYSTEM_INFO_BUFFER_SIZE] = { 0 };

    char path[PATH_MAX] = { 0 };
    if (realpath(filePath.c_str(), path) == nullptr) {
        MST_LOG("path error.");
        return FLOAT_RET_ERROR;
    }

    fp = fopen(filePath.c_str(), FILE_READ_PERMISSION.c_str());
    if (fp == nullptr) {
        perror("fopen:");
        return FLOAT_RET_ERROR;
    }

    fgets(buff, sizeof(buff), fp);
    if (sscanf_s(buff, "%f", &val_) != 1) {
        fclose(fp);
        return FLOAT_RET_ERROR;
    }
    fclose(fp);
    return val_;
}

float DistributedTestSysInfo::GetSysMeanCurrentVal(
    const std::string &filePath, int totalCount, uint64_t microSeconds)
{
    float meanVal = 0.0f;
    if (totalCount <= 0 || microSeconds <= 0) {
        return 0.0f;
    }

    for (int cnt = 0; cnt < totalCount; cnt++) {
        float val = ReadSysValFromFile(filePath.c_str());
        if (val < 1e-4) {
            continue;
        }
        meanVal += val;
        std::this_thread::sleep_for(std::chrono::microseconds(microSeconds));
    }
    return meanVal / totalCount;
}

void DistributedTestSysInfo::GetSysCurrentPower(SeqNo seqNo, int totalCount, uint64_t microSeconds)
{
    float* powerCons = nullptr;
    if (seqNo == FIRST) {
        powerCons = &powerStatFirst_;
    } else {
        powerCons = &powerStatSecond_;
    }

    *powerCons = GetSysMeanCurrentVal(POWER_FOLLOW_FILE, totalCount, microSeconds) \
        * GetSysMeanCurrentVal(VOLTAGE_FILE, totalCount, microSeconds);
    MST_LOG(" [Power] = %.2f", *powerCons);
}

uint64_t DistributedTestSysInfo::GetFirstMemFree() const
{
    return memStatFirst_.memFree_;
}

uint64_t DistributedTestSysInfo::GetSecondMemFree() const
{
    return memStatSecond_.memFree_;
}

float DistributedTestSysInfo::GetFirstCpuUsage() const
{
    return cpuStatFirstUsage_;
}

float DistributedTestSysInfo::GetSecondCpuUsage() const
{
    return cpuStatSecondUsage_;
}

float DistributedTestSysInfo::GetFirstPower() const
{
    return powerStatFirst_;
}

float DistributedTestSysInfo::GetSecondPower() const
{
    return powerStatSecond_;
}

void DistributedTestSysInfo::SaveSecondToFirst()
{
    if ((memcpy_s(&memStatFirst_, sizeof(memStatFirst_), &memStatSecond_, sizeof(struct MemOccupy)) != EOK) &&
        memcpy_s(&cpuStatFirst_, sizeof(cpuStatFirst_), &cpuStatSecond_, sizeof(struct CpuOccupy)) != EOK) {
        MST_LOG("set mem or cpu state failed.");
    }
    cpuStatFirstUsage_ = cpuStatSecondUsage_;
    powerStatFirst_ = powerStatSecond_;
}
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

#ifndef PERFORMANCE_ANALYSIS_H
#define PERFORMANCE_ANALYSIS_H

#include <vector>
#include <fstream>

#include "db_types.h"

namespace DistributedDB {
enum PT_TEST_RECORDS : uint32_t {
    RECORD_PUT_DATA = 1,
    RECORD_SYNC_TOTAL,
    RECORD_WATERMARK_SYNC,
    RECORD_READ_DATA,
    RECORD_SAVE_DATA,
    RECORD_SAVE_LOCAL_WATERMARK,
    RECORD_SAVE_PEER_WATERMARK,
    RECORD_DATA_SEND_REQUEST_TO_ACK_RECV,
    RECORD_DATA_REQUEST_RECV_TO_SEND_ACK,
    RECORD_MACHINE_START_TO_PUSH_SEND,
    RECORD_ACK_RECV_TO_USER_CALL_BACK,
};

enum MV_TEST_RECORDS : uint32_t {
    RECORD_SEND_LOCAL_DATA_CHANGED_TO_COMMIT_REQUEST_RECV = 3,
    RECORD_GET_DEVICE_LATEST_COMMIT,
    RECORD_COMMIT_SEND_REQUEST_TO_ACK_RECV,
    RECORD_GET_COMMIT_TREE,
    RECORD_DATA_GET_VALID_COMMIT,
    RECORD_DATA_ENTRY_SEND_REQUEST_TO_ACK_RECV,
    RECORD_GET_COMMIT_DATA,
    RECORD_GET_VALUE_SLICE_NODE,
    RECORD_VALUE_SLICE_SEND_REQUEST_TO_ACK_RECV,
    RECORD_READ_VALUE_SLICE,
    RECORD_SAVE_VALUE_SLICE,
    RECORD_PUT_COMMIT_DATA,
    RECORD_MERGE,
};

struct TimePair {
    TimeStamp startTime = 0;
    TimeStamp endTime = 0;
};

struct StatisticsInfo {
    TimeStamp max = 0;
    TimeStamp min = 0;
    float average = 0.0;
};

struct SingleStatistics {
    std::vector<TimePair> timeInfo;
};

class PerformanceAnalysis {
public:
    explicit PerformanceAnalysis(uint32_t step);
    ~PerformanceAnalysis();

    static PerformanceAnalysis *GetInstance(int stepNum = 20);

    void TimeRecordStart();

    void TimeRecordEnd();

    void StepTimeRecordStart(uint32_t step);

    void StepTimeRecordEnd(uint32_t step);

    std::string GetStatistics();

    void OpenPerformanceAnalysis();

    void ClosePerformanceAnalysis();

    void SetFileNumber(const std::string &FileID);

private:

    bool IsStepValid(uint32_t step) const;

    bool IsOpen() const;

    bool InsertTimeRecord(const TimePair &timePair, uint32_t step);

    bool GetTimeRecord(uint32_t step, TimePair &timePair) const;

    void OutStatistics();

    void Clear();

    void Close();

    const static int MAX_TIMERECORD_STEP_NUM = 200;
    const static std::string STATISTICAL_DATA_FILE_NAME_HEADER;
    const static std::string CSV_FILE_EXTENSION;
    const static std::string DEFAULT_FILE_NAME;
    SingleStatistics timeRecordData_;
    std::vector<StatisticsInfo> stepTimeRecordInfo_;
    std::vector<uint64_t> counts_;
    uint32_t stepNum_;
    bool isOpen_;
    std::ofstream outFile;
    int fileNumber_;
    std::string fileID_;
};
}  // namespace DistributedDB
#endif

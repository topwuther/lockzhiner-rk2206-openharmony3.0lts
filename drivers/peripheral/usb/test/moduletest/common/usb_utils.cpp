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

#include "usb_utils.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sys/time.h>

using namespace std;

bool HasLog(const string &target, double startTs, const string &file)
{
    bool ret = false;
    ifstream logFile(file);
    string::size_type pos;
    string lineStr;
    const string flagStr = "[XTSCHECK]";
    const int tsStartPos = 11;
    const int tsLength = 17;
    while (getline(logFile, lineStr)) {
        double logTs;
        pos = lineStr.find(flagStr);
        if (pos != string::npos) {
            logTs = atof(lineStr.substr(pos + tsStartPos, tsLength).c_str());
            if ((logTs - startTs) >= 0) {
                if (lineStr.find(target) != string::npos) {
                    ret = true;
                }
            }
        }
        lineStr.clear();
    }
    logFile.close();
    return ret;
}

double GetNowTs(void)
{
    const double transUsecNum = 1000000.0;
    timeval tv = {0};
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec + tv.tv_usec / transUsecNum);
}

char *ParseSysCmdResult(FILE &result, int line, int word)
{
    char s[1024];
    char *pch = nullptr;
    int lineCnt = 1;
    int wordCnt = 1;
    while (1) {
        if (fgets(s, sizeof(s), &result) == nullptr) {
            break;
        }
        pch = strtok(s, " ");
        while (pch != nullptr) {
            if (lineCnt == line && wordCnt == word) {
                return pch;
            }
            pch = strtok(nullptr, " ");
            wordCnt++;
        }
        lineCnt++;
        wordCnt = 1;
    }
    return pch;
}

void CalcProcInfoFromFile(struct ProcInfo &info, const string &file)
{
    const char *ramFlag = "VmRSS";
    const char *cpuFlag = "Cpu";
    const char *threadFlag = "Threads";
    static double ramTotal, ramPeak, ramCur, cpuTotal, cpuPeak, cpuCur;
    static int ramCount, cpuCount, threadPeak, threadCur;
    char s[100];
    char *pch;
    FILE *f = fopen(file.c_str(), "r");
    while (1) {
        if (fgets(s, sizeof(s), f) == nullptr) {
            break;
        }
        pch = strtok(s, " \t");
        while (pch != nullptr) {
            if (strstr(pch, ramFlag)) {
                pch = strtok(nullptr, " ");
                ramCur = atof(pch);
                ramCount += 1;
                ramTotal += ramCur;
                if (ramCur > ramPeak) {
                    ramPeak = ramCur;
                }
                break;
            }
            if (strstr(pch, cpuFlag)) {
                pch = strtok(nullptr, " ");
                cpuCur = atof(pch);
                cpuCount += 1;
                cpuTotal += cpuCur;
                if (cpuCur > cpuPeak) {
                    cpuPeak = cpuCur;
                }
                break;
            }
            if (strstr(pch, threadFlag)) {
                pch = strtok(nullptr, " ");
                threadCur = atoi(pch);
                if (threadCur > threadPeak) {
                    threadPeak = threadCur;
                }
                break;
            }
            pch = strtok(nullptr, " ");
        }
    }
    info.ramPeak = ramPeak;
    info.ramAvg = ramTotal / ramCount;
    info.cpuPeak = cpuPeak;
    info.cpuAvg = cpuTotal / cpuCount;
    info.threadPeak = threadPeak;
}

double GetTsFromLog(const string &target, double startTs, const string &file)
{
    double logTs;
    ifstream logFile(file);
    string lineStr;
    const int tsStartPos = 11;
    const int tsLength = 17;
    while (getline(logFile, lineStr)) {
        if (lineStr.find(target) != string::npos) {
            logTs = atof(lineStr.substr(tsStartPos, tsLength).c_str());
            if ((logTs - startTs) >= 0) {
                return logTs;
            }
        }
        lineStr.clear();
    }
    logFile.close();
    return 0;
}

/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "init_jobs.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "init_cmds.h"
#include "init_log.h"
#include "securec.h"


#define JOBS_ARR_NAME_IN_JSON "jobs"
#define CMDS_ARR_NAME_IN_JSON "cmds"
#define MAX_JOBS_COUNT        100

static Job* g_jobs = NULL;
static int g_jobCnt = 0;

void DumpAllJobs()
{
    INIT_LOGD("Ready to dump all jobs:");
    for (int i = 0; i < g_jobCnt; i++) {
        INIT_LOGD("\tjob name: %s", g_jobs[i].name);
        INIT_LOGD("\tlist all commands:");
        for (int j = 0; j < g_jobs[i].cmdLinesCnt; j++) {
            INIT_LOGD("\t\tcommand name : %s, command options: %s",
                g_jobs[i].cmdLines[j].name, g_jobs[i].cmdLines[j].cmdContent);
        }
    }
    INIT_LOGD("To dump all jobs finished");
}

static int GetJobName(const cJSON* jobItem, Job* resJob)
{
    char* jobNameStr = cJSON_GetStringValue(cJSON_GetObjectItem(jobItem, "name"));
    if (jobNameStr == NULL) {
        return 0;
    }

    if (memcpy_s(resJob->name, MAX_JOB_NAME_LEN, jobNameStr, strlen(jobNameStr)) != EOK) {
        INIT_LOGE("Get job name \"%s\" failed", jobNameStr);
        return 0;
    }
    resJob->name[strlen(jobNameStr)] = '\0';
    return 1;
}

static void ParseJob(const cJSON* jobItem, Job* resJob)
{
    if (!GetJobName(jobItem, resJob)) {
        INIT_LOGE("get JobName failed");
        (void)memset_s(resJob, sizeof(*resJob), 0, sizeof(*resJob));
        return;
    }

    cJSON* cmdsItem = cJSON_GetObjectItem(jobItem, CMDS_ARR_NAME_IN_JSON);
    if (!cJSON_IsArray(cmdsItem)) {
        INIT_LOGE("job %s is not an arrary", resJob->name);
        return;
    }

    int cmdLinesCnt = cJSON_GetArraySize(cmdsItem);
    if (cmdLinesCnt <= 0) {  // empty job, no cmd
        INIT_LOGE("empty job \"%s\"", resJob->name);
        return;
    }

    INIT_LOGD("job = %s, cmdLineCnt = %d", resJob->name, cmdLinesCnt);
    if (cmdLinesCnt > MAX_CMD_CNT_IN_ONE_JOB) {
        INIT_LOGE("ParseAllJobs, too many cmds[cnt %d] in one job, it should not exceed %d.",
            cmdLinesCnt, MAX_CMD_CNT_IN_ONE_JOB);
        return;
    }

    resJob->cmdLines = (CmdLine*)malloc(cmdLinesCnt * sizeof(CmdLine));
    if (resJob->cmdLines == NULL) {
        INIT_LOGE("allocate memory for command line failed");
        return;
    }

    if (memset_s(resJob->cmdLines, cmdLinesCnt * sizeof(CmdLine), 0, cmdLinesCnt * sizeof(CmdLine)) != EOK) {
        free(resJob->cmdLines);
        resJob->cmdLines = NULL;
        return;
    }
    resJob->cmdLinesCnt = cmdLinesCnt;

    for (int i = 0; i < cmdLinesCnt; ++i) {
        char* cmdLineStr = cJSON_GetStringValue(cJSON_GetArrayItem(cmdsItem, i));
        ParseCmdLine(cmdLineStr, &(resJob->cmdLines[i]));
    }
}

void ParseAllJobs(const cJSON* fileRoot)
{
    if (fileRoot == NULL) {
        INIT_LOGE("ParseAllJobs, input fileRoot is NULL!");
        return;
    }

    cJSON* jobArr = cJSON_GetObjectItemCaseSensitive(fileRoot, JOBS_ARR_NAME_IN_JSON);
    if (!cJSON_IsArray(jobArr)) {
        INIT_LOGE("ParseAllJobs, job item is not array!");
        return;
    }

    int jobArrSize = cJSON_GetArraySize(jobArr);
    if (jobArrSize <= 0 || jobArrSize > MAX_JOBS_COUNT) {
        INIT_LOGE("ParseAllJobs, jobs count %d is invalid, should be positive and not exceeding %d.",
            jobArrSize, MAX_JOBS_COUNT);
        return;
    }

    Job* retJobs = (Job*)realloc(g_jobs, sizeof(Job) * (g_jobCnt + jobArrSize));
    if (retJobs == NULL) {
        INIT_LOGE("ParseAllJobs, malloc failed! job arrSize %d.", jobArrSize);
        return;
    }

    Job* tmp = retJobs + g_jobCnt;
    if (memset_s(tmp, sizeof(Job) * jobArrSize, 0, sizeof(Job) * jobArrSize) != EOK) {
        INIT_LOGE("ParseAllJobs, memset_s failed.");
        free(retJobs);
        retJobs = NULL;
        return;
    }

    for (int i = 0; i < jobArrSize; ++i) {
        cJSON* jobItem = cJSON_GetArrayItem(jobArr, i);
        ParseJob(jobItem, &(tmp[i]));
    }
    g_jobs = retJobs;
    g_jobCnt += jobArrSize;
}

void DoJob(const char* jobName)
{
    if (jobName == NULL) {
        INIT_LOGE("DoJob, input jobName NULL!");
        return;
    }

    INIT_LOGD("Call job with name %s", jobName);
    for (int i = 0; i < g_jobCnt; ++i) {
        if (strncmp(jobName, g_jobs[i].name, strlen(g_jobs[i].name)) == 0) {
            CmdLine* cmdLines = g_jobs[i].cmdLines;
            for (int j = 0; j < g_jobs[i].cmdLinesCnt; ++j) {
                DoCmd(&(cmdLines[j]));
            }
        }
    }
}

void ReleaseAllJobs()
{
    if (g_jobs == NULL) {
        return;
    }

    for (int i = 0; i < g_jobCnt; ++i) {
        if (g_jobs[i].cmdLines != NULL) {
            free(g_jobs[i].cmdLines);
            g_jobs[i].cmdLines = NULL;
            g_jobs[i].cmdLinesCnt = 0;
        }
    }

    free(g_jobs);
    g_jobs = NULL;
    g_jobCnt = 0;
}


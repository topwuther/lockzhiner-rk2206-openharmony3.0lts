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

#include "init_read_cfg.h"

#include <errno.h>
#include <dirent.h>
#include <linux/capability.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "init_import.h"
#include "init_jobs.h"
#include "init_log.h"
#include "init_perms.h"
#include "init_service_manager.h"
#include "init_utils.h"

#ifndef OHOS_LITE
#include "init_param.h"
#endif
#include "securec.h"
#ifndef __LINUX__
#ifdef OHOS_LITE
#include "init_stage.h"
#endif
#endif

#define FILE_NAME_MAX_SIZE 100
static void ParseInitCfgContents(const cJSON *root)
{
    if (root == NULL) {
        INIT_LOGE("ParseInitCfgContents root is NULL");
        return;
    }
    // parse services
    ParseAllServices(root);
#ifdef OHOS_LITE
    // parse jobs
    ParseAllJobs(root);
#else
    ParseTriggerConfig(root);
#endif

    // parse imports
    ParseAllImports(root);
}

void ParseInitCfg(const char *configFile)
{
    if (configFile == NULL || *configFile == '\0') {
        INIT_LOGE("Invalid config file");
        return;
    }

    char *fileBuf = ReadFileToBuf(configFile);
    if (fileBuf == NULL) {
        INIT_LOGE("Read %s failed", configFile);
        return;
    }
    cJSON* fileRoot = cJSON_Parse(fileBuf);
    free(fileBuf);
    fileBuf = NULL;

    if (fileRoot == NULL) {
        INIT_LOGE("InitReadCfg, parse failed! please check file %s format.", configFile);
        return;
    }
    ParseInitCfgContents(fileRoot);
    // Release JSON object
    cJSON_Delete(fileRoot);
    return;
}

static void ReadCfgs(const char *dirPath)
{
    DIR *pDir = opendir(dirPath);
    if (pDir == NULL) {
        INIT_LOGE("ParseCfgs open cfg dir :%s failed.%d", dirPath, errno);
        return;
    }
    struct dirent *dp;
    while ((dp = readdir(pDir)) != NULL) {
        char fileName[FILE_NAME_MAX_SIZE];
        if (snprintf_s(fileName, FILE_NAME_MAX_SIZE, FILE_NAME_MAX_SIZE - 1, "%s/%s", dirPath, dp->d_name) == -1) {
            INIT_LOGE("ParseCfgs snprintf_s failed.");
            closedir(pDir);
            return;
        }
        struct stat st;
        if (stat(fileName, &st) == 0) {
            if (strstr(dp->d_name, ".cfg") == NULL) {
                continue;
            }
            INIT_LOGI("ReadCfgs :%s from %s success.", fileName, dirPath);
            ParseInitCfg(fileName);
        }
    }
    closedir(pDir);
    return;
}

static void ParseOtherCfgs()
{
    ReadCfgs("/system/etc/init");
    return;
}

void InitReadCfg()
{
#ifndef OHOS_LITE
    InitParamService();
    LoadDefaultParams("/system/etc/ohos.para");
#endif
    ParseInitCfg(INIT_CONFIGURATION_FILE);
    ParseOtherCfgs();
    INIT_LOGI("Parse init config file done.");
#ifdef OHOS_SERVICE_DUMP
    DumpAllServices();
#endif

#ifdef OHOS_LITE
    // do jobs
    DoJob("pre-init");
#ifndef __LINUX__
    TriggerStage(EVENT1, EVENT1_WAITTIME, QS_STAGE1);
#endif

    DoJob("init");
#ifndef __LINUX__
    TriggerStage(EVENT2, EVENT2_WAITTIME, QS_STAGE2);
#endif

    DoJob("post-init");
#ifndef __LINUX__
    TriggerStage(EVENT3, EVENT3_WAITTIME, QS_STAGE3);

    InitStageFinished();
#endif
    ReleaseAllJobs();
#else
    PostTrigger(EVENT_BOOT, "pre-init", strlen("pre-init"));

    PostTrigger(EVENT_BOOT, "init", strlen("init"));

    PostTrigger(EVENT_BOOT, "post-init", strlen("post-init"));
#endif
}


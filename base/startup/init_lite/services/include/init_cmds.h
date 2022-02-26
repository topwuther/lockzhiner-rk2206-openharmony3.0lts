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

#ifndef BASE_STARTUP_INITLITE_CMDS_H
#define BASE_STARTUP_INITLITE_CMDS_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MAX_CMD_NAME_LEN 32
#define MAX_CMD_CONTENT_LEN 256
#define MAX_CMD_CNT_IN_ONE_JOB 200
#define MAX_COPY_BUF_SIZE 256
#define DEFAULT_COPY_ARGS_CNT 2

#ifndef OHOS_LITE
// Limit max length of parameter value to 96
#define MAX_PARAM_VALUE_LEN 96
// Limit max length of parameter name to 96
#define MAX_PARAM_NAME_LEN 96
#else
// For lite ohos, do not support parameter operation
#define MAX_PARAM_VALUE_LEN 0
#define MAX_PARAM_NAME_LEN 0
#endif

// one cmd line
typedef struct {
    char name[MAX_CMD_NAME_LEN + 1];
    char cmdContent[MAX_CMD_CONTENT_LEN + 1];
} CmdLine;

struct CmdArgs {
    int argc;
    char **argv;
};

int GetParamValue(const char *symValue, char *paramValue, unsigned int paramLen);
struct CmdArgs* GetCmd(const char *cmdContent, const char *delim, int argsCount);
void FreeCmd(struct CmdArgs *cmd);

void ParseCmdLine(const char* cmdStr, CmdLine* resCmd);
void DoCmd(const CmdLine* curCmd);

void DoCmdByName(const char *name, const char *cmdContent);
const char *GetMatchCmd(const char *cmdStr);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // BASE_STARTUP_INITLITE_CMDS_H

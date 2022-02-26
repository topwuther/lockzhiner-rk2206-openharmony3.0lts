/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#include "init_cmds.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#ifndef OHOS_LITE
#include <linux/module.h>
#endif
#include <limits.h>
#include <net/if.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <unistd.h>
#include "init_jobs.h"
#include "init_log.h"
#ifndef OHOS_LITE
#include "init_param.h"
#endif
#include "init_reboot.h"
#include "init_service_manager.h"
#include "init_utils.h"
#include "securec.h"

#define DEFAULT_DIR_MODE (S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH | S_IROTH)  // mkdir, default mode
#define SPACES_CNT_IN_CMD_MAX 10   // mount, max number of spaces in cmdline
#define SPACES_CNT_IN_CMD_MIN 2    // mount, min number of spaces in cmdline

#define LOADCFG_BUF_SIZE  128  // loadcfg, max buffer for one cmdline
#define LOADCFG_MAX_FILE_LEN 51200  // loadcfg, max file size is 50K
#define LOADCFG_MAX_LOOP 20  // loadcfg, to prevent to be trapped in infite loop
#define OCTAL_TYPE 8  // 8 means octal to decimal
#define MAX_BUFFER 256UL
#define AUTHORITY_MAX_SIZE 128
#define WAIT_MAX_COUNT 10

static const char *g_supportCfg[] = {
    "/etc/patch.cfg",
    "/patch/fstab.cfg",
};

#ifndef OHOS_LITE
int GetParamValue(const char *symValue, char *paramValue, unsigned int paramLen)
{
    if ((symValue == NULL) || (paramValue == NULL) || (paramLen == 0)) {
        return -1;
    }
    char tmpName[MAX_PARAM_NAME_LEN] = {0};
    char tmpValue[MAX_PARAM_VALUE_LEN] = {0};
    char *p = NULL;
    char *tmpptr = NULL;
    p = strchr(symValue, '$');
    if (p == NULL) { // not has '$' copy the original string
        INIT_CHECK_RETURN_VALUE(strncpy_s(paramValue, paramLen, symValue, paramLen - 1) == EOK, -1);
        return 0;
    }
    unsigned int tmpLen = p - symValue;
    if (tmpLen > 0) { // copy '$' front string
        INIT_CHECK_RETURN_VALUE(strncpy_s(paramValue, paramLen, symValue, tmpLen) == EOK, -1);
    }
    p++;
    if (*p == '{') {
        p++;
        char *right = strchr(p, '}');
        if (right == NULL) {
            INIT_LOGE("Invalid cfg file name, miss '}'.");
            return -1;
        }
        tmpLen = right - p;
        if (tmpLen > MAX_PARAM_NAME_LEN) {
            INIT_LOGE("Parameter name longer than %d", MAX_PARAM_NAME_LEN);
            return -1;
        }
        INIT_CHECK_RETURN_VALUE(strncpy_s(tmpName, MAX_PARAM_NAME_LEN, p, tmpLen) == EOK, -1);
        int ret = SystemReadParam(tmpName, tmpValue, &tmpLen); // get param
        if (ret != 0) {
            INIT_LOGE("Failed to read parameter \" %s \"", tmpName);
            return -1;
        }
        // change param to new string
        INIT_CHECK_RETURN_VALUE(strncat_s(paramValue, paramLen, tmpValue, MAX_PARAM_VALUE_LEN) == EOK, -1);
        tmpptr = right + 1;
        tmpLen = paramLen - (tmpptr - symValue);
        if (*tmpptr != '\0') { // copy last string
            INIT_CHECK_RETURN_VALUE(strncat_s(paramValue, paramLen, tmpptr, tmpLen) == EOK, -1);
        }
        INIT_LOGI("paramValue is %s ", paramValue);
        return 0;
    } else {
        INIT_LOGE("Invalid cfg file name, miss '{'.");
        return -1;
    }
}
#else
// For ite ohos, do not support parameter operation. just do string copy
inline int GetParamValue(const char *symValue, char *paramValue, unsigned int paramLen)
{
    return (strncpy_s(paramValue, paramLen, symValue, strlen(symValue)) == EOK) ? 0 : -1;
}
#endif

static struct CmdArgs *CopyCmd(struct CmdArgs *ctx, const char *cmd, size_t allocSize)
{
    if (cmd == NULL) {
        FreeCmd(ctx);
        return NULL;
    }

    ctx->argv[ctx->argc] = calloc(sizeof(char), allocSize);
    INIT_CHECK(ctx->argv[ctx->argc] != NULL, FreeCmd(ctx);
        return NULL);
    INIT_CHECK(GetParamValue(cmd, ctx->argv[ctx->argc], allocSize) == 0, FreeCmd(ctx);
        return NULL);
    ctx->argc += 1;
    ctx->argv[ctx->argc] = NULL;
    return ctx;
}

#define SKIP_SPACES(p)          \
    do {                        \
        while (isspace(*(p))) { \
            (p)++;              \
        }                       \
    } while (0)

struct CmdArgs *GetCmd(const char *cmdContent, const char *delim, int argsCount)
{
    INIT_CHECK_RETURN_VALUE(cmdContent != NULL && delim != NULL, NULL);
    struct CmdArgs *ctx = (struct CmdArgs *)calloc(sizeof(struct CmdArgs), 1);
    INIT_CHECK_RETURN_VALUE(ctx != NULL, NULL);

    ctx->argv = (char**)calloc(sizeof(char*), (size_t)(argsCount + 1));
    INIT_CHECK(ctx->argv != NULL, FreeCmd(ctx);
        return NULL);

    char tmpCmd[MAX_BUFFER] = {0};
    size_t cmdLength = strlen(cmdContent);
    if (cmdLength > MAX_BUFFER - 1) {
        FreeCmd(ctx);
        return NULL;
    }

    INIT_CHECK(strncpy_s(tmpCmd, MAX_BUFFER - 1, cmdContent, cmdLength) == EOK, FreeCmd(ctx);
        return NULL);

    char *p = tmpCmd;
    char *token = NULL;
    size_t allocSize;

    // Skip lead whitespaces
    SKIP_SPACES(p);
    ctx->argc = 0;
    token = strstr(p, delim);
    if (token == NULL) { // No whitespaces
        // Make surce there is enough memory to store parameter value
        allocSize = (size_t)(cmdLength + MAX_PARAM_VALUE_LEN + 1);
        return CopyCmd(ctx, p, allocSize);
    }

    while (token != NULL) {
        // Too more arguments, treat rest of data as one argument
        if (ctx->argc == (argsCount - 1)) {
            break;
        }
        *token = '\0'; // replace it with '\0';
        allocSize = (size_t)((token - p) + MAX_PARAM_VALUE_LEN + 1);
        ctx = CopyCmd(ctx, p, allocSize);
        INIT_CHECK_RETURN_VALUE(ctx != NULL, NULL);
        p = token + 1; // skip '\0'
        // Skip lead whitespaces
        SKIP_SPACES(p);
        token = strstr(p, delim);
    }

    if (p < tmpCmd + cmdLength) {
        // no more white space or encounter max argument count
        size_t restSize = tmpCmd + cmdLength - p;
        allocSize = restSize + MAX_PARAM_VALUE_LEN + 1;
        ctx = CopyCmd(ctx, p, allocSize);
        INIT_CHECK_RETURN_VALUE(ctx != NULL, NULL);
    }
    return ctx;
}

void FreeCmd(struct CmdArgs *cmd)
{
    INIT_CHECK_ONLY_RETURN(cmd != NULL);
    for (int i = 0; i < cmd->argc; ++i) {
        INIT_CHECK(cmd->argv[i] == NULL, free(cmd->argv[i]));
    }
    INIT_CHECK(cmd->argv == NULL, free(cmd->argv));
    free(cmd);
    return;
}

static void WriteCommon(const char *file, char *buffer, int flags, mode_t mode)
{
    if (file == NULL || *file == '\0' || buffer == NULL || *buffer == '\0') {
        INIT_LOGE("Invalid arugment");
        return;
    }
    char realPath[PATH_MAX] = {0};
    if (realpath(file, realPath) != NULL) {
        int fd = open(realPath, flags, mode);
        if (fd >= 0) {
            size_t totalSize = strlen(buffer);
            size_t written = WriteAll(fd, buffer, totalSize);
            if (written != totalSize) {
                INIT_LOGE("Write %lu bytes to file failed", totalSize, file);
            }
            close(fd);
        }
        fd = -1;
    }
}

static void DoSetDomainname(const char *cmdContent, int maxArg)
{
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);

    if ((ctx == NULL) || (ctx->argv == NULL) || (ctx->argc != maxArg)) {
        INIT_LOGE("Command setdomainname  with invalid arugment");
        FreeCmd(ctx);
        return;
    }

    WriteCommon("/proc/sys/kernel/domainname", ctx->argv[0], O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC,
        S_IRUSR | S_IWUSR);
    FreeCmd(ctx);
    return;
}

static void DoSetHostname(const char *cmdContent, int maxArg)
{
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);

    if ((ctx == NULL) || (ctx->argv == NULL) || (ctx->argc != maxArg)) {
        INIT_LOGE("Command sethostname with invalid arugment");
        FreeCmd(ctx);
        return;
    }
    WriteCommon("/proc/sys/kernel/hostname", ctx->argv[0], O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC,
        S_IRUSR | S_IWUSR);
    FreeCmd(ctx);
    return;
}

#ifndef OHOS_LITE
static void DoIfup(const char *cmdContent, int maxArg)
{
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if ((ctx == NULL) || (ctx->argv == NULL) || (ctx->argc != maxArg)) {
        INIT_LOGE("Command ifup with invalid arguments");
        FreeCmd(ctx);
        return;
    }

    struct ifreq interface;
    if (strncpy_s(interface.ifr_name, IFNAMSIZ - 1, ctx->argv[0], strlen(ctx->argv[0])) != EOK) {
        INIT_LOGE("Failed to copy interface name");
        FreeCmd(ctx);
        return;
    }

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd >= 0) {
        do {
            if (ioctl(fd, SIOCGIFFLAGS, &interface) < 0) {
                INIT_LOGE("Failed to do ioctl with command \"SIOCGIFFLAGS\", err = %d", errno);
                break;
            }
            interface.ifr_flags |= IFF_UP;
            if (ioctl(fd, SIOCSIFFLAGS, &interface) < 0) {
                INIT_LOGE("Failed to do ioctl with command \"SIOCSIFFLAGS\", err = %d", errno);
                break;
            }
        } while (0);
        close(fd);
        fd = -1;
    }

    FreeCmd(ctx);
    return;
}
#endif

static void DoSleep(const char *cmdContent, int maxArg)
{
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoSleep invalid arguments :%s", cmdContent);
        goto out;
    }

    errno = 0;
    unsigned long sleepTime = strtoul(ctx->argv[0], NULL, DECIMAL_BASE);
    if (errno != 0) {
        INIT_LOGE("cannot covert sleep time in command \" sleep \"");
        goto out;
    }

    // Limit sleep time in 5 seconds
    const unsigned long sleepTimeLimit = 5;
    if (sleepTime > sleepTimeLimit) {
        sleepTime = sleepTimeLimit;
    }
    INIT_LOGI("Sleeping %d second(s)", sleepTime);
    sleep((unsigned int)sleepTime);
out:
    FreeCmd(ctx);
    return;
}

static void DoStart(const char* cmdContent, int maxArg)
{
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoStart invalid arguments :%s", cmdContent);
        goto out;
    }
    INIT_LOGD("DoStart %s", cmdContent);
    StartServiceByName(cmdContent);
out:
    FreeCmd(ctx);
    return;
}

static void DoStop(const char* cmdContent, int maxArg)
{
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoStop invalid arguments :%s", cmdContent);
        goto out;
    }
    INIT_LOGD("DoStop %s", cmdContent);
    StopServiceByName(cmdContent);
out:
    FreeCmd(ctx);
    return;
}

static void DoReset(const char* cmdContent, int maxArg)
{
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoReset invalid arguments :%s", cmdContent);
        goto out;
    }
    INIT_LOGD("DoReset %s", cmdContent);
    DoStop(cmdContent, maxArg);
    DoStart(cmdContent, maxArg);
out:
    FreeCmd(ctx);
    return;
}

static void DoCopy(const char* cmdContent, int maxArg)
{
    int srcFd = -1;
    int dstFd = -1;
    int rdLen = 0;
    char buf[MAX_COPY_BUF_SIZE] = {0};
    char *realPath1 = NULL;
    char *realPath2 = NULL;
    struct stat fileStat = {0};
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argv[0] == NULL || ctx->argv[1] == NULL ||
        ctx->argc != DEFAULT_COPY_ARGS_CNT) {
        INIT_LOGE("DoCopy invalid arguments :%s", cmdContent);
        goto out;
    }
    realPath1 = realpath(ctx->argv[0], NULL);
    if (realPath1 == NULL) {
        goto out;
    }
    realPath2 = realpath(ctx->argv[1], NULL);
    if (realPath2 == NULL) {
        goto out;
    }
    srcFd = open(realPath1, O_RDONLY);
    INIT_ERROR_CHECK(srcFd >= 0, goto out, "copy open %s fail %d! ", ctx->argv[0], errno);
    INIT_ERROR_CHECK(stat(ctx->argv[0], &fileStat) == 0, goto out, "stat fail ");
    mode_t mode = fileStat.st_mode;
    dstFd = open(realPath2, O_WRONLY | O_TRUNC | O_CREAT, mode);
    INIT_ERROR_CHECK(dstFd >= 0, goto out, "copy open %s fail %d! ", ctx->argv[1], errno);
    while ((rdLen = read(srcFd, buf, sizeof(buf) - 1)) > 0) {
        int rtLen = write(dstFd, buf, rdLen);
        INIT_ERROR_CHECK(rtLen == rdLen, goto out, "write %s file fail %d! ", ctx->argv[1], errno);
    }
    fsync(dstFd);
out:
    FreeCmd(ctx);
    ctx = NULL;
    INIT_CHECK(srcFd < 0, close(srcFd); srcFd = -1);
    INIT_CHECK(dstFd < 0, close(dstFd); dstFd = -1);
    INIT_CHECK(realPath1 == NULL, free(realPath1); realPath1 = NULL);
    INIT_CHECK(realPath2 == NULL, free(realPath2); realPath2 = NULL);
    return;
}

static void DoChown(const char* cmdContent, int maxArg)
{
    // format: chown owner group /xxx/xxx/xxx
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoChown invalid arguments :%s", cmdContent);
        goto out;
    }

    uid_t owner = DecodeUid(ctx->argv[0]);
    INIT_ERROR_CHECK(owner != (uid_t)-1, goto out, "DoChown invalid uid :%s.", ctx->argv[0]);

    gid_t group = DecodeUid(ctx->argv[1]);
    INIT_ERROR_CHECK(group != (gid_t)-1, goto out, "DoChown invalid gid :%s.", ctx->argv[1]);

    const int pathPos = 2;
    if (chown(ctx->argv[pathPos], owner, group) != 0) {
        INIT_LOGE("DoChown, failed for %s, err %d.", cmdContent, errno);
    }
out:
    FreeCmd(ctx);
    return;
}

static void DoMkDir(const char* cmdContent, int maxArg)
{
    // mkdir support format:
    //    1.mkdir path
    //    2.mkdir path mode
    //    3.mkdir path mode owner group
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc < 1) {
        INIT_LOGE("DoMkDir invalid arguments :%s", cmdContent);
        goto out;
    }

    const int withModeArg = 2;
    if (ctx->argc != 1 && ctx->argc != maxArg && ctx->argc != withModeArg) {
        INIT_LOGE("DoMkDir invalid arguments: %s", cmdContent);
        goto out;
    }

    mode_t mode = DEFAULT_DIR_MODE;
    if (mkdir(ctx->argv[0], mode) != 0 && errno != EEXIST) {
        INIT_LOGE("DoMkDir, failed for %s, err %d.", cmdContent, errno);
        goto out;
    }

    if (ctx->argc > 1) {
        mode = strtoul(ctx->argv[1], NULL, OCTAL_TYPE);
        if (chmod(ctx->argv[0], mode) != 0) {
            INIT_LOGE("DoMkDir failed for %s, err %d.", cmdContent, errno);
        }
        if (ctx->argc == withModeArg) {
            goto out;
        }
        const int ownerPos = 2;
        const int groupPos = 3;

        uid_t owner = DecodeUid(ctx->argv[ownerPos]);
        INIT_ERROR_CHECK(owner != (uid_t)-1, goto out, "DoMkDir invalid uid :%s.", ctx->argv[ownerPos]);

        gid_t group = DecodeUid(ctx->argv[groupPos]);
        INIT_ERROR_CHECK(group != (gid_t)-1, goto out, "DoMkDir invalid gid :%s.", ctx->argv[groupPos]);

        if (chown(ctx->argv[0], owner, group) != 0) {
            INIT_LOGE("DoMkDir, chown failed for %s, err %d.", cmdContent, errno);
        }
    }
out:
    FreeCmd(ctx);
    return;
}

static void DoChmod(const char* cmdContent, int maxArg)
{
    // format: chmod xxxx /xxx/xxx/xxx
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoChmod invalid arguments :%s", cmdContent);
        goto out;
    }

    mode_t mode = strtoul(ctx->argv[0], NULL, OCTAL_TYPE);
    if (mode == 0) {
        INIT_LOGE("DoChmod, strtoul failed for %s, er %d.", cmdContent, errno);
        goto out;
    }

    if (chmod(ctx->argv[1], mode) != 0) {
        INIT_LOGE("DoChmod, failed for %s, err %d.", cmdContent, errno);
    }
out:
    FreeCmd(ctx);
    return;
}

static char* CopySubStr(const char* srcStr, size_t startPos, size_t endPos)
{
    if (endPos <= startPos) {
        INIT_LOGE("DoMount, invalid params<%zu, %zu> for %s.", endPos, startPos, srcStr);
        return NULL;
    }

    size_t mallocLen = endPos - startPos + 1;
    char* retStr = (char*)malloc(mallocLen);
    if (retStr == NULL) {
        INIT_LOGE("DoMount, malloc failed! malloc size %zu, for %s.", mallocLen, srcStr);
        return NULL;
    }

    const char* copyStart = srcStr + startPos;
    if (memcpy_s(retStr, mallocLen, copyStart, endPos - startPos) != EOK) {
        INIT_LOGE("DoMount, memcpy_s failed for %s.", srcStr);
        free(retStr);
        return NULL;
    }
    retStr[mallocLen - 1] = '\0';

    // for example, source may be none
    if (strncmp(retStr, "none", strlen("none")) == 0) {
        retStr[0] = '\0';
    }
    return retStr;
}

static int GetMountFlag(unsigned long* mountflags, const char* targetStr, const char *source)
{
    if (targetStr == NULL) {
        return 0;
    }

    if (strncmp(targetStr, "nodev", strlen("nodev")) == 0) {
        (*mountflags) |= MS_NODEV;
    } else if (strncmp(targetStr, "noexec", strlen("noexec")) == 0) {
        (*mountflags) |= MS_NOEXEC;
    } else if (strncmp(targetStr, "nosuid", strlen("nosuid")) == 0) {
        (*mountflags) |= MS_NOSUID;
    } else if (strncmp(targetStr, "rdonly", strlen("rdonly")) == 0) {
        (*mountflags) |= MS_RDONLY;
    } else if (strncmp(targetStr, "noatime", strlen("noatime")) == 0) {
        (*mountflags) |= MS_NOATIME;
    } else if (strncmp(targetStr, "wait", strlen("wait")) == 0) {
        WaitForFile(source, WAIT_MAX_COUNT);
    } else {
        return 0;
    }
    return 1;
}

static int CountSpaces(const char* cmdContent, size_t* spaceCnt, size_t* spacePosArr, size_t spacePosArrLen)
{
    *spaceCnt = 0;
    size_t strLen = strlen(cmdContent);
    for (size_t i = 0; i < strLen; ++i) {
        if (cmdContent[i] == ' ') {
            ++(*spaceCnt);
            if ((*spaceCnt) > spacePosArrLen) {
                INIT_LOGE("DoMount, too many spaces, bad format for %s.", cmdContent);
                return 0;
            }
            spacePosArr[(*spaceCnt) - 1] = i;
        }
    }

    if ((*spaceCnt) < SPACES_CNT_IN_CMD_MIN ||           // spaces count should not less than 2(at least 3 items)
        spacePosArr[0] == 0 ||                           // should not start with space
        spacePosArr[(*spaceCnt) - 1] == strLen - 1) {    // should not end with space
        INIT_LOGE("DoMount, bad format for %s.", cmdContent);
        return 0;
    }

    // spaces should not be adjacent
    for (size_t i = 1; i < (*spaceCnt); ++i) {
        if (spacePosArr[i] == spacePosArr[i - 1] + 1) {
            INIT_LOGE("DoMount, bad format for %s.", cmdContent);
            return 0;
        }
    }
    return 1;
}

static void DoMount(const char* cmdContent, int maxArg)
{
    size_t spaceCnt = 0;
    size_t spacePosArr[SPACES_CNT_IN_CMD_MAX] = {0};
    if (!CountSpaces(cmdContent, &spaceCnt, spacePosArr, SPACES_CNT_IN_CMD_MAX)) {
        return;
    }

    // format: fileSystemType source target mountFlag1 mountFlag2... data
    unsigned long mountflags = 0;
    size_t strLen = strlen(cmdContent);
    size_t indexOffset = 0;
    char* fileSysType = CopySubStr(cmdContent, 0, spacePosArr[indexOffset]);
    if (fileSysType == NULL) {
        return;
    }

    char* source = CopySubStr(cmdContent, spacePosArr[indexOffset] + 1, spacePosArr[indexOffset + 1]);
    if (source == NULL) {
        free(fileSysType);
        return;
    }
    ++indexOffset;

    // maybe only has "filesystype source target", 2 spaces
    size_t targetEndPos = (indexOffset == spaceCnt - 1) ? strLen : spacePosArr[indexOffset + 1];
    char* target = CopySubStr(cmdContent, spacePosArr[indexOffset] + 1, targetEndPos);
    if (target == NULL) {
        free(fileSysType);
        free(source);
        return;
    }
    ++indexOffset;

    // get mountflags, if fail, the rest part of string will be data
    while (indexOffset < spaceCnt) {
        size_t tmpStrEndPos = (indexOffset == spaceCnt - 1) ? strLen : spacePosArr[indexOffset + 1];
        char* tmpStr = CopySubStr(cmdContent, spacePosArr[indexOffset] + 1, tmpStrEndPos);
        int ret = GetMountFlag(&mountflags, tmpStr, source);
        free(tmpStr);
        tmpStr = NULL;

        // get flag failed, the rest part of string will be data
        if (ret == 0) {
            break;
        }
        ++indexOffset;
    }

    int mountRet;
    if (indexOffset >= spaceCnt) {    // no data
        mountRet = mount(source, target, fileSysType, mountflags, NULL);
    } else {
        const char* dataStr = cmdContent + spacePosArr[indexOffset] + 1;
        mountRet = mount(source, target, fileSysType, mountflags, dataStr);
    }

    if (mountRet != 0) {
        INIT_LOGE("DoMount, failed for %s, err %d.", cmdContent, errno);
    }

    free(fileSysType);
    free(source);
    free(target);
}

#ifndef OHOS_LITE
#define OPTIONS_SIZE 128u
static void DoInsmodInternal(const char *fileName, const char *secondPtr, const char *restPtr, int flags)
{
    char options[OPTIONS_SIZE] = {0};
    if (flags == 0) { //  '-f' option
        if (restPtr != NULL && secondPtr != NULL) { // Reset arugments, combine then all.
            if (snprintf_s(options, sizeof(options), OPTIONS_SIZE -1, "%s %s", secondPtr, restPtr) == -1) {
                return;
            }
        } else if (secondPtr != NULL) {
            if (strncpy_s(options, OPTIONS_SIZE - 1, secondPtr, strlen(secondPtr)) != 0) {
                return;
            }
        }
    } else { // Only restPtr is option
        if (restPtr != NULL) {
            if (strncpy_s(options, OPTIONS_SIZE - 1, restPtr, strlen(restPtr)) != 0) {
                return;
            }
        }
    }
    if (fileName == NULL) {
        return;
    }
    char *realPath = realpath(fileName, NULL);
    if (realPath == NULL) {
        return;
    }
    int fd = open(realPath, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        INIT_LOGE("failed to open %s: %d", realPath, errno);
        free(realPath);
        realPath = NULL;
        return;
    }
    int rc = syscall(__NR_finit_module, fd, options, flags);
    if (rc == -1) {
        INIT_LOGE("finit_module for %s failed: %d", realPath, errno);
    }
    if (fd >= 0) {
        close(fd);
    }
    free(realPath);
    realPath = NULL;
    return;
}

// format insmod <ko name> [-f] [options]
static void DoInsmod(const char *cmdContent, int maxArg)
{
    char *p = NULL;
    char *restPtr = NULL;
    char *fileName = NULL;
    char *line = NULL;
    int flags = 0;

    size_t count = strlen(cmdContent);
    if (count > OPTIONS_SIZE) {
        INIT_LOGE("DoInsmod options too long, maybe lost some of options");
    }
    line = (char *)malloc(count + 1);
    if (line == NULL) {
        INIT_LOGE("DoInsmod allocate memory failed.");
        return;
    }

    if (memcpy_s(line, count + 1, cmdContent, count) != EOK) {
        INIT_LOGE("DoInsmod memcpy failed");
        free(line);
        return;
    }
    line[count] = '\0';
    do {
        if ((p = strtok_r(line, " ", &restPtr)) == NULL) {
            INIT_LOGE("DoInsmod cannot get filename.");
            free(line);
            return;
        }
        fileName = p;
        INIT_LOGI("DoInsmod fileName is [%s].", fileName);
        if ((p = strtok_r(NULL, " ", &restPtr)) == NULL) {
            break;
        }
        if (!strcmp(p, "-f")) {
            flags = MODULE_INIT_IGNORE_VERMAGIC | MODULE_INIT_IGNORE_MODVERSIONS;
        }
    } while (0);
    DoInsmodInternal(fileName, p, restPtr, flags);
    if (line != NULL) {
        free(line);
    }
    return;
}

static void DoSetParam(const char* cmdContent, int maxArg)
{
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoSetParam invalid arguments :%s", cmdContent);
        goto out;
    }
    INIT_LOGE("param name: %s, value %s ", ctx->argv[0], ctx->argv[1]);
    SystemWriteParam(ctx->argv[0], ctx->argv[1]);
out:
    FreeCmd(ctx);
    return;
}

#endif // OHOS_LITE

static bool CheckValidCfg(const char *path)
{
    size_t cfgCnt = sizeof(g_supportCfg) / sizeof(g_supportCfg[0]);
    struct stat fileStat = {0};

    if (stat(path, &fileStat) != 0 || fileStat.st_size <= 0 || fileStat.st_size > LOADCFG_MAX_FILE_LEN) {
        return false;
    }

    for (size_t i = 0; i < cfgCnt; ++i) {
        if (strcmp(path, g_supportCfg[i]) == 0) {
            return true;
        }
    }
    return false;
}

static void DoLoadCfg(const char *path, int maxArg)
{
    char buf[LOADCFG_BUF_SIZE] = {0};
    FILE *fp = NULL;
    size_t maxLoop = 0;
    CmdLine *cmdLine = NULL;
    size_t len;
    INIT_CHECK_ONLY_RETURN(path != NULL);
    INIT_LOGI("DoLoadCfg cfg file %s", path);
    if (!CheckValidCfg(path)) {
        INIT_LOGE("CheckCfg file %s Failed", path);
        return;
    }
    INIT_ERROR_CHECK(path != NULL, return, "CheckCfg path is NULL.");
    char *realPath = realpath(path, NULL);
    INIT_CHECK_ONLY_RETURN(realPath != NULL);
    fp = fopen(realPath, "r");
    if (fp == NULL) {
        INIT_LOGE("open cfg error = %d", errno);
        free(realPath);
        realPath = NULL;
        return;
    }

    cmdLine = (CmdLine *)malloc(sizeof(CmdLine));
    if (cmdLine == NULL) {
        INIT_LOGE("malloc cmdline error");
        fclose(fp);
        free(realPath);
        realPath = NULL;
        return;
    }

    while (fgets(buf, LOADCFG_BUF_SIZE - 1, fp) != NULL && maxLoop < LOADCFG_MAX_LOOP) {
        maxLoop++;
        len = strlen(buf);
        if (len < 1) {
            continue;
        }
        if (buf[len - 1] == '\n') {
            buf[len - 1] = '\0'; // we replace '\n' with '\0'
        }
        (void)memset_s(cmdLine, sizeof(CmdLine), 0, sizeof(CmdLine));
        ParseCmdLine(buf, cmdLine);
        DoCmd(cmdLine);
        (void)memset_s(buf, sizeof(char) * LOADCFG_BUF_SIZE, 0, sizeof(char) * LOADCFG_BUF_SIZE);
    }
    free(realPath);
    realPath = NULL;
    free(cmdLine);
    fclose(fp);
}

static void DoWrite(const char *cmdContent, int maxArg)
{
    // format: write path content
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argv[0] == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoWrite: invalid arguments :%s", cmdContent);
        goto out;
    }
    char *realPath = realpath(ctx->argv[0], NULL);
    if (realPath == NULL) {
        goto out;
    }
    int fd = open(realPath, O_WRONLY | O_CREAT | O_NOFOLLOW | O_CLOEXEC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        INIT_LOGE("DoWrite: open %s failed: %d", ctx->argv[0], errno);
        free(realPath);
        realPath = NULL;
        goto out;
    }

    ssize_t ret = write(fd, ctx->argv[1], strlen(ctx->argv[1]));
    if (ret < 0) {
        INIT_LOGE("DoWrite: write to file %s failed: %d", ctx->argv[0], errno);
        free(realPath);
        realPath = NULL;
        close(fd);
        goto out;
    }
    free(realPath);
    realPath = NULL;
    close(fd);
out:
    FreeCmd(ctx);
    return;
}

static void DoRmdir(const char *cmdContent, int maxArg)
{
    // format: rmdir path
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoRmdir: invalid arguments :%s", cmdContent);
        goto out;
    }

    int ret = rmdir(ctx->argv[0]);
    if (ret == -1) {
        INIT_LOGE("DoRmdir: remove %s failed: %d.", ctx->argv[0], errno);
        goto out;
    }
out:
    FreeCmd(ctx);
    return;
}

static void DoRebootCmd(const char *cmdContent, int maxArg)
{
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoReboot invalid arguments :%s", cmdContent);
        goto out;
    }
    DoReboot(cmdContent);
out:
    FreeCmd(ctx);
    return;
}

static void DoLoadPersistParams(const char *cmdContent, int maxArg)
{
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoLoadPersistParams invalid arguments :%s", cmdContent);
        goto out;
    }
    INIT_LOGD("load persist params : %s", cmdContent);
    LoadPersistParams();
out:
    FreeCmd(ctx);
    return;
}

static void DoTriggerCmd(const char *cmdContent, int maxArg)
{
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoTrigger invalid arguments :%s", cmdContent);
        goto out;
    }
    INIT_LOGD("DoTrigger :%s", cmdContent);
    DoTriggerExec(cmdContent);
out:
    FreeCmd(ctx);
    return;
}

static void DoLoadDefaultParams(const char *cmdContent, int maxArg)
{
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoLoadDefaultParams invalid arguments :%s", cmdContent);
        goto out;
    }
    INIT_LOGD("load persist params : %s", cmdContent);
    LoadDefaultParams(cmdContent);
out:
    FreeCmd(ctx);
    return;
}

static void DoSetrlimit(const char *cmdContent, int maxArg)
{
    char *resource[] = {
        "RLIMIT_CPU", "RLIMIT_FSIZE", "RLIMIT_DATA", "RLIMIT_STACK", "RLIMIT_CORE", "RLIMIT_RSS",
        "RLIMIT_NPROC", "RLIMIT_NOFILE", "RLIMIT_MEMLOCK", "RLIMIT_AS", "RLIMIT_LOCKS", "RLIMIT_SIGPENDING",
        "RLIMIT_MSGQUEUE", "RLIMIT_NICE", "RLIMIT_RTPRIO", "RLIMIT_RTTIME", "RLIM_NLIMITS"
    };
    // format: setrlimit resource curValue maxValue
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    const int rlimMaxPos = 2;
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoSetrlimit: invalid arguments :%s", cmdContent);
        goto out;
    }

    struct rlimit limit;
    limit.rlim_cur = (rlim_t)atoi(ctx->argv[1]);
    limit.rlim_max = (rlim_t)atoi(ctx->argv[rlimMaxPos]);
    int rcs = -1;
    for (unsigned int i = 0; i < ARRAY_LENGTH(resource); ++i) {
        if (strcmp(ctx->argv[0], resource[i]) == 0) {
            rcs = (int)i;
        }
    }
    if (rcs == -1) {
        INIT_LOGE("DoSetrlimit failed, resouces :%s not support.", ctx->argv[0]);
        goto out;
    }
    int ret = setrlimit(rcs, &limit);
    if (ret) {
        INIT_LOGE("DoSetrlimit failed : %d", errno);
        goto out;
    }
out:
    FreeCmd(ctx);
    return;
}

static void DoRm(const char *cmdContent, int maxArg)
{
    // format: rm /xxx/xxx/xxx
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoRm: invalid arguments :%s", cmdContent);
        goto out;
    }
    int ret = unlink(ctx->argv[0]);
    if (ret == -1) {
        INIT_LOGE("DoRm: unlink %s failed: %d.", ctx->argv[0], errno);
        goto out;
    }
out:
    FreeCmd(ctx);
    return;
}

static void DoExport(const char *cmdContent, int maxArg)
{
    // format: export xxx /xxx/xxx/xxx
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoExport: invalid arguments :%s", cmdContent);
        goto out;
    }
    int ret = setenv(ctx->argv[0], ctx->argv[1], 1);
    if (ret != 0) {
        INIT_LOGE("DoExport: set %s with %s failed: %d", ctx->argv[0], ctx->argv[1], errno);
        goto out;
    }
out:
    FreeCmd(ctx);
    return;
}

static void DoExec(const char *cmdContent, int maxArg)
{
    // format: exec /xxx/xxx/xxx xxx
    pid_t pid = fork();
    if (pid < 0) {
        INIT_LOGE("DoExec: failed to fork child process to exec \"%s\"", cmdContent);
        return;
    }
    if (pid == 0) {
        struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
        if (ctx == NULL || ctx->argv == NULL || ctx->argv[0] == NULL) {
            INIT_LOGE("DoExec: invalid arguments :%s", cmdContent);
            _exit(0x7f);
        }
#ifdef OHOS_LITE
        int ret = execve(ctx->argv[0], ctx->argv, NULL);
#else
        int ret = execv(ctx->argv[0], ctx->argv);
#endif
        if (ret == -1) {
            INIT_LOGE("DoExec: execute \"%s\" failed: %d.", cmdContent, errno);
        }
        FreeCmd(ctx);
        _exit(0x7f);
    }
    return;
}

#ifndef __LITEOS__
static void DoSymlink(const char *cmdContent, int maxArg)
{
    // format: symlink /xxx/xxx/xxx /xxx/xxx/xxx
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoSymlink: invalid arguments :%s", cmdContent);
        goto out;
    }

    int ret = symlink(ctx->argv[0], ctx->argv[1]);
    if (ret != 0) {
        INIT_LOGE("DoSymlink: link %s to %s failed: %d", ctx->argv[0], ctx->argv[1], errno);
        goto out;
    }
out:
    FreeCmd(ctx);
    return;
}

static mode_t GetDeviceMode(const char *deviceStr)
{
    switch (*deviceStr) {
        case 'b':
        case 'B':
            return S_IFBLK;
        case 'c':
        case 'C':
            return S_IFCHR;
        case 'f':
        case 'F':
            return S_IFIFO;
        default:
            return -1;
    }
}

static void DoMakeNode(const char *cmdContent, int maxArg)
{
    // format: mknod path b 0644 1 9
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    const int deviceTypePos = 1;
    const int authorityPos = 2;
    const int majorDevicePos = 3;
    const int minorDevicePos = 4;
    const int decimal = 10;
    const int octal = 8;
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoMakeNode: invalid arguments :%s", cmdContent);
        goto out;
    }

    if (!access(ctx->argv[1], F_OK)) {
        INIT_LOGE("DoMakeNode failed, path has not sexisted");
        goto out;
    }
    mode_t deviceMode = GetDeviceMode(ctx->argv[deviceTypePos]);
    unsigned int major = strtoul(ctx->argv[majorDevicePos], NULL, decimal);
    unsigned int minor = strtoul(ctx->argv[minorDevicePos], NULL, decimal);
    mode_t authority = strtoul(ctx->argv[authorityPos], NULL, octal);

    int ret = mknod(ctx->argv[0], deviceMode | authority, makedev(major, minor));
    if (ret != 0) {
        INIT_LOGE("DoMakeNode: path: %s failed: %d", ctx->argv[0], errno);
        goto out;
    }
out:
    FreeCmd(ctx);
    return;
}

static void DoMakeDevice(const char *cmdContent, int maxArg)
{
    // format: makedev major minor
    struct CmdArgs *ctx = GetCmd(cmdContent, " ", maxArg);
    const int decimal = 10;
    if (ctx == NULL || ctx->argv == NULL || ctx->argc != maxArg) {
        INIT_LOGE("DoMakedevice: invalid arguments :%s", cmdContent);
        goto out;
    }
    unsigned int major = strtoul(ctx->argv[0], NULL, decimal);
    unsigned int minor = strtoul(ctx->argv[1], NULL, decimal);
    dev_t deviceId = makedev(major, minor);
    if (deviceId < 0) {
        INIT_LOGE("DoMakedevice \" %s \" failed :%d ", cmdContent, errno);
        goto out;
    }
out:
    FreeCmd(ctx);
    return;
}
#endif // __LITEOS__

void DoCmd(const CmdLine* curCmd)
{
    // null curCmd or empty command, just quit.
    if (curCmd == NULL || curCmd->name[0] == '\0') {
        return;
    }
    DoCmdByName(curCmd->name, curCmd->cmdContent);
}

struct CmdTable {
    char name[MAX_CMD_NAME_LEN];
    int maxArg;
    void (*DoFuncion)(const char *cmdContent, int maxArg);
};

static const struct CmdTable CMD_TABLE[] = {
    { "start ", 1, DoStart },
    { "mkdir ", 4, DoMkDir },
    { "chmod ", 2, DoChmod },
    { "chown ", 3, DoChown },
    { "mount ", 10, DoMount },
    { "export ", 2, DoExport },
    { "loadcfg ", 1, DoLoadCfg },
    { "rm ", 1, DoRm },
    { "rmdir ", 1, DoRmdir },
    { "write ", 2, DoWrite },
    { "exec ", 10, DoExec },
#ifndef OHOS_LITE
    { "mknode ", 5, DoMakeNode },
    { "makedev ", 2, DoMakeDevice },
    { "symlink ", 2, DoSymlink },
    { "trigger ", 1, DoTriggerCmd },
    { "insmod ", 10, DoInsmod },
    { "setparam ", 2, DoSetParam },
    { "load_persist_params ", 1, DoLoadPersistParams },
    { "load_param ", 1, DoLoadDefaultParams },
    { "ifup ", 1, DoIfup },
#endif
    { "stop ", 1, DoStop },
    { "reset ", 1, DoReset },
    { "copy ", 2, DoCopy },
    { "reboot ", 1, DoRebootCmd },
    { "setrlimit ", 3, DoSetrlimit },
    { "sleep ", 1, DoSleep },
    { "hostname ", 1, DoSetHostname },
    { "domainname ", 1, DoSetDomainname }
};

void DoCmdByName(const char *name, const char *cmdContent)
{
    if (name == NULL || cmdContent == NULL) {
        return;
    }

    size_t cmdCnt = sizeof(CMD_TABLE) / sizeof(CMD_TABLE[0]);
    unsigned int i = 0;
    for (; i < cmdCnt; ++i) {
        if (strncmp(name, CMD_TABLE[i].name, strlen(CMD_TABLE[i].name)) == 0) {
            CMD_TABLE[i].DoFuncion(cmdContent, CMD_TABLE[i].maxArg);
            break;
        }
    }
    if (i == cmdCnt) {
        INIT_LOGE("DoCmd, unknown cmd name %s.", name);
    }
}

const char *GetMatchCmd(const char *cmdStr)
{
    if (cmdStr == NULL) {
        return NULL;
    }
    size_t supportCmdCnt = sizeof(CMD_TABLE) / sizeof(CMD_TABLE[0]);
    for (size_t i = 0; i < supportCmdCnt; ++i) {
        size_t curCmdNameLen = strlen(CMD_TABLE[i].name);
        if (strncmp(CMD_TABLE[i].name, cmdStr, curCmdNameLen) == 0) {
            return CMD_TABLE[i].name;
        }
    }
    return NULL;
}

void ParseCmdLine(const char* cmdStr, CmdLine* resCmd)
{
    size_t cmdLineLen = 0;
    if (cmdStr == NULL || resCmd == NULL || (cmdLineLen = strlen(cmdStr)) == 0) {
        return;
    }

    size_t supportCmdCnt = sizeof(CMD_TABLE) / sizeof(CMD_TABLE[0]);
    int foundAndSucceed = 0;
    for (size_t i = 0; i < supportCmdCnt; ++i) {
        size_t curCmdNameLen = strlen(CMD_TABLE[i].name);
        if (cmdLineLen > curCmdNameLen && cmdLineLen <= (curCmdNameLen + MAX_CMD_CONTENT_LEN) &&
            strncmp(CMD_TABLE[i].name, cmdStr, curCmdNameLen) == 0) {
            if (memcpy_s(resCmd->name, MAX_CMD_NAME_LEN, cmdStr, curCmdNameLen) != EOK) {
                break;
            }
            resCmd->name[curCmdNameLen] = '\0';

            const char* cmdContent = cmdStr + curCmdNameLen;
            size_t cmdContentLen = cmdLineLen - curCmdNameLen;
            if (memcpy_s(resCmd->cmdContent, MAX_CMD_CONTENT_LEN, cmdContent, cmdContentLen) != EOK) {
                break;
            }
            resCmd->cmdContent[cmdContentLen] = '\0';
            foundAndSucceed = 1;
            break;
        }
    }

    if (!foundAndSucceed) {
        INIT_LOGE("Cannot parse command: %s", cmdStr);
        (void)memset_s(resCmd, sizeof(*resCmd), 0, sizeof(*resCmd));
    }
}


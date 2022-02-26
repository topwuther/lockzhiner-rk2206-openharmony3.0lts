/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "init_utils.h"

#include <ctype.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "init_log.h"
#include "securec.h"

#define WAIT_MAX_COUNT 10
#define MAX_BUF_SIZE  1024
#ifdef STARTUP_UT
#define LOG_FILE_NAME "/media/sf_ubuntu/test/log.txt"
#else
#define LOG_FILE_NAME "/data/startup_log.txt"
#endif

#define MAX_JSON_FILE_LEN 102400    // max init.cfg size 100KB
#define CONVERT_MICROSEC_TO_SEC(x) ((x) / 1000 / 1000.0)

int DecodeUid(const char *name)
{
    if (name == NULL) {
        return -1;
    }
    bool digitFlag = true;
    for (unsigned int i = 0; i < strlen(name); ++i) {
        if (isalpha(name[i])) {
            digitFlag = false;
            break;
        }
    }
    if (digitFlag) {
        errno = 0;
        uid_t result = strtoul(name, 0, DECIMAL_BASE);
        if (errno != 0) {
            return -1;
        }
        return result;
    } else {
        struct passwd *userInf = getpwnam(name);
        if (userInf == NULL) {
            return -1;
        }
        return userInf->pw_uid;
    }
}

void CheckAndCreateDir(const char *fileName)
{
    if (fileName == NULL || *fileName == '\0') {
        return;
    }
    char *path = strndup(fileName, strrchr(fileName, '/') - fileName);
    if (path != NULL && access(path, F_OK) != 0) {
        mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    }
    free(path);
}

char* ReadFileToBuf(const char *configFile)
{
    char* buffer = NULL;
    FILE* fd = NULL;
    struct stat fileStat = {0};
    if (configFile == NULL || *configFile == '\0') {
        return NULL;
    }

    do {
        if (stat(configFile, &fileStat) != 0 ||
            fileStat.st_size <= 0 || fileStat.st_size > MAX_JSON_FILE_LEN) {
            INIT_LOGE("Unexpected config file \" %s \", check if it exist. if exist, check file size", configFile);
            break;
        }
        fd = fopen(configFile, "r");
        if (fd == NULL) {
            INIT_LOGE("Open %s failed. err = %d", configFile, errno);
            break;
        }
        buffer = (char*)malloc((size_t)(fileStat.st_size + 1));
        if (buffer == NULL) {
            INIT_LOGE("Failed to allocate memory for config file, err = %d", errno);
            break;
        }

        if (fread(buffer, fileStat.st_size, 1, fd) != 1) {
            free(buffer);
            buffer = NULL;
            break;
        }
        buffer[fileStat.st_size] = '\0';
    } while (0);

    if (fd != NULL) {
        fclose(fd);
        fd = NULL;
    }
    return buffer;
}

int SplitString(char *srcPtr, char **dstPtr, int maxNum)
{
    if ((!srcPtr) || (!dstPtr)) {
        return -1;
    }
    char *buf = NULL;
    dstPtr[0] = strtok_r(srcPtr, " ", &buf);
    int counter = 0;
    while (dstPtr[counter] != NULL && (counter < maxNum)) {
        counter++;
        dstPtr[counter] = strtok_r(NULL, " ", &buf);
    }
    dstPtr[counter] = NULL;
    return counter;
}

void WaitForFile(const char *source, unsigned int maxCount)
{
    if (maxCount > WAIT_MAX_COUNT) {
        INIT_LOGE("WaitForFile max time is 5s");
        maxCount = WAIT_MAX_COUNT;
    }
    struct stat sourceInfo;
    const unsigned int waitTime = 500000;
    unsigned int count = 0;
    do {
        usleep(waitTime);
        count++;
    } while ((stat(source, &sourceInfo) < 0) && (errno == ENOENT) && (count < maxCount));
    if (count == maxCount) {
        INIT_LOGE("wait for file:%s failed after %f.", source, maxCount * CONVERT_MICROSEC_TO_SEC(waitTime));
    }
    return;
}

size_t WriteAll(int fd, char *buffer, size_t size)
{
    if (fd < 0 || buffer == NULL || *buffer == '\0') {
        return 0;
    }

    char *p = buffer;
    size_t left = size;
    ssize_t written = -1;

    while (left > 0) {
        do {
            written = write(fd, p, left);
        } while (written < 0 && errno == EINTR);
        if (written < 0) {
            INIT_LOGE("Failed to write %lu bytes, err = %d", left, errno);
            break;
        }
        p += written;
        left -= written;
    }
    return size - left;
}


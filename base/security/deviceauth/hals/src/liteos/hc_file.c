/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hc_file.h"
#include <errno.h>
#include <fcntl.h>
#include <securec.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common_defs.h"
#include "hc_error.h"
#include "hc_log.h"

#define MAX_FILE_PATH_SIZE 256
#define MAX_FOLDER_NAME_SIZE 128
#define GET_FOLDER_OK 0
#define GET_FOLDER_FAILED (-1)
#define GET_FILE_OK 1
#define DEFAULT_FILE_PERMISSION 0666

typedef struct {
    FileIdEnum fileId;
    const char* filePath;
} FileDefInfo;

static FileDefInfo g_fileDefInfo[FILE_ID_LAST] = {
    { FILE_ID_GROUP, "user/Hichain/hcgroup.dat" }
};

static char g_groupPath[MAX_FILE_PATH_SIZE] = { 0 };

void SetFilePath(FileIdEnum fileId, const char *path)
{
    if (path == NULL) {
        LOGE("Invalid path param");
        return;
    }
    if (sprintf_s(g_groupPath, MAX_FILE_PATH_SIZE, "%s", path) != -1) {
        g_fileDefInfo[fileId].filePath = g_groupPath;
    }
}

int GetNextFolder(const char *filePath, int *beginPos, char *dst, int size)
{
    int pos = (*beginPos);
    while (1) {
        char c = filePath[pos++];
        if (c == '\0') {
            if (memcpy_s(dst, size, filePath, pos - 1) != EOK) {
                return GET_FOLDER_FAILED;
            }
            dst[pos - 1] = '\0';
            (*beginPos) = pos;
            return GET_FILE_OK;
        } else if (c == '/') {
            if (memcpy_s(dst, size, filePath, pos - 1) != EOK) {
                return GET_FOLDER_FAILED;
            }
            dst[pos - 1] = '\0';
            (*beginPos) = pos;
            return GET_FOLDER_OK;
        } else {
            if (pos >= size) {
                (*beginPos) = pos;
                return GET_FOLDER_FAILED;
            }
        }
    }
}

int HcFileOpenRead(const char *path)
{
    return open(path, O_RDONLY);
}

static int IsFileValid(const char *path)
{
    struct stat fileStat;
    int32_t ret = stat(path, &fileStat);
    if (ret == -1) {
        if (errno == ENOENT) {
            return HAL_ERR_FILE_NOT_EXIST;
        } else {
            LOGE("file stat failed, errno = 0x%x", errno);
            return HAL_ERR_FILE;
        }
    }

    return 0;
}

int HcFileOpenWrite(const char *path)
{
    char filePath[MAX_FOLDER_NAME_SIZE + 1];
    int beginPos = 0;
    while (1) {
        int ret = GetNextFolder(path, &beginPos, filePath, sizeof(filePath));
        if (ret == GET_FOLDER_OK) {
            if (IsFileValid(filePath) == 0) {
                continue;
            }
            if (mkdir(filePath, DEFAULT_FILE_PERMISSION) != 0) {
                return -1;
            }
        } else if (ret == GET_FOLDER_FAILED) {
            return -1;
        } else {
            int fd = open(filePath, O_RDWR | O_CREAT);
            if (fd == -1) {
                LOGE("file stat failed, errno = 0x%x", errno);
            }
            return fd;
        }
    }
}

int HcFileOpen(int fileId, int mode, FileHandle *file)
{
    if (fileId < 0 || fileId >= FILE_ID_LAST || file == NULL) {
        return -1;
    }
    if (mode == MODE_FILE_READ) {
        file->fileHandle.fd = HcFileOpenRead(g_fileDefInfo[fileId].filePath);
    } else {
        file->fileHandle.fd = HcFileOpenWrite(g_fileDefInfo[fileId].filePath);
    }
    if (file->fileHandle.fd == -1) {
        return -1;
    }
    return 0;
}

int HcFileSize(FileHandle file)
{
    int fp = file.fileHandle.fd;
    int size = lseek(fp, 0, SEEK_END);
    (void)lseek(fp, 0, SEEK_SET);
    return size;
}

int HcFileRead(FileHandle file, void *dst, int dstSize)
{
    int fp = file.fileHandle.fd;
    if (fp  == -1 || dstSize < 0 || dst == NULL) {
        return -1;
    }

    char *dstBuffer = (char *)dst;
    int total = 0;
    while (total < dstSize) {
        int readCount = read(fp, dstBuffer + total, dstSize - total);
        if (readCount < 0 || readCount > (dstSize - total)) {
            LOGE("read errno :%x", errno);
            return -1;
        }
        if (readCount == 0) {
            LOGE("read errno :%x", errno);
            return total;
        }
        total += readCount;
    }

    return total;
}

int HcFileWrite(FileHandle file, const void *src, int srcSize)
{
    int fp = file.fileHandle.fd;
    if (fp == -1 || srcSize < 0 || src == NULL) {
        return -1;
    }

    const char *srcBuffer = (const char *)src;
    int total = 0;
    while (total < srcSize) {
        int writeCount = write(fp, srcBuffer + total, srcSize - total);
        if (writeCount < 0 || writeCount > (srcSize - total)) {
            return -1;
        }
        total += writeCount;
    }
    return total;
}

void HcFileClose(FileHandle file)
{
    int fp = file.fileHandle.fd;
    if (fp == 0) {
        return;
    }

    close(fp);
}

void HcFileRemove(int fileId)
{
    if (fileId >= FILE_ID_LAST) {
        LOGE("Invalid fileId:%d", fileId);
        return;
    }
    unlink(g_fileDefInfo[fileId].filePath);
}
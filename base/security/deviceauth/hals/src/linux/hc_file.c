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
#include "securec.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "hc_log.h"
#include "hc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_FILE_PATH_SIZE 256
#define MAX_FOLDER_NAME_SIZE 128

typedef struct {
    FileIdEnum fileId;
    const char *filePath;
} FileDefInfo;

static char g_groupPath[MAX_FILE_PATH_SIZE] = { 0 };

static FileDefInfo g_fileDefInfo[FILE_ID_LAST] = {
    { FILE_ID_GROUP, "/data/data/deviceauth/hcgroup.dat" },
};

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

static int32_t CreateDirectory(const char *filePath)
{
    int32_t ret;
    errno_t eno;
    char *chPtr = NULL;
    char dirCache[MAX_FOLDER_NAME_SIZE];

    chPtr = (char *)filePath;
    while ((chPtr = strchr(chPtr, '/')) != NULL) {
        unsigned long len = (unsigned long)((uintptr_t)chPtr - (uintptr_t)filePath);
        if (len == 0ul) {
            chPtr++;
            continue;
        }
        eno = memcpy_s(dirCache, sizeof(dirCache), filePath, len);
        if (eno != EOK) {
            LOGE("memory copy failed");
            return -1;
        }
        dirCache[len] = 0;
        if (access(dirCache, F_OK) != 0) {
            ret = mkdir(dirCache, S_IRWXU);
            if (ret != 0) {
                LOGE("make dir failed, err code %d", ret);
                return -1;
            }
        }
        chPtr++;
    }
    return 0;
}

static FILE *HcFileOpenRead(int fileId, const char *path)
{
    (void)fileId;
    return fopen(path, "rb");
}

static FILE *HcFileOpenWrite(int32_t fileId, const char *path)
{
    (void)fileId;
    if (access(path, F_OK) != 0) {
        int32_t ret = CreateDirectory(path);
        if (ret != 0) {
            return NULL;
        }
    }
    return fopen(path, "w+");
}

int HcFileOpen(int fileId, int mode, FileHandle *file)
{
    if (fileId < 0 || fileId >= FILE_ID_LAST || file == NULL) {
        return -1;
    }
    if (mode == MODE_FILE_READ) {
        file->pfd = HcFileOpenRead(fileId, g_fileDefInfo[fileId].filePath);
    } else {
        file->pfd = HcFileOpenWrite(fileId, g_fileDefInfo[fileId].filePath);
    }
    if (file->pfd == NULL) {
        return -1;
    } else {
        return 0;
    }
}

int HcFileSize(FileHandle file)
{
    FILE *fp = (FILE *)file.pfd;
    if (fp != NULL) {
        if (fseek(fp, 0L, SEEK_END) != 0) {
            return -1;
        }
        int size = ftell(fp);
        if (fseek(fp, 0L, SEEK_SET) != 0) {
            return -1;
        }
        return size;
    } else {
        return -1;
    }
}

int HcFileRead(FileHandle file, void *dst, int dstSize)
{
    FILE *fp = (FILE *)file.pfd;
    if (fp == NULL || dstSize < 0 || dst == NULL) {
        return -1;
    }

    char *dstBuffer = (char *)dst;
    int total = 0;
    while (total < dstSize) {
        int readCount = fread(dstBuffer + total, 1, dstSize - total, fp);
        if ((readCount < 0) || (readCount > (dstSize - total))) {
            return -1;
        }
        if (readCount == 0) {
            return total;
        }
        total += readCount;
    }

    return total;
}

int HcFileWrite(FileHandle file, const void *src, int srcSize)
{
    FILE *fp = (FILE *)file.pfd;
    if (fp == NULL || srcSize < 0 || src == NULL) {
        return -1;
    }

    const char *srcBuffer = (const char *)src;
    int total = 0;
    while (total < srcSize) {
        int writeCount = fwrite(srcBuffer + total, 1, srcSize - total, fp);
        if (writeCount < 0 || writeCount > (srcSize - total)) {
            return -1;
        }
        total += writeCount;
    }
    return total;
}

void HcFileClose(FileHandle file)
{
    FILE *fp = (FILE *)file.pfd;
    if (fp == NULL) {
        return;
    }

    fclose(fp);
}

void HcFileRemove(int fileId)
{
    if (fileId >= FILE_ID_LAST) {
        LOGE("Invalid fileId:%d", fileId);
        return;
    }
    unlink(g_fileDefInfo[fileId].filePath);
}

#ifdef __cplusplus
}
#endif

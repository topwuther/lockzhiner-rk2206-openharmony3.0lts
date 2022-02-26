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
#include "common_defs.h"
#include "hc_error.h"
#include "hc_log.h"
#include "utils_file.h"

#define MAX_FILE_PATH_SIZE 64
#define MAX_FOLDER_NAME_SIZE 128
#define GET_FOLDER_OK 0
#define GET_FOLDER_FAILED (-1)
#define GET_FILE_OK 1
#define DEFAULT_FILE_PERMISSION 0666

typedef struct {
    FileIdEnum fileId;
    char filePath[MAX_FILE_PATH_SIZE];
} FileDefInfo;

static FileDefInfo g_fileDefInfo[FILE_ID_LAST] = {
    { FILE_ID_GROUP, "/data/hcgroup.dat" }
};

void SetFilePath(FileIdEnum fileId, const char *path)
{
    if (fileId < 0 || fileId >= FILE_ID_LAST || path == NULL) {
        LOGE("Invalid path param");
        return;
    }
    if (sprintf_s(g_fileDefInfo[fileId].filePath, MAX_FILE_PATH_SIZE, "%s", path) == HAL_FAILED) {
        LOGE("Set file path failed fileId:%d", fileId);
    }
}

int HcFileOpenRead(const char *path)
{
    int ret = UtilsFileOpen(path, O_RDONLY, 0);
    LOGI("ret = %d", ret);
    return ret;
}

int HcFileOpenWrite(const char *path)
{
    int ret = UtilsFileOpen(path, O_RDWR_FS | O_CREAT_FS | O_TRUNC_FS, 0);
    LOGI("ret = %d", ret);
    return ret;
}

int HcFileOpen(int fileId, int mode, FileHandle *file)
{
    if (fileId < 0 || fileId >= FILE_ID_LAST || file == NULL) {
        return HAL_FAILED;
    }
    if (mode == MODE_FILE_READ) {
        file->fileHandle.fd = HcFileOpenRead(g_fileDefInfo[fileId].filePath);
        file->filePath = g_fileDefInfo[fileId].filePath;
    } else {
        file->fileHandle.fd = HcFileOpenWrite(g_fileDefInfo[fileId].filePath);
        file->filePath = g_fileDefInfo[fileId].filePath;
    }
    if (file->fileHandle.fd == HAL_FAILED) {
        return HAL_FAILED;
    }
    return HAL_SUCCESS;
}

int HcFileSize(FileHandle file)
{
    int fileSize = 0;
    int ret = UtilsFileStat(file.filePath, (unsigned int *)&fileSize);
    LOGI("ret = %d, fileSize = %d\n", ret, fileSize);
    if (ret == HAL_SUCCESS) {
        return fileSize;
    } else {
        return HAL_FAILED;
    }
    return HAL_FAILED;
}

int HcFileRead(FileHandle file, void *dst, int dstSize)
{
    int fp = file.fileHandle.fd;
    if (fp  == HAL_FAILED || dstSize < 0 || dst == NULL) {
        return HAL_FAILED;
    }
    int ret = UtilsFileRead(fp, (char *)dst, dstSize);
    LOGI("ret = %d", ret);
    return ret;
}

int HcFileWrite(FileHandle file, const void *src, int srcSize)
{
    int fp = file.fileHandle.fd;
    if (fp  == HAL_FAILED || srcSize < 0 || src == NULL) {
        return HAL_FAILED;
    }
    int ret = UtilsFileWrite(fp, src, srcSize);
    LOGI("ret = %d", ret);
    return ret;
}

void HcFileClose(FileHandle file)
{
    int fp = file.fileHandle.fd;
    if (fp < 0) {
        return;
    }
    int ret = UtilsFileClose(fp);
    LOGI("ret = %d", ret);
}

void HcFileRemove(int fileId)
{
    if (fileId >= FILE_ID_LAST) {
        LOGE("Invalid fileId:%d", fileId);
        return;
    }
    int ret = UtilsFileDelete(g_fileDefInfo[fileId].filePath);
    LOGI("File delete result:%d", ret);
}

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

#ifndef INIT_UTILS_H
#define INIT_UTILS_H
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define BINARY_BASE 2
#define OCTAL_BASE 8
#define DECIMAL_BASE 10
#define ARRAY_LENGTH(array) (sizeof((array)) / sizeof((array)[0]))

int DecodeUid(const char *name);
void CheckAndCreateDir(const char *fileName);
char* ReadFileToBuf(const char *configFile);
int SplitString(char *srcPtr, char **dstPtr, int maxNum);
void WaitForFile(const char *source, unsigned int maxCount);
size_t WriteAll(int fd, char *buffer, size_t size);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // INIT_UTILS_H

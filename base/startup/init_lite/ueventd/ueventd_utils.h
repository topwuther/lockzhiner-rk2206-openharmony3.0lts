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

#ifndef BASE_STARTUP_INITLITE_UEVENTD_UTILS_H
#define BASE_STARTUP_INITLITE_UEVENTD_UTILS_H
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define DECIMALISM 10
#define OCTONARY 8
#define DEVICE_FILE_SIZE 128U
#define SYSPATH_SIZE 512U
#define BLOCKDEVICE_LINKS 3

#define STARTSWITH(str, prefix) (strncmp((str), (prefix), strlen(prefix)) == 0)
#define STRINGEQUAL(src, tgt) (strcmp((src), (tgt)) == 0)
#define INVALIDSTRING(str) ((str) == NULL || *(str) == '\0')

#define DIRMODE (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
// Default device mode is 0660
#define DEVMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

int MakeDirRecursive(const char *dir, mode_t mode);
int MakeDir(const char *dir, mode_t mode);
int StringToInt(const char *str, int defaultValue);
#endif // BASE_STARTUP_INITLITE_UEVENTD_UTILS_H
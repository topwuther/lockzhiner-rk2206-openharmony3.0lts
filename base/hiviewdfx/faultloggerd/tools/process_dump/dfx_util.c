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
#include "dfx_util.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dfx_define.h"

BOOL ReadStringFromFile(const char *path, char *buf, size_t len)
{
    if ((len <= 1) || (buf == NULL) || (path == NULL)) {
        return FALSE;
    }

    char realPath[PATH_MAX];
    if (realpath(path, realPath) == NULL) {
        return FALSE;
    }

    FILE *fp = fopen(realPath, "r");
    if (fp == NULL) {
        return FALSE;
    }

    char *ptr = buf;
    for (size_t i = 0; i < len; i++) {
        int c = getc(fp);
        if (c == EOF) {
            *ptr++ = 0;
            break;
        } else {
            *ptr++ = c;
        }
    }
    fclose(fp);
    return TRUE;
}

char *TrimAndDupStr(const char *source)
{
    if (source == NULL) {
        return NULL;
    }

    const char *begin = source;
    const char *end = begin + strlen(source);
    if (begin == end) {
        return NULL;
    }

    while ((begin < end) && isspace(*begin)) {
        begin++;
    }

    while ((begin < end) && isspace(*(end - 1))) {
        end--;
    }

    if (begin == end) {
        return NULL;
    }

    uint32_t maxStrLen = NAME_LEN;
    uint32_t offset = end - begin;
    if (maxStrLen > offset) {
        maxStrLen = offset;
    }
    return strndup(begin, maxStrLen);
}
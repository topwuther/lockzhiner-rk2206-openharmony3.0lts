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
#ifndef DFX_ELF_H
#define DFX_ELF_H

#include <elf.h>
#include <inttypes.h>
#include <link.h>

#include <sys/types.h>

#include "dfx_define.h"

typedef struct LoadInfo {
    uint64_t vaddr;
    uint64_t offset;
    struct LoadInfo *next;
} LoadInfo;

typedef struct {
    char *name;
    char *path;
    int32_t fd;
    size_t loadBias;
    uint64_t size;
    ElfW(Ehdr)header;
    LoadInfo *infos;
} DfxElfImage;

BOOL InitDfxElfImage(const char *path, DfxElfImage **image);
BOOL ParseElfHeader(DfxElfImage *image);
BOOL ParseElfProgramHeader(DfxElfImage *image);
uint64_t FindRealLoadOffset(DfxElfImage *image, uint64_t offset);
void CreateLoadInfo(DfxElfImage *image, uint64_t vaddr, uint64_t offset);
void DestroyDfxElfImage(DfxElfImage *image);

#endif
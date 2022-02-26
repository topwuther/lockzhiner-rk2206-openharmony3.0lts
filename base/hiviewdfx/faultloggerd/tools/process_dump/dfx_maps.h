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
#ifndef DFX_MAPS_H
#define DFX_MAPS_H

#include <inttypes.h>

#include "dfx_elf.h"

typedef struct {
    uint64_t begin;
    uint64_t end;
    uint64_t offset;
    char perms[5]; // 5:rwxp
    char *path;
    DfxElfImage *image;
} DfxElfMap;

typedef struct DfxElfMapsNode {
    DfxElfMap *map;
    struct DfxElfMapsNode *next;
} DfxElfMapsNode;

BOOL InitElfMaps(DfxElfMapsNode **head, pid_t pid);
BOOL InitElfMap(DfxElfMap **map, const char *mapInfo);
void InsertMapToElfMaps(DfxElfMapsNode *head, DfxElfMap *map);
DfxElfImage *GetMapElf(DfxElfMap *map, DfxElfMapsNode *head);
BOOL IsElfMap(DfxElfMap *map);
void DestroyMap(DfxElfMap *map);
void DestroyMaps(DfxElfMapsNode *head);
BOOL FindMapByPath(DfxElfMapsNode *head, const char *path, DfxElfMap **map);
BOOL FindMapByAddr(DfxElfMapsNode *head, uintptr_t address, DfxElfMap **map);
void PrintMap(DfxElfMap *map, int32_t fd);

#endif
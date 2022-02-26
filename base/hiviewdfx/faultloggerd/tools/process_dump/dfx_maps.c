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
#include "dfx_maps.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <securec.h>

#include "dfx_define.h"
#include "dfx_elf.h"
#include "dfx_log.h"
#include "dfx_util.h"

#define MAPINFO_SIZE 256

BOOL InitElfMaps(DfxElfMapsNode **head, pid_t pid)
{
    *head = (DfxElfMapsNode *)calloc(1, sizeof(DfxElfMapsNode));
    if ((*head) == NULL) {
        DfxLogWarn("Fail to create ElfMaps.");
        return FALSE;
    }

    char path[NAME_LEN] = {0};
    if (snprintf_s(path, sizeof(path), sizeof(path) - 1, "/proc/%d/maps", pid) <= 0) {
        DfxLogWarn("Fail to print path.");
        return FALSE;
    }

    char realPath[PATH_MAX] = {0};
    if (realpath(path, realPath) == NULL) {
        DfxLogWarn("Maps path(%s) is not exist.", path);
        return FALSE;
    }

    FILE *fp = fopen(realPath, "r");
    if (fp == NULL) {
        DfxLogWarn("Fail to open maps info.");
        free(*head);
        (*head) = NULL;
        return FALSE;
    }

    DfxElfMap *map = NULL;
    char mapInfo[MAPINFO_SIZE] = {0};
    while (fgets(mapInfo, sizeof(mapInfo), fp)) {
        if (!InitElfMap(&map, mapInfo)) {
            DfxLogWarn("Fail to init map info:%s.", mapInfo);
            continue;
        }

        if (map != NULL) {
            InsertMapToElfMaps(*head, map);
        }
    }
    fclose(fp);
    return TRUE;
}

BOOL InitElfMap(DfxElfMap **map, const char *mapInfo)
{
    *map = (DfxElfMap *)calloc(1, sizeof(DfxElfMap));
    if ((*map) == NULL) {
        DfxLogWarn("Fail to create ElfMap.");
        return FALSE;
    }

    int pos = 0;
    // 7658d38000-7658d40000 rw-p 00000000 00:00 0                              [anon:thread signal stack]
    if (sscanf_s(mapInfo, "%" SCNxPTR "-%" SCNxPTR " %4s %" SCNxPTR " %*x:%*x %*d%n", &((*map)->begin), &((*map)->end),
        (*map)->perms, sizeof((*map)->perms), &((*map)->offset),
        &pos) != 4) { // 4:scan size
        DfxLogWarn("Fail to parse maps info.");
        free(*map);
        (*map) = NULL;
        return FALSE;
    }

    (*map)->path = TrimAndDupStr(mapInfo + pos);
    return TRUE;
}

void InsertMapToElfMaps(DfxElfMapsNode *head, DfxElfMap *map)
{
    DfxElfMapsNode *curNode = head;
    while (curNode->next != NULL) {
        curNode = curNode->next;
    }

    if (curNode->map == NULL) {
        curNode->map = map;
    } else {
        curNode->next = (DfxElfMapsNode *)calloc(1, sizeof(DfxElfMapsNode));
        if (curNode->next != NULL) {
            curNode->next->map = map;
        }
    }
}

void DestroyMap(DfxElfMap *map)
{
    if (map == NULL) {
        return;
    }

    if (map->path != NULL) {
        free(map->path);
        map->path = NULL;
    }

    if (map->image != NULL) {
        DestroyDfxElfImage(map->image);
        map->image = NULL;
    }
}

void DestroyMaps(DfxElfMapsNode *head)
{
    DfxElfMapsNode *cur = head;
    while (cur != NULL && cur->next != NULL) {
        DfxElfMapsNode *tmp = cur;
        cur = cur->next;
        DestroyMap(tmp->map);
        free(tmp);
    }

    DestroyMap(cur->map);
    free(cur);
}

BOOL FindMapByPath(DfxElfMapsNode *head, const char *path, DfxElfMap **map)
{
    if (map == NULL) {
        return FALSE;
    }

    DfxElfMapsNode *curNode = head;
    while (curNode != NULL) {
        if (curNode->map == NULL || curNode->map->path == NULL) {
            curNode = curNode->next;
            continue;
        }

        if (strcmp(path, curNode->map->path) == 0) {
            (*map) = curNode->map;
            return TRUE;
        }
        curNode = curNode->next;
    }
    return FALSE;
}

BOOL FindMapByAddr(DfxElfMapsNode *head, uintptr_t address, DfxElfMap **map)
{
    if (map == NULL) {
        return FALSE;
    }

    DfxElfMapsNode *curNode = head;
    while (curNode != NULL) {
        if (curNode->map == NULL) {
            curNode = curNode->next;
            continue;
        }

        if ((curNode->map->begin < address) && (curNode->map->end > address)) {
            (*map) = curNode->map;
            return TRUE;
        }
        curNode = curNode->next;
    }
    return FALSE;
}

BOOL IsElfMap(DfxElfMap *map)
{
    if (map == NULL || map->path == NULL || strlen(map->path) == 0) {
        return FALSE;
    }

    if (strncmp(map->path, "/dev/", 5) == 0) { // 5:length of "/dev/"
        return FALSE;
    }

    if (strncmp(map->path, "[anon:", 6) == 0) { // 6:length of "[anon:"
        return FALSE;
    }

    if (strncmp(map->path, "/system/framework/", 18) == 0) { // 18:length of "/system/framework/"
        return FALSE;
    }
    return TRUE;
}

void PrintMap(DfxElfMap *map, int32_t fd)
{
    if (map == NULL) {
        return;
    }

    dprintf(fd, "%" PRIx64 "-%" PRIx64 " %s %08" PRIx64 " %s\n",
        map->begin, map->end, map->perms, map->offset, map->path);
}
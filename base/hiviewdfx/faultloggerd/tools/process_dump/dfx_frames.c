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
#include "dfx_frames.h"

#include <stdio.h>
#include <stdlib.h>

#include "dfx_elf.h"
#include "dfx_log.h"
#include "dfx_maps.h"

DfxFrame *CreateNewFrame(DfxFramesNode **node)
{
    if ((*node) == NULL) {
        *node = (DfxFramesNode *)calloc(1, sizeof(DfxFramesNode));
    }

    if ((*node) == NULL) {
        return NULL;
    }

    (*node)->frame = (DfxFrame *)calloc(1, sizeof(DfxFrame));
    return (*node)->frame;
}

void DestroyFrames(DfxFramesNode *head) {}

uint64_t GetRelativePc(DfxFrame *frame, DfxElfMapsNode *head)
{
    if (frame == NULL || head == NULL) {
        return 0;
    }

    if (frame->map == NULL) {
        if (!FindMapByAddr(head, frame->pc, &(frame->map))) {
            return 0;
        }
    }

    if (!IsElfMap(frame->map)) {
        DfxLogWarn("No elf map:%s.", frame->map->path);
        return 0;
    }

    DfxElfMap *map = NULL;
    if (!FindMapByPath(head, frame->map->path, &map)) {
        DfxLogWarn("Fail to find Map:%s.", frame->map->path);
        return 0;
    }

    return CalculateRelativePc(frame, map);
}

uint64_t CalculateRelativePc(DfxFrame *frame, DfxElfMap *elfMap)
{
    if (frame == NULL || elfMap == NULL || frame->map == NULL) {
        return 0;
    }

    if (elfMap->image == NULL) {
        InitDfxElfImage(elfMap->path, &(elfMap->image));
    }

    if (elfMap->image == NULL) {
        frame->relativePc = frame->pc - (frame->map->begin - frame->map->offset);
    } else {
        frame->relativePc = (frame->pc - frame->map->begin) + FindRealLoadOffset(elfMap->image, frame->map->offset);
    }

    return frame->relativePc;
}

void PrintFrame(DfxFrame *frame, int32_t fd)
{
    if (frame == NULL) {
        return;
    }

    if (frame->pc == 0) {
        return;
    }

    if (frame->funcName == NULL) {
        WriteLog(fd, "#%02zu pc %016" PRIx64 "(%016" PRIx64 ") %s\n", frame->index, frame->relativePc,
            frame->pc, (frame->map == NULL) ? "Unknown" : frame->map->path);
        return;
    }

    WriteLog(fd, "#%02zu pc %016" PRIx64 "(%016" PRIx64 ") %s(%s+%" PRIu64 ")\n", frame->index, frame->relativePc,
        frame->pc, (frame->map == NULL) ? "Unknown" : frame->map->path, frame->funcName, frame->funcOffset);
}

void PrintFrames(DfxFramesNode *head, int32_t fd)
{
    DfxFramesNode *curNode = head;
    while (curNode != NULL) {
        PrintFrame(curNode->frame, fd);
        curNode = curNode->next;
    }
}
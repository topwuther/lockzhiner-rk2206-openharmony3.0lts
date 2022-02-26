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
#ifndef DFX_FRAME_H
#define DFX_FRAME_H

#include "dfx_elf.h"
#include "dfx_maps.h"
#include "dfx_regs.h"

typedef struct {
    size_t index;
    uint64_t funcOffset;
    uint64_t pc;
    uint64_t sp;
    uint64_t relativePc;
    char *funcName;
    DfxElfMap *map; // managed in DfxProcess struct
} DfxFrame;

typedef struct DfxFramesNode {
    DfxFrame *frame;
    struct DfxFramesNode *next;
} DfxFramesNode;

DfxFrame *CreateNewFrame(DfxFramesNode **node);
uint64_t GetRelativePc(DfxFrame *frame, DfxElfMapsNode *head);
uint64_t CalculateRelativePc(DfxFrame *frame, DfxElfMap *elfMap);
void DestroyFrames(DfxFramesNode *head);
void PrintFrames(DfxFramesNode *head, int32_t fd);

#endif
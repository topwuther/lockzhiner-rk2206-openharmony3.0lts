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

#include "dfx_unwind_remote.h"

#include <elf.h>
#include <link.h>
#include <stdio.h>
#include <string.h>

#include <sys/ptrace.h>

#include <libunwind-ptrace.h>
#include <libunwind.h>

#include "dfx_log.h"
#include "dfx_maps.h"
#include "dfx_process.h"
#include "dfx_regs.h"
#include "dfx_thread.h"
#include "dfx_util.h"

#define SYMBOL_BUF_SIZE 4096
BOOL UnwindProcess(DfxProcess *process)
{
    if (process == NULL) {
        return FALSE;
    }

    if (process->threads == NULL) {
        return FALSE;
    }

    DfxThreadsNode *node = process->threads;
    while (node != NULL) {
        if (!UnwindThread(process, node->thread)) {
            DfxLogWarn("Fail to unwind thread.");
        }
        node = node->next;
    }
    return TRUE;
}

BOOL UnwindThread(DfxProcess *process, DfxThread *thread)
{
    if (thread == NULL) {
        DfxLogWarn("NULL thread needs unwind.");
        return FALSE;
    }

    unw_addr_space_t as = unw_create_addr_space(&_UPT_accessors, 0);
    if (as == NULL) {
        return FALSE;
    }

    pid_t tid = thread->tid;
    void *context = _UPT_create(tid);
    if (context == NULL) {
        unw_destroy_addr_space(as);
        return FALSE;
    }

    unw_cursor_t cursor;
    if (unw_init_remote(&cursor, as, context) != 0) {
        DfxLogWarn("Fail to init cursor for remote unwind.");
        _UPT_destroy(context);
        unw_destroy_addr_space(as);
        return FALSE;
    }

    size_t index = 0;
    do {
        DfxFrame *frame = GetAvaliableFrame(thread);
        if (frame == NULL) {
            DfxLogWarn("Fail to create Frame.");
            break;
        }

        frame->index = index;
        char sym[SYMBOL_BUF_SIZE] = {0};
        if (unw_get_reg(&cursor, UNW_REG_IP, (unw_word_t*)(&(frame->pc)))) {
            DfxLogWarn("Fail to get program counter.");
            break;
        }

        if (unw_get_reg(&cursor, UNW_REG_SP, (unw_word_t*)(&(frame->sp)))) {
            DfxLogWarn("Fail to get stack pointer.");
            break;
        }

        if (FindMapByAddr(process->maps, frame->pc, &(frame->map))) {
            frame->relativePc = GetRelativePc(frame, process->maps);
        }

        if (unw_get_proc_name(&cursor, sym, sizeof(sym), (unw_word_t*)(&(frame->funcOffset))) == 0) {
            frame->funcName = TrimAndDupStr(sym);
        }
        index++;
    } while (unw_step(&cursor) > 0);
    _UPT_destroy(context);
    unw_destroy_addr_space(as);
    return TRUE;
}

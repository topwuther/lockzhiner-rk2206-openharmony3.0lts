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
#ifndef DFX_UNWIND_H
#define DFX_UNWIND_H

#include "dfx_define.h"
#include "dfx_process.h"
#include "dfx_thread.h"

BOOL UnwindProcess(DfxProcess *process);
BOOL UnwindThread(DfxProcess *process, DfxThread *thread);

#endif
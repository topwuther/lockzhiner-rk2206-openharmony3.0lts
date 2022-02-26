/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**@defgroup los_config System configuration items
 * @ingroup kernel
 */

#ifndef _TARGET_CONFIG_H
#define _TARGET_CONFIG_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#include "link.h"
#include "soc.h"

/*=============================================================================
                                        System clock module configuration
=============================================================================*/
#define OS_SYS_CLOCK                                        96000000UL
#define LOSCFG_BASE_CORE_TICK_PER_SECOND                    (1000UL)
#define LOSCFG_BASE_CORE_TICK_HW_TIME                       0
#define LOSCFG_BASE_CORE_TICK_WTIMER                        0
#define LOSCFG_BASE_CORE_TICK_RESPONSE_MAX                  0xFFFFFFUL

/*=============================================================================
                                        Hardware interrupt module configuration
=============================================================================*/
#define LOSCFG_PLATFORM_HWI                                 1
#define LOSCFG_USE_SYSTEM_DEFINED_INTERRUPT                 1
#define LOSCFG_PLATFORM_HWI_LIMIT                           128
/*=============================================================================
                                       Task module configuration
=============================================================================*/
#define LOSCFG_BASE_CORE_TSK_LIMIT                          63
#define LOSCFG_BASE_CORE_TSK_IDLE_STACK_SIZE                (0x1000U)
#define LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE             (0x1000U)
#define LOSCFG_BASE_CORE_TSK_MIN_STACK_SIZE                 (0x200U)
#define LOSCFG_BASE_CORE_TIMESLICE                          1
#define LOSCFG_BASE_CORE_TIMESLICE_TIMEOUT                  20000
/*=============================================================================
                                       Semaphore module configuration
=============================================================================*/
#define LOSCFG_BASE_IPC_SEM                                 1
#define LOSCFG_BASE_IPC_SEM_LIMIT                           48
/*=============================================================================
                                       Mutex module configuration
=============================================================================*/
#define LOSCFG_BASE_IPC_MUX                                 1
#define LOSCFG_BASE_IPC_MUX_LIMIT                           48
/*=============================================================================
                                       Queue module configuration
=============================================================================*/
#define LOSCFG_BASE_IPC_QUEUE                               1
#define LOSCFG_BASE_IPC_QUEUE_LIMIT                         48
/*=============================================================================
                                       Software timer module configuration
=============================================================================*/
#define LOSCFG_BASE_CORE_SWTMR                              1
#define LOSCFG_BASE_CORE_SWTMR_ALIGN                        0
#define LOSCFG_BASE_CORE_SWTMR_LIMIT                        48
/*=============================================================================
                                       Memory module configuration
=============================================================================*/
#define LOSCFG_MEM_MUL_POOL                                 1
#define OS_SYS_MEM_NUM                                      20
/*=============================================================================
                                       Exception module configuration
=============================================================================*/
#define LOSCFG_PLATFORM_EXC                                 1
/* =============================================================================
                                       printf module configuration
============================================================================= */
#define LOSCFG_KERNEL_PRINTF                                1

#define LOSCFG_BASE_CORE_SCHED_SLEEP                        1

#define LOSCFG_BACKTRACE_TYPE                               1

#define LOSCFG_BASE_CORE_TSK_MONITOR                        1
#define LOSCFG_BASE_CORE_CPUP                               1
#define LOSCFG_BASE_CORE_EXC_TSK_SWITCH                     1

#define LOSCFG_SYS_EXTERNAL_HEAP                            1
//#define LOSCFG_SYS_HEAP_SIZE                                102400
#define LOSCFG_MEM_FREE_BY_TASKID                           1
#define LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK                1
#define LOSCFG_MEM_LEAKCHECK                                1

//#define LOSCFG_BASE_CORE_HILOG                            1
//#define LOSCFG_DEBUG_HOOK                                 1

#define LOSCFG_HAL_TRACE_PRINTF                             1
#define HAL_TRACE_DEBUG                                     8

/*=============================================================================
                                       Memory module configuration
=============================================================================*/
extern unsigned int _heap_start;
#define LOSCFG_SYS_HEAP_ADDR                                &_heap_start
#define LOSCFG_SYS_HEAP_SIZE                                (PSRAM_SIZE - SYS_STACK_SIZE)


/*=============================================================================
                                       Hardware interrupt configuration
=============================================================================*/
#define OS_HWI_WITH_ARG                                     1

/*=============================================================================
                                       Section configuration
=============================================================================*/
// #define LITE_OS_SEC_VEC __attribute__ ((section(".data")))

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _TARGET_CONFIG_H */

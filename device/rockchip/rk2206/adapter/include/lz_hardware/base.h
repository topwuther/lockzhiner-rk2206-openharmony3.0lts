/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
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

/**
 * @addtogroup Lockzhiner
 *
 * @file base.h
 *
 */
#ifndef LZ_HARDWARE_BASE_H
#define LZ_HARDWARE_BASE_H

#define ID2BANK(id, num)                ((id) / (num))
#define ID2INDEX(id, num)               ((id) % (num))

#define CNTLR_INIT(NAME, id, CNTLR_S)                                               \
CNTLR_S *cntlr;                                                                     \
unsigned int nr_cntlr = sizeof(g_cntlrs) / sizeof(g_cntlrs[0]);                     \
unsigned int nr_id = (NR_ ## NAME ## _CHN) * nr_cntlr;                              \
do {                                                                                \
    if ( (id) >= nr_id ) {                                                          \
        LZ_HARDWARE_LOGE(NAME ## _TAG, "%s: id %d is invalid", __func__, (id));             \
        return LZ_HARDWARE_FAILURE;                                                         \
    }                                                                               \
    cntlr = &g_cntlrs[id];                                                          \
    if (cntlr->init) {                                                              \
        LZ_HARDWARE_LOGE(NAME ## _TAG, "%s: id %d, controller has already been initialized", __func__, (id));\
        return LZ_HARDWARE_FAILURE;                                                         \
    }                                                                               \
 } while(0)

#define CNTLR_READY(NAME, id, CNTLR_S)                                              \
CNTLR_S *cntlr;                                                                     \
unsigned int nr_cntlr = sizeof(g_cntlrs) / sizeof(g_cntlrs[0]);                     \
unsigned int nr_id = (NR_ ## NAME ## _CHN) * nr_cntlr;                              \
do {                                                                                \
    if ( (id) >= nr_id ) {                                                          \
        LZ_HARDWARE_LOGE(NAME ## _TAG, "%s: id %d is invalid", __func__, (id));             \
        return LZ_HARDWARE_FAILURE;                                                         \
    }                                                                               \
    cntlr = &g_cntlrs[id];                                                           \
    if (!cntlr->init) {                                                             \
        LZ_HARDWARE_LOGE(NAME ## _TAG, "%s: id %d, controller has NOT been initialized", __func__, (id));\
        return LZ_HARDWARE_FAILURE;                                                         \
    }                                                                               \
 } while(0)

#define CNTLR_INIT_M(NAME, id, finit, CNTLR_S, CHN_S)                               \
CNTLR_S *cntlr;                                                                     \
CHN_S *chn;                                                                         \
unsigned int nr_cntlr = sizeof(g_cntlrs) / sizeof(g_cntlrs[0]);                     \
unsigned int nr_id = (NR_ ## NAME ## _CHN) * nr_cntlr;                              \
unsigned int bank = ID2BANK(id, (NR_ ## NAME ## _CHN));                             \
unsigned int index = ID2INDEX(id, (NR_ ## NAME ## _CHN));                           \
do {                                                                                \
    if ( (id) >= nr_id ) {                                                          \
        LZ_HARDWARE_LOGE(NAME ## _TAG, "%s: id %d is invalid", __func__, (id));             \
        return LZ_HARDWARE_FAILURE;                                                         \
    }                                                                               \
    cntlr = &g_cntlrs[bank];                                                        \
    if (!cntlr->init) {                                                             \
        if(finit(cntlr) != LZ_HARDWARE_SUCCESS)                                             \
            return LZ_HARDWARE_FAILURE;                                                     \
        cntlr->init = 1;                                                            \
    }                                                                               \
    chn = &cntlr->chns[index];                                                      \
    if (chn->init) {                                                                \
        LZ_HARDWARE_LOGE(NAME ## _TAG, "%s: id %d, channel has already been initialized", __func__, (id));\
        return LZ_HARDWARE_FAILURE;                                                         \
    }                                                                               \
 } while(0)

#define CNTLR_READY_M(NAME, id, CNTLR_S, CHN_S)                                     \
CNTLR_S *cntlr;                                                                     \
CHN_S *chn;                                                                         \
unsigned int nr_cntlr = sizeof(g_cntlrs) / sizeof(g_cntlrs[0]);                     \
unsigned int nr_id = (NR_ ## NAME ## _CHN) * nr_cntlr;                              \
unsigned int bank = ID2BANK(id, (NR_ ## NAME ## _CHN));                             \
unsigned int index = ID2INDEX(id, (NR_ ## NAME ## _CHN));                           \
do {                                                                                \
    if ( (id) >= nr_id ) {                                                          \
        LZ_HARDWARE_LOGE(NAME ## _TAG, "%s: id %d is invalid", __func__, (id));             \
        return LZ_HARDWARE_FAILURE;                                                         \
    }                                                                               \
    cntlr = &g_cntlrs[bank];                                                         \
    if (!cntlr->init) {                                                             \
        LZ_HARDWARE_LOGE(NAME ## _TAG, "%s: id %d, controller has NOT been initialized", __func__, (id));\
        return LZ_HARDWARE_FAILURE;                                                         \
    }                                                                               \
    chn = &cntlr->chns[index];                                                      \
    if (!chn->init) {                                                               \
        LZ_HARDWARE_LOGE(NAME ## _TAG, "%s: id %d, channel has NOT been initialized", __func__, (id));\
        return LZ_HARDWARE_FAILURE;                                                         \
    }                                                                               \
 } while(0)

#endif
/** @} */

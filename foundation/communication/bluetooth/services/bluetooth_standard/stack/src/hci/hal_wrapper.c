/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "hal_wrapper.h"

#include <dlfcn.h>

#include "log.h"
#include "platform/include/allocator.h"

#ifdef __x86_64__
#define HAL_LIB "libbluetooth_hal.so"
#else
#define HAL_LIB "libbluetooth_hal.z.so"
#endif

HALLib *LoadHalLib()
{
    HALLib *lib = MEM_CALLOC.alloc(sizeof(HALLib));
    if (lib != NULL) {
        do {
            lib->lib = dlopen(HAL_LIB, RTLD_LAZY | RTLD_NODELETE);
            if (lib->lib == NULL) {
                LOG_ERROR("Load libbluetooth_hal.so failed");
                break;
            }

            lib->halInit = dlsym(lib->lib, "HalInit");
            if (lib->halInit == NULL) {
                LOG_ERROR("Load symbol HalInit failed");
            }

            lib->halSendHciPacket = dlsym(lib->lib, "HalSendHciPacket");
            if (lib->halSendHciPacket == NULL) {
                LOG_ERROR("Load symbol fHalSendData failed");
            }

            lib->halClose = dlsym(lib->lib, "HalClose");
            if (lib->halClose == NULL) {
                LOG_ERROR("Load symbol fHalClose failed");
            }
        } while (0);

        if ((lib->lib == NULL) ||
            (lib->halInit == NULL) ||
            (lib->halSendHciPacket == NULL) ||
            (lib->halClose == NULL)) {
            MEM_CALLOC.free(lib);
            lib = NULL;
        }
    }
    return lib;
}

void UnloadHALLib(HALLib *lib)
{
    if (lib != NULL) {
        lib->halInit = NULL;
        lib->halSendHciPacket = NULL;
        lib->halClose = NULL;
        if (lib->lib != NULL) {
            dlclose(lib->lib);
            lib->lib = NULL;
        }
        MEM_CALLOC.free(lib);
    }
}
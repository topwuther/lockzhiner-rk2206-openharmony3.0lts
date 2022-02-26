/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef HOS_CAMERA_IVENC_OBJECT_H
#define HOS_CAMERA_IVENC_OBJECT_H

#include <string>
#include "mpi_adapter.h"

namespace OHOS::Camera {
class IVencObject {
public:
    static std::shared_ptr<IVencObject> CreateVencObject();
    virtual ~IVencObject() {};
    virtual void ConfigVenc(uint32_t width, uint32_t height) = 0;
    virtual void StartVenc() = 0;
    virtual void StopVenc() = 0;
    virtual void StartEncoder(uint32_t mode, uint32_t w, uint32_t h) = 0;
    virtual void EncoderProc(const void *buffer, std::string path) = 0;
    virtual void StopEncoder() = 0;
    virtual void dump() = 0;
};
}

#endif // OHOS::Camera



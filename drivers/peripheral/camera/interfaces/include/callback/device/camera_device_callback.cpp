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

#include "camera_device_callback.h"
#include <iostream>

namespace OHOS::Camera {
void CameraDeviceCallback::OnError(ErrorType type, int32_t errorMsg)
{
    std::cout << "~###############CameraDeviceCallback OnError" << std::endl;
}

void CameraDeviceCallback::OnResult(uint64_t timestamp, const std::shared_ptr<CameraStandard::CameraMetadata> &result)
{
    std::cout << "~###############CameraDeviceCallback OnResult" << std::endl;
}
}
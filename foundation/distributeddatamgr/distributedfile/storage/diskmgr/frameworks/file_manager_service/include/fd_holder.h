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
#ifndef FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_FD_HOLDER_H
#define FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_FD_HOLDER_H
#pragma once

namespace OHOS {
namespace FileManager {
class FDHolder {
public:
    FDHolder() = default;
    explicit FDHolder(int fd);
    ~FDHolder();

    int GetFD() const;
    void SetFD(int fd);
    void ClearFD();

private:
    int fd_ = -1;
};
} // namespace FileManager
} // namespace OHOS
#endif
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

#ifndef HEADER_CONVERTER_H
#define HEADER_CONVERTER_H

#include "frame_header.h"

namespace DistributedDB {
class HeaderConverter {
public:
    static void ConvertHostToNet(const CommPhyHeader &headerOriginal, CommPhyHeader &headerConverted);
    static void ConvertHostToNet(const CommPhyOptHeader &headerOriginal, CommPhyOptHeader &headerConverted);
    static void ConvertHostToNet(const CommDivergeHeader &headerOriginal, CommDivergeHeader &headerConverted);
    static void ConvertHostToNet(const MessageHeader &headerOriginal, MessageHeader &headerConverted);

    static void ConvertNetToHost(const CommPhyHeader &headerOriginal, CommPhyHeader &headerConverted);
    static void ConvertNetToHost(const CommPhyOptHeader &headerOriginal, CommPhyOptHeader &headerConverted);
    static void ConvertNetToHost(const CommDivergeHeader &headerOriginal, CommDivergeHeader &headerConverted);
    static void ConvertNetToHost(const MessageHeader &headerOriginal, MessageHeader &headerConverted);
};
} // namespace DistributedDB

#endif // HEADER_CONVERTER_H
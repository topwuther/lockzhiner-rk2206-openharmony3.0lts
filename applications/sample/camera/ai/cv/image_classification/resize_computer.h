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

#ifndef RESIZE_COMPUTER_H
#define RESIZE_COMPUTER_H

#include "picture_utils.h"

namespace IC {
class ResizeComputer {
public:
    explicit ResizeComputer(const PicInfo &pInfo);
    ~ResizeComputer() = default;
    int Compute(uint8_t *pDest, uint8_t *src, int pDestSize, int srcSize);

private:
    int sw_;
    int sh_;
    int dw_;
    int dh_;
    int B_;
    int N_;
    int x_;
    int y_;
    uint8_t *pLinePrev_;
    uint8_t *pLineNext_;
    uint8_t *pA_;
    uint8_t *pB_;
    uint8_t *pC_;
    uint8_t *pD_;
};
}  // namespace IC
#endif  // RESIZE_COMPUTER_H
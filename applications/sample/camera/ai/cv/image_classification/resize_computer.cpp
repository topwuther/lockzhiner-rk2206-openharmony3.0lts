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

#include "ic_retcode.h"
#include "resize_computer.h"

namespace IC {
const int RESIZE_CONSTANT = 2;

ResizeComputer::ResizeComputer(const PicInfo &pInfo)
{
    // Initialize parameter for resize computer
    sw_ = pInfo.widthSrc - 1;
    sh_ = pInfo.heightSrc - 1;
    dw_ = pInfo.widthDest - 1;
    dh_ = pInfo.heightDest - 1;
    B_ = 0;
    N_ = 0;
    x_ = 0;
    y_ = 0;
    pLinePrev_ = nullptr;
    pLineNext_ = nullptr;
    pA_ = nullptr;
    pB_ = nullptr;
    pC_ = nullptr;
    pD_ = nullptr;
}

int ResizeComputer::Compute(uint8_t *pDest, uint8_t *src, int pDestSize, int srcSize)
{
    if (pDest == nullptr || src == nullptr || pDestSize <= 0 || srcSize <= 0) {
        printf("ResizeComputer::Compute input is nullptr.\n");
        return IC_RETCODE_FAILURE;
    }
    uint8_t *tmp = nullptr;

    // This is linear stretch for picture resize
    for (int i = 0; i <= dh_; ++i) {
        tmp = pDest + i * (dw_ + 1) * NUM_CHANNELS;
        y_ = i * sh_ / dh_;
        N_ = dh_ - i * sh_ % dh_;
        pLinePrev_ = src + y_ * (sw_ + 1) * NUM_CHANNELS;
        y_++;
        pLineNext_ = (N_ == dh_) ? pLinePrev_ : (src + y_ * (sw_ + 1) * NUM_CHANNELS);
        for (int j = 0; j <= dw_; ++j) {
            x_ = j * sw_ / dw_ * NUM_CHANNELS;
            B_ = dw_ - j * sw_ % dw_;
            pA_ = pLinePrev_ + x_;
            pB_ = pA_ + NUM_CHANNELS;
            pC_ = pLineNext_ + x_;
            pD_ = pC_ + NUM_CHANNELS;
            if (B_ == dw_) {
                pB_ = pA_;
                pD_ = pC_;
            }
            for (int k = 0; k < NUM_CHANNELS; ++k, ++tmp, ++pA_, ++pB_, ++pC_, ++pD_) {
                *tmp = static_cast<uint8_t>(
                    (B_ * N_ * (*pA_ - *pB_ - *pC_ + *pD_) + dw_ * N_ * (*pB_) +
                    dh_ * B_ * (*pC_) + (dw_ * dh_ - dh_ * B_ - dw_ * N_) * (*pD_) +
                    dw_ * dh_ / RESIZE_CONSTANT) / (dw_ * dh_));
            }
        }
    }
    return IC_RETCODE_SUCCESS;
}
}  // namespace IC
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

#ifndef OHOS_BLOCK_INTEGER_H
#define OHOS_BLOCK_INTEGER_H

class BlockInteger {
public:
    explicit BlockInteger(int interval)
        : interval_(interval)
    {
    };
    BlockInteger(const BlockInteger &integer)
        : interval_(integer.interval_), value_(integer.value_)
    {
    };
    BlockInteger &operator=(const BlockInteger &integer) = default;

    ~BlockInteger() = default;

    operator int () const
    {
        return value_;
    }
    bool operator < (int other) const
    {
        return value_ < other;
    }

    BlockInteger &operator=(int value);

    BlockInteger &operator++();

    BlockInteger operator++(int);
private:
    int interval_ = 0;
    int value_ = 0;
};


#endif // OHOS_BLOCK_INTEGER_H

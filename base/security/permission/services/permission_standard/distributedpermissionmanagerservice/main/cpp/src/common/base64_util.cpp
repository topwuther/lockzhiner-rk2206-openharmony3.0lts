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

#include <memory>
#include "base64_util.h"

namespace OHOS {
namespace Security {
namespace Permission {
// namespace {
// static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "Base64Util"};
// }
static const char *encodeTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

static const unsigned char decodeTable[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    62,  // '+'
    0,
    0,
    0,
    63,  // '/'
    52,
    53,
    54,
    55,
    56,
    57,
    58,
    59,
    60,
    61,  // '0'-'9'
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,  // 'A'-'Z'
    0,
    0,
    0,
    0,
    0,
    0,
    26,
    27,
    28,
    29,
    30,
    31,
    32,
    33,
    34,
    35,
    36,
    37,
    38,
    39,
    40,
    41,
    42,
    43,
    44,
    45,
    46,
    47,
    48,
    49,
    50,
    51,  // 'a'-'z'
};

void Base64Util::Encode(const unsigned char *input, unsigned long inLen, std::string &output)
{
    unsigned long i;
    std::unique_ptr<char[]> result = std::make_unique<char[]>((inLen + 1) * 4 / 3 + 1);
    char *dstData = result.get();
    int j = 0;
    for (i = 0; i < (inLen > 2 ? inLen : 0); i += 3) {
        dstData[j++] += encodeTable[input[0] >> 2];
        dstData[j++] += encodeTable[((input[0] & 3) << 4) + (input[1] >> 4)];
        dstData[j++] += encodeTable[((input[1] & 0xf) << 2) + (input[2] >> 6)];
        dstData[j++] += encodeTable[input[2] & 0x3f];
        input += 3;
    }

    if (i < inLen) {
        unsigned char byte1 = input[0];
        unsigned char byte2 = ((i + 1) < inLen) ? input[1] : 0;
        unsigned char byte3 = 0;

        dstData[j++] += encodeTable[byte1 >> 2];
        dstData[j++] += encodeTable[((byte1 & 3) << 4) + (byte2 >> 4)];
        dstData[j++] += ((i + 1) < inLen) ? encodeTable[((byte2 & 0xf) << 2) + (byte3 >> 6)] : '=';
        dstData[j++] += '=';
    }
    output = result.get();
}

void Base64Util::Decode(const std::string &input, unsigned char *output, unsigned long &outLen)
{
    unsigned char *Data = (unsigned char *)input.c_str();
    int DataByte = input.length();

    int nValue;
    int i = 0;
    unsigned long j = 0;
    while (i < DataByte && j < outLen) {
        if (*Data != '\r' && *Data != '\n') {
            nValue = decodeTable[*Data++] << 18;
            nValue += decodeTable[*Data++] << 12;
            output[j++] = (nValue & 0x00FF0000) >> 16;
            if (*Data != '=') {
                nValue += decodeTable[*Data++] << 6;
                output[j++] = (nValue & 0x0000FF00) >> 8;
                if (*Data != '=') {
                    nValue += decodeTable[*Data++];
                    output[j++] = nValue & 0x000000FF;
                }
            }
            i += 4;
        } else {
            Data++;
            i++;
        }
    }
    output[outLen] = '\0';
}

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

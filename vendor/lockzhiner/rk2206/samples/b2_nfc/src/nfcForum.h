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

#ifndef  NFCFORUM_H_
#define  NFCFORUM_H_

#include <stdbool.h>

#include "rtdTypes.h"
#include "NT3H.h"

#define NDEF_START_BYTE     0x03
#define NDEF_END_BYTE       0xFE

#define NTAG_ERASED         0xD0

typedef struct {
    uint8_t startByte;
    uint8_t payloadLength;
}NDEFHeaderStr;

#define BIT_MB (1<<7)
#define BIT_ME (1<<6)
#define BIT_CF (1<<5)
#define BIT_SR (1<<4)
#define BIT_IL (1<<3)
#define BIT_TNF (1<<0)
#define MASK_MB  0x80
#define MASK_ME  0x40
#define MASK_CF  0x20
#define MASK_SR  0x10
#define MASK_IL  0x08
#define MASK_TNF 0x07

typedef struct {
	uint8_t     header;
	uint8_t		typeLength;
	uint8_t		payloadLength;
	RTDTypeStr type;
}NDEFRecordStr;

uint8_t composeRtdText(const NDEFDataStr *ndef,  NDEFRecordStr *ndefRecord, uint8_t *I2CMsg);
uint8_t composeRtdUri(const NDEFDataStr *ndef,  NDEFRecordStr *ndefRecord, uint8_t *I2CMsg);
void composeNDEFMBME(bool isFirstRecord, bool isLastRecord, NDEFRecordStr *ndefRecord);

#endif /* NFCFORUM.H_H_ */

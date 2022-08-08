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

/**
 * @addtogroup Lockzhiner
 *
 * @file flash.h
 */

#ifndef LZ_HARDWARE_FLASH_H
#define LZ_HARDWARE_FLASH_H

/**
 * @brief Get flash block size.
 *
 * This function get flash block size.
 *
 * @return Returns flash block size if the size is got successfully;
 * returns 0 otherwise. For details about other return values, see the chip description.
 */
unsigned int FlashGetBlockSize(void);

/**
 * @brief Reads data from a flash memory address.
 *
 * This function reads a specified length of data from a specified flash memory address.
 *
 * @param flashOffset Indicates the address of the flash memory from which data is to read.
 * @param size Indicates the length of the data to read.
 * @param ramData Indicates the pointer to the RAM for storing the read data.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the data is read successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int FlashRead(unsigned int flashOffset, unsigned int size, unsigned char *ramData);

/**
 * @brief Writes data to a flash memory address.
 *
 * This function writes a specified length of data to a specified flash memory address.
 *
 * @param flashOffset Indicates the address of the flash memory to which data is to be written.
 * @param size Indicates the length of the data to write.
 * @param ramData Indicates the pointer to the RAM for storing the data to write.
 * @param doErase Specifies whether to automatically erase existing data.
 *  Note: if doErase is set, size should be divided by block size(4K).
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the data is written successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int FlashWrite(unsigned int flashOffset, unsigned int size,
                           const unsigned char *ramData, unsigned char doErase);

/**
 * @brief Erases data in a specified flash memory address.
 *
 * @param flashOffset Indicates the flash memory address.
 * @param size Indicates the data length in bytes.
 *  Note: size should be divided by block size(4K).
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the data is erased successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int FlashErase(unsigned int flashOffset, unsigned int size);

/**
 * @brief Initializes a flash device.
 *
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the flash device is initialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int FlashInit(void);

/**
 * @brief Deinitializes a flash device.
 *
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the flash device is deinitialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int FlashDeinit(void);

void FlashSetResidentFlag(int flag);

#endif
/** @} */

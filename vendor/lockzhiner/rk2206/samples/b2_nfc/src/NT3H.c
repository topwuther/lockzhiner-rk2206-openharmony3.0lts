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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lz_hardware.h"
#include "NT3H.h"
#include "ndef.h"
#include "nfcForum.h"

/* NFC使用i2c的总线ID */
static unsigned int NFC_I2C_PORT = 2;

/* i2c配置 */
static I2cBusIo m_i2c2m0 =
{
    .scl =  {.gpio = GPIO0_PD6, .func = MUX_FUNC1, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .sda =  {.gpio = GPIO0_PD5, .func = MUX_FUNC1, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_I2C2,
    .mode = FUNC_MODE_M0,
};
/* i2c的时钟频率 */
static unsigned int m_i2c2_freq = 400000;

uint8_t     nfcPageBuffer[NFC_PAGE_SIZE];
NT3HerrNo   errNo;
// due to the nature of the NT3H a timeout is required to
// protectd 2 consecutive I2C access

inline const uint8_t* get_last_ncf_page(void)
{
    return nfcPageBuffer;
}

static bool writeTimeout(  uint8_t *data, uint8_t dataSend)
{
    uint32_t status = 0;
    
    status = LzI2cWrite(NFC_I2C_PORT, NT3H1X_SLAVE_ADDRESS, data, dataSend);
    if (status != LZ_HARDWARE_SUCCESS)
    {
        printf("===== Error: I2C write status1 = 0x%x! =====\r\n", status);
        return 0;
    }
    usleep(300000);
    
    return 1;
}

static bool readTimeout(uint8_t address, uint8_t *block_data)
{
    uint32_t status = 0;
    uint8_t  buffer[1] = {address};
    
    status = LzI2cWrite(NFC_I2C_PORT, NT3H1X_SLAVE_ADDRESS, &buffer[0], 1);
    if (status != LZ_HARDWARE_SUCCESS)
    {
        printf("===== Error: I2C write status1 = 0x%x! =====\r\n", status);
        return 0;
    }
    
    status = LzI2cRead(NFC_I2C_PORT, NT3H1X_SLAVE_ADDRESS, block_data, NFC_PAGE_SIZE);
    if (status != 0)
    {
        printf("===== Error: I2C write status = 0x%x! =====\r\n", status);
        return 0;
    }
    
    return 1;
}

unsigned int NT3HI2cInit()
{
    uint32_t *pGrf = (uint32_t *)0x41050000U;
    uint32_t ulValue;
    
    ulValue = pGrf[7];
    ulValue &= ~((0x7 << 8) | (0x7 << 4));
    ulValue |= ((0x1 << 8) | (0x1 << 4));
    pGrf[7] = ulValue | (0xFFFF << 16);
    printf("%s, %d: GRF_GPIO0D_IOMUX_H(0x%x) = 0x%x\n", __func__, __LINE__, &pGrf[7], pGrf[7]);
    
    if (I2cIoInit(m_i2c2m0) != LZ_HARDWARE_SUCCESS)
    {
        printf("%s, %s, %d: I2cIoInit failed!\n", __FILE__, __func__, __LINE__);
        return __LINE__;
    }
    if (LzI2cInit(NFC_I2C_PORT, m_i2c2_freq) != LZ_HARDWARE_SUCCESS)
    {
        printf("%s, %s, %d: LzI2cInit failed!\n", __FILE__, __func__, __LINE__);
        return __LINE__;
    }
    
    return 0;
}

unsigned int NT3HI2cDeInit()
{
    LzI2cDeinit(NFC_I2C_PORT);
}

bool NT3HReadHeaderNfc(uint8_t *endRecordsPtr, uint8_t *ndefHeader)
{
    *endRecordsPtr = 0;
    bool ret = NT3HReadUserData(0);
    
    // read the first page to see where is the end of the Records.
    if (ret == true)
    {
        // if the first byte is equals to NDEF_START_BYTE there are some records
        // store theend of that
        if ((NDEF_START_BYTE == nfcPageBuffer[0]) && (NTAG_ERASED != nfcPageBuffer[2]))
        {
            *endRecordsPtr = nfcPageBuffer[1];
            *ndefHeader    = nfcPageBuffer[2];
        }
        return true;
    }
    else
    {
        errNo = NT3HERROR_READ_HEADER;
    }
    
    return ret;
}


bool NT3HWriteHeaderNfc(uint8_t endRecordsPtr, uint8_t ndefHeader)
{

    // read the first page to see where is the end of the Records.
    bool ret = NT3HReadUserData(0);
    if (ret == true)
    {
    
        nfcPageBuffer[1] = endRecordsPtr;
        nfcPageBuffer[2] = ndefHeader;
        ret = NT3HWriteUserData(0, nfcPageBuffer);
        if (ret == false)
        {
            errNo = NT3HERROR_WRITE_HEADER;
        }
    }
    else
    {
        errNo = NT3HERROR_READ_HEADER;
    }
    
    return ret;
}



bool NT3HEraseAllTag(void)
{
    bool ret = true;
    uint8_t erase[NFC_PAGE_SIZE + 1] = {USER_START_REG, 0x03, 0x03, 0xD0, 0x00, 0x00, 0xFE};
    ret = writeTimeout(erase, sizeof(erase));
    
    if (ret == false)
    {
        errNo = NT3HERROR_ERASE_USER_MEMORY_PAGE;
    }
    return ret;
}

bool NT3HReaddManufactoringData(uint8_t *manuf)
{
    return readTimeout(MANUFACTORING_DATA_REG, manuf);
}

bool NT3HReadConfiguration(uint8_t *configuration)
{
    return readTimeout(CONFIG_REG, configuration);
}

bool getSessionReg(void)
{
    return readTimeout(SESSION_REG, nfcPageBuffer);
}


bool NT3HReadUserData(uint8_t page)
{
    uint8_t reg = USER_START_REG + page;
    // if the requested page is out of the register exit with error
    if (reg > USER_END_REG)
    {
        errNo = NT3HERROR_INVALID_USER_MEMORY_PAGE;
        return false;
    }
    
    bool ret = readTimeout(reg, nfcPageBuffer);
    
    if (ret == false)
    {
        errNo = NT3HERROR_READ_USER_MEMORY_PAGE;
    }
    
    return ret;
}


bool NT3HWriteUserData(uint8_t page, const uint8_t* data)
{
    bool ret = true;
    uint8_t dataSend[NFC_PAGE_SIZE + 1]; // data plus register
    uint8_t reg = USER_START_REG + page;
    
    // if the requested page is out of the register exit with error
    if (reg > USER_END_REG)
    {
        errNo = NT3HERROR_INVALID_USER_MEMORY_PAGE;
        ret = false;
        goto end;
    }
    
    dataSend[0] = reg; // store the register
    memcpy(&dataSend[1], data, NFC_PAGE_SIZE);
    ret = writeTimeout(dataSend, sizeof(dataSend));
    if (ret == false)
    {
        errNo = NT3HERROR_WRITE_USER_MEMORY_PAGE;
        goto end;
    }
    
end:
    return ret;
}


bool NT3HReadSram(void)
{
    bool ret = false;
    for (int i = SRAM_START_REG, j = 0; i <= SRAM_END_REG; i++, j++)
    {
        ret = readTimeout(i, nfcPageBuffer);
        if (ret == false)
        {
            return ret;
        }
        //memcpy(&userData[offset], pageBuffer, sizeof(pageBuffer));
    }
    return ret;
}


void NT3HGetNxpSerialNumber(char* buffer)
{
    uint8_t manuf[16];
    
    if (NT3HReaddManufactoringData(manuf))
    {
        for (int i = 0; i < 6; i++)
        {
            buffer[i] = manuf[i];
        }
    }
}

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
 * @file uart.h
 */

#ifndef LZ_HARDWARE_UART_H
#define LZ_HARDWARE_UART_H

/**
 * @brief Enumerates the number of UART data bits.
 *
 * @since 2.2
 * @version 2.2
 */
typedef enum {
    /** 5 data bits */
    UART_DATA_BIT_5 = 5,
    /** 6 data bits */
    UART_DATA_BIT_6,
    /** 7 data bits */
    UART_DATA_BIT_7,
    /** 8 data bits */
    UART_DATA_BIT_8,
} UartIdxDataBit;

/**
 * @brief Enumerates the number of UART stop bits.
 *
 * @since 2.2
 * @version 2.2
 */
typedef enum {
    /** 1 stop bit */
    UART_STOP_BIT_1 = 0,
    /** 2 stop bits */
    UART_STOP_BIT_2 = 1,
} UartStopBit;

/**
 * @brief Enumerates UART parity bits.
 *
 * @since 2.2
 * @version 2.2
 */
typedef enum {
    /** Odd parity */
    UART_PARITY_ODD = 0,
    /** Even parity */
    UART_PARITY_EVEN = 1,
    /** No parity */
    UART_PARITY_NONE = 2,
} UartParity;

/**
 * @brief Enumerates UART block states.
 *
 * @since 2.2
 * @version 2.2
 */
typedef enum {
    /** Block disabled */
    UART_BLOCK_STATE_NONE_BLOCK = 0,
    /** Block enabled */
    UART_BLOCK_STATE_BLOCK,
} UartBlockState;

/**
 * @brief Enumerates hardware flow control modes.
 *
 * @since 2.2
 * @version 2.2
 */
typedef enum {
    /** Hardware flow control disabled */
    FLOW_CTRL_NONE,
    /** RTS and CTS hardware flow control enabled */
    FLOW_CTRL_RTS_CTS,
    /** RTS hardware flow control enabled */
    FLOW_CTRL_RTS_ONLY,
     /** CTS hardware flow control enabled */
    FLOW_CTRL_CTS_ONLY,
} FlowCtrl;

/**
 * @brief Defines basic attributes of a UART port.
 *
 * @since 2.2
 * @version 2.2
 */
typedef struct {
    /** Baud rate */
    unsigned int baudRate;
    /** Data bits */
    UartIdxDataBit dataBits; 
    /** Stop bit */
    UartStopBit stopBits; 
    /** Parity */
    UartParity parity; 
    /** Rx block state */
    UartBlockState rxBlock;
    /** Tx block state */
    UartBlockState txBlock;
    /** Padding bit */
    unsigned char pad;
} UartAttribute;

/**
 * @brief Configures a UART device with the port number specified by <b>id</b>
 * based on the basic and extended attributes.
 *
 *
 *
 * @param id Indicates the port number of the UART device.
 * @param param Indicates the pointer to the UART attributes.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the UART device is configured successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzUartInit(unsigned int id, const UartAttribute *param);

/**
 * @brief Reads a specified length of data from a UART device with the port number specified by <b>id</b>.
 *
 *
 *
 * @param id Indicates the port number of the UART device.
 * @param data Indicates the pointer to the start address of the data to read.
 * @param dataLen Indicates the number of bytes to read.
 * @return Returns the number of bytes read if the operation is successful; returns <b>-1</b> otherwise.
 */
unsigned int LzUartRead(unsigned int id, unsigned char *data, unsigned int dataLen);

/**
 * @brief Writes a specified length of data to a UART device with the port number specified by <b>id</b>.
 *
 *
 *
 * @param id Indicates the port number of the UART device.
 * @param data Indicates the pointer to the start address of the data to write.
 * @param dataLen Indicates the number of bytes to write.
 * @return Returns the number of bytes written if the operation is successful; returns <b>-1</b> otherwise.
 */
unsigned int LzUartWrite(unsigned int id, const unsigned char *data, unsigned int dataLen);

/**
 * @brief Writes a character to a UART device with the port number specified by <b>id</b>.
 *
 *
 *
 * @param id Indicates the port number of the UART device.
 * @param c Indicates a character to write.
 * @return Returns the number of bytes written if the operation is successful; returns <b>-1</b> otherwise.
 */
unsigned int LzUartPutc(unsigned int id, char c);

/**
 * @brief Deinitializes a UART device.
 *
 * @param id Indicates the port number of the UART device.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the UART device is deinitialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzUartDeinit(unsigned int id);

/**
 * @brief Sets flow control for a UART device with the port number specified by <b>id</b>.
 *
 *
 *
 * @param id Indicates the port number of the UART device.
 * @param flowCtrl Indicates the flow control parameters, as enumerated in {@link FlowCtrl}.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if flow control is set successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzUartSetFlowCtrl(unsigned int id, FlowCtrl flowCtrl);

/**
 * 串口读操作，如有有数据则返回0 
 * 参数：
 *      @id: 串口号
 * 返回：0为成功，反之则失败
 */
unsigned int LzUartReadAny(unsigned int id);


unsigned int DebugWrite(unsigned int id, const unsigned char *data, unsigned int dataLen);
unsigned int DebugPutc(unsigned int id, char c);

#endif
/** @} */

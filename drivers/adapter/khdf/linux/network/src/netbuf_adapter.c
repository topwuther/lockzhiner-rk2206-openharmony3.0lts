/*
 * netbuf_adapter.c
 *
 * net buffer adapter of linux
 *
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "netbuf_adapter.h"

#include "hdf_base.h"
#include "hdf_netbuf.h"
#include "securec.h"

/**
 * @brief Initializes a network data buffer queue.
 *
 * @param q Indicates the pointer to the network data buffer queue.
 *
 * @since 1.0
 */
void NetBufQueueInit(NetBufQueue *q)
{
    skb_queue_head_init(q);
}

/**
 * @brief Obtains the size of a network data buffer queue.
 *
 * @param q Indicates the pointer to the network data buffer queue.
 *
 * @return Returns the size of the network data buffer queue.
 *
 * @since 1.0
 */
uint32_t NetBufQueueSize(const NetBufQueue *q)
{
    return skb_queue_len(q);
}

/**
 * @brief Checks whether the network data buffer queue is empty.
 *
 * @param q Indicates the pointer to the network data buffer queue.
 *
 * @return Returns <b>true</b> if the queue is empty; returns <b>false</b> otherwise.
 *
 * @since 1.0
 */
bool NetBufQueueIsEmpty(const NetBufQueue *q)
{
    return skb_queue_empty(q);
}

/**
 * @brief Adds a network data buffer to the tail of a queue.
 *
 * @param q Indicates the pointer to the network data buffer queue.
 * @param nb Indicates the pointer to the network data buffer.
 *
 * @since 1.0
 */
void NetBufQueueEnqueue(NetBufQueue *q, NetBuf *nb)
{
    skb_queue_tail(q, nb);
}

/**
 * @brief Adds a network data buffer to the header of a queue.
 *
 * @param q Indicates the pointer to the network data buffer queue.
 * @param nb Indicates the pointer to the network data buffer.
 *
 * @since 1.0
 */
void NetBufQueueEnqueueHead(NetBufQueue *q, NetBuf *nb)
{
    skb_queue_head(q, nb);
}

/**
 * @brief Obtains a network data buffer from the header of a queue and deletes it from the queue.
 *
 * @param q Indicates the pointer to the network data buffer queue.
 *
 * @return Returns the pointer to the first network data buffer if the queue is not empty;
 * returns <b>NULL</b> otherwise.
 *
 * @since 1.0
 */
NetBuf *NetBufQueueDequeue(NetBufQueue *q)
{
    return skb_dequeue(q);
}

/**
 * @brief Obtains a network data buffer from the tail of a queue and deletes it from the queue.
 *
 * @param q Indicates the pointer to the network data buffer queue.
 *
 * @return Returns the pointer to the last network data buffer if the queue is not empty;
 * returns <b>NULL</b> otherwise.
 *
 * @since 1.0
 */
NetBuf *NetBufQueueDequeueTail(NetBufQueue *q)
{
    return skb_dequeue_tail(q);
}

/**
 * @brief Obtains the network data buffer from the header of a queue, without deleting it from the queue.
 *
 * @param q Indicates the pointer to the network data buffer queue.
 *
 * @return Returns the pointer to the first network data buffer if the queue is not empty;
 * returns <b>NULL</b> otherwise.
 *
 * @since 1.0
 */
NetBuf *NetBufQueueAtHead(const NetBufQueue *q)
{
    return skb_peek(q);
}

/**
 * @brief Obtains the network data buffer from the tail of a queue, without deleting it from the queue.
 *
 * @param q Indicates the pointer to the network data buffer queue.
 *
 * @return Returns the pointer to the last network data buffer if the queue is not empty;
 * returns <b>NULL</b> otherwise.
 *
 * @since 1.0
 */
NetBuf *NetBufQueueAtTail(const NetBufQueue *q)
{
    return skb_peek_tail(q);
}

/**
 * @brief Clears a network data buffer queue and releases the network data buffer in the queue.
 *
 * @param q Indicates the pointer to the network data buffer queue.
 *
 * @since 1.0
 */
void NetBufQueueClear(NetBufQueue *q)
{
    skb_queue_purge(q);
}

/**
 * @brief Moves all network data buffers from one queue to another and clears the source queue.
 *
 * @param q Indicates the pointer to the target network data buffer queue.
 * @param add Indicates the pointer to the source network data buffer queue.
 *
 * @since 1.0
 */
void NetBufQueueConcat(NetBufQueue *q, NetBufQueue *add)
{
    skb_queue_splice_init(add, q);
}

/**
 * @brief Applies for a network data buffer.
 *
 * @param size Indicates the size of the network data buffer.
 *
 * @return Returns the pointer to the network data buffer if the operation is successful;
 * returns <b>NULL</b> otherwise.
 *
 * @since 1.0
 */
NetBuf *NetBufAlloc(uint32_t size)
{
    return dev_alloc_skb(size);
}

/**
 * @brief Releases a network data buffer.
 *
 * @param nb Indicates the pointer to the network data buffer.
 *
 * @since 1.0
 */
void NetBufFree(NetBuf *nb)
{
    kfree_skb(nb);
}

/**
 * @brief Obtains the actual data length of the data segment of a network data buffer.
 *
 * @param nb Indicates the pointer to the network data buffer.
 *
 * @return Returns the actual data length of the data segment.
 *
 * @since 1.0
 */
uint32_t NetBufGetDataLen(const NetBuf *nb)
{
    if (nb == NULL) {
        return 0;
    }
    return nb->len;
}

/**
 * @brief Adjusts the size of a network data buffer space.
 *
 * This function is used to apply for a new network data buffer based on the configured reserved space and
 * the size of the source network data buffer, and copy the actual data to the new network data buffer.
 *
 * @param nb Indicates the pointer to the network data buffer.
 * @param head Indicates the size of the header buffer segment reserved.
 * @param tail Indicates the size of the tail buffer segment reserved.
 *
 * @return Returns <b>0</b> if the operation is successful; returns a non-zero value otherwise.
 *
 * @since 1.0
 */
int32_t NetBufResizeRoom(NetBuf *nb, uint32_t head, uint32_t tail)
{
    return pskb_expand_head(nb, head, tail, GFP_ATOMIC);
}

/**
 * @brief Performs operations based on the segment ID of a network data buffer.
 * The function is opposite to that of {@link NetBufPop}.
 *
 * Description:
 * ID Type | Result
 * -------|---------
 * E_HEAD_BUF | The length of the header buffer segment is increased and the length of the data segment is reduced.
 * E_DATA_BUF | The length of the data segment is increased and the length of the tail buffer segment is reduced.
 * E_TAIL_BUF | The length of the tail buffer segment is increased and the length of the data segment is reduced.
 *
 * @param nb Indicates the pointer to the network data buffer.
 * @param id Indicates the buffer segment ID.
 * @param len Indicates the operation length.
 *
 * @return Returns the start address of the data segment if the operation is successful;
 * returns <b>NULL</b> if the operation length exceeds the space of a specified buffer segment.
 *
 * @since 1.0
 */
void *NetBufPush(NetBuf *nb, uint32_t id, uint32_t len)
{
    if (nb == NULL) {
        return NULL;
    }

    switch (id) {
        case E_HEAD_BUF:
            nb->data += len;
            break;
        case E_DATA_BUF:
            nb->tail += len;
            nb->len += len;
            break;
        case E_TAIL_BUF:
            if (unlikely(len > nb->len) || unlikely(len > nb->tail)) {
                return NULL;
            }

            nb->tail -= len;
            nb->len -= len;
            break;
        default:
            break;
    }

    return nb->data;
}

/**
 * @brief Performs operations based on the segment ID of a network data buffer.
 * The function is opposite to that of {@link NetBufPush}.
 *
 * Description:
 * ID Type | Result
 * -------|---------
 * E_HEAD_BUF | The length of the header buffer segment is reduced and the length of the data segment is increased.
 * E_DATA_BUF| The length of the data segment is reduced and the length of the tail buffer segment is increased.
 * E_TAIL_BUF | The length of the tail buffer segment is reduced and the length of the data segment is increased.
 *
 * @param nb Indicates the pointer to the network data buffer.
 * @param id Indicates the buffer segment ID.
 * @param len Indicates the operation length.
 *
 * @return Returns the start address of the data segment if the operation is successful;
 * returns <b>NULL</b> if the operation length exceeds the space of a specified buffer segment.
 *
 * @since 1.0
 */
void *NetBufPop(NetBuf *nb, uint32_t id, uint32_t len)
{
    if (nb == NULL) {
        return NULL;
    }

    switch (id) {
        case E_HEAD_BUF:
            if (unlikely(len > nb->data)) {
                return NULL;
            }
            nb->data -= len;
            nb->len += len;
            break;
        case E_DATA_BUF:
            if (unlikely(len > nb->len)) {
                return NULL;
            }
            nb->data += len;
            nb->len -= len;
            break;
        case E_TAIL_BUF:
            nb->tail += len;
            nb->len += len;
            break;
        default:
            break;
    }

    return nb->data;
}

/**
 * @brief Obtains the address of a specified buffer segment in a network data buffer.
 *
 * @param nb Indicates the pointer to the network data buffer.
 * @param id Indicates the buffer segment ID.
 *
 * @return Returns the address of the specified buffer segment if the operation is successful;
 * returns <b>NULL</b> if the buffer segment ID is invalid.
 *
 * @since 1.0
 */
uint8_t *NetBufGetAddress(const NetBuf *nb, uint32_t id)
{
    uint8_t *addr = NULL;

    if (nb == NULL) {
        return addr;
    }

    switch (id) {
        case E_HEAD_BUF:
            addr = nb->head;
            break;
        case E_DATA_BUF:
            addr = nb->data;
            break;
        case E_TAIL_BUF:
            addr = skb_tail_pointer(nb);
            break;
        default:
            break;
    }

    return addr;
}

/**
 * @brief Obtains the size of a specified buffer segment space in a network data buffer.
 *
 * @param nb Indicates the pointer to the network data buffer.
 * @param id Indicates the buffer segment ID.
 *
 * @return Returns the size of the specified buffer segment space if the operation is successful;
 * returns <b>NULL</b> if the buffer segment ID is invalid.
 *
 * @since 1.0
 */
uint32_t NetBufGetRoom(const NetBuf *nb, uint32_t id)
{
    uint32_t size = 0;

    if (nb == NULL) {
        return size;
    }

    switch (id) {
        case E_HEAD_BUF:
            size = skb_headroom(nb);
            break;
        case E_DATA_BUF:
            size = skb_is_nonlinear(nb) ?
                0 : (skb_tail_pointer(nb) - nb->data);
            break;
        case E_TAIL_BUF:
            size = skb_tailroom(nb);
            break;
        default:
            break;
    }

    return size;
}

/**
 * @brief Copies data in a network data buffer to another network data buffer.
 *
 * @param nb Indicates the pointer to the network data buffer.
 * @param cnb Indicates the pointer to the target network data buffer.
 *
 * @return Returns <b>0</b> if the operation is successful; returns a non-zero value otherwise.
 *
 * @since 1.0
 */
int32_t NetBufConcat(NetBuf *nb, NetBuf *cnb)
{
    uint32_t tailroom = skb_tailroom(nb) < 0 ? 0 : skb_tailroom(nb);

    if (cnb == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (tailroom < cnb->len) {
        return HDF_FAILURE;
    }
    if (memcpy_s(skb_tail_pointer(nb), tailroom, cnb->data, cnb->len) != EOK) {
        return HDF_FAILURE;
    }

    skb_put(nb, cnb->len);
    NetBufFree(cnb);
    return HDF_SUCCESS;
}

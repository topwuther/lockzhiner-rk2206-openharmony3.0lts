/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HDF_SBU_IMPL_H
#define HDF_SBU_IMPL_H

#include "hdf_base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct HdfSbufConstructor {
    struct HdfSbufImpl *(*obtain)(size_t capacity);
    struct HdfSbufImpl *(*bind)(uintptr_t base, size_t size);
};

struct HdfRemoteService;

struct HdfSbufImpl {
    bool (*writeBuffer)(struct HdfSbufImpl *sbuf, const uint8_t *data, uint32_t writeSize);
    bool (*writeUnpadBuffer)(struct HdfSbufImpl *sbuf, const uint8_t *data, uint32_t writeSize);
    bool (*writeUint64)(struct HdfSbufImpl *sbuf, uint64_t value);
    bool (*writeUint32)(struct HdfSbufImpl *sbuf, uint32_t value);
    bool (*writeUint16)(struct HdfSbufImpl *sbuf, uint16_t value);
    bool (*writeUint8)(struct HdfSbufImpl *sbuf, uint8_t value);
    bool (*writeInt64)(struct HdfSbufImpl *sbuf, int64_t value);
    bool (*writeInt32)(struct HdfSbufImpl *sbuf, int32_t value);
    bool (*writeInt16)(struct HdfSbufImpl *sbuf, int16_t value);
    bool (*writeInt8)(struct HdfSbufImpl *sbuf, int8_t value);
    bool (*writeString)(struct HdfSbufImpl *sbuf, const char *value);
    bool (*writeFileDescriptor)(struct HdfSbufImpl *sbuf, int fd);
    bool (*writeFloat)(struct HdfSbufImpl *sbuf, float value);
    bool (*writeDouble)(struct HdfSbufImpl *sbuf, double value);
    bool (*readDouble)(struct HdfSbufImpl *sbuf, double *value);
    bool (*readFloat)(struct HdfSbufImpl *sbuf, float *value);
    int (*readFileDescriptor)(struct HdfSbufImpl *sbuf);
    bool (*writeString16)(struct HdfSbufImpl *sbuf, const char16_t *value, uint32_t size);
    bool (*readBuffer)(struct HdfSbufImpl *sbuf, const uint8_t **data, uint32_t *readSize);
    const uint8_t *(*readUnpadBuffer)(struct HdfSbufImpl *sbuf, size_t length);
    bool (*readUint64)(struct HdfSbufImpl *sbuf, uint64_t *value);
    bool (*readUint32)(struct HdfSbufImpl *sbuf, uint32_t *value);
    bool (*readUint16)(struct HdfSbufImpl *sbuf, uint16_t *value);
    bool (*readUint8)(struct HdfSbufImpl *sbuf, uint8_t *value);
    bool (*readInt64)(struct HdfSbufImpl *sbuf, int64_t *value);
    bool (*readInt32)(struct HdfSbufImpl *sbuf, int32_t *value);
    bool (*readInt16)(struct HdfSbufImpl *sbuf, int16_t *value);
    bool (*readInt8)(struct HdfSbufImpl *sbuf, int8_t *value);
    const char *(*readString)(struct HdfSbufImpl *sbuf);
    const char16_t *(*readString16)(struct HdfSbufImpl *sbuf);
    int32_t (*writeRemoteService)(struct HdfSbufImpl *sbuf, const struct HdfRemoteService *service);
    struct HdfRemoteService *(*readRemoteService)(struct HdfSbufImpl *sbuf);
    const uint8_t *(*getData)(const struct HdfSbufImpl *sbuf);
    void (*flush)(struct HdfSbufImpl *sbuf);
    size_t (*getCapacity)(const struct HdfSbufImpl *sbuf);
    size_t (*getDataSize)(const struct HdfSbufImpl *sbuf);
    void (*setDataSize)(struct HdfSbufImpl *sbuf, size_t size);
    void (*recycle)(struct HdfSbufImpl *sbuf);
    struct HdfSbufImpl *(*move)(struct HdfSbufImpl *sbuf);
    struct HdfSbufImpl *(*copy)(const struct HdfSbufImpl *sbuf);
    void (*transDataOwnership)(struct HdfSbufImpl *sbuf);
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HDF_SBUF_H */
/** @} */

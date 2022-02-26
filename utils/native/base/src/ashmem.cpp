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

#include "ashmem.h"

#include <cerrno>
#include <cstdio>
#include <string>
#include <fcntl.h>
#include <linux/ashmem.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#include "securec.h"
#include "utils_log.h"

namespace OHOS {
static pthread_mutex_t g_ashmemLock = PTHREAD_MUTEX_INITIALIZER;

using openFdFunction = int (*)();
static openFdFunction g_openFdApi = nullptr;

openFdFunction ProbeAshmemFdFunction()
{
    auto handle = dlopen("libashmemd_client.so", RTLD_NOW);
    if (!handle) {
        UTILS_LOGE("Failed to open libashmemd_client.so");
        return nullptr;
    }
    openFdFunction func = reinterpret_cast<openFdFunction>(dlsym(handle, "openAshmemdFd"));
    if (!func) {
        dlclose(handle);
        UTILS_LOGE("Failed to obtain address of openFdFunction");
    }
    return func;
}

static int AshmemOpenLocked()
{
    int fd = -1;

    if (g_openFdApi == nullptr) {
        g_openFdApi = ProbeAshmemFdFunction();
    }

    if (g_openFdApi != nullptr) {
        fd = g_openFdApi();
    } else {
        fd = TEMP_FAILURE_RETRY(open("/dev/ashmem", O_RDWR | O_CLOEXEC));
    }

    if (fd < 0) {
        UTILS_LOGE("fd is invalid");
        return fd;
    }

    struct stat st;
    int ret = TEMP_FAILURE_RETRY(fstat(fd, &st));
    if (ret < 0) {
        UTILS_LOGE("Failed to exec fstat");
        close(fd);
        return ret;
    }

    if (!S_ISCHR(st.st_mode) || !st.st_rdev) {
        UTILS_LOGE("stat status is invalid");
        close(fd);
        return -1;
    }
    return fd;
}

static int AshmemOpen()
{
    pthread_mutex_lock(&g_ashmemLock);
    int fd = AshmemOpenLocked();
    pthread_mutex_unlock(&g_ashmemLock);
    return fd;
}

/*
 * AshmemCreate - create a new ashmem region and returns the file descriptor
 * fd < 0 means failed
 *
 */
int AshmemCreate(const char *name, size_t size)
{
    int ret;
    int fd = AshmemOpen();
    if (fd < 0) {
        UTILS_LOGE("Failed to exec AshmemOpen");
        return fd;
    }

    if (name != nullptr) {
        char buf[ASHMEM_NAME_LEN] = {0};
        ret = strcpy_s(buf, sizeof(buf), name);
        if (ret != EOK) {
            UTILS_LOGE("Failed to exec strcpy_s");
            close(fd);
            return -1;
        }
        ret = TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_SET_NAME, buf));
        if (ret < 0) {
            UTILS_LOGE("Failed to exec ioctl");
            close(fd);
            return ret;
        }
    }

    ret = TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_SET_SIZE, size));
    if (ret < 0) {
        UTILS_LOGE("Failed to exec ioctl");
        close(fd);
        return ret;
    }
    return fd;
}

int AshmemSetProt(int fd, int prot)
{
    return TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_SET_PROT_MASK, prot));
}

int AshmemGetSize(int fd)
{
    return TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_GET_SIZE, NULL));
}

Ashmem::Ashmem(int fd, int size) : memoryFd_(fd), memorySize_(size), flag_(0), startAddr_(nullptr)
{
}

Ashmem::~Ashmem()
{
}

sptr<Ashmem> Ashmem::CreateAshmem(const char *name, int32_t size)
{
    if ((name == nullptr) || (size <= 0)) {
        UTILS_LOGE("Parameter is invalid");
        return nullptr;
    }

    int fd = AshmemCreate(name, size);
    if (fd < 0) {
        UTILS_LOGE("Failed to exec AshmemCreate");
        return nullptr;
    }

    return new Ashmem(fd, size);
}

void Ashmem::CloseAshmem()
{
    if (memoryFd_ > 0) {
        ::close(memoryFd_);
        memoryFd_ = -1;
    }
    memorySize_ = 0;
    flag_ = 0;
    startAddr_ = nullptr;
}

bool Ashmem::MapAshmem(int mapType)
{
    void *startAddr = ::mmap(nullptr, memorySize_, mapType, MAP_SHARED, memoryFd_, 0);
    if (startAddr == MAP_FAILED) {
        UTILS_LOGE("Failed to exec mmap");
        return false;
    }

    startAddr_ = startAddr;
    flag_ = mapType;

    return true;
}

bool Ashmem::MapReadAndWriteAshmem()
{
    return MapAshmem(PROT_READ | PROT_WRITE);
}

bool Ashmem::MapReadOnlyAshmem()
{
    return MapAshmem(PROT_READ);
}

void Ashmem::UnmapAshmem()
{
    if (startAddr_ != nullptr) {
        ::munmap(startAddr_, memorySize_);
        startAddr_ = nullptr;
    }
    flag_ = 0;
}

bool Ashmem::SetProtection(int protectionType)
{
    int result = AshmemSetProt(memoryFd_, protectionType);
    return result >= 0;
}

int Ashmem::GetProtection()
{
    return TEMP_FAILURE_RETRY(ioctl(memoryFd_, ASHMEM_GET_PROT_MASK));
}

int32_t Ashmem::GetAshmemSize()
{
    return AshmemGetSize(memoryFd_);
}

bool Ashmem::WriteToAshmem(const void *data, int32_t size, int32_t offset)
{
    if (data == nullptr) {
        return false;
    }

    if (!CheckValid(size, offset, PROT_WRITE)) {
        UTILS_LOGE("%{public}s: invalid input or not map", __func__);
        return false;
    }

    auto tmpData = reinterpret_cast<char *>(startAddr_);
    int ret = memcpy_s(tmpData + offset, memorySize_ - offset, reinterpret_cast<const char *>(data), size);
    if (ret != EOK) {
        UTILS_LOGE("%{public}s: Failed to memcpy, ret = %{public}d", __func__, ret);
        return false;
    }

    return true;
}

const void *Ashmem::ReadFromAshmem(int32_t size, int32_t offset)
{
    if (!CheckValid(size, offset, PROT_READ)) {
        UTILS_LOGE("%{public}s: invalid input or not map", __func__);
        return nullptr;
    }

    return reinterpret_cast<const char *>(startAddr_) + offset;
}

bool Ashmem::CheckValid(int32_t size, int32_t offset, int cmd)
{
    if (startAddr_ == nullptr) {
        return false;
    }
    if ((size < 0) || (size > memorySize_) || (offset < 0) || (offset > memorySize_)) {
        return false;
    }
    if (offset + size > memorySize_) {
        return false;
    }
    if (!(static_cast<uint32_t>(GetProtection()) & static_cast<uint32_t>(cmd)) ||
        !(static_cast<uint32_t>(flag_) & static_cast<uint32_t>(cmd))) {
        return false;
    }

    return true;
}
}

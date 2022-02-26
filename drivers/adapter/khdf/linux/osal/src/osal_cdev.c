/*
 * osal_cdev.c
 *
 * osal driver
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
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include "osal_cdev.h"
#include "hdf_log.h"
#include "osal_file.h"
#include "osal_mem.h"
#include "osal_uaccess.h"

#define HDF_LOG_TAG osal_cdev

#define HDF_MAJOR 231
#define HDF_MINOR_START 0
#define HDF_MAX_CHAR_DEVICES 1024

static DEFINE_IDA(hdf_vnode_ids);

struct OsalCdev {
    struct device dev;
    struct cdev cdev;
    struct file_operations fops;
    const struct OsalCdevOps* opsImpl;
    void* priv;
};

static const char* StringRfindChar(const char* str, char chr)
{
    const char* p = NULL;
    if (str == NULL) {
        return NULL;
    }

    p = str + strlen(str);
    while (p >= str) {
        if (*p == chr) {
            return p;
        }
        p--;
    }

    return NULL;
}

static char* hdfDevnode(struct device* dev, umode_t* mode)
{
    (void)mode;
    return kasprintf(GFP_KERNEL, "hdf/%s", dev_name(dev));
}

static bool g_hdfClassInitted = false;

struct class g_hdfClass = {
    .name = "hdf",
    .devnode = hdfDevnode,
};

static dev_t g_hdfDevt = 0;

static int HdfClassInit(void)
{
    int ret;

    if (g_hdfClassInitted) {
        return HDF_SUCCESS;
    }

    ret = class_register(&g_hdfClass);
    if (ret) {
        HDF_LOGE("fail to register hdf class");
        return ret;
    }
    HDF_LOGI("register hdf class success");

    ret = alloc_chrdev_region(&g_hdfDevt, HDF_MINOR_START, HDF_MAX_CHAR_DEVICES, "hdf");
    if (ret) {
        HDF_LOGE("failed to alloc hdf char major");
        class_unregister(&g_hdfClass);
        return ret;
    }

    g_hdfClassInitted = true;
    return HDF_SUCCESS;
}

static void HdfVnodeDevRelease(struct device* dev)
{
    (void)dev;
}

static int RegisterDev(struct OsalCdev* cdev, const char* devName)
{
    int devMinor;
    int ret;

    ret = HdfClassInit();
    if (ret != HDF_SUCCESS) {
        return ret;
    }

    devMinor = ida_simple_get(&hdf_vnode_ids, HDF_MINOR_START, HDF_MAX_CHAR_DEVICES, GFP_KERNEL);
    if (devMinor < 0) {
        HDF_LOGE("failed to get hdf dev minor");
        return HDF_DEV_ERR_NO_DEVICE;
    }

    dev_set_name(&cdev->dev, "%s", devName);
    cdev->dev.devt = MKDEV(MAJOR(g_hdfDevt), devMinor);
    cdev->dev.class = &g_hdfClass;
    cdev->dev.parent = NULL;
    cdev->dev.release = HdfVnodeDevRelease;
    device_initialize(&cdev->dev);

    cdev_init(&cdev->cdev, &cdev->fops);

    ret = cdev_add(&cdev->cdev, cdev->dev.devt, 1);
    if (ret) {
        ida_simple_remove(&hdf_vnode_ids, devMinor);
        HDF_LOGE("failed to add hdf cdev(%s)\n", devName);
        return ret;
    }

    ret = device_add(&cdev->dev);
    if (ret) {
        HDF_LOGE("device(%s) add failed\n", devName);
        cdev_del(&cdev->cdev);
        ida_simple_remove(&hdf_vnode_ids, devMinor);
        return ret;
    }

    HDF_LOGI("add cdev %s success\n", devName);
    return 0;
}

static loff_t OsalCdevSeek(struct file* filep, loff_t offset, int whence)
{
    struct OsalCdev* dev = container_of(filep->f_inode->i_cdev, struct OsalCdev, cdev);
    return dev->opsImpl->seek(filep, offset, whence);
}

static ssize_t OsalCdevRead(struct file* filep, char __user* buf, size_t buflen, loff_t* offset)
{
    struct OsalCdev* dev = container_of(filep->f_inode->i_cdev, struct OsalCdev, cdev);
    return dev->opsImpl->read(filep, buf, buflen, offset);
}

static ssize_t OsalCdevWrite(struct file* filep, const char __user* buf, size_t buflen, loff_t* offset)
{
    struct OsalCdev* dev = container_of(filep->f_inode->i_cdev, struct OsalCdev, cdev);
    return dev->opsImpl->write(filep, buf, buflen, offset);
}

static unsigned int OsalCdevPoll(struct file* filep, struct poll_table_struct* pollTable)
{
    struct OsalCdev* dev = container_of(filep->f_inode->i_cdev, struct OsalCdev, cdev);
    return dev->opsImpl->poll(filep, pollTable);
}

static long OsalCdevIoctl(struct file* filep, unsigned int cmd, unsigned long arg)
{
    struct OsalCdev* dev = container_of(filep->f_inode->i_cdev, struct OsalCdev, cdev);
    return dev->opsImpl->ioctl(filep, cmd, arg);
}

static int OsalCdevOpen(struct inode* inode, struct file* filep)
{
    struct OsalCdev* dev = container_of(inode->i_cdev, struct OsalCdev, cdev);
    return dev->opsImpl->open(dev, filep);
}

static int OsalCdevRelease(struct inode* inode, struct file* filep)
{
    struct OsalCdev* dev = container_of(inode->i_cdev, struct OsalCdev, cdev);
    return dev->opsImpl->release(dev, filep);
}

static void AssignFileOps(struct file_operations* fops, const struct OsalCdevOps* src)
{
    fops->llseek = src->seek != NULL ? OsalCdevSeek : NULL;
    fops->read = src->read != NULL ? OsalCdevRead : NULL;
    fops->write = src->write != NULL ? OsalCdevWrite : NULL;
    fops->poll = src->poll != NULL ? OsalCdevPoll : NULL;
    fops->unlocked_ioctl = src->ioctl != NULL ? OsalCdevIoctl : NULL;
    fops->open = src->open != NULL ? OsalCdevOpen : NULL;
    fops->release = src->release != NULL ? OsalCdevRelease : NULL;
#ifdef CONFIG_COMPAT
    fops->compat_ioctl = src->ioctl != NULL ? OsalCdevIoctl : NULL;
#endif
}

struct OsalCdev* OsalAllocCdev(const struct OsalCdevOps* fops)
{
    struct OsalCdev* cdev = OsalMemCalloc(sizeof(struct OsalCdev));
    if (cdev == NULL) {
        return NULL;
    }

    AssignFileOps(&cdev->fops, fops);
    cdev->opsImpl = fops;

    return cdev;
}

int OsalRegisterCdev(struct OsalCdev* cdev, const char* name, unsigned int mode, void* priv)
{
    const char* lastSlash;
    (void)mode;
    if (cdev == NULL || name == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    lastSlash = StringRfindChar(name, '/');
    int ret = RegisterDev(cdev, lastSlash == NULL ? name : lastSlash + 1);
    if (ret == HDF_SUCCESS) {
        cdev->priv = priv;
    }

    return ret;
}

void OsalUnregisterCdev(struct OsalCdev* cdev)
{
    if (cdev == NULL) {
        return;
    }
    device_del(&cdev->dev);
    cdev_del(&cdev->cdev);
    ida_simple_remove(&hdf_vnode_ids, MINOR(cdev->dev.devt));
}

void OsalFreeCdev(struct OsalCdev* cdev)
{
    if (cdev != NULL) {
        OsalMemFree(cdev);
    }
}

void* OsalGetCdevPriv(struct OsalCdev* cdev)
{
    return cdev != NULL ? cdev->priv : NULL;
}

void OsalSetFilePriv(struct file* filep, void* priv)
{
    if (filep != NULL) {
        filep->private_data = priv;
    }
}
void* OsalGetFilePriv(struct file* filep)
{
    return filep != NULL ? filep->private_data : NULL;
}

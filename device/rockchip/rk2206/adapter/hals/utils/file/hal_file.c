/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Copyright (c) 2021 Lockzhiner Electronics Co., Ltd.
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

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "link.h"
#include "lz_hardware.h"
#include "lfs.h"
#include "los_config.h"
#include "lfs_api.h"

/* flash的基地址 */
static uint32_t m_flash_base_addr = PART_USERFS_ADDR;

/* 挂载点 */
#define MOUNT_PATH       "/TEXT"
#define DIR_PATH         "/XTS"
#define STR_SLASHES      "/"

////////////////////////////////////////////////////////

static int lzflash_read(const struct lfs_config *c, lfs_block_t block,
                        lfs_off_t off, void *buffer, lfs_size_t size)
{
    uint32_t addr = m_flash_base_addr + (c->block_size * block) + off;
    return FlashRead(addr, size, buffer);
}

static int lzflash_prog(const struct lfs_config *c, lfs_block_t block,
                        lfs_off_t off, const void *buffer, lfs_size_t size)
{
    uint32_t addr = m_flash_base_addr + (c->block_size * block) + off;
    return FlashWrite(addr, size, buffer, 0);
}

static int lzflash_erase(const struct lfs_config *c, lfs_block_t block)
{
    uint32_t addr = m_flash_base_addr + (c->block_size * block);
    return FlashErase(addr, c->block_size);
}

static int lzflash_sync(const struct lfs_config *c)
{
    return 0;
}

/* variables used by the filesystem */
// static lfs_t m_lfs;

/* configuration of the filesystem is provided by this struct */
static const struct lfs_config m_lfs_cfg =
{
    // block device operations
    .read  = lzflash_read,
    .prog  = lzflash_prog,
    .erase = lzflash_erase,
    .sync  = lzflash_sync,
    
    // block device configuration
    .read_size = 4,
    .prog_size = 4,
    .block_size = 4096,
    .block_count = 50,
    .cache_size = 256,
    .lookahead_size = 64,
    .block_cycles = 500,
    .file_max = LFS_FILE_MAX,
    .name_max = LFS_NAME_MAX,
};

#define KV_SUM_FILE        "KV_FILE_SUM"
#define KV_SUM_INDEX       4

static int SetCurrentItem(void)
{
    char value[KV_SUM_INDEX] = {"0"};
    
    int fd = LfsOpen(KV_SUM_FILE, O_RDWR_FS | O_CREAT_FS, 0);
    if (fd < 0)
    {
        return -1;
    }
    int ret = LfsWrite(fd, value, KV_SUM_INDEX);
    LfsClose(fd);
    fd = -1;
    return (ret < 0) ? -1 : 0;
}

static void delect_file(void)
{
#define PATH_STR_LEN 64
    int i = 0;
    int ret = -1;
    char path[PATH_STR_LEN] = {0};
    
    HalFileDelete("_._..__...___");
    HalFileDelete("rw.sys.version_100");
    HalFileDelete("rw.sys.version.utilskvparameter");
    
    for (i = 1; i < LITTLE_FS_MAX_OPEN_FILES; ++i)
    {
        sprintf_s(path, PATH_STR_LEN, "rw.sys.version_%d", i);
        HalFileDelete(path);
        memset_s(path, 0, PATH_STR_LEN);
    }
    for (i = 1; i < 11; ++i)
    {
        sprintf_s(path, PATH_STR_LEN, "testfile2%02d", i);
        HalFileDelete(path);
        memset_s(path, 0, PATH_STR_LEN);
    }
    
    SetCurrentItem();
}

static int clear_fs(void)
{
    int block = 0;
    uint32_t addr = 0;
    int ret = -1;
    for (block = 0; block < m_lfs_cfg.block_count; ++block)
    {
        addr = m_flash_base_addr + (4096 * block);
        ret = FlashErase(addr, 4096);
        printf("@#@#FlashErase addr:%#x block:%d,%d\n", addr, block, ret);
    }
    sleep(10);
}

static int HalFileInit(void)
{
    static uint8_t m_flash_init = 0;
    int err;
    
    if (m_flash_init == 0)
    {
        if (FlashInit() == 0)
        {
            // clear_fs();
            m_flash_init = 1;
            struct FileOpInfo *fileOpInfo = NULL;
            
            SetDefaultMountPath(0, MOUNT_PATH);
            if (CheckPathIsMounted(MOUNT_PATH, &fileOpInfo) == 0)
            {
                err = LfsMount(NULL, MOUNT_PATH, "littlefs", 0, &m_lfs_cfg);
                if (err)
                {
                    printf("@#@#LfsMount err line:%d path:%s\n", __LINE__, MOUNT_PATH);
                    LfsUmount(MOUNT_PATH);
                    return -1;
                }
                
                DIR *dir = LfsOpendir(MOUNT_PATH);
                if (dir == NULL)
                {
                    err = LfsMkdir(MOUNT_PATH, 0);
                    if (err)
                    {
                        printf("@#@#LfsMkdir err line:%d path:%s\n", __LINE__, MOUNT_PATH);
                    }
                    else
                    {
                        err = LfsMkdir(MOUNT_PATH DIR_PATH, 0);
                        if (err)
                        {
                            printf("@#@#LfsMkdir err line:%d path:%s\n", __LINE__, MOUNT_PATH DIR_PATH);
                        }
                        else
                        {
                            DIR *dir = LfsOpendir(MOUNT_PATH DIR_PATH);
                            if (dir == NULL)
                            {
                                printf("@#@#LfsOpendir err line:%d path:%s\n", __LINE__, MOUNT_PATH DIR_PATH);
                            }
                        }
                    }
                }
                else
                {
                    DIR *dir = LfsOpendir(MOUNT_PATH DIR_PATH);
                    if (dir == NULL)
                    {
                        err = LfsMkdir(MOUNT_PATH DIR_PATH, 0);
                        if (err)
                        {
                            printf("@#@#LfsMkdir err line:%d path:%s\n", __LINE__, MOUNT_PATH);
                        }
                    }
                    else
                    {
                        delect_file();
                    }
                }
            }
        }
    }
    
    return 0;
}

////////////////////////////////////////////////////////

int HalFileOpen(const char* path, int oflag, int mode)
{
    int ret = -1;
    int value = 1000;
    int reli_flag = 0;
    char path_buf[LFS_NAME_MAX] = {0};
    
    if (strlen(path) + strlen((MOUNT_PATH DIR_PATH STR_SLASHES)) >= LFS_NAME_MAX)
    {
        return ret;
    }
    strcpy(path_buf, (MOUNT_PATH DIR_PATH STR_SLASHES));
    strcat(path_buf, path);
    
    if (HalFileInit() == 0)
    {
        ret = LfsOpen(path_buf, oflag, mode);
    }
    return ret;
}

int HalFileClose(int fd)
{
    return LfsClose(fd);
}

int HalFileRead(int fd, char *buf, unsigned int len)
{
    return LfsRead(fd, buf, len);;
}

int HalFileWrite(int fd, const char *buf, unsigned int len)
{
    return LfsWrite(fd, buf, len);
}

int HalFileSeek(int fd, int offset, unsigned int whence)
{
    return LfsSeek(fd, offset, whence);
}

int HalFileStat(const char* path, unsigned int* fileSize)
{
    struct stat st_buf = { 0 };
    char path_buf[LFS_NAME_MAX] = {0};
    strcpy(path_buf, (MOUNT_PATH DIR_PATH STR_SLASHES));
    strcat(path_buf, path);
    
    if (LfsStat(path_buf, &st_buf) != 0)
    {
        return -1;
    }
    
    if (fileSize)
    {
        *fileSize = st_buf.st_size;
    }
    
    return 0;
}

int HalFileDelete(const char* path)
{
    int value = 1000;
    char path_buf[LFS_NAME_MAX] = {0};
    
    strcpy(path_buf, (MOUNT_PATH DIR_PATH STR_SLASHES));
    strcat(path_buf, path);
    
    return LfsUnlink(path_buf);
}

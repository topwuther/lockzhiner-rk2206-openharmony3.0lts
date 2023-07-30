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

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "link.h"
#include "lz_hardware.h"
#include "lfs.h"
#include "los_config.h"
#include "lfs_api.h"

/* flash的基地址 */
static uint32_t m_flash_base_addr = PART_USERFS_ADDR + 4096;

/* 挂载点 */
#define MOUNT_PATH       "/data"
#define DIR_PATH         ""
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
    .block_count = 20,
    .cache_size = 256,
    .lookahead_size = 64,
    .block_cycles = 500,
    .file_max = LFS_FILE_MAX,
    .name_max = 32,
};

/* 获取文件夹层级数 */
static int get_folder_level_cnt(const char *pathName, int *cnt)
{
    if(pathName == NULL)
    {
        return -1;
    }

    *cnt = 1;
    for (int i = 1; i < strlen(pathName); i++) {
        if (pathName[i] == '/') {
            (*cnt)++;
        }
    }

    return 0;
}

/* 获取文件夹名长度 */
static int get_folder_name_len(int start, const char *pathName)
{
    int len = 0;

    if(pathName == NULL || start > strlen(pathName))
    {
        return -1;
    }

    start++;
    len = 1;
    for (int i = start; i < strlen(pathName); i++) {
        if (pathName[i] == '/') {
            break;
        }
        len++;
    }

    return len;
}

/* 打开路径 */
static int fs_open_dir(const char* path)
{
    int cnt     = 0;
    int start   = 0;
    int len     = 0;
    int ret     = -1;
    DIR *dir    = NULL;
    char dirpath[LITTLE_FS_MAX_NAME_LEN+1] = {0};

    if(path == NULL || strlen(path) > LITTLE_FS_MAX_NAME_LEN || get_folder_level_cnt(path, &cnt) < 0)
    {
        return -1;
    }

    for(int i = 0; i < cnt; i++)
    {
        len = get_folder_name_len(start, path);
        if(len < 0)
        {
            return len;
        }
        start += len;
        if(len == 1)
        {
           continue;
        }
        memset(dirpath, 0, start+1);
        memcpy(dirpath, path, start);

        dir = LfsOpendir(dirpath);
        if (dir == NULL)
        {
            ret = LfsMkdir(dirpath, 0);
            if (ret)
            {
                printf("@#@#LfsMkdir err line:%d path:%s ret:%d\n", __LINE__, dirpath, ret);
                return -1;
            }
            dir = LfsOpendir(dirpath);
            if(dir == NULL)
            {
                printf("@#@#LfsOpendir err line:%d path:%s\n", __LINE__, dirpath);
                return -1;
            }
        }
    }

    return 0;
}

/* 挂载文件系统 */
static int fs_mount(unsigned index, const char* path)
{
    int err = 0;
    struct FileOpInfo *fileOpInfo = NULL;
        
    SetDefaultMountPath(index, path);
    if (CheckPathIsMounted(path, &fileOpInfo) == 0)
    {
        err = LfsMount(NULL, path, "littlefs", 0, &m_lfs_cfg);
        if (err)
        {
            printf("@#@#LfsMount err line:%d path:%s\n", __LINE__, path);
            LfsUmount(path);
            return -1;
        }
    }
    else
    {
        printf("%s CheckPathIsMounted failed\n", path);
        return -1;
    }

    return 0;
}

/* 文件系统初始化 */
static int fs_init(void)
{
    int ret = -1;
    int block = 0;
    uint32_t addr = 0;

    FlashDeinit();
    FlashInit();

    /* 挂载文件系统 */
    ret =  fs_mount(0, MOUNT_PATH);
    if(ret != 0)
    {
        printf("fs_mount failed(%d) and flahs erase\n");
        /* 挂载不成功，则初始化flash空间 */
        for (block = 0; block < m_lfs_cfg.block_count; ++block)
        {
            addr = m_flash_base_addr + (4096 * block);
            ret = FlashErase(addr, 4096);
            printf("FlashErase: addr(%08x), block(%08x), ret(%d)\n", addr, block, ret);
            if(ret != 0)
            {
                return ret;
            }
        }

        /* 重新挂载 */
        ret =  fs_mount(0, MOUNT_PATH);
        if(ret != 0)
        {
            printf("remount fs failed(%d)\n", ret);
            return ret;
        }
    }

    /* 打开路径 */
    ret = fs_open_dir(MOUNT_PATH DIR_PATH);
    if (ret != 0)
    {
        printf("%s mount failed\n", MOUNT_PATH DIR_PATH);
    }

    return ret;
}

static int HalFileInit(void)
{
    static uint8_t m_flash_init = 0;
    int ret = -1;

    if (m_flash_init == 0)
    {
        ret = fs_init();
        if(ret == 0)
        {
            printf("HalFileInit: Flash Init Successful!\n");
            m_flash_init = 1;
        }
    }

    return 0;
}

////////////////////////////////////////////////////////
int HalFileOpen(const char* path, int oflag, int mode)
{
    int ret = -1;
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
    char path_buf[LFS_NAME_MAX] = {0};

    strcpy(path_buf, (MOUNT_PATH DIR_PATH STR_SLASHES));
    strcat(path_buf, path);
    
    return LfsUnlink(path_buf);
}

/***************************************************************
 * 函数名称: HalFileReadDir
 * 说    明: 列出某目录下所有文件和文件夹
 * 参    数: 
 *      @path           要求列表的目录
 *      @entry          struct dirent数组
 *      @entry_max      entry数组有多少个struct dirent
 *      @entry_length   当前目录下所有的文件和文件夹的数量
 * 返 回 值: 0为成功，反之为失败
***************************************************************/
int HalFileReadDir(const char *path, struct dirent *entry, int entry_max, int *entry_length)
{
    if (path == NULL) {
        printf("%s, %s, %d: path is null\n", __FILE__, __func__, __LINE__);
        return -1;
    }
    if (entry == NULL) {
        printf("%s, %s, %d: entry is null\n", __FILE__, __func__, __LINE__);
        return -1;
    }
    if (entry_max <= 0) {
        printf("%s, %s, %d: entry_max(%d) <= 0\n", __FILE__, __func__, __LINE__, entry_length);
        return -1;
    }
    if (entry_length == NULL) {
        printf("%s, %s, %d: entry_length is null\n", __FILE__, __func__, __LINE__);
        return -1;
    }

    DIR *dir = NULL;
    struct dirent *rent = NULL;
    int i;
    char path_buf[LFS_NAME_MAX] = {0};

    HalFileInit();
    if (strlen(path) + strlen((MOUNT_PATH DIR_PATH STR_SLASHES)) >= LFS_NAME_MAX) {
        return -1;
    }
    strcpy(path_buf, (MOUNT_PATH DIR_PATH STR_SLASHES));
    strcat(path_buf, path);

    dir = LfsOpendir(path_buf);
    if (dir == NULL) {
        printf("%s, %s, %d: LfsOpendir(%s) failed\n", __FILE__, __func__, __LINE__, path_buf);
        return -1;
    }

    i = 0;

    do {
        rent = LfsReaddir(dir);
        if (rent == NULL) {
            // 结束
            break;
        }
        if (i >= entry_max) {
            // 超出
            break;
        }
        
        memcpy(&entry[i], rent, sizeof(struct dirent));
        i++;
        //printf("i(%d), name(%s), type(%d)\n", i, rent->d_name, rent->d_type);
    } while (1);

    *entry_length = i;
    LfsClosedir(dir);

    return 0;
}

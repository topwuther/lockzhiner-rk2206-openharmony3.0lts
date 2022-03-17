#!/bin/bash
# Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# error out on errors
set -e
CPUs=`sed -n "N;/processor/p" /proc/cpuinfo|wc -l`

OUT_DIR="$1"
SDK_DIR=$(pwd)
TOOLS_DIR=${SDK_DIR}/../../tools/package

PART_SYSTEM_BLOCKS=$(cat include/link.h | grep "#define PART_SYSTEM_BLOCKS" | awk -F' ' '{print $3}')
PART_LOADER_BLOCKS=$(cat include/link.h | grep "#define PART_LOADER_BLOCKS" | awk -F' ' '{print $3}')
PART_LITEOS_BLOCKS=$(cat include/link.h | grep "#define PART_LITEOS_BLOCKS" | awk -F' ' '{print $3}')
PART_ROOTFS_BLOCKS=$(cat include/link.h | grep "#define PART_ROOTFS_BLOCKS" | awk -F' ' '{print $3}')
PART_USERFS_BLOCKS=$(cat include/link.h | grep "#define PART_USERFS_BLOCKS" | awk -F' ' '{print $3}')

LDSCRIPT=board.ld

CFLAGS="SDK_DIR=${SDK_DIR} LDSCRIPT=${LDSCRIPT}"

function main()
{
    # make ld script
    make ${CFLAGS} -f build/link.mk
    # make liteos
    make OUTDIR=${OUT_DIR} -j${CPUs}
    # make images
    ${TOOLS_DIR}/mkimage.sh ${PART_SYSTEM_BLOCKS} ${PART_LOADER_BLOCKS} ${PART_LITEOS_BLOCKS} ${PART_ROOTFS_BLOCKS} ${PART_USERFS_BLOCKS}
}

main "$@"

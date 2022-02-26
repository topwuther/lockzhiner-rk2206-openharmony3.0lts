#!/bin/bash
# Copyright (c) 2021 Huawei Device Co., Ltd.
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

set -e

function set_ccache() {
    # user can use environment variable "CCACHE_BASE" to customize ccache directory
    if [ -z "$CCACHE_BASE" ]; then
        CCACHE_BASE=${OHOS_ROOT_PATH}/.ccache
        if [ ! -d "$CCACHE_BASE" ]; then
            mkdir -p $CCACHE_BASE
            chmod -R 777 $CCACHE_BASE
        fi
    fi
    echo "CCACHE_DIR="$CCACHE_BASE
    export USE_CCACHE=1
    export CCACHE_DIR=$CCACHE_BASE
    export CCACHE_UMASK=002
    if [ -f "${OHOS_ROOT_PATH}"/ccache.log ]; then
        mv ${OHOS_ROOT_PATH}/ccache.log ${OHOS_ROOT_PATH}/ccache.log.old
    fi
    export CCACHE_LOGFILE=${OHOS_ROOT_PATH}/ccache.log
    ${CCACHE_EXEC} -M 50G
}

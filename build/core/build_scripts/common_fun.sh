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

rename_last_log()
{
    local log=$1
    if [ -e "${log}" ]; then
        if [ "${HOST_OS}x" == "macx" ]; then
            epoch=$(stat -f %m $log)
        else
            epoch=$(stat --format=%Y $log)
        fi
        mv $log ${TARGET_OUT_DIR}/build.$epoch.log
    fi
}

log_prepare()
{
    mkdir -p $TARGET_OUT_DIR
    log=$1
    rename_last_log $log
    touch $log
}

log()
{
    if [ "$#" -lt 1 ]; then
        return
    fi
    echo "$@" | tee -a $LOG_FILE
}

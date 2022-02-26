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

ninja_trace()
{
    if [ "${NINJA_START_TIME}x" == "x" ]; then
        return
    fi
    #generate build.trace to TARGET_OUT_DIR dir.
    if [ -f "${TARGET_OUT_DIR}"/.ninja_log ]; then
        if [ -f "${BASE_HOME}"/build/scripts/ninja2trace.py ]; then
            python3 ${BASE_HOME}/build/scripts/ninja2trace.py --ninja-log ${TARGET_OUT_DIR}/.ninja_log \
                --trace-file ${TARGET_OUT_DIR}/build.trace --ninja-start-time $NINJA_START_TIME \
                --duration-file ${TARGET_OUT_DIR}/sorted_action_duration.txt
        fi
    fi
}

calc_build_time()
{
    END_TIME=$(date "+%s")
    log "used: $(expr $END_TIME - $BEGIN_TIME) seconds"
}

get_build_warning_list()
{
    if [ -f "${BASE_HOME}"/build/scripts/get_warnings.py ];then
        python3 ${BASE_HOME}/build/scripts/get_warnings.py --build-log-file ${TARGET_OUT_DIR}/build.log --warning-out-file ${TARGET_OUT_DIR}/packages/WarningList.txt
    fi
}

generate_opensource_package()
{
    log "generate opensource package"
    if [ -f "${BASE_HOME}"/build/scripts/code_release.py ];then
        python3 "${BASE_HOME}"/build/scripts/code_release.py

        if [ ! -d "${TARGET_OUT_DIR}"/packages/code_opensource ];then
            mkdir -p "${TARGET_OUT_DIR}"/packages/code_opensource
        fi

        cp "${BASE_HOME}"/out/Code_Opensource.tar.gz "${TARGET_OUT_DIR}"/packages/code_opensource/Code_Opensource.tar.gz -f
    fi
}

ccache_stat()
{
    if [[ ! -z "${CCACHE_EXEC}" ]] && [[ ! -z "${USE_CCACHE}" ]] && [[ "${USE_CCACHE}" == 1 ]]; then
        log "ccache statistics"
        if [ -e "${LOG_FILE}" -a -e "${CCACHE_LOGFILE}" ]; then
            python3 ${BASE_HOME}/build/scripts/summary_ccache_hitrate.py $CCACHE_LOGFILE | tee -a $LOG_FILE
        fi
    fi
}

pycache_stat()
{
    log "pycache statistics"
    python3 ${BASE_HOME}/build/scripts/util/pyd.py --stat
}

pycache_manage()
{
    log "manage pycache contents"
    python3 ${BASE_HOME}/build/scripts/util/pyd.py --manage
}

pycache_stop()
{
    log "pycache daemon exit"
    python3 ${BASE_HOME}/build/scripts/util/pyd.py --stop
}
post_process()
{
    if [ "${OPEN_SOURCE}" == true ];then
        generate_opensource_package
    fi

    calc_build_time
    pycache_stat
    pycache_manage
    pycache_stop
    ninja_trace
    ccache_stat

    python3 ${BASE_HOME}/build/ohos/statistics/build_overlap_statistics.py --build-out-dir ${TARGET_OUT_DIR} --subsystem-config-file ${BASE_HOME}/build/subsystem_config.json --root-source-dir ${BASE_HOME}
    get_build_warning_list
    echo "post_process"
}

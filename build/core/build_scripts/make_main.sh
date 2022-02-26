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

do_make()
{
    TARGET_OUT_DIR=${BASE_HOME}/${OUT_DIR}/${TARGET_OS}-${TARGET_ARCH}-${BUILD_VARIANT}
    if [[ ! -d "${TARGET_OUT_DIR}" ]];then
        mkdir -p ${TARGET_OUT_DIR}
    fi
    if [[ "${TARGET_OS}" == "ohos" && "${TARGET_ARCH}" == "arm64" ]];then
        TARGET_OUT_DIR_LINK=${BASE_HOME}/${OUT_DIR}/${BUILD_VARIANT}
        if [[ -d "${TARGET_OUT_DIR_LINK}" ]] || [[ -L "${TARGET_OUT_DIR_LINK}" ]];then
            rm -r ${TARGET_OUT_DIR_LINK}
        fi
        ln -s ${TARGET_OS}-${TARGET_ARCH}-${BUILD_VARIANT} ${TARGET_OUT_DIR_LINK}
    fi

    # prepare to save build log
    LOG_FILE=${TARGET_OUT_DIR}/build.log
    log_prepare $LOG_FILE
    log "$@"

    BEGIN_TIME=$(date "+%s")

    source ${BUILD_SCRIPT_DIR}/get_gn_parameters.sh
    get_gn_parameters

    if [ "${SKIP_GN_PARSE}"x = falsex ]; then
        ${BUILD_TOOLS_DIR}/gn gen ${TARGET_OUT_DIR} \
            --args="target_os=\"${TARGET_OS}\" target_cpu=\"${TARGET_ARCH}\" is_debug=false \
            product_name=\"${PRODUCT_NAME}\" \
            is_component_build=true \
            ${GN_ARGS} ${TEST_BUILD_PARA_STRING}  ${IS_ASAN} \
            release_test_suite=${RELEASE_TEST_SUITE}" 2>&1 | tee -a $log

        if [ "${PIPESTATUS[0]}" != 0 ]; then
            log "build: gn gen error"
            RET=1
            return
        fi

        if [[ "${BUILD_ONLY_GN}" = true ]];then
            RET=0
            return
        fi
    fi

    if [[ "${REPO_CBUILD}" == true ]];then
        collect_module_info_args="--root-build-dir ${TARGET_OUT_DIR} \
            --output-file ${TARGET_OUT_DIR}/cbuild/targets_info.json"
        python ${BASE_HOME}/build/misc/cbuild/collect_module_info.py $collect_module_info_args
    fi

    if [ "${BUILD_TARGET_NAME}" == "all" ]; then
        BUILD_TARGET_NAME="make_all make_test"
    elif [ "${BUILD_TARGET_NAME}" == "" ]; then
        BUILD_TARGET_NAME=packages
    fi

    log "Starting Ninja..."
    NINJA_START_TIME=$(date +%s%N)
    echo python version: $(python --version)
    ninja_build_args="--source-root-dir ${BASE_HOME} --root-build-dir ${TARGET_OUT_DIR} \
            --build-target-name ${BUILD_TARGET_NAME}"
    if [ "${TARGET_PLATFORM}" != "" ];then
        ninja_build_args="$ninja_build_args --target-platform ${TARGET_PLATFORM}"
    fi
    real_build_target=$(python ${BASE_HOME}/build/scripts/build_target_handler.py $ninja_build_args)
    echo "build_target: "$real_build_target

    if [ "${USE_NARUTO}"x = "truex" ];then
        ${BUILD_TOOLS_DIR}/naruto -d keepdepfile -p ${BASE_HOME}/.naruto_cache -C ${TARGET_OUT_DIR} ${real_build_target} ${NINJA_ARGS} 2>&1 | tee -a $log
    else
        ${BUILD_TOOLS_DIR}/ninja -d keepdepfile -C ${TARGET_OUT_DIR} ${real_build_target} ${NINJA_ARGS} 2>&1 | tee -a $log
    fi

    if [ "${PIPESTATUS[0]}" != 0 ]; then
        log "build: ninja error"
        RET=1
        return
    fi
}

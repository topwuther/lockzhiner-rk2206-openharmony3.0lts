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

get_gn_parameters()
{
    if [ "${TARGET_VERSION_MODE}" == "sanitizer" ]; then
        IS_ASAN="is_asan=true"
    fi

    if [ "${COVERAGE}" == true ];then
        if [ "${TARGET_OS}" == "ohos" ];then
            GN_ARGS="$GN_ARGS use_clang_coverage=true"
        fi
    fi

    if [ "${CUSTOM_CLANG}" == true ];then
        GN_ARGS="$GN_ARGS use_custom_clang=true"
    fi

    if [ "${DOUBLE_FRAMEWORK}" == true ];then
        GN_ARGS="$GN_ARGS is_double_framework=true"
    fi

    if [ "${EBPF_ENABLE}" == true ];then
        GN_ARGS="$GN_ARGS ebpf_enable=true"
    fi

    if [ "${BUILD_XTS}" == true ];then
        GN_ARGS="$GN_ARGS build_xts=true"
        log "Build xts enabled"
    fi

    if [ "${BUILD_OHOS_SDK}" == true ];then
        GN_ARGS="$GN_ARGS build_ohos_sdk=true"
    fi

    if [ "${INTERFACE_CHECK}" == false ];then
        GN_ARGS="$GN_ARGS check_innersdk_interface=false check_sdk_interface=false"
    fi

    if [ "${TARGET_PLATFORM}" != "" ];then
        GN_ARGS="build_platform=\"${TARGET_PLATFORM}\" $GN_ARGS"
    fi

    if [ "${SDK_VERSION}"x != x ];then
        GN_ARGS="$GN_ARGS sdk_version=\"${SDK_VERSION}\""
    fi

    if [ "${HOSP_VERSION}"x != x ];then
        GN_ARGS="$GN_ARGS hosp_version=\"${HOSP_VERSION}\""
    fi

    if [ "${RELEASE_TYPE}"x != x ];then
        GN_ARGS="$GN_ARGS release_type=\"${RELEASE_TYPE}\""
    fi

    if [ "${META_VERSION}"x != x ];then
        GN_ARGS="$GN_ARGS meta_version=\"${META_VERSION}\""
    fi

    if [ "${API_VERSION}"x != x ];then
        GN_ARGS="$GN_ARGS api_version=\"${API_VERSION}\""
    fi

    if [ "${BUILD_EXAMPLE}" == true ];then
        GN_ARGS="$GN_ARGS build_example=true"
    fi

    if [ "${PYCACHE_ENABLE}" == true ];then
        GN_ARGS="$GN_ARGS pycache_enable=true"
    fi
}

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

parse_cmdline()
{
    while [ -n "$1" ]
    do
        var="$1"
        OPTIONS=$(echo ${var%%=*})
        PARAM=$(echo ${var#*=})
        echo "OPTIONS=$OPTIONS"
        echo "PARAM=$PARAM"
        echo "-------------------"
        case "$OPTIONS" in
        test_build_para)  TEST_BUILD_PARA_STRING="$PARAM" ;;
        product_name)     PRODUCT_NAME="$PARAM" ;;
        build_target)     BUILD_TARGET_NAME="$BUILD_TARGET_NAME $PARAM" ;;
        target_os)        TARGET_OS="$PARAM" ;;
        target_cpu)       TARGET_ARCH="$PARAM" ;;
        variant)          BUILD_VARIANT="$PARAM" ;;
        out_dir)          OUT_DIR="$PARAM" ;;
        gn_args)          GN_ARGS="$GN_ARGS $PARAM" ;;
        ninja_args)       NINJA_ARGS="$PARAM" ;;
        versionmode)      TARGET_VERSION_MODE="$PARAM" ;;
        coverage)         COVERAGE="$PARAM" ;;
        custom_clang)     CUSTOM_CLANG="$PARAM" ;;
        double_framework) DOUBLE_FRAMEWORK="$PARAM" ;;
        build_only_gn)    BUILD_ONLY_GN="$PARAM" ;;
        skip_gn_parse)    SKIP_GN_PARSE="$PARAM" ;;
        target_platform)  TARGET_PLATFORM="$PARAM" ;;
        ebpf_enable)      EBPF_ENABLE="$PARAM" ;;
        build_xts)        BUILD_XTS="$PARAM" ;;
        release_test_suite)  RELEASE_TEST_SUITE="$PARAM" ;;
        build_ohos_sdk)      BUILD_OHOS_SDK="$PARAM" ;;
        interface_check)     INTERFACE_CHECK="$PARAM" ;;
        lite_param)          LITE_PARAM="$PARAM" ;;
        sdk_version)         SDK_VERSION="$PARAM" ;;
        hosp_version)        HOSP_VERSION="$PARAM" ;;
        api_version)         API_VERSION="$PARAM" ;;
        release_type)        RELEASE_TYPE="$PARAM" ;;
        meta_version)        META_VERSION="$PARAM" ;;
        build_example)       BUILD_EXAMPLE="$PARAM" ;;
        pycache_enable)      PYCACHE_ENABLE="$PARAM" ;;
        use_naruto)          USE_NARUTO="$PARAM" ;;
        open_source)         OPEN_SOURCE="$PARAM" ;;
        esac
        shift
    done
    COMMAND_ARGS="$@"
}

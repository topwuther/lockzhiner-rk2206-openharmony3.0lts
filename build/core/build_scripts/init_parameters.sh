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

init_parameter()
{
    BUILD_TOOLS_DIR=${BASE_HOME}/prebuilts/build-tools/${HOST_DIR}/bin
    TEST_BUILD_PARA_STRING=""
    DEVICE_TYPE=""
    TARGET_OS=ohos
    BUILD_VARIANT=release
    TARGET_ARCH=arm64
    OUT_DIR=out
    NINJA_ARGS=""
    TARGET_VERSION_MODE=""
    DOUBLE_FRAMEWORK=""
    BUILD_ONLY_GN=false
    SKIP_GN_PARSE=false
    TARGET_PLATFORM=""
    EBPF_ENABLE=false
    MAPLE_JOBS=0
    BUILD_XTS=false
    RELEASE_TEST_SUITE=false
    BUILD_MAPLE_TARGETS=false
    BUILD_OHOS_SDK=false
    BUILD_VERSION="0"
    INTERFACE_CHECK=true
    LITE_PARAM="--chip hi3518ev300"
    BUILD_EXAMPLE=false
    PYCAHCE_ENABLE=false
    USE_NARUTO=false
    OPEN_SOURCE=false
    REPO_CBUILD=false
}

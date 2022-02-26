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

pre_process()
{
    echo "pre_process"
    case $(uname -s) in
        Darwin)
            HOST_DIR="darwin-x86"
            HOST_OS="mac"
            ;;
        Linux)
            HOST_DIR="linux-x86"
            HOST_OS="linux"
            ;;
        *)
            echo "Unsupported host platform: $(uname -s)"
            RET=1
            exit $RET
    esac

    export PATH=${BASE_HOME}/prebuilts/python/${HOST_DIR}/3.8.5/bin:${BASE_HOME}/prebuilts/build-tools/${HOST_DIR}/bin:$PATH
    python --version

    source ${BUILD_SCRIPT_DIR}/init_parameters.sh
    source ${BUILD_SCRIPT_DIR}/parse_cmdline.sh
    source ${BUILD_SCRIPT_DIR}/common_fun.sh
    source ${BUILD_SCRIPT_DIR}/trap_ctrlc.sh

    init_parameter "$@"
    parse_cmdline "$@"
    # Trap SIGINT
    trap "trap_ctrlc" 2

    if [ "${PYCACHE_ENABLE}" == true ];then
        source ${BUILD_SCRIPT_DIR}/set_pycache.sh
        set_pycache
    fi
}

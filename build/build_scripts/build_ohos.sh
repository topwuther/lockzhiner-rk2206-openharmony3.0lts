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
echo "++++++++++++++++++++++++++++++++++++++++"
date +%F' '%H:%M:%S
echo $@

BIN_PATH=$(cd $(dirname $0);pwd)
BASE_HOME=$(dirname $(dirname ${BIN_PATH}))
BUILD_SCRIPT_DIR=${BASE_HOME}/build/core/build_scripts

main()
{
    source ${BUILD_SCRIPT_DIR}/pre_process.sh
    pre_process "$@"

    source ${BUILD_SCRIPT_DIR}/make_main.sh
    do_make "$@"

    source ${BUILD_SCRIPT_DIR}/post_process.sh
    post_process "$@"
    exit $RET
}

main "$@"

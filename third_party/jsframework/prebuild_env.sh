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

script_path=$(cd $(dirname $0);pwd)
code_dir=$(dirname ${script_path})/../

cd ${code_dir}/third_party/jsframework/
export PATH=${code_dir}/prebuilts/build-tools/common/nodejs/node-v12.18.4-linux-x64/bin:$PATH
npm config set registry http://registry.npm.taobao.org
npm install

cd ${code_dir}
if [ -d "${code_dir}/prebuilts/build-tools/common/js-framework" ]; then
    echo -e "\n"
    echo "${code_dir}/prebuilts/build-tools/common/js-framework already exist, it will be replaced with node-${node_js_ver}"
    /bin/rm -rf ${code_dir}/prebuilts/build-tools/common/js-framework
    echo -e "\n"
fi

mkdir -p ${code_dir}/prebuilts/build-tools/common/js-framework
/bin/cp -rf ${code_dir}/third_party/jsframework/node_modules ${code_dir}/prebuilts/build-tools/common/js-framework/

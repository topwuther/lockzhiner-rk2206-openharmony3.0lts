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

function do_make_ohos() {
  local build_cmd="build/build_scripts/build_ohos.sh"
  build_cmd+=" product_name=${product_name} target_os=${target_os} target_cpu=${target_cpu}"
  build_cmd+=" gn_args=is_standard_system=true"
  if [[ "${build_target}x" != "x" ]]; then
    for target_name in ${build_target[@]}; do
      echo $target_name
      build_cmd+=" build_target=$target_name"
    done
  elif [[ "${product_name}" == "ohos-sdk" ]]; then
    build_cmd+=" build_target=build_ohos_sdk"
  else
    build_cmd+=" build_target=images"
  fi

  if [[ "${gn_args}x" != "x" ]]; then
    for _args in ${gn_args[@]}; do
      build_cmd+=" gn_args=$_args"
    done
  fi
  if [[ "${ninja_args}x" != "x" ]]; then
    for _args in ${ninja_args[@]}; do
      build_cmd+=" ninja_args=$_args"
    done
  fi
  if [[ "${PYCACHE_ENABLE}" == true ]]; then
    build_cmd+=" pycache_enable=true"
  fi
  if [[ "${build_only_gn}" == true ]]; then
    build_cmd+=" build_only_gn=true"
  fi
  if [[ "${sparse_image}" == true ]]; then
    build_cmd+=" gn_args=sparse_image=true"
  fi
  echo "build_ohos_cmd: $build_cmd"
  $build_cmd
}

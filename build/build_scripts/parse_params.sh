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

# set default value
target_os=ohos
target_cpu=arm64
use_ccache=false
sparse_image=false


while test $# -gt 0
do
  case "$1" in
  --product-name)
    shift
    product_name="$1"
    ;;
  --device-name)
    shift
    device_name="$1"
    ;;
  --target-cpu)
    shift
    target_cpu="$1"
    ;;
  --target-os)
    shift
    target_os="$1"
    ;;
  --build-target | -t)
    shift
    build_target="${build_target} $1"
    ;;
  --gn-args)
    shift
    gn_args="${gn_args} $1"
    ;;
  --ninja-args)
    shift
    ninja_args="${ninja_args} $1"
    ;;
  --ccache)
    use_ccache=true
    ;;
  --sparse-image)
    sparse_image=true
    ;;
  --jobs)
    shift
    jobs="$1"
    ;;
  --export-para)
    shift
    PARAM1=$(echo "$1" | sed 's/\(.*\):\(.*\)/\1/')
    PARAM2=$(echo "$1" | sed 's/.*://')
    export $PARAM1=$PARAM2
    ;;
  --build-only-gn)
    build_only_gn=true;;
  -* | *)
    echo "Unrecognized option: $1"
    exit 1
    ;;
  esac
  shift
done

if [[ "${product_name}x" == "x" ]]; then
  echo "Error: the product name should be specified!"
  exit 1
fi

if [[ "${use_ccache}" == true ]]; then
  set +e
  which ccache > /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "\033[31mError: the ccache is not available, please install ccache.\033[0m"
    exit 1
  fi
  source ${OHOS_ROOT_PATH}/build/core/build_scripts/set_ccache.sh
  export CCACHE_EXEC=$(which ccache)
  set_ccache
  set -e
fi

if [[ "${sparse_image}" == true ]]; then
  set +e
  which img2simg > /dev/null 2>&1
  if [[ $? -ne 0 ]]; then
    echo -e "\033[31mError: the img2simg is not available, please install img2simg.\033[0m"
    exit 1
  fi
  set -e
fi

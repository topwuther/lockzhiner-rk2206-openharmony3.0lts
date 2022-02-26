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

function help() {
  echo
  echo "Usage:"
  echo "  ./build.sh --product-name {product-name} [options]"
  echo
  echo "Examples:"
  echo "  ./build.sh --product-name Hi3516DV300 --ccache"
  echo
  echo "options"
  echo "  --ccache          use ccache, default: false"
  echo "  --jobs N          run N jobs in parallel"
  echo "  --build-target    build target name"

  echo "  --gn-args         gn args"
  echo "  --export-para     export env"
  echo "  --help, -h        print help info"
  echo
  exit 1
}


script_path=$(cd $(dirname $0);pwd)

source_root_dir="${script_path}"
while [[ ! -f "${source_root_dir}/.gn" ]]; do
    source_root_dir="$(dirname "${source_root_dir}")"
    if [[ "${source_root_dir}" == "/" ]]; then
        echo "Cannot find source tree containing $(pwd)"
        exit 1
    fi
done

build_params=""

while test $# -gt 0
do
  case "$1" in
  --product-name)
    shift
    product_name="$1"
    ;;
  --help | -h)
    help
    exit 0
    ;;
  *)
    build_params+=" $1"
    ;;
  esac
  shift
done


if [[ "${source_root_dir}x" == "x" ]]; then
  echo "Error: source_root_dir cannot be empty."
  exit 1
fi
if [[ ! -d "${source_root_dir}" ]]; then
  echo "Error: source_root_dir is incorrect."
  exit 1
fi
if [[ "${product_name}x" == "x" ]]; then
  echo -e "\033[31mError: the product name should be specified!\033[0m"
  help
  exit 1
fi


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

# set python3
PYTHON3=${source_root_dir}/prebuilts/python/${HOST_DIR}/3.8.5/bin/python3

if [[ ! -f "${PYTHON3}" ]]; then
  echo -e "\033[33m Please execute the build/prebuilts_download.sh \033[0m"
  exit 1
fi

${PYTHON3} ${source_root_dir}/build/scripts/tools_checker.py

export OHOS_ROOT_PATH="${source_root_dir}"
export PYTHON3="${PYTHON3}"
export USE_OHOS_INIT=true
export BUILD_IMAGE=true

cd ${source_root_dir}

# preloader
${PYTHON3} ${source_root_dir}/build/loader/preloader/preloader.py \
  --product-name ${product_name} \
  --source-root-dir ${source_root_dir} \
  --products-config-dir "productdefine/common/products" \
  --preloader-output-root-dir "out/build_configs"

source ${source_root_dir}/out/build_configs/${product_name}/preloader/build.prop

# call build
${source_root_dir}/build/build_scripts/build_${system_type}.sh \
  --product-name ${product_name} \
  --device-name ${device_name} \
  --target-os ${target_os} \
  --target-cpu ${target_cpu} \
  ${build_params}

if [[ "${PIPESTATUS[0]}" -ne 0 ]]; then
    echo -e "\033[31m=====build ${product_name} error.\033[0m"
    exit 1
fi
echo -e "\033[32m=====build ${product_name} successful.\033[0m"

date +%F' '%H:%M:%S
echo "++++++++++++++++++++++++++++++++++++++++"

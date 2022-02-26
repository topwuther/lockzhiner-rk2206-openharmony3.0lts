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

rm -rf ./gen/third_party/jsframework/snapshot
LD_LIBRARY_PATH=./clang_x64/ark/ark:./clang_x64/global/i18n:./../../libs/android_libs/ndk/libcxx/linux_x86:./../../libs/android_libs/ndk/libcxx/linux_x86_64:./../../libs/android_libs/ndk/libcxx/aosp_x86_64:./../../prebuilts/clang/ohos/linux-x86_64/llvm/lib/:$LD_LIBRARY_PATH $@
echo "general snapshot file"

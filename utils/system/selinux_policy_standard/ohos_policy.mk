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

ifeq ($(system_type), standard)
    # compile in the vendorimg
    OHOS_VENDOR_SEPOLICY_DIR :=
    # interface for vendor and system, compile in the vendorimg and systemimg
    OHOS_PUBLIC_SEPOLICY_DIR :=
    # compile in the systemimg
    OHOS_PRIVATE_SEPOLICY_DIR :=

    OHOS_SEPOLICY_PATH := $(TOPDIR)../../../utils/system/selinux_policy_standard

    define search-sub-policy
    $(wildcard $(1)/*/policy.mk)
    endef

    include $(call search-sub-policy, $(OHOS_SEPOLICY_PATH))
endif

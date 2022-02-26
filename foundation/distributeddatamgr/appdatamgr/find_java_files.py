#!/usr/bin/env python
# -*- coding: utf-8 -*-
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

import os
import sys
import fnmatch


def FindInDirectory(directory, filename_filter):
    for root, _dirnames, filenames in os.walk(directory):
        matched_files = fnmatch.filter(filenames, filename_filter)
        for file in matched_files:
            print(os.path.join(root, file))


def main(args):
    dirs = args
    for directory in dirs:
        if os.path.isdir(directory):
            FindInDirectory(directory, "*.java")
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
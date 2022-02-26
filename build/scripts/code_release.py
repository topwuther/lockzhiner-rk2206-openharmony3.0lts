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

import sys
import os
import shutil
import tarfile
sys.path.append(os.path.abspath(os.path.dirname(
    os.path.abspath(os.path.dirname(__file__)))))
from scripts.util.file_utils import read_json_file  # noqa: E402

RELEASE_FILENAME = 'README.OpenSource'
scan_dir_list = ['third_party']


def get_source_top_dir():
    top_dir = os.path.abspath(os.path.dirname(
        os.path.abspath(os.path.dirname(
            os.path.abspath(os.path.dirname(__file__))))))
    return top_dir


def get_package_dir():
    top_dir = get_source_top_dir()
    package_dir = os.path.join(top_dir, 'out', 'Code_Opensource')
    return package_dir


def copy_opensource_file(opensource_config_file):
    if not os.path.exists(opensource_config_file):
        print("Warning, the opensource config file is not exists.")
        return False

    top_dir = get_source_top_dir()
    package_dir = get_package_dir()
    src_dir = os.path.dirname(opensource_config_file)
    dst_dir = os.path.join(package_dir, os.path.relpath(src_dir, top_dir))

    # copy opensource folder to out dir
    if os.path.exists(dst_dir):
        shutil.rmtree(dst_dir)
    shutil.copytree(src_dir, dst_dir, symlinks=True,
                    ignore=shutil.ignore_patterns('*.pyc', 'tmp*', '.git*'))

    # delete the README.OpenSource file
    release_file = os.path.join(dst_dir, RELEASE_FILENAME)
    os.remove(release_file)
    return True


def parse_opensource_file(opensource_config_file):
    if not os.path.exists(opensource_config_file):
        print("Warning, the opensource config file is not exists.")
        return False

    opensource_config = read_json_file(opensource_config_file)
    if opensource_config is None:
        raise Exception("read opensource config file [{}] failed.".format(
            opensource_config_file))

    result = False
    for info in opensource_config:
        license = info.get('License')
        if license.count('GPL') > 0 or license.count('LGPL') > 0:
            result = copy_opensource_file(opensource_config_file)

    return result


def scan_and_package_code_release(scan_dir):
    file_dir_names = os.listdir(scan_dir)
    for file_dir_name in file_dir_names:
        file_dir_path = os.path.join(scan_dir, file_dir_name)
        if os.path.isdir(file_dir_path):
            scan_and_package_code_release(file_dir_path)
        elif file_dir_path == os.path.join(scan_dir, RELEASE_FILENAME):
            parse_opensource_file(file_dir_path)


def scan_opensource_dir_list(scan_list):
    for scan_dir in scan_list:
        scan_and_package_code_release(scan_dir)


def tar_opensource_package_file():
    package_dir = get_package_dir()
    top_dir = get_source_top_dir()
    result = -1
    if os.path.exists(package_dir):
        package_filename = os.path.join(
            top_dir, 'out', 'Code_Opensource.tar.gz')
        try:
            with tarfile.open(package_filename, "w:gz") as tar:
                tar.add(package_dir, arcname=os.path.basename(package_dir))
                result = 0
        except IOError as err:
            raise err
    return result


def main():
    # get the source top directory to be scan
    top_dir = get_source_top_dir()

    # generate base_dir/out/Code_Opensource dir
    package_dir = get_package_dir()
    if os.path.exists(package_dir):
        shutil.rmtree(package_dir)
    os.makedirs(package_dir)

    # scan the target dir and copy release code to out/opensource dir
    dir_list = [os.path.join(top_dir, dir) for dir in scan_dir_list]
    print(dir_list)
    scan_opensource_dir_list(dir_list)

    # package the opensource to Code_Opensource.tar.gz
    if tar_opensource_package_file() == 0:
        print('Generate the opensource package successfully.')
    else:
        print('Generate the opensource package failed.')

    return 0


if __name__ == '__main__':
    sys.exit(main())

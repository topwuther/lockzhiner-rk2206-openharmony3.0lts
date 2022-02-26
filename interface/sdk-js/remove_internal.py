#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.

import os
import sys
import optparse
import shutil

# d.ts directories to be deleted
remove_list = ["@internal", "common", "config", "form", "liteWearable",
    "phone", "router", "smartVision", "tablet", "tv", "wearable"]


# traversal all fill in project folder
def copy_files(input_path, output_path):
    for file in os.listdir(input_path):
        src = os.path.join(input_path, file)
        dst = os.path.join(output_path, file)
        if os.path.isdir(src) and (not file in remove_list):
            shutil.copytree(src, dst, dirs_exist_ok=True)
        elif os.path.isfile(src) and (not file.startswith('@system')):
            shutil.copy(src, dst)


def parse_args(args):
    parser = optparse.OptionParser()
    parser.add_option('--input', help='d.ts document input path')
    parser.add_option('--output', help='d.ts document output path')
    options, _ = parser.parse_args(args)
    return options


def main(argv):
    options = parse_args(argv)
    if not os.path.exists(options.output):
        os.makedirs(options.output)
    copy_files(options.input, options.output)


if __name__ == "__main__":
    exit(main(sys.argv))
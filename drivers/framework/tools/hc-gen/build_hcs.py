#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Copyright (c) 2021, Huawei Device Co., Ltd. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of Willow Garage, Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import fcntl
import os
import sys
import argparse
import platform
import subprocess
import time

_LOCK_FILE_NAME = "._lock"
_BUILD_DIR = 'build'
_LOCK_FILE = ''


def lock():
    global _LOCK_FILE
    global _BUILD_DIR
    global _LOCK_FILE_NAME
    if not os.path.exists(_BUILD_DIR):
        os.mkdir(_BUILD_DIR)
    lock_file = os.path.join(_BUILD_DIR, _LOCK_FILE_NAME)
    if not os.path.exists(lock_file):
        with open(lock_file, 'w') as l_file:
            l_file.write("lock")
            l_file.close
    print('hc-gen lock file ' + lock_file)
    _LOCK_FILE = open(lock_file, 'r')
    fcntl.flock(_LOCK_FILE.fileno(), fcntl.LOCK_EX)


def unlock():
    global _LOCK_FILE
    _LOCK_FILE.close()


def exec_command(cmd):
    process = subprocess.Popen(cmd)
    process.wait()
    ret_code = process.returncode

    if ret_code != 0:
        raise Exception("{} failed, return code is {}".format(cmd, ret_code))


def make_hc_gen(current_dir):
    exec_command(['make', '-C', current_dir, '-j8'])


def prepare(current_dir):
    make_hc_gen(current_dir)


def main(argv):
    lock()
    current_dir = os.path.split(os.path.realpath(__file__))[0]
    hc_gen = os.path.join(current_dir, 'build', 'hc-gen')

    host_hc_gen = argv[1]
    if (os.path.exists(host_hc_gen)):
        build_hcs_cmd = argv[1:]
    else:
        prepare(current_dir)
        build_hcs_cmd = [hc_gen] + argv[1:]

    prepare(current_dir)
    exec_command(build_hcs_cmd)
    unlock()

if __name__ == '__main__':
    sys.exit(main(sys.argv))

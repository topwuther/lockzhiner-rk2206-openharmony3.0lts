#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
#    of conditions and the following disclaimer in the documentation and/or other materials
#    provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used
#    to endorse or promote products derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import sys
import argparse

from command_line.hdf_tool_commands import HdfToolCommands
from hdf_tool_daemon_server import HdfToolDaemonServer
from hdf_tool_exception import HdfToolException
from command_line.hdf_command_error_code import CommandErrorCode


def check_python_version():
    if sys.version_info < (3, 0):
        print('Please run with python version >= 3.0')
        sys.exit(-1)


def main():
    check_python_version()
    commands = HdfToolCommands()
    help_info = 'Tools backend for hdf driver development.'
    arg_parser = argparse.ArgumentParser(description=help_info)
    arg_parser.add_argument('--run_as_daemon', action='store_true')
    arg_parser.add_argument('--server_type', help='command_line or ls_hcs,'
                                                  'default command_line',
                            default='command_line')
    arg_parser.add_argument('command', help=commands.help())
    arg_parser.add_argument('remainder_args', nargs=argparse.REMAINDER)
    args = arg_parser.parse_args()
    if args.run_as_daemon:
        HdfToolDaemonServer(args.server_type).run()
        return
    try:
        ret = commands.run(args.command, args.remainder_args)
        if ret:
            print(ret)
    except HdfToolException as exc:
        print('error: {}, {}'.format(exc.error_code, exc.exc_msg))
    except Exception as exc:
        print('error: {}, {}'.format(CommandErrorCode.UNKNOWN_ERROR, str(exc)))


if __name__ == "__main__":
    main()

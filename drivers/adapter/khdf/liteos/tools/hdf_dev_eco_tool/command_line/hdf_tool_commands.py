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

from .hdf_add_handler import HdfAddHandler
from .hdf_delete_handler import HdfDeleteHandler
from .hdf_get_handler import HdfGetHandler
from .hdf_set_handler import HdfSetHandler
from .hdf_ping_handler import HdfPingHandler
from .hdf_command_error_code import CommandErrorCode
from hdf_tool_exception import HdfToolException


class HdfToolCommands(object):
    def __init__(self):
        self.commands = {
            'add': HdfAddHandler,
            'delete': HdfDeleteHandler,
            'get': HdfGetHandler,
            'set': HdfSetHandler,
            'ping': HdfPingHandler
        }

    def run(self, cmd, args):
        if cmd in self.commands:
            return self.commands[cmd](args).run()
        else:
            raise HdfToolException('unknown cmd: "%s"' % cmd,
                                   CommandErrorCode.INTERFACE_ERROR)

    def help(self):
        helps = ['command list:']
        for cmd in self.commands.keys():
            helps.append(cmd)
        return ' '.join(helps)

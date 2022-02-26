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

import os

from .hdf_command_error_code import CommandErrorCode
from .hdf_tool_argument_parser import HdfToolArgumentParser
from hdf_tool_exception import HdfToolException
import hdf_utils


class HdfCommandHandlerBase(object):
    def __init__(self):
        self.cmd = 'base'
        self.action_type = 'base_action_type'
        self.handlers = {}
        self.args = {}
        self.parser = HdfToolArgumentParser()

    def run(self):
        self.action_type = self._get_action_type()
        if self.action_type in self.handlers:
            return self.handlers[self.action_type]()
        else:
            raise HdfToolException('unknown action_type: "%s" for "%s" cmd' %
                                   (self.action_type, self.cmd),
                                   CommandErrorCode.INTERFACE_ERROR)

    def _get_action_type(self):
        try:
            return getattr(self.args, 'action_type')
        except AttributeError:
            return ''

    def check_arg_raise_if_not_exist(self, arg):
        try:
            value = getattr(self.args, arg)
            if not value:
                raise AttributeError()
        except AttributeError:
            raise HdfToolException('argument "--%s" is required for "%s" cmd' %
                                   (arg, self.cmd),
                                   CommandErrorCode.INTERFACE_ERROR)

    def _get_arg(self, arg):
        try:
            value = getattr(self.args, arg)
            return value
        except AttributeError:
            return ''

    def get_args(self):
        args = ['vendor_name', 'module_name', 'driver_name', 'board_name']
        ret = [self._get_arg('root_dir')]
        for arg in args:
            value = self._get_arg(arg)
            if value:
                value = hdf_utils.WordsConverter(value).lower_case()
            ret.append(value)
        return tuple(ret)

    @staticmethod
    def check_path_raise_if_not_exist(full_path):
        if not os.path.exists(full_path):
            raise HdfToolException('%s not exist' % full_path,
                                   CommandErrorCode.TARGET_NOT_EXIST)

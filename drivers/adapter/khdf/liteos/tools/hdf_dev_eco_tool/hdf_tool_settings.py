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
import json

from hdf_tool_exception import HdfToolException
from command_line.hdf_command_error_code import CommandErrorCode


def singleton(clazz):
    _instances = {}

    def create_instance():
        if clazz not in _instances:
            _instances[clazz] = clazz()
        return _instances[clazz]
    return create_instance


def get_hdf_tool_settings_path():
    cur_dir = os.path.realpath(os.path.dirname(__file__))
    return os.path.join(cur_dir, 'resources', 'settings.json')


@singleton
class HdfToolSettings(object):
    def __init__(self):
        self.file_path = get_hdf_tool_settings_path()
        self.settings = {}
        if not os.path.exists(self.file_path):
            return
        with open(self.file_path) as file:
            try:
                self.settings = json.load(file)
            except ValueError as exc:
                raise HdfToolException('file: %s format wrong, %s' %
                                       (self.file_path, str(exc)),
                                       CommandErrorCode.FILE_FORMAT_WRONG)
        self.supported_boards_key = 'supported_boards'
        self.drivers_path_key = 'drivers_path_relative_to_vendor'
        self.dot_configs_key = 'dot_configs'
        self.board_path_key = 'board_parent_path'
        self.dot_config_path_key = 'dot_config_path'

    def get_supported_boards(self):
        key = self.supported_boards_key
        if key in self.settings:
            return ','.join(self.settings[key].keys())
        return ''

    def get_board_parent_path(self, board_name):
        key = self.supported_boards_key
        board_entry = {}
        if key in self.settings:
            if board_name in self.settings[key]:
                board_entry = self.settings[key][board_name]
        key = self.board_path_key
        return board_entry.get(key, '')

    def get_drivers_path(self):
        key = self.drivers_path_key
        return self.settings.get(key, 'hdf')

    def get_dot_configs(self, board_name):
        key = self.supported_boards_key
        boards = self.settings.get(key, None)
        if not boards:
            return []
        board = boards.get(board_name, None)
        if not board:
            return []
        dot_config_path = board.get(self.dot_config_path_key, '')
        if not dot_config_path:
            return []
        configs = board.get(self.dot_configs_key, [])
        return [os.path.join(dot_config_path, config) for config in configs]

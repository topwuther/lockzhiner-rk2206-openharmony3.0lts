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
from string import Template

from .hdf_manager_config_file import HdfManagerConfigFile
from .hdf_hcs_file import HdfHcsFile
from hdf_tool_settings import HdfToolSettings
from hdf_tool_exception import HdfToolException
from command_line.hdf_command_error_code import CommandErrorCode
import hdf_utils


class HdfDriverConfigFile(object):
    def __init__(self, root, board, module, driver, only_path=False):
        self.root = root
        self.board = board
        self.module = module
        self.driver = driver
        bpp = HdfToolSettings().get_board_parent_path(self.board)
        board_path = os.path.join(self.root, bpp, self.board)
        if not os.path.exists(board_path):
            raise HdfToolException('board: %s not exist' % board_path,
                                   CommandErrorCode.TARGET_NOT_EXIST)
        self.config_dir = os.path.join(board_path, 'config')
        self.drv_dir = os.path.join(self.config_dir, self.module)
        self.drv_config_path = os.path.join(self.drv_dir,
                                            '%s_config.hcs' % self.driver)
        if only_path:
            return
        manager_hcs_path = os.path.join(self.config_dir, 'device_info',
                                        'device_info.hcs')
        self.manager_hcs = HdfManagerConfigFile(manager_hcs_path)
        hdf_hcs_path = os.path.join(self.config_dir, 'hdf.hcs')
        self.hdf_hcs = HdfHcsFile(hdf_hcs_path)

    def _check_and_create_common_config(self):
        self.manager_hcs.check_and_create()
        self.hdf_hcs.check_and_create()
        if not os.path.exists(self.drv_dir):
            os.makedirs(self.drv_dir)

    def create_driver(self):
        self._check_and_create_common_config()
        template_str = hdf_utils.get_template('hdf_driver_config.template')
        config_content = Template(template_str).safe_substitute({})
        hdf_utils.write_file(self.drv_config_path, config_content)
        self.manager_hcs.add_device(self.module, self.driver)
        self.hdf_hcs.add_driver(self.module, self.driver)

    def delete_driver(self):
        if not os.path.exists(self.drv_config_path):
            return
        os.remove(self.drv_config_path)
        self.manager_hcs.delete_device(self.module, self.driver)
        self.hdf_hcs.delete_driver(self.module, self.driver)

    def get_drv_config_path(self):
        if os.path.exists(self.drv_config_path):
            return os.path.realpath(self.drv_config_path)
        else:
            return ''

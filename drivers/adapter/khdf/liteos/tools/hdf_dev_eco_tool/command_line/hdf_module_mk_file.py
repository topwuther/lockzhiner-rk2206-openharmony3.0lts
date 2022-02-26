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
import re
from string import Template

from hdf_tool_exception import HdfToolException
from .hdf_command_error_code import CommandErrorCode
import hdf_utils


class HdfModuleMkFile(object):
    def __init__(self, root, vendor, module):
        self.module = module
        self.file_path = hdf_utils.get_module_mk_path(root, vendor, module)
        if os.path.exists(self.file_path):
            self.contents = hdf_utils.read_file(self.file_path)
        else:
            self.contents = ''

    def _begin_end(self, driver):
        driver_id = hdf_utils.get_id(self.module, driver)
        begin = '\n# <begin %s\n' % driver_id
        end = '\n# %s end>\n' % driver_id
        return begin, end

    def _create_driver_item(self, driver):
        drv_converter = hdf_utils.WordsConverter(driver)
        data_model = {
            'driver_upper_case': drv_converter.upper_case(),
            'driver_lower_case': drv_converter.lower_case()
        }
        template_str = hdf_utils.get_template('hdf_module_mk_driver.template')
        content = Template(template_str).safe_substitute(data_model)
        begin_flag, end_flag = self._begin_end(driver)
        return hdf_utils.SectionContent(begin_flag, content, end_flag)

    def add_driver(self, driver):
        drv_section = self._create_driver_item(driver)
        old_range = hdf_utils.find_section(self.contents, drv_section)
        if old_range:
            hdf_utils.replace_and_save(self.contents, self.file_path,
                                       old_range, drv_section)
            return
        tail_pattern = r'include\s+\$\(HDF_DRIVER\)'
        replacement = '%sinclude $(HDF_DRIVER)' % drv_section.to_str()
        new_content, count = re.subn(tail_pattern, replacement, self.contents)
        if count != 1:
            raise HdfToolException('Makefile: %s has more than one include'
                                   ' $(HDF_DRIVER)' % self.file_path,
                                   CommandErrorCode.FILE_FORMAT_WRONG)
        hdf_utils.write_file(self.file_path, new_content)

    def delete_driver(self, driver):
        begin_flag, end_flag = self._begin_end(driver)
        empty_content = hdf_utils.SectionContent(begin_flag, '', end_flag)
        drv_range = hdf_utils.find_section(self.contents, empty_content)
        if drv_range:
            hdf_utils.delete_and_save(self.contents, self.file_path, drv_range)

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

import hdf_utils


class HdfHcsFile(object):
    def __init__(self, file_path):
        self.file_path = file_path
        self.file_dir = os.path.dirname(self.file_path)
        if os.path.exists(self.file_path):
            self.lines = hdf_utils.read_file_lines(self.file_path)
        else:
            self.lines = []
        self.line_template = '#include "%s/%s_config.hcs"\n'
        self.line_pattern = r'^\s*#include\s+"%s/%s_config.hcs"'
        self.include_pattern = r'^\s*#include'

    def _save(self):
        if self.lines:
            hdf_utils.write_file(self.file_path, ''.join(self.lines))

    def _find_line(self, pattern):
        for index, line in enumerate(self.lines):
            if re.search(pattern, line):
                return index, line
        return 0, ''

    def _find_last_include(self):
        if not self.lines:
            return 0
        i = len(self.lines) - 1
        while i >= 0:
            line = self.lines[i]
            if re.search(self.include_pattern, line):
                return i + 1
            i -= 1
        return 0

    def _create_makefile(self):
        mk_path = os.path.join(self.file_dir, 'Makefile')
        template_str = hdf_utils.get_template('hdf_hcs_makefile.template')
        hdf_utils.write_file(mk_path, template_str)

    def check_and_create(self):
        if self.lines:
            return
        if not os.path.exists(self.file_dir):
            os.makedirs(self.file_dir)
        self._create_makefile()
        self.lines.append('#include "hdf_manager/manager_config.hcs"\n')
        self._save()

    def add_driver(self, module, driver):
        target_line = self.line_template % (module, driver)
        target_pattern = self.line_pattern % (module, driver)
        idx, line = self._find_line(target_pattern)
        if line:
            self.lines[idx] = target_line
        else:
            pos = self._find_last_include()
            self.lines.insert(pos, target_line)
        self._save()

    def delete_driver(self, module, driver):
        target_pattern = self.line_pattern % (module, driver)
        idx, line = self._find_line(target_pattern)
        if not line:
            return
        self.lines[idx] = ''
        self._save()

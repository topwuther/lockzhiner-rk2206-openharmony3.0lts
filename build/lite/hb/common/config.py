#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Copyright (c) 2020 Huawei Device Co., Ltd.
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

import os
import platform
from distutils.spawn import find_executable

from hb import CONFIG_JSON
from hb import CONFIG_STRUCT
from hb import BUILD_TOOLS_CFG
from hb.common.utils import read_json_file
from hb.common.utils import dump_json_file
from hb.common.utils import Singleton
from hb.common.utils import OHOSException
from hb.common.utils import download_tool
from hb.common.utils import makedirs


class Config(metaclass=Singleton):
    def __init__(self):
        self.config_json = CONFIG_JSON

        config_content = read_json_file(self.config_json)
        self._root_path = config_content.get('root_path', None)
        self._board = config_content.get('board', None)
        self._kernel = config_content.get('kernel', None)
        self._product = config_content.get('product', None)
        self._product_path = config_content.get('product_path', None)
        self._device_path = config_content.get('device_path', None)
        self._patch_cache = config_content.get('patch_cache', None)
        self._out_path = None
        self.fs_attr = set()
        self.platform = platform.system()

    @property
    def root_path(self):
        if self._root_path is None:
            raise OHOSException('Please run command "hb set" to '
                                'init OHOS development environment')

        return self._root_path

    @root_path.setter
    def root_path(self, value):
        self._root_path = os.path.abspath(value)
        if not os.path.isdir(self._root_path):
            raise OHOSException(f'{self._root_path} is not a valid path')

        config_path = os.path.join(self._root_path, 'ohos_config.json')
        if not os.path.isfile(config_path):
            self.config_create(config_path)
        self.config_update('root_path', self._root_path)

    @property
    def board(self):
        if self._board is None:
            raise OHOSException('Please run command "hb set" to '
                                'init OHOS development environment')
        return self._board

    @board.setter
    def board(self, value):
        self._board = value
        self.config_update('board', self._board)

    @property
    def kernel(self):
        if self._kernel is None:
            raise OHOSException('Please run command "hb set" to '
                                'init OHOS development environment')
        return self._kernel

    @kernel.setter
    def kernel(self, value):
        self._kernel = value
        self.config_update('kernel', self._kernel)

    @property
    def product(self):
        if self._product is None:
            raise OHOSException('Please run command "hb set" to '
                                'init OHOS development environment')
        return self._product

    @product.setter
    def product(self, value):
        self._product = value
        self.config_update('product', self._product)

    @property
    def product_path(self):
        if self._product_path is None:
            raise OHOSException('Please run command "hb set" to '
                                'init OHOS development environment')
        return self._product_path

    @product_path.setter
    def product_path(self, value):
        self._product_path = value
        self.config_update('product_path', self._product_path)

    @property
    def gn_product_path(self):
        return self.product_path.replace(self.root_path, '/')

    @property
    def device_path(self):
        if self._device_path is None:
            raise OHOSException('Please run command "hb set" to '
                                'init OHOS development environment')
        return self._device_path

    @device_path.setter
    def device_path(self, value):
        self._device_path = value
        self.config_update('device_path', self._device_path)

    @property
    def gn_device_path(self):
        return self.device_path.replace(self.root_path, '/')

    @property
    def build_path(self):
        _build_path = os.path.join(self.root_path, 'build', 'lite')
        if not os.path.isdir(_build_path):
            raise OHOSException(f'Invalid build path: {_build_path}')
        return _build_path

    @property
    def out_path(self):
        return self._out_path

    @out_path.setter
    def out_path(self, value):
        self._out_path = value

    @property
    def log_path(self):
        return os.path.join(self.out_path, 'build.log')

    @property
    def vendor_path(self):
        _vendor_path = os.path.join(self.root_path, 'vendor')
        if not os.path.isdir(_vendor_path):
            raise OHOSException(f'Invalid vendor path: {_vendor_path}')
        return _vendor_path

    @property
    def build_tools_path(self):
        try:
            tools_path = BUILD_TOOLS_CFG[self.platform]['build_tools_path']
            return os.path.join(self.root_path, tools_path)
        except KeyError:
            raise OHOSException(f'unidentified platform: {self.platform}')

    @property
    def gn_path(self):
        repo_gn_path = os.path.join(self.build_tools_path, 'gn')
        # gn exist.
        if os.path.isfile(repo_gn_path):
            return repo_gn_path

        # gn not install, download and extract it.
        makedirs(self.build_tools_path, exist_ok=True)

        gn_url = BUILD_TOOLS_CFG[self.platform].get('gn')
        gn_dst = os.path.join(self.build_tools_path, 'gn_pkg')
        download_tool(gn_url, gn_dst, tgt_dir=self.build_tools_path)

        return repo_gn_path

    @property
    def ninja_path(self):
        repo_ninja_path = os.path.join(self.build_tools_path, 'ninja')
        # ninja exist.
        if os.path.isfile(repo_ninja_path):
            return repo_ninja_path

        # ninja not install, download and extract.
        makedirs(self.build_tools_path, exist_ok=True)

        ninja_url = BUILD_TOOLS_CFG[self.platform].get('ninja')
        ninja_dst = os.path.join(self.build_tools_path, 'ninja_pkg')
        download_tool(ninja_url, ninja_dst, tgt_dir=self.build_tools_path)

        return repo_ninja_path

    @property
    def clang_path(self):
        repo_clang_path = os.path.join('prebuilts',
                                       'clang',
                                       'ohos',
                                       'linux-x86_64',
                                       'llvm')
        # clang exist
        if os.path.isdir(repo_clang_path):
            return f'//{repo_clang_path}'
        # clang installed manually or auto download
        else:
            # already installed manually
            env_clang_bin_path = find_executable('clang')
            if env_clang_bin_path is not None:
                env_clang_path = os.path.abspath(os.path.join(env_clang_bin_path,
                                                              os.pardir,
                                                              os.pardir))

                if os.path.basename(env_clang_path) == 'llvm':
                    return env_clang_path

            # need auto download and extract clang.
            clang_path = os.path.abspath(os.path.join(repo_clang_path,
                                                      os.pardir))
            makedirs(clang_path, exist_ok=True)

            clang_url = BUILD_TOOLS_CFG[self.platform].get('clang')
            clang_dst = os.path.join(clang_path, 'clang_pkg')
            download_tool(clang_url, clang_dst, tgt_dir=clang_path)
            return f'//{repo_clang_path}'

    @property
    def patch_cache(self):
        return self._patch_cache

    @patch_cache.setter
    def patch_cache(self, value):
        self._patch_cache = value
        self.config_update('patch_cache', self._patch_cache)

    def config_create(self, config_path):
        dump_json_file(config_path, CONFIG_STRUCT)
        self.config_json = config_path

    def config_update(self, key, value):
        config_content = read_json_file(self.config_json)
        config_content[key] = value
        dump_json_file(self.config_json, config_content)

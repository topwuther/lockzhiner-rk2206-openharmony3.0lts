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
#

import os
import sys
from collections import defaultdict

from hb.common.utils import exec_command
from hb.common.utils import makedirs
from hb.common.utils import remove_path
from hb.common.utils import hb_info
from hb.common.utils import hb_warning
from hb.common.utils import OHOSException
from hb.common.utils import get_current_time
from hb.common.config import Config
from hb.cts.cts import CTS
from hb.common.device import Device
from hb.common.product import Product
from hb.build.fs_process import Packer
from hb.build.patch_process import Patch
from distutils.spawn import find_executable


class Build():
    def __init__(self, component=None):
        self.config = Config()

        # Get gn args ready
        self._args_list = []
        self._target = None
        self._compiler = None
        self._test = None

        self.target = component
        self.start_time = get_current_time()
        self.check_in_device()

    @property
    def target(self):
        return self._target

    @target.setter
    def target(self, component):
        if component is None or not len(component):
            return
        component = component[0]

        cts = CTS()
        cts.init_from_json()
        for subsystem_cls in cts:
            for cname, component_cls in subsystem_cls:
                if cname == component:
                    if not len(component_cls.adapted_board) or\
                       self.config.board in component_cls.adapted_board:
                        if not len(component_cls.adapted_kernel) or\
                           self.config.kernel in component_cls.adapted_kernel:
                            self._target = component_cls.targets
                            self.register_args('ohos_build_target',
                                               self._target)
                            return

        raise OHOSException(f'Component {component} not found')

    @property
    def compiler(self):
        return self._compiler

    @compiler.setter
    def compiler(self, value):
        self._compiler = value
        self.register_args('ohos_build_compiler_specified', self._compiler)
        if self._compiler == 'clang':
            self.register_args('ohos_build_compiler_dir',
                               self.config.clang_path)

    @property
    def test(self):
        return self._test

    @test.setter
    def test(self, test_args):
        cmd_list = ['xts', 'notest']
        if test_args[0] in cmd_list:
            if test_args[0] == 'notest':
                self.register_args('ohos_test_args', 'notest')
            else:
                self._test = test_args[1]
                if len(test_args) > 1:
                    self.register_args('ohos_xts_test_args', self._test)
        else:
            raise OHOSException('Error: wrong input of test')

    @property
    def build_time(self):
        return get_current_time() - self.start_time

    def register_args(self, args_name, args_value, quota=True):
        quota = False if args_value in ['true', 'false'] else quota
        if quota:
            if isinstance(args_value, list):
                self._args_list += ['{}="{}"'.format(args_name,
                                                     "&&".join(args_value))]
            else:
                self._args_list += ['{}="{}"'.format(args_name, args_value)]
        else:
            self._args_list += ['{}={}'.format(args_name, args_value)]
        if args_name == 'ohos_build_target' and len(args_value):
            self.config.fs_attr = None

    def build(self, full_compile, patch=False, ninja=True, cmd_args=None):
        cmd_list = self.get_cmd(full_compile, patch, ninja)

        # enable ccache if it installed.
        ccache_path = find_executable('ccache')
        if ccache_path is not None:
            self.register_args('ohos_build_enable_ccache', 'true',  quota=False)

        if cmd_args is None:
            cmd_args = defaultdict(list)
        for exec_cmd in cmd_list:
            exec_cmd(cmd_args)

        hb_info(f'{os.path.basename(self.config.out_path)} build success')
        hb_info(f'cost time: {self.build_time}')
        return 0

    def get_cmd(self, full_compile, patch, ninja):
        if not ninja:
            self.register_args('ohos_full_compile', 'true', quota=False)
            return [self.gn_build]

        cmd_list = []

        build_ninja = os.path.join(self.config.out_path, 'build.ninja')
        if not os.path.isfile(build_ninja):
            self.register_args('ohos_full_compile', 'true', quota=False)
            makedirs(self.config.out_path)
            cmd_list = [self.gn_build, self.ninja_build]
        elif full_compile:
            self.register_args('ohos_full_compile', 'true', quota=False)
            remove_path(self.config.out_path)
            makedirs(self.config.out_path)
            cmd_list = [self.gn_build, self.ninja_build]
        else:
            self.register_args('ohos_full_compile', 'false', quota=False)
            cmd_list = [self.ninja_build]

        if patch:
            patch = Patch()
            cmd_list = [patch.patch_make] + cmd_list

        if self.config.fs_attr is not None:
            packer = Packer()
            cmd_list.append(packer.fs_make)

        return cmd_list

    def gn_build(self, cmd_args):
        # Clean out path
        remove_path(self.config.out_path)
        makedirs(self.config.out_path)

        # Gn cmd init and execute
        gn_path = self.config.gn_path
        gn_args = cmd_args.get('gn', [])
        gn_cmd = [gn_path,
                  'gen',
                  self.config.out_path,
                  '--root={}'.format(self.config.root_path),
                  '--dotfile={}/.gn'.format(self.config.build_path),
                  f'--script-executable={sys.executable}',
                  '--args={}'.format(" ".join(self._args_list))] + gn_args
        exec_command(gn_cmd, log_path=self.config.log_path)

    def gn_clean(self, out_path=None):
        # Gn cmd init and execute
        gn_path = self.config.gn_path

        if out_path is not None:
            self.config.out_path = os.path.abspath(out_path)
        else:
            self.config.out_path = os.path.join(self.config.root_path,
                                                'out',
                                                self.config.board,
                                                self.config.product)

        if not os.path.isdir(self.config.out_path):
            hb_warning('{} not found'.format(self.config.out_path))
            return

        gn_cmd = [gn_path,
                  '--root={}'.format(self.config.root_path),
                  '--dotfile={}/.gn'.format(self.config.build_path),
                  'clean',
                  self.config.out_path]
        exec_command(gn_cmd, log_path=self.config.log_path)

    def ninja_build(self, cmd_args):
        ninja_path = self.config.ninja_path

        ninja_args = cmd_args.get('ninja', [])
        ninja_cmd = [ninja_path,
                     '-w',
                     'dupbuild=warn',
                     '-C',
                     self.config.out_path] + ninja_args
        exec_command(ninja_cmd, log_path=self.config.log_path, log_filter=True)

    def check_in_device(self):
        if self._target is None and Device.is_in_device():
            # Compile device board
            device_path, kernel, board = Device.device_menuconfig()
            hb_info(f'{device_path}')
            # build device, no need to set root manually,
            # so set it speculatively.
            self.config.root_path = os.path.abspath(os.path.join(device_path,
                                                                 os.pardir,
                                                                 os.pardir,
                                                                 os.pardir,
                                                                 os.pardir))
            self.config.out_path = os.path.join(self.config.root_path,
                                                'out',
                                                board)
            self.compiler = Device.get_compiler(device_path)
            gn_device_path = os.path.dirname(device_path)
            gn_kernel_path = device_path
            self.register_args('ohos_build_target', [gn_device_path])
            self.register_args('device_path', gn_kernel_path)
            self.register_args('ohos_kernel_type', kernel)
        else:
            # Compile product in "hb set"
            self.compiler = Device.get_compiler(self.config.device_path)
            self.register_args('product_path', self.config.product_path)
            self.register_args('device_path', self.config.device_path)
            self.register_args('ohos_kernel_type', self.config.kernel)

            product_json = os.path.join(self.config.product_path,
                                        'config.json')
            self._args_list += Product.get_features(product_json)
            self.config.out_path = os.path.join(self.config.root_path,
                                                'out',
                                                self.config.board,
                                                self.config.product)

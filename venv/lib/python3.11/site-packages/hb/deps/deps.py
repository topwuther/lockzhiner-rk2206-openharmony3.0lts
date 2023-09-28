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


from hb.deps.check_deps import check_deps
from hb.deps.gen_deps import gen_deps
from hb.common.utils import OHOSException


def add_options(parser):
    parser.add_argument('deps_type',
                        help='select the operation for deps (check or gen)',
                        nargs=1,
                        default=[])
    parser.add_argument('-s', '--subsystems', help='involved subsystems',
                        nargs='*', default=[])
    parser.add_argument('-p', '--products', help='involved products',
                        nargs='*', default=[])
    parser.add_argument('--work_path', nargs=1)


def exec_command(args):
    if len(args.deps_type):
        if args.deps_type[0] == 'check':
            return check_deps(args.subsystems,
                              args.products,
                              args.work_path[0])
        elif args.deps_type[0] == 'gen':
            return gen_deps(args.subsystems, args.products)

    raise OHOSException('please select the operation for deps (check or gen)')

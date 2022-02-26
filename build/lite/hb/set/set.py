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

from hb.common.config import Config
from hb.common.utils import get_input
from hb.common.utils import OHOSException
from hb.common.utils import hb_info
from hb.common.product import Product
from hb.common.device import Device


def add_options(parser):
    parser.add_argument('-root', '--root_path',
                        nargs='?',
                        default=None,
                        help='Set OHOS root path')
    parser.add_argument('-p', '--product',
                        action='store_true',
                        help='Set OHOS board and kernel')


def exec_command(args):
    if args.root_path is not None:
        return set_root_path(root_path=args.root_path)

    if args.product:
        return set_product()

    return set_root_path() == 0 and set_product() == 0


def set_root_path(root_path=None):
    config = Config()
    if root_path is None:
        try:
            hb_info(f'hb root path: {config.root_path}')
            return 0
        except OHOSException:
            root_path = get_input('[OHOS INFO] Input code path: ')
    config.root_path = root_path
    return 0


def set_product(product_name=None, company=None):
    if product_name is None or company is None:
        product_path, product_name = Product.product_menuconfig()
    else:
        product_path = Product.get_product_path(product_name, company)

    config = Config()
    config.product = product_name
    config.product_path = product_path

    product_json = os.path.join(config.product_path, 'config.json')
    config.board, config.kernel, kernel_version, dev_company =\
        Product.get_device_info(product_json)

    board_path = os.path.join(config.root_path, 'device',
                              dev_company, config.board)
    config.device_path = Device.get_device_path(board_path,
                                                config.kernel,
                                                kernel_version)

    return 0

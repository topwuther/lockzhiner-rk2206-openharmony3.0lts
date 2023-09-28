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
from collections import defaultdict

from hb.common.product import Product
from hb.build.build import exec_command
from hb.common.utils import args_factory
from hb.common.utils import dump_json_file
from hb.common.utils import hb_info
from hb.common.config import Config
from hb.cts.cts import CTS
from hb.set.set import set_product
from hb.set.set import set_root_path


def gen_deps(subsystems, products):
    args = {
        'component': '',
        'build_type': ['release'],
        'compiler': [],
        'dmverity': False,
        'test': None,
        'product': '',
        'full': True,
        'ndk': False,
        'ninja': False
    }
    build_result_dict = defaultdict(list)
    config = Config()
    src_path = os.getcwd()
    set_root_path(root_path=src_path)

    cts = CTS()
    cts.init_from_json()

    for cur_company, cur_product, product_path in Product.get_products():
        if len(products) and cur_product not in products:
            continue

        set_product(cur_product, cur_company)
        hb_info(f'now product: {cur_product}')

        config.out_path = os.path.join(config.root_path,
                                       'out',
                                       config.board,
                                       config.product)
        product_json = os.path.join(product_path, 'config.json')

        components_dict, cts.board, cts.kernel =\
            Product.get_components(product_json, subsystems)
        cts.update_subsystems_product()

        try:
            status = exec_command(args_factory(args))
        except Exception:
            status = 1
            with open(config.log_path, 'rt', encoding='utf-8') as log_file:
                log = log_file.read()
        else:
            log = ''
            for sname, cname_list in components_dict.items():
                for cname in cname_list:
                    for subsystem_cls in cts:
                        for now_cname, now_component_cls in subsystem_cls:
                            if cname == now_cname:
                                now_component_cls.get_deps(cts.board,
                                                           cts.kernel,
                                                           config.out_path,
                                                           cts.comp_fields,
                                                           cts.fields)

        build_result_dict[f'{cur_product}@{cur_company}'].\
            append({"status": status ^ 1, "log": log})

    cts.update_subsystems_platform(subsystems)

    component_build_file = os.path.join(src_path,
                                        'gen_deps.json')
    dump_json_file(component_build_file, build_result_dict)

    return 0

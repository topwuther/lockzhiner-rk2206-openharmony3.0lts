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
from hb.common.config import Config
from hb.cts.cts import CTS
from hb.set.set import set_product
from hb.set.set import set_root_path


def check_deps(subsystems, products, work_path):
    args = {
        'component': '',
        'build_type': ['debug'],
        'compiler': [],
        'dmverity': False,
        'test': None,
        'product': '',
        'full': True,
        'ndk': False
    }
    build_result_dict = defaultdict(list)
    config = Config()
    src_path = os.getcwd()
    set_root_path(root_path=src_path)

    for cur_company, cur_product, product_path in Product.get_products():
        if len(products) and cur_product not in products:
            continue

        set_product(cur_product, cur_company)
        product_json = os.path.join(product_path, 'config.json')

        cts = CTS()
        cts.init_from_json()
        components_dict, cts.board, cts.kernel =\
            Product.get_components(product_json, subsystems)
        cts.update_subsystems_product()

        for sname, cname_list in components_dict.items():
            for cname in cname_list:
                args['component'] = [cname]
                for subsystem_cls in cts:
                    for now_cname, now_component_cls in subsystem_cls:
                        if cname == now_cname:
                            now_component_cls.dirs += [
                                config.product_path.replace(config.root_path,
                                                            '')[1:],
                                os.path.dirname(config.device_path).
                                replace(config.root_path, '')[1:]
                            ]
                            now_component_cls.get_deps_ready(work_path,
                                                             src_path)

                            set_root_path(root_path=work_path)
                            try:
                                status = exec_command(args_factory(args))
                            except Exception:
                                status = 1
                            set_root_path(root_path=src_path)
                            now_component_cls.remove_copy_dirs(work_path)

                            if status == 1:
                                with open(config.log_path, 'rt') as log_file:
                                    log = log_file.read()
                            else:
                                log = ''
                            build_result_dict[sname].append({
                                    "component_name": cname,
                                    "product": f'{cur_product}@{cur_company}',
                                    "status": status ^ 1,
                                    "log": log
                            })

    component_build_file = os.path.join(work_path,
                                        'component_build.json')
    dump_json_file(component_build_file, build_result_dict)

    return 0

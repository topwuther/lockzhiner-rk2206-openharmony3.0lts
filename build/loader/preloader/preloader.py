#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2021 Huawei Device Co., Ltd.
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
import sys
import argparse

sys.path.append(
    os.path.dirname(os.path.dirname(os.path.dirname(
        os.path.abspath(__file__)))))
from scripts.util.file_utils import read_json_file, write_json_file, write_file  # noqa: E402, E501


def _get_product_config(config_dir, product_name):
    config_file = os.path.join(config_dir, '{}.json'.format(product_name))
    if not os.path.exists(config_file):
        raise Exception(
            "product configuration '{}' doesn't exist.".format(config_file))
    return config_file


def _get_base_parts(base_config_dir, system_type):
    system_base_config_file = os.path.join(
        base_config_dir, '{}_system.json'.format(system_type))
    if not os.path.exists(system_base_config_file):
        raise Exception("product configuration '{}' doesn't exist.".format(
            system_base_config_file))
    data = read_json_file(system_base_config_file)
    if data is None:
        raise Exception(
            "read file '{}' failed.".format(system_base_config_file))
    return data


def _get_inherit_parts(inherit_config, current_path):
    _parts = {}
    for _config in inherit_config:
        _file = os.path.join(current_path, _config)
        _info = read_json_file(_file)
        if _info is None:
            raise Exception("read file '{}' failed.".format(_file))
        if _info.get('parts'):
            _parts.update(_info.get('parts'))
    return _parts


def _get_device_info(device_name, device_config_path):
    device_config_file = os.path.join(device_config_path,
                                      '{}.json'.format(device_name))
    device_info = read_json_file(device_config_file)
    if device_info.get('device_name') != device_name:
        raise Exception("device name configuration incorrect in '{}'".format(device_config_file))
    return device_info


def _parse_config_v2(config_info, products_config_path, base_config_dir,
                     device_config_path):
    system_type = config_info.get("type")
    base_parts = _get_base_parts(base_config_dir, system_type)
    all_parts = base_parts

    inherit_config = config_info.get('inherit')
    if inherit_config:
        inherit_parts = _get_inherit_parts(inherit_config,
                                           products_config_path)
        all_parts.update(inherit_parts)

    current_product_parts = config_info.get("parts")
    all_parts.update(current_product_parts)

    product_name = config_info.get('product_name')
    product_company = config_info.get('product_company')
    product_device_name = config_info.get('product_device')
    device_info = _get_device_info(product_device_name, device_config_path)
    build_configs = {}
    build_configs['system_type'] = system_type
    build_configs['product_name'] = product_name
    build_configs['product_company'] = product_company
    build_configs['device_name'] = product_device_name
    build_configs.update(device_info)
    return all_parts, build_configs


def _parse_config_v1(config_info):
    build_configs = {"system_type": 'large'}
    return {}, build_configs


def _parse_config(product_name, products_config_path, base_parts_config_path,
                  device_config_path):
    curr_config_file = _get_product_config(products_config_path, product_name)
    config_info = read_json_file(curr_config_file)
    config_version = config_info.get('version')
    if not config_version:
        config_version = "1.0"
    if config_version == "2.0":
        if product_name != config_info.get('product_name'):
            raise Exception("product name configuration incorrect in '{}'".format(curr_config_file))
        return _parse_config_v2(config_info, products_config_path,
                                base_parts_config_path, device_config_path)
    else:
        return _parse_config_v1(config_info)


def _copy_platforms_config(platforms_template, parts_info_file,
                           platform_config_output_path):
    if not os.path.exists(platforms_template):
        raise Exception(
            "template file '{}' doesn't exist.".format(platforms_template))
    data = read_json_file(platforms_template)
    if data is None:
        raise Exception("read file '{}' failed.".format(platforms_template))
    _parts_config_file = os.path.relpath(parts_info_file,
                                         platform_config_output_path)
    for _, _config in data.get('platforms').items():
        for _info in _config:
            _info['parts_config'] = _parts_config_file
    output_file = os.path.join(platform_config_output_path, 'platforms.build')
    write_json_file(output_file, data)


def _get_platform_template_file(source_root_dir):
    platforms_template = os.path.join(source_root_dir,
                                      'build/loader/preloader',
                                      'platforms.template')
    return platforms_template


def _get_merge_subsystem_config(product_config_path, device_config_path, 
                                subsystem_config_file, output_dir, 
                                product_name):
    product_config_file = os.path.join(product_config_path,
                                      '{}.json'.format(product_name))
    output_file = os.path.join(output_dir, 'subsystem_config.json')
    subsystem_info = read_json_file(subsystem_config_file)

    product_subsystem_info = {}
    product_info = read_json_file(product_config_file)
    product_build_path = product_info.get('product_build_path', 'no_path')
    if product_build_path != 'no_path' and product_build_path != '':
        product_subsystem_info['path'] = product_build_path
        product_subsystem_name = "product_{}".format(product_name)
        product_subsystem_info['name'] = product_subsystem_name
        subsystem_info[product_subsystem_name] = product_subsystem_info

    product_device_name = product_info.get('product_device')
    device_info = _get_device_info(product_device_name, device_config_path)

    device_subsystem_info = {}
    device_build_path = device_info.get('device_build_path', 'no_path')
    if device_build_path != 'no_path' and device_build_path != '':
        device_subsystem_info['path'] = device_build_path
        device_subsystem_name = "device_{}".format(product_device_name)
        device_subsystem_info['name'] = device_subsystem_name
        subsystem_info[device_subsystem_name] = device_subsystem_info
    write_json_file(output_file, subsystem_info)


def _run(args):
    products_config_path = os.path.join(args.source_root_dir,
                                        args.products_config_dir)
    product_config_root_path = os.path.dirname(products_config_path)
    if args.base_parts_config_dir:
        base_parts_config_path = os.path.join(args.source_root_dir,
                                              args.base_parts_config_dir)
    else:
        base_parts_config_path = os.path.join(product_config_root_path, 'base')
    if args.device_config_dir:
        device_config_path = os.path.join(args.source_root_dir,
                                          args.device_config_dir)
    else:
        device_config_path = os.path.join(product_config_root_path, 'device')

    all_parts, build_configs = _parse_config(args.product_name,
                                             products_config_path,
                                             base_parts_config_path,
                                             device_config_path)

    system_type = build_configs.get('system_type')
    if system_type not in ['standard', 'large']:
        raise Exception("product config incorrect.")

    product_info_output_path = os.path.join(args.source_root_dir,
                                            args.preloader_output_root_dir,
                                            args.product_name, 'preloader')
    platform_config_output_path = os.path.join(args.source_root_dir,
                                               args.preloader_output_root_dir,
                                               '{}_system'.format(system_type))

    parts_info_file = os.path.join(product_info_output_path, 'parts.json')
    parts_config_info = {"parts": list(all_parts.keys())}
    write_json_file(parts_info_file, parts_config_info)

    platforms_template_file = _get_platform_template_file(args.source_root_dir)
    _copy_platforms_config(platforms_template_file, parts_info_file,
                           platform_config_output_path)

    _build_info_list = []
    build_info_file = os.path.join(product_info_output_path, 'build.prop')
    for k, v in build_configs.items():
        _build_info_list.append('{}={}'.format(k, v))
    write_file(build_info_file, '\n'.join(_build_info_list))
    build_info_json_file = os.path.join(product_info_output_path, 'build_config.json')
    write_json_file(build_info_json_file, build_configs)

    subsystem_config_file = os.path.join(args.source_root_dir,
                                         'build',
                                         'subsystem_config.json')
    output_dir = os.path.join(args.source_root_dir,
                              args.preloader_output_root_dir)
    _get_merge_subsystem_config(products_config_path, device_config_path, 
                                subsystem_config_file, output_dir, 
                                args.product_name)


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument('--product-name', required=True)
    parser.add_argument('--source-root-dir', required=True)
    parser.add_argument('--products-config-dir', required=True)
    parser.add_argument('--base-parts-config-dir')
    parser.add_argument('--device-config-dir')
    parser.add_argument('--preloader-output-root-dir', required=True)
    args = parser.parse_args(argv)
    _run(args)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

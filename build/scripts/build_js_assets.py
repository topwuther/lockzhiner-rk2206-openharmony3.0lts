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

import optparse
import os
import sys
import tempfile
import json
import shutil

from util import build_utils  # noqa: E402


def parse_args(args):
    args = build_utils.expand_file_args(args)

    parser = optparse.OptionParser()
    build_utils.add_depfile_option(parser)
    parser.add_option('--output', help='stamp file')
    parser.add_option('--js-assets-dir', help='js assets directory')
    parser.add_option('--nodejs-path', help='path to nodejs app')
    parser.add_option('--webpack-js', help='path to webpack.js')
    parser.add_option('--webpack-config-js', help='path to webpack.config.js')
    parser.add_option('--hap-profile', help='path to hap profile')
    parser.add_option('--build-mode', help='debug mode or release mode')
    parser.add_option('--js2abc',
                      action='store_true',
                      default=False,
                      help='whether to transform js to ark bytecode')

    options, _ = parser.parse_args(args)
    options.js_assets_dir = build_utils.parse_gn_list(options.js_assets_dir)
    return options


def build_ace(cmd, options):
    with build_utils.temp_dir() as build_dir:
        gen_dir = os.path.join(build_dir, 'gen')
        manifest = os.path.join(build_dir, 'manifest.json')
        my_env = {
            "aceModuleRoot": options.js_assets_dir[0],
            "aceModuleBuild": gen_dir,
            "aceManifestPath": manifest,
            "buildMode": options.build_mode,
            "PATH": os.environ.get('PATH'),
        }
        if not os.path.exists(manifest) and options.hap_profile:
            with open(options.hap_profile) as profile:
                config = json.load(profile)
                data = dict()
                data['appID'] = config['app']['bundleName']
                data['appName'] = config['module']['abilities'][0]['label']
                data['versionName'] = config['app']['version']['name']
                data['versionCode'] = config['app']['version']['code']
                data['pages'] = config['module']['js'][0]['pages']
                data['deviceType'] = config['module']['deviceType']
                data['window'] = config['module']['js'][0]['window']
                build_utils.write_json(data, manifest)
        build_utils.check_output(cmd, env=my_env)
        for root, _, files in os.walk(gen_dir):
            for file in files:
                filename = os.path.join(root, file)
                if filename.endswith('.js.map'):
                    os.unlink(filename)
        build_utils.zip_dir(options.output,
                            gen_dir,
                            zip_prefix_path='assets/js/default/')


def main(args):
    options = parse_args(args)

    inputs = ([
        options.nodejs_path, options.webpack_js, options.webpack_config_js
    ])
    depfiles = (build_utils.get_all_files(options.js_assets_dir[0]))

    cmd = [
        options.nodejs_path,
        options.webpack_js,
        '--config',
        options.webpack_config_js,
    ]
    if options.js2abc:
        cmd.extend(['--compilerType', 'ark'])

    build_utils.call_and_write_depfile_if_stale(
        lambda: build_ace(cmd, options),
        options,
        depfile_deps=depfiles,
        input_paths=depfiles + inputs,
        input_strings=cmd + [options.build_mode],
        output_paths=([options.output]),
        force=False,
        add_pydeps=False)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

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
import subprocess
import sys
import shutil
import os
import tempfile
from util import build_utils  # noqa: E402


def sign_hap(hapsigner, private_key_path, sign_algo, certificate_profile,
             keystore_path, keystorepasswd, keyalias, certificate_file,
             unsigned_hap_path, signed_hap_path):
    cmd = ['java', '-jar', hapsigner, 'sign']
    cmd.extend(['-mode', 'localjks'])
    cmd.extend(['-signAlg', sign_algo])
    cmd.extend(['-privatekey', private_key_path])
    cmd.extend(['-inputFile', unsigned_hap_path])
    cmd.extend(['-outputFile', signed_hap_path])
    cmd.extend(['-profile', certificate_profile])
    cmd.extend(['-keystore', keystore_path])
    cmd.extend(['-keystorepasswd', keystorepasswd])
    cmd.extend(['-keyaliaspasswd', keyalias])
    cmd.extend(['-certpath', certificate_file])
    cmd.extend(['-profileSigned', '1'])
    child = subprocess.Popen(cmd,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    stdout, stderr = child.communicate()
    if child.returncode:
        print(stdout.decode(), stderr.decode())
        raise Exception("Failed to sign hap")


def create_hap(options, signed_hap):
    with build_utils.temp_dir() as package_dir, tempfile.NamedTemporaryFile(
            suffix='.hap') as output:
        packing_cmd = ['java', '-jar', options.hap_packing_tool]
        packing_cmd.extend(
            ['--mode', 'hap', '--force', 'true', '--out-path', output.name])

        hap_profile_path = os.path.join(package_dir,
                                        os.path.basename(options.hap_profile))
        shutil.copy(options.hap_profile, hap_profile_path)
        packing_cmd.extend(['--json-path', hap_profile_path])

        if options.dso:
            lib_path = os.path.join(package_dir, "lib")
            os.mkdir(lib_path)
            for dso in options.dso:
                shutil.copy(dso, lib_path)
            packing_cmd.extend(['--lib-path', lib_path])

        if options.packaged_resources:
            build_utils.extract_all(options.packaged_resources,
                                    package_dir,
                                    no_clobber=False)
            packing_cmd.extend(
                ['--index-path',
                 os.path.join(package_dir, 'resources.index')])
            packing_cmd.extend(
                ['--res-path',
                 os.path.join(package_dir, 'resources')])

        assets_dir = os.path.join(package_dir, 'assets')
        if options.packaged_js_assets or options.assets:
            packing_cmd.extend(['--assets-path', assets_dir])

        if options.packaged_js_assets:
            build_utils.extract_all(options.packaged_js_assets,
                                    package_dir,
                                    no_clobber=False)
        if options.assets:
            if not os.path.exists(assets_dir):
                os.mkdir(assets_dir)
            for dire in options.assets:
                shutil.copytree(
                    dire,
                    os.path.join(assets_dir, os.path.basename(dire)))

        build_utils.check_output(packing_cmd)

        sign_hap(options.hapsigner, options.private_key_path,
                 options.sign_algo, options.certificate_profile,
                 options.keystore_path, options.keystorepasswd,
                 options.keyalias, options.certificate_file, output.name,
                 signed_hap)


def parse_args(args):
    args = build_utils.expand_file_args(args)

    parser = optparse.OptionParser()
    build_utils.add_depfile_option(parser)
    parser.add_option('--hap-path', help='path to output hap')
    parser.add_option('--hapsigner', help='path to signer')
    parser.add_option('--assets', help='path to assets')
    parser.add_option('--dso',
                      action="append",
                      help='path to dynamic shared objects')
    parser.add_option('--hap-profile', help='path to hap profile')
    parser.add_option('--hap-packing-tool', help='path to hap packing tool')
    parser.add_option('--private-key-path', help='path to private key')
    parser.add_option('--sign-algo', help='signature algorithm')
    parser.add_option('--certificate-profile',
                      help='path to certificate profile')
    parser.add_option('--keyalias', help='keyalias')
    parser.add_option('--keystore-path', help='path to keystore')
    parser.add_option('--keystorepasswd', help='password of keystore')
    parser.add_option('--certificate-file', help='path to certificate file')
    parser.add_option('--packaged-resources',
                      help='path to packaged resources')
    parser.add_option('--packaged-js-assets',
                      help='path to packaged js assets')

    options, _ = parser.parse_args(args)
    if options.assets:
        options.assets = build_utils.parse_gn_list(options.assets)
    return options


def main(args):
    options = parse_args(args)

    inputs = [
        options.hap_profile, options.packaged_js_assets,
        options.packaged_resources, options.certificate_file,
        options.keystore_path, options.certificate_profile
    ]
    depfiles = []
    for dire in options.assets:
        depfiles += (build_utils.get_all_files(dire))
    if options.dso:
        depfiles.extend(options.dso)

    build_utils.call_and_write_depfile_if_stale(
        lambda: create_hap(options, options.hap_path),
        options,
        depfile_deps=depfiles,
        input_paths=inputs + depfiles,
        input_strings=[
            options.keystorepasswd, options.keyalias, options.sign_algo,
            options.private_key_path
        ],
        output_paths=([options.hap_path]),
        force=False,
        add_pydeps=False)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

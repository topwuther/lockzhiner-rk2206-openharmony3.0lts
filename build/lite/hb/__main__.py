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
sys.path.insert(0, os.path.abspath(os.path.join(__file__,
                                                os.pardir,
                                                os.pardir)))
import argparse
import importlib
import traceback

from hb import VERSION
from hb.common.utils import hb_warning
from hb.common.utils import hb_error
from hb.common.utils import OHOSException


def main():
    parser = argparse.ArgumentParser(description='OHOS Build System '
                                                 f'version {VERSION}')
    parser.add_argument('-v', '--version',
                        action='version',
                        version=f'[OHOS INFO] hb version {VERSION}')

    subparsers = parser.add_subparsers()
    parser_list = []
    parser_list.append({
        'name': 'build',
        'help': 'Build source code'
    })

    parser_list.append({
        'name': 'set',
        'help': 'OHOS build settings'
    })

    parser_list.append({
        'name': 'env',
        'help': 'Show OHOS build env'
    })

    parser_list.append({
        'name': 'clean',
        'help': 'Clean output'
    })

    parser_list.append({
        'name': 'deps',
        'help': 'OHOS components deps'
    })

    for each in parser_list:
        module_parser = subparsers.add_parser(name=each.get('name'),
                                              help=each.get('help'))
        module = importlib.import_module('.{}'.format(each.get('name')),
                                         'hb.{}'.format(each.get('name')))
        module.add_options(module_parser)
        module_parser.set_defaults(parser=module_parser,
                                   command=module.exec_command)

    args = parser.parse_args()

    try:
        status = args.command(args)
    except KeyboardInterrupt:
        hb_warning('User Abort')
        status = -1
    except OHOSException as exception:
        hb_error(exception.args[0])
        status = -1
    except Exception as exception:
        if not hasattr(args, 'command'):
            parser.print_help()
        else:
            hb_error(traceback.format_exc())
            hb_error(f'Unhandled error: {exception}')
        status = -1

    return status


if __name__ == "__main__":
    sys.exit(main())

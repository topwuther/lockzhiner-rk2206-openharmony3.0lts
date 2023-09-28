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

from hb.cts.prompt import prompt
from hb.cts.common import get_style


class Menuconfig():
    def _promt(self, questions, **kwargs):
        return prompt(questions, style=get_style('answer'), **kwargs)

    def list_promt(self, name, message, choices, **kwargs):
        questions = self.get_questions('list', name, message, choices)

        return self._promt(questions, **kwargs)

    def checkbox_promt(self, name, message, choices, **kwargs):
        questions = self.get_questions('checkbox', name, message, choices)

        return self._promt(questions, **kwargs)

    def get_questions(self, promt_type, name, message, choices):
        questions = [
            {
                'type': promt_type,
                'qmark': 'OHOS',
                'name': name,
                'message': message,
                'choices': choices
            }
        ]

        return questions


if __name__ == "__main__":
    pass

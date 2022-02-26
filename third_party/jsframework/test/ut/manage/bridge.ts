/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

import chai from 'chai';
import {
  before,
  after,
  describe,
  it
} from 'mocha';
import {
  fakeLog,
  fakeLogRestore
} from '../../fakeLog';
import { initFramework } from '../../../runtime/preparation/init';
import framework from '../../../runtime/preparation/methods';
import { receiveTasks } from '../../../runtime/main/manage/event/bridge';

const expect = chai.expect;

describe('receiveTasks', () => {
  fakeLog();

  const instanceId = Date.now() + '';
  const options = {
    orientation: 'portrait',
    deviceType: 'phone',
    resolution: '3.0',
    aspectRatio: 'string',
    deviceWidth: '1176',
    deviceHeight: '2400',
    roundScreen: false,
    width: '0',
    height: '0',
    isInit: true,
    pcPreview: 'disable',
    darkMode: 'false',
    appInstanceId: '10002',
    packageName: 'com.example.helloworld',
    resourcesConfiguration: [],
    i18n: {
      resources: [
        {'strings': {'hello': 'hello', 'world': 'world'},
          'Files': {}},
        {'strings': {'hello': 'Hello', 'world': 'World'},
          'Files': {}}
      ]
    },
    language: 'zh_CN',
    appCreate: true,
    appCode: '',
    bundleUrl: ''
  };
  const code: string = `
      $app_define$('@app-component/index', [],
        function($app_require$, $app_exports$, $app_module$) {
          $app_module$.exports = {
            data: {},
          }
          $app_module$.exports.template = {
            'type': 'div',
            'attr': {},
            'children': [
              {
                'type': 'text',
                'attr': {
                  'value': 'This is the index page.'
                },
                'classList': [
                  'title'
                ],
                "events": {
                  "click": '1'
                },
              }
            ]
          }
        })

      $app_bootstrap$('@app-component/index',undefined,undefined)
    `;

  before(() => {
    initFramework();
    framework.createInstance(instanceId, code, options, null);
  });

  after(() => {
    framework.destroyInstance(instanceId);
  });

  it('normal check of tasks', () => {
    expect(receiveTasks).to.be.an.instanceof(Function);
    const test1 = receiveTasks('invalid id', undefined);
    expect(test1).to.be.an.instanceof(Error);

    // @ts-ignore
    const test2 = receiveTasks(instanceId, {});
    expect(test2).to.be.an.instanceof(Error);
    const test3 = receiveTasks('invalid id', [{
      method: 'whatever',
      args: []
    }]);
    expect(test3).to.be.an.instanceof(Error);
  });

  fakeLogRestore();
});

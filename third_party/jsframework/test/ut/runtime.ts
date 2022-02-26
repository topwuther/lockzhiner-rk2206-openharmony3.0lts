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
} from '../fakeLog';
import { initFramework } from '../../runtime/preparation/init';
import framework from '../../runtime/preparation/methods';
import {
  getModule,
  clearModules,
  allModules
} from '../../runtime/main/page/register';
import { App } from '../../runtime/main/app/App';
import Page from '../../runtime/main/page';

const expect = chai.expect;

function clearRefs(json) {
  delete json.ref;
  if (json.children) {
    json.children.forEach(clearRefs);
  }
}

describe('framework entry', () => {
  fakeLog();

  const pageMap: Map<string, Page> = App.pageMap;
  let instanceId;
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
    darkMode: 'false',
    pcPreview: 'disable',
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

  before(() => {
    initFramework();
    global.callNative = (id, tasks, callbackId) => {
      if (callbackId !== '-1') {
        framework.callJS(id, [{
          method: 'callback',
          args: [callbackId, null, true]
        }]);
      }
    };
  });

  after(() => {
    fakeLogRestore();
    framework.destroyInstance(instanceId);
  });

  it('createInstance', () => {
    instanceId = Date.now() + '';
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
                ]
              }
            ]
          }
          $app_module$.exports.style = {
            '.title': {
              'fontSize': '50px'
            }
          }
        })
      $app_bootstrap$('@app-component/index',undefined,undefined)
    `;
    const expectComponent = {
      'index': {
        'data': {},
        'template': {
          'type': 'div',
          'attr': {},
          'children': [{'type': 'text', 'attr': {'value': 'This is the index page.'}, 'classList': ['title']}]
        },
        'style': {'.title': {'fontSize': '50px'}}
      }
    };
    framework.createInstance(instanceId, code, options, null);
    expect(pageMap.get(instanceId).customComponentMap).eql(expectComponent);
  });

  describe('getRoot', () => {
    it('with an exist instanceId', () => {
      const json = framework.getRoot(instanceId);
      expect(json.ref).eql('_root');
      clearRefs(json);
      const expectJSON = {
        type: 'div',
        attr: {},
        style: {},
        children: [{
          type: 'text',
          attr: {
            value: 'This is the index page.'
          },
          customComponent: false,
          style: {fontSize: '50px'}
        }],
        event: ['viewappear', 'viewdisappear', 'viewsizechanged'],
        customComponent: true
      };
      expect(json).eql(expectJSON);
    });
  });

  describe('callJS', () => {
    it('fireEvent with no params', () => {
      const result = framework.callJS(undefined, undefined);
      expect(result).to.be.an.instanceof(Error);
    });

    it('non-exist instanceId', () => {
      const result = framework.callJS('123', [{
        method: 'fireEvent',
        args: []
      }]);
      expect(result).to.be.an.instanceof(Error);
    });

    it('non-array tasks', () => {
      const result = framework.callJS(instanceId, {
        // @ts-ignore
        method: 'fireEvent',
        args: []
      });
      expect(result).to.be.an.instanceof(Error);
    });
  });

  describe('destroyInstance', () => {
    it('with no params', () => {
      const result = framework.destroyInstance(undefined);
      expect(result).to.be.an.instanceof(Error);
    });

    it('with an exist instanceId', () => {
      const result = framework.destroyInstance(instanceId);
      expect(result[instanceId]).to.be.undefined;
    });

    it('non-exist instanceId', () => {
      const result = framework.destroyInstance('123');
      expect(result).to.be.an.instanceof(Error);
    });
  });

  describe('registerModules', () => {
    it('with object of modules', () => {
      clearModules();
      expect(allModules()).to.deep.equal({});
      const modules = {
        'system.test': ['getInfo', 'getAvailableStorage', 'getCpuInfo']
      };
      framework.registerModules(modules);
      expect(getModule('system')).to.be.an('object');
      clearModules();
    });
  });
});

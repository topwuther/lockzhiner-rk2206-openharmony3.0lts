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
  describe,
  it
} from 'mocha';
import {
  fakeLog,
  fakeLogRestore
} from '../../fakeLog';
import Document from '../../../runtime/vdom/Document';
import {
  defineFn,
  bootstrap
} from '../../../runtime/main/app/bundle';

const expect = chai.expect;

describe('use defineFn/bootstrap', () => {
  fakeLog();

  let doc: Document;
  const options = {
    orientation: 'portrait',
    deviceType: 'phone',
    resolution: '3.0',
    aspectRatio: 'string',
    deviceWidth: '1176',
    deviceHeight: '2400',
    width: '0',
    height: '0',
    isInit: true,
    appInstanceId: '10002',
    packageName: 'com.example.test'
  };
  const componentTemplate = {
    type: 'div',
    children: [{
      type: 'text',
      attr: {
        value: 'value'
      }
    }]
  };
  const page = { doc, customComponentMap: {}, options };

  before(() => {
    const id = Date.now() + '';
    const url = ""
    doc = new Document(id, url);
  });

  describe('defineFn', () => {
    it('application with factory and deps', () => {
      // @ts-ignore
      defineFn(page, options.packageName, '@app-application/a', [], (require, exports, module) => {
        module.exports = {
          template: componentTemplate
        }
      });
      expect(page.customComponentMap['a'].template).eql(componentTemplate);
    });
  });

  describe('bootstrap', () => {

    before(() => {
      // @ts-ignore
      defineFn(page, options.packageName, '@app-application/main', [], (require, exports, module) => {
        module.exports = {
          template: componentTemplate,
        }
      });
    });

    it('not an application', () => {
      // @ts-ignore
      const result = bootstrap(page, options.packageName, '@app-module/dom', undefined, undefined);
      expect(result).instanceof(Error);
    });

    it('an application', () => {
      // @ts-ignore
      const result = bootstrap(page, options.packageName, '@app-application/dom', undefined, undefined);
      expect(result).not.instanceof(Error);
    });
  });

  fakeLogRestore();
});

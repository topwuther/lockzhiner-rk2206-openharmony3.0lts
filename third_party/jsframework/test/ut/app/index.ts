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
import { App } from '../../../runtime/main/app/App';

const expect = chai.expect;

describe('App Instance', () => {
  fakeLog();

  let app: App;
  let appInstanceId: string;

  before(() => {
    const appInstanceId = Date.now() + '';
    app = new App('test', appInstanceId);
  });

  describe('normal check', () => {
    it('is a class', () => {
      expect(typeof App).eql('function');
    });

    it('being created', () => {
      expect(app).to.be.an('object');
      expect(app).to.be.instanceof(App);
    });

    it('with some apis', () => {
      expect(typeof app.getAppInstance).eql('function');
      expect(typeof app.deleteGlobalKeys).eql('function');
    });

    it('run apis', () => {
      expect(app.getAppInstance()).eql(appInstanceId);
      expect(app.deleteGlobalKeys()).to.be.undefined;
    });
  });

  fakeLogRestore();
});
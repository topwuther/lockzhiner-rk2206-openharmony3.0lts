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
import sinon from 'sinon';
import {
  describe,
  before,
  it
} from 'mocha';
import {
  fakeLog,
  fakeLogRestore
} from '../../fakeLog';
import {
  bindSubVm,
  bindSubVmAfterInitialized
} from '../../../runtime/main/model/directive';
import { initState } from '../../../runtime/main/reactivity/state';
import config from '../../../runtime/main/config';

const expect = chai.expect;
const { nativeComponentMap } = config;
const directive = {};

function extendVm(vm, methodNames) {
  methodNames.forEach((name) => {
    vm[name] = directive[name];
  });
  initState(vm);
}

function initElement(el) {
  el.setAttr = function(k, v) {
    this.attr[k] = v;
  };
  el.setStyle = function(k, v) {
    this.style[k] = v;
  };
  el.setClassStyle = function(style) {
    this.classStyle = style;
  };
  el.addEvent = function(t, h) {
    this.event[t] = h;
  };
  el.setClassList = function(classList) {
    this.classList = classList;
  };
}

describe('bind external infomations to sub vm', () => {
  fakeLog();

  let vm: any;
  let subVm: any;

  before(() => {
    vm = {
      data: { a: 1, b: 2, c: 'class-style1' },
      watchers: [],
      app: { eventManager: { add: () => {} }},
      options: {
        style: {
          'class-style1': {
            aaa: 1,
            bbb: 2
          },
          'class-style2': {
            aaa: 2,
            ccc: 3
          }
        }
      },
      foo: function() {}
    };
    extendVm(vm, []);
    subVm = {
      options: {
        props: {
          a: String,
          b: String
        }
      },
      props: []
    };
  });

  it('bind to no-root-element sub vm', () => {
    bindSubVm(vm, subVm, {
      // @ts-ignore
      attr: { a: 3, c: 4 },

      // @ts-ignore
      style: { a: 2 },
      events: { click: 'foo' }
    }, {});
    expect(subVm.a).eql(3);
    expect(subVm.b).to.be.undefined;
    expect(subVm.rootEl).to.be.undefined;
  });

  it('bind props with external data', () => {
    bindSubVm(vm, subVm, {
      // @ts-ignore
      attr: { a: function() {
        return this.data.b;
      } }
    }, {});
    expect(subVm.a).eql(2);
  });

  it('bind styles to a sub vm with root element', () => {
    subVm.rootEl = {
      attr: {},
      style: {},
      event: []
    };
    const template: any = {
      style: { aaa: 2, bbb: function() {
        return this.data.a;
      } }
    };
    initElement(subVm.rootEl);
    bindSubVm(vm, subVm, template, {});

    // @ts-ignore
    bindSubVmAfterInitialized(vm, subVm, template, {});
    expect(subVm.rootEl.style.aaa).eql(2);
    expect(subVm.rootEl.style.bbb).eql(1);
  });

  fakeLogRestore();
});

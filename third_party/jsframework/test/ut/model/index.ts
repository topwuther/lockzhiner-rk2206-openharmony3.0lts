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
  it
} from 'mocha';
import {
  fakeLog,
  fakeLogRestore
} from '../../fakeLog';
import Vm from '../../../runtime/main/model';
import Doc from '../../../runtime/vdom/Document';
import Differ from '../../../runtime/main/page/api/Differ';

const expect = chai.expect;

describe('api of communication between vm and data methods', () => {
  fakeLog();

  const options = {
    orientation: 1,
    width: 1,
    height: 1,
    aspectRatio: 1,
    deviceWidth: 1,
    deviceHeight: 1,
    resolution: 1,
    accessType: 1
  };

  const doc = new Doc('1', '');
  const differ = new Differ('test');
  const customComponentMap: any = {};
  const spyEl = sinon.spy();

  customComponentMap.parent = {
    template: {
      type: 'div',
      id: 'myDiv',
      children: [
        {
          type: 'text',
          id: 'myBar',
          events: {
            'click': 'proxyClick'
          }
        },
        {
          type: 'child',
          id: 'myChild'
        }
      ]
    },
    proxyClick: function(e) {
      spyEl(e);
    }
  };

  customComponentMap.child = {
    template: {
      type: 'div'
    }
  };

  const page = {doc, customComponentMap, options};

  const vm = new Vm(
    'parent',
    null,
    {
      _app: page,
      _rootVm: true
    },
    null,
    null,
    null
  );

  const spyParent = sinon.spy();
  const spyChild = sinon.spy();
  const subVm = vm.childrenVms[0];
  vm.$on('event1', spyParent);
  subVm.$on('event1', spyChild);

  it('$on && $off', () => {
    const fn = function() {};
    vm.$on('event1', fn);
    expect(vm.vmEvents['event1'].length).eql(2);
    expect(typeof vm.vmEvents['event1'][1]).eql('function');

    vm.$off('event1', fn);
    expect(vm.vmEvents['event1'].length).eql(1);
    expect(typeof vm.vmEvents['event1'][1]).eql('undefined');
  });

  it('$emit', () => {
    expect(vm.type).eql('parent');
    expect(subVm.type).eql('child');

    let detail = { lunch: 'banana' };
    vm.$emit('event1', detail);
    expect(spyChild.args.length).eql(0);
    expect(spyParent.args.length).eql(1);

    detail = { lunch: 'banana' };
    subVm.$emit('event1', detail);
    expect(spyChild.args.length).eql(1);
    expect(spyParent.args.length).eql(1);
  });

  it('$dispatch', () => {
    const detail = { lunch: 'banana' };
    vm.$dispatch('event1', detail);
    expect(spyParent.args.length).eql(2);
    expect(spyChild.args.length).eql(1);

    subVm.$dispatch('event1', detail);
    expect(spyParent.args.length).eql(3);
    expect(spyChild.args.length).eql(2);
  });

  it('$broadCast', () => {
    const detail = { lunch: 'banana' };
    vm.$broadcast('event1', detail);
    expect(spyParent.args.length).eql(4);
    expect(spyChild.args.length).eql(3);

    subVm.$broadcast('event1', detail);
    expect(spyParent.args.length).eql(4);
    expect(spyChild.args.length).eql(4);
  });

  it('$emitDirect', () => {
    const detail = { lunch: 'banana' };
    vm.$emitDirect('event1', detail);
    expect(spyParent.args.length).eql(5);
  });

  it('$emitElement', () => {
    const detail = { lunch: 'banana' };
    vm.$emitElement('click', detail, 'myBar');
    sinon.assert.calledOnce(spyEl);
    expect(spyEl.args.length).eql(1);
    expect(spyEl.args[0][0].detail).to.deep.equal(detail);
  });

  it('$root', () => {
    expect(subVm.$root().type).eql('parent');
  });

  it('$parent', () => {
    expect(subVm.$parent().type).eql('parent');
  });

  it('$child', () => {
    expect(vm.$child('myChild').type).eql('child');
  });

  it('$element', () => {
    expect(vm.$element('myDiv').type).eql('div');
    expect(vm.$element('myChild').type).eql('div');
  });

  it('$set && $delete', () => {
    const data = {
      x: 11,
      y: 22,
      z: 'test-style'
    };
    customComponentMap.foo = {
      template: {
        type: 'test.aaa',
        children: [{ type: 'type1', component: true }]
      }
    };
    customComponentMap.bar = {
      template: {
        type: 'test.bbb'
      }
    };

    const app = { doc, customComponentMap, differ, options };
    const vm = new Vm('foo', customComponentMap.foo, { _app: app, _rootVm: true }, null, data, null);
    doc.destroy();
    const detail = { aaa: 1 };
    vm.$set('test.aaa', detail);
    expect(typeof vm.data['test.aaa']).eql('object');
    vm.$delete('test.aaa');
    expect(typeof vm.data['test.aaa']).eql('undefined');
  });

  it('$watch', () => {
    const data = {
      x: {
        y: 1
      }
    };
    customComponentMap.foo = {
      template: {
        type: 'div'
      },
      data: data
    };

    const app = { doc, customComponentMap, differ, options };
    const vm = new Vm('foo', customComponentMap.foo, { _app: app, _rootVm: true }, null, data, null);
    expect(vm.app).to.deep.equal(app);
    expect(typeof doc.body).eql('object');

    data.x.y = 5;
    vm.$watch('x.y', (value) => {
      expect(value).eql(5);
    });
  });

  fakeLogRestore();
});

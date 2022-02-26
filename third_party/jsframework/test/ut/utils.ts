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
  describe,
  it
} from 'mocha';
const expect = chai.expect;

import * as utils from '../../runtime/utils/index';

describe('utils', () => {
  it('should be an array or object', () => {
    expect(utils.isObject).to.be.an.instanceof(Function);
    expect(utils.isObject({})).eql(true);
    expect(utils.isObject([])).eql(true);
    expect(utils.isObject('a')).eql(false);
    expect(utils.isObject(0)).eql(false);
    expect(utils.isObject(true)).eql(false);
    expect(utils.isObject(null)).eql(false);
    expect(utils.isObject(undefined)).eql(false);
    expect(utils.isObject(function() {})).eql(false);
    expect(utils.isObject(/\w*/)).eql(true);
    expect(utils.isObject(new Date())).eql(true);
  });

  it('should be an real object', () => {
    expect(utils.isPlainObject).to.be.an.instanceof(Function);
    expect(utils.isPlainObject({})).eql(true);
    expect(utils.isPlainObject([])).eql(false);
    expect(utils.isPlainObject('a')).eql(false);
    expect(utils.isPlainObject(0)).eql(false);
    expect(utils.isPlainObject(true)).eql(false);
    expect(utils.isPlainObject(null)).eql(false);
    expect(utils.isPlainObject(undefined)).eql(false);
    expect(utils.isPlainObject(function() {})).eql(false);
    expect(utils.isPlainObject(/\w*/)).eql(false);
    expect(utils.isPlainObject(new Date())).eql(false);
  });

  it('has own property', () => {
    expect(utils.hasOwn).to.be.an.instanceof(Function);
    function Point() {
      this.x = 0;
    }
    Point.prototype.y = 1;
    const p = new Point();
    expect(p.x).eql(0);
    expect(p.y).eql(1);
    expect(utils.hasOwn(p, 'x')).eql(true);
    expect(utils.hasOwn(p, 'y')).eql(false);
  });

  it('own property is empty or not', () => {
    expect(utils.isEmpty).to.be.an.instanceof(Function);
    expect(utils.isEmpty({})).eql(true);
    expect(utils.isEmpty([])).eql(true);
    expect(utils.isEmpty('a')).eql(true);
    expect(utils.isEmpty(0)).eql(true);
    expect(utils.isEmpty(true)).eql(true);
    expect(utils.isEmpty(null)).eql(true);
    expect(utils.isEmpty(undefined)).eql(true);
    expect(utils.isEmpty(function() {})).eql(true);
    expect(utils.isEmpty(/\w*/)).eql(true);
    expect(utils.isEmpty(new Date())).eql(true);
    function Point() {
      this.x = 0;
    }
    const p = new Point();
    expect(p.x).eql(0);
    expect(utils.isEmpty(p)).eql(false);
    Point.prototype.y = 1;
    expect(p.y).eql(1);
    expect(utils.isEmpty(p)).eql(false);
  });

  it('is null or undefined', () => {
    expect(utils.isNull).to.be.an.instanceof(Function);
    expect(utils.isNull({})).eql(false);
    expect(utils.isNull([])).eql(false);
    expect(utils.isNull('a')).eql(false);
    expect(utils.isNull(0)).eql(false);
    expect(utils.isNull(true)).eql(false);
    expect(utils.isNull(null)).eql(true);
    expect(utils.isNull(undefined)).eql(true);
    expect(utils.isNull(function() {})).eql(false);
    expect(utils.isNull(/\w*/)).eql(false);
    expect(utils.isNull(new Date())).eql(false);
  });
});

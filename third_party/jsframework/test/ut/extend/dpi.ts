/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import chai from 'chai';
import {
  describe,
  it,
} from 'mocha';
import {
  fakeLog,
  fakeLogRestore
} from '../../fakeLog';
import dpi from '../../../runtime/main/extend/dpi';

const expect = chai.expect;

describe('api of dpi', () => {
  fakeLog();

  const dpiOptions = {
    images: [
      {
        'image': {
          'wearable': 'common/wearable.png',
          'computer': 'image/computer.jpg',
          'object': {
            'image0': 'common/wearable.png',
            'image1': 'image/computer.jpg'
          },
          'array': [
            'common/wearable.png',
            'image/computer.jpg'
          ]
        }
      }
    ]
  };

  const Dpi = dpi.dpi.create(0).instance.dpi;
  const dpiInstance = new Dpi(dpiOptions);

  describe('dpi', () => {
    it('$r(path)', () => {
      expect(dpiInstance.$r).to.be.an.instanceof(Function);
      expect(dpiInstance.$r('image.wearable')).eql('common/wearable.png');
      expect(dpiInstance.$r('image.com')).eql('image.com');
      expect(dpiInstance.$r('image.object')).eql({
        'image0': 'common/wearable.png',
        'image1': 'image/computer.jpg'
      });
      expect(dpiInstance.$r('image.array')[0]).eql('common/wearable.png');
      expect(dpiInstance.$r(null)).to.be.undefined;
      expect(dpiInstance.$r(undefined)).to.be.undefined;
    });

    // @ts-ignore
    expect(dpiInstance.$r(1)).to.be.undefined;
  });

  fakeLogRestore();
});

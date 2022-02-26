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
import sinon from 'sinon';
import {
  describe,
  it,
  after
} from 'mocha';
import {
  fakeLog,
  fakeLogRestore
} from '../../fakeLog';
import i18n from '../../../runtime/main/extend/i18n';
import { getValue } from '../../../runtime/utils';

const expect = chai.expect;

describe('api of i18n', () => {
  fakeLog();

  const i18nOptions0 = {
    locale: 'zh',
    messages: [
      {
        'strings': {
          'hello': 'hello'
        },
        'files': {
          'image': 'image/rain_bg.jpg'
        }
      },
      {
        'strings': {
          'hello': 'Hello world!',
          'object': 'Object parameter substitution-{name}',
          'object1': 'Object parameter substitution-{name1}-{name0}',
          'array': 'Array type parameter substitution-{0}',
          'array1': 'Array type parameter substitution-{1}-{0}-{2}',
          'num': 1,
          'bool': true,
          'plurals': {
            'one': 'one person',
            'other': '{count} people'
          }
        },
        'files': {
          'image': 'image/en_picture.PNG'
        }
      }
    ]
  };
  const i18nOptions1 = {
    locale: 'en',
    messages: [[], {'strings': 'hello'}]
  };
  const i18nPluralRules = {
    select: (count) => {
      return count === 1 ? 'one' : 'other';
    }
  };
  const I18n = i18n.I18n.create(0).instance.I18n;
  const i18nInstance0 = new I18n(i18nOptions0);
  const i18nInstance1 = new I18n(i18nOptions1);
  const fakeGetChoice = sinon.stub(i18nInstance0, '_getChoice').callsFake((count, path, message) => {
    const pluralChoice = i18nPluralRules.select(count);
    if (!pluralChoice) {
      return path;
    }
    return getValue(pluralChoice, message);
  });

  after(() => {
    fakeGetChoice.restore();
  });

  describe('i18n', () => {
    it('$t(path)', () => {
      expect(i18nInstance0.$t).to.be.an.instanceof(Function);
      expect(i18nInstance0.$t('strings.hello')).eql('hello');
      expect(i18nInstance0.$t('strings.num')).eql(1);
      expect(i18nInstance0.$t('strings.bool')).to.be.true;
      expect(i18nInstance0.$t('files.image')).eql('image/rain_bg.jpg');
      expect(i18nInstance0.$t('strings.plurals.one')).eql('one person');
      expect(i18nInstance0.$t('strings.plurals')).eql({
        'one': 'one person',
        'other': '{count} people'
      });
      expect(i18nInstance0.$t('strings.plurals')['one']).eql('one person');
      expect(i18nInstance0.$t('strings.plurals')['two']).eql(undefined);
      expect(i18nInstance0.$t('strings.hell.key')).eql('strings.hell.key');
      expect(i18nInstance0.$t(null)).to.be.undefined;
      expect(i18nInstance0.$t(undefined)).to.be.undefined;

      // @ts-ignore
      expect(i18nInstance0.$t(1)).to.be.undefined;
    });

    it('$t(path, params)', () => {
      expect(i18nInstance0.$t('strings.object', {'name': 'object_value'}))
        .eql('Object parameter substitution-object_value');
      expect(i18nInstance0.$t('strings.object1', {'name0': 'object_value_0', 'name1': 'object_value_1'}))
        .eql('Object parameter substitution-object_value_1-object_value_0');
      expect(i18nInstance0.$t('strings.array', [1]))
        .eql('Array type parameter substitution-1');
      expect(i18nInstance0.$t('strings.array1', [1, false]))
        .eql('Array type parameter substitution-false-1-');
      expect(i18nInstance0.$t('strings.array1', null))
        .eql('Array type parameter substitution-{1}-{0}-{2}');
      expect(i18nInstance0.$t('strings.array1', undefined))
        .eql('Array type parameter substitution-{1}-{0}-{2}');
      expect(i18nInstance0.$t('strings.array1', 1))
        .eql('Array type parameter substitution-{1}-{0}-{2}');
      expect(i18nInstance0.$t('strings.array1', {0: 'object_value_0', 1: 'object_value_1'}))
        .eql('Array type parameter substitution-object_value_1-object_value_0-');
      expect(i18nInstance0.$t('strings.array1', {'name0': 'object_value_0', 'name1': 'object_value_1'}))
        .eql('Array type parameter substitution---');
    });

    it('$tc(path, count)', () => {
      expect(i18nInstance0.$tc).to.be.an.instanceof(Function);
      expect(i18nInstance0.$tc('strings.plurals', 0)).eql('0 people');
      expect(i18nInstance0.$tc('strings.plurals', 1)).eql('one person');
      expect(i18nInstance0.$tc('strings.plurals', undefined)).eql('one person');
      expect(i18nInstance0.$tc('strings.plurals')).eql('one person');
      expect(i18nInstance0.$tc('strings.plurals', null)).eql('one person');
      expect(i18nInstance0.$tc('strings.hello', 0)).eql('strings.hello');
      expect(i18nInstance0.$tc('strings.hello')).eql('strings.hello');
      expect(i18nInstance0.$tc(null)).to.be.undefined;
      expect(i18nInstance0.$tc(undefined)).to.be.undefined;

      // @ts-ignore
      expect(i18nInstance0.$tc(1)).to.be.undefined;
    });

    it('messages are null', () => {
      expect(i18nInstance1.$t('strings.hello')).eql('strings.hello');
      expect(i18nInstance1.$t('strings')).eql('hello');
    });
  });

  fakeLogRestore();
});

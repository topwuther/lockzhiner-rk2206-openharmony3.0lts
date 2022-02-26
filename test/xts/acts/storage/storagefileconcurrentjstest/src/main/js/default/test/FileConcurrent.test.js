/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

import fileio from '@system.fileio'
import file from '@system.file';
import {
  describe,
  beforeAll,
  beforeEach,
  afterEach,
  afterAll,
  it,
  expect
}
  from 'deccjsunit/index'

describe('fileconcurrent', function () {

  /**
   * @tc.number SUB_STORAGE_file_test_0000
   * @tc.name file_test_0000 Function of API, out of package, Virtual path(create and give 777 authority).
   * @tc.desc Function of API, Simultaneous write.
   */
  it('file_test_000', 0, async function (done) {
    for (let i = 0; i < 10; i++) {
      file.writeText({
        uri: 'internal://app/file_test_000',
        text: 'Text',
        success: function () {
          console.log('file_test_000 => pass, call writeText success. ');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_000 => call writeText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.writeText({
        uri: 'internal://app/file_test_000',
        text: 'Text1',
        success: function () {
          console.log('file_test_000 => pass, call writeText success. ');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_000 => call writeText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.writeText({
        uri: 'internal://app/file_test_000',
        text: 'Text2',
        success: function () {
          console.log('file_test_000 => pass, call writeText success. ');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_000 => call writeText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_readText_0100
   * @tc.name file_test_001
   * @tc.desc Function of API Simultaneous reading.
   */
  it('file_test_001', 0, async function (done) {
    file.writeText({
      uri: 'internal://app/file_test_001',
      text: 'Text',
      success: function () {
        console.log('file_test_001 call writeText success.');
        done();
      },
      fail: function (data, code) {
        console.log('file_test_001 call writeText fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    for (let i = 0; i < 10; i++) {
      file.readText({
        uri: 'internal://app/file_test_001',
        success: function () {
          console.log('file_test_001 => pass, call readText success. data.text: ' );
          done();
        },
        fail: function (data, code) {
          console.log('file_test_001 => call readText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.readText({
        uri: 'internal://app/file_test_001',
        success: function () {
          console.log('file_test_001 => pass, call readText success. data.text: ' );
          done();
        },
        fail: function (data, code) {
          console.log('file_test_001 => call readText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.readText({
        uri: 'internal://app/file_test_001',
        success: function () {
          console.log('file_test_001 => pass, call readText success. data.text: ' );
          done();
        },
        fail: function (code, data) {
          console.log('file_test_001 => call readText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_file_test_0200
   * @tc.name file_test_002
   * @tc.desc Function of API Simultaneous copying.
   */
  it('file_test_002', 0, async function (done) {
    file.writeText({
      uri: 'internal://app/file_test_002',
      text: 'Text',
      append: true,
      success: function () {
        console.log('file_test_002 call writeText success.');
        done();
      },
      fail: function (data, code) {
        console.log('file_test_002 call writeText fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    for (let i = 0; i < 10; i++) {
      file.copy({
        srcUri: 'internal://app/file_test_002',
        dstUri: 'internal://app/file_test_002',
        success: function () {
          console.log('file_test_002 => pass,call copy success. uri:' );
          done();
        },
        fail: function (data, code) {
          console.log('file_test_002 => call copy fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.copy({
        srcUri: 'internal://app/file_test_002',
        dstUri: 'internal://app/file_test_002' + '100',
        success: function () {
          console.log('file_test_002 => pass,call copy success. uri:' );
          done();
        },
        fail: function (data, code) {
          console.log('file_test_002 => call copy fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.copy({
        srcUri: 'internal://app/file_test_002',
        dstUri: 'internal://app/file_test_002' + '200',
        success: function () {
          console.log('file_test_002 => pass,call copy success. uri:' );
          done();
        },
        fail: function (data, code) {
          console.log('file_test_002 => call copy fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_file_test_0300
   * @tc.name file_test_003
   * @tc.desc Function of API, Move files while writing to them continuously
   */
  it('file_test_003', 0, async function (done) {
    for (let i = 0; i < 10; i++) {
      file.writeText({
        uri: 'internal://app/file_test_003',
        text: 'Text',
        append: true,
        success: function () {
          console.log('file_test_003 => pass,call writeText success.');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_003 => call writeText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.move({
        srcUri: 'internal://app/file_test_003',
        dstUri: 'internal://file_test_003',
        success: function () {
          console.log('file_test_003 => pass,call move success. uri:' );
          done();
        },
        fail: function (data, code) {
          console.log('file_test_003 =>call move fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_file_test_0400
   * @tc.name file_test_004
   * @tc.desc Function of API,Continuous reading
   */
  it('file_test_004', 0, async function (done) {
    let buf = new Uint8Array([48, 49, 50, 51, 65, 66, 67, 68, 32, 33]);
    file.writeArrayBuffer({
      uri: 'internal://app/file_test_004',
      buffer: buf,
      success: function () {
        console.log('file_test_004 call writeArrayBuffer success.');
        done();
      },
      fail: function (data, code) {
        console.error('file_test_004 call writeArrayBuffer fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    for (let i = 0; i < 10; i++) {
      file.readArrayBuffer({
        uri: 'internal://app/file_test_004',
        success: function () {
          console.log('file_test_004 => pass,call readArrayBuffer success. ');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_004 => call readArrayBuffer fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.readArrayBuffer({
        uri: 'internal://app/file_test_004',
        success: function () {
          console.log('file_test_004 => pass,call readArrayBuffer success. ');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_004 => call readArrayBuffer fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.readArrayBuffer({
        uri: 'internal://app/file_test_004',
        success: function () {
          console.log('file_test_004 => pass,call readArrayBuffer success. ');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_004 => call readArrayBuffer fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_file_test_0500
   * @tc.name file_test_005
   * @tc.desc Function of API,Continuous writeing
   */
  it('file_test_005', 0, async function (done) {
    let buf = new Uint8Array([48, 49, 50, 51, 65, 66, 67, 68, 32, 33]);
    for (let i = 0; i < 10; i++) {
      file.writeArrayBuffer({
        uri: 'internal://app/file_test_005',
        buffer: buf,
        success: function () {
          console.log('file_test_005 => pass,call writeArrayBuffer success.');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_005 => call writeArrayBuffer fail, code: ' + code);
          expect(null).assertFail();
        },
      });
      file.writeArrayBuffer({
        uri: 'internal://app/file_test_005',
        buffer: buf,
        success: function () {
          console.log('file_test_005 => pass,call writeArrayBuffer success.');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_005 => call writeArrayBuffer fail, code: ' + code);
          expect(null).assertFail();
        },
      });
      file.writeArrayBuffer({
        uri: 'internal://app/file_test_005',
        buffer: buf,
        success: function () {
          console.log('file_test_005 => pass,call writeArrayBuffer success.');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_005 => call writeArrayBuffer fail, code: ' + code);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_file_test_0600
   * @tc.name file_test_006
   * @tc.desc Function of API,Write and read simultaneously for one file
   */
  it('file_test_006', 0, async function (done) {
    for (let i = 0; i < 10; i++) {
      file.writeText({
        uri: 'internal://app/file_test_006',
        text: 'Text',
        success: function () {
          console.log('file_test_006 => pass,call writeText success.');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_006 => call fail callback fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.readText({
        uri: 'internal://app/file_test_006',
        success: function () {
          console.log('file_test_006 => pass,call readText success: ');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_006 => call readText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_file_test_0700
   * @tc.name file_test_007
   * @tc.desc Function of API,Write and read simultaneously for one file
   */
  it('file_test_007', 0, async function (done) {
    let buf = new Uint8Array([48, 49, 50, 51, 65, 66, 67, 68, 32, 33]);
    for (let i = 0; i < 10; i++) {
      file.writeArrayBuffer({
        uri: 'internal://app/file_test_007',
        buffer: buf,
        success: function () {
          console.log('file_test_007 => pass,call writeArrayBuffer success.');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_007 => call writeArrayBuffer fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.readArrayBuffer({
        uri: 'internal://app/file_test_007',
        success: function () {
          console.log('file_test_007 => pass,call readArrayBuffer success: ');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_007 => call readArrayBuffer fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_file_test_0800
   * @tc.name file_test_008
   * @tc.desc Function of API, Copy files while writing
   */
  it('file_test_008', 0, async function (done) {
    for (let i = 0; i < 10; i++) {
      file.writeText({
        uri: 'internal://app/file_test_008',
        text: 'Text',
        success: function () {
          console.log('file_test_008 => pass, call writeText success.');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_008 => call writeText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.copy({
        srcUri: 'internal://app/file_test_008',
        dstUri: 'internal://app/file_test_008',
        success: function () {
          console.log('file_test_008 => pass,call copy success. uri:');
          done();
        },
        fail: function (data, code) {
          console.log('file_test_008 => call copy fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });
});

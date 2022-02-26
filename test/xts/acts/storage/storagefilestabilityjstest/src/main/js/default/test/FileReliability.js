/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import fileio from '@system.fileio'
import file from '@system.file';
import {
  describe,
  it,
  expect
}
  from 'deccjsunit/index'
import {
  FILE_CONTENT,
  prepareFile,
  nextFileName,
  differentFileName
}
  from './Common'

describe('FileReliability', function () {

  /**
   * @tc.number SUB_STORAGE_FileReliability
   * @tc.name FileReliability
   * @tc.desc Function of API, Reliability test 7*24.
   */
  it('FileReliability', 0, async function (done) {
    for (let i = 0; i < 100000; i++) {
      let promiseMkdir = new Promise(function (resolve, reject) {
        file.mkdir({
          uri: 'internal://app/fileStability' + i,
          success: function () {
            console.log(i + 'call mkdir success.');
            done();
          },
          fail: function (data, code) {
            console.log(i + 'call mkdir fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        });
        setTimeout(function () {
          resolve('mkdir success!');
        }, 10);
      });
      promiseMkdir.then(function () {
        console.log('mkdir success');
      });
      let promiseWriteText = new Promise(function (resolve, reject) {
        file.writeText({
          uri: 'internal://app/fileStability' + i + '/fileStability01' + i,
          text: 'Text that just for test.',
          success: function () {
            console.log(i + 'call writeText success.');
            done();
          },
          fail: function (data, code) {
            console.log(i + 'call writeText fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        });
        setTimeout(function () {
          resolve('writeText success!');
        }, 15);
      });
      promiseWriteText.then(function () {
        console.log('writeText success');
      });
      let promiseReadText = new Promise(function (resolve, reject) {
        file.readText({
          uri: 'internal://app/fileStability' + i + '/fileStability01' + i,
          success: function (data) {
            console.log(i + 'call readText success: ' + data.text);
            done();
          },
          fail: function (data, code) {
            console.log(i + 'call readText fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        });
        setTimeout(function () {
          resolve('readText success!');
        }, 15);
      });
      promiseReadText.then(function () {
        console.log('readText success');
      });
      let promiseAccess = new Promise(function (resolve, reject) {
        file.access({
          uri: 'internal://app/fileStability' + i + '/fileStability01' + i,
          success: function () {
            console.log(i + 'call access success.');
            done();
          },
          fail: function (data, code) {
            console.log(i + 'call access fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        });
        setTimeout(function () {
          resolve('access success!');
        }, 15);
      });
      promiseAccess.then(function () {
        console.log('access success');
      });
      let promiseGet = new Promise(function (resolve, reject) {
        file.get({
          uri: 'internal://app/fileStability' + i + '/fileStability01' + i,
          success: function (data) {
            console.log(i + 'call readText success: ' + data.uri);
            done();
          },
          fail: function (data, code) {
            console.log(i + 'call callback fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        });
        setTimeout(function () {
          resolve('get success!');
        }, 15);
      });
      promiseGet.then(function () {
        console.log('get success');
      });
      let promiseList = new Promise(function (resolve, reject) {
        file.list({
          uri: 'internal://app/fileStability' + i,
          success: function (data) {
            console.log(i + 'call readText success: ' + data.fileList);
            done();
          },
          fail: function (data, code) {
            console.log(i + 'call fail callback fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        });
        setTimeout(function () {
          resolve('list success!');
        }, 15);
      });
      promiseList.then(function () {
        console.log('list success');
      });
      let promiseCopy = new Promise(function (resolve, reject) {
        file.copy({
          srcUri: 'internal://app/fileStability' + i + '/fileStability01' + i,
          dstUri: 'internal://app/fileStability' + i + '/fileStability02' + i,
          success: function (uri) {
            console.log(i + 'call success copy success uri: ' + uri);
            done();
          },
          fail: function (data, code) {
            console.log(i + 'call fail copy fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        });
        setTimeout(function () {
          resolve('copy success!');
        }, 15);
      });
      promiseCopy.then(function () {
        console.log('copy success');
      });
      let promiseMove = new Promise(function (resolve, reject) {
        file.move({
          srcUri: 'internal://app/fileStability' + i + '/fileStability01' + i,
          dstUri: 'internal://app/fileStability01' + i,
          success: function (uri) {
            console.log(i + 'call success move success. uri: ' + uri);
            done();
          },
          fail: function (data, code) {
            console.log(i + 'call fail move fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        });
        setTimeout(function () {
          resolve('move success!');
        }, 15);
      });
      promiseMove.then(function () {
        console.log('move success');
      });
      file.delete({
        uri: 'internal://app/fileStability01' + i,
        success: function () {
          console.log(i + 'call delete success.');
          done();
        },
        fail: function (data, code) {
          console.log(i + 'call fail delete fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      file.delete({
        uri: 'internal://app/fileStability' + i + '/fileStability02' + i,
        success: function () {
          console.log(i + 'call delete success.');
          done();
        },
        fail: function (data, code) {
          console.log(i + 'call fail delete fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      let promiseWriteArrayBuffer = new Promise(function (resolve, reject) {
        let buf = new Uint8Array([48, 49, 50, 51, 65, 66, 67, 68, 32, 33]);
        file.writeArrayBuffer({
          uri: 'internal://app/fileStability' + i + '/fileStability02' + i,
          buffer: buf,
          success: function () {
            console.log(i + 'call writeArrayBuffer success.');
            done();
          },
          fail: function (data, code) {
            console.log(i + 'call fail writeArrayBuffer fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        });
        setTimeout(function () {
          resolve('writeArrayBuffer success!');
        }, 15);
      });
      promiseWriteArrayBuffer.then(function () {
        console.log('writeArrayBuffer success');
      });
      let promiseReadArrayBuffer = new Promise(function (resolve, reject) {
        file.readArrayBuffer({
          uri: 'internal://app/fileStability' + i + '/fileStability02' + i,
          success: function (data) {
            console.log(i + 'call readArrayBuffer success: ' + data.buffer);
            done();
          },
          fail: function (data, code) {
            console.log(i + 'call fail readArrayBuffer fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        });
        setTimeout(function () {
          resolve('readArrayBuffer success!');
        }, 15);
      });
      promiseReadArrayBuffer.then(function () {
        console.log('readArrayBuffer success');
      });
      file.delete({
        uri: 'internal://app/fileStability' + i + '/fileStability02' + i,
        success: function () {
          console.log(i + 'call delete success.');
          done();
        },
        fail: function (data, code) {
          console.log(i + 'call fail delete fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      let promiseRmdir = new Promise(function (resolve, reject) {
        file.rmdir({
          uri: 'internal://app/fileStability' + i,
          recursive: true,
          success: function () {
            console.log(i + 'call rmdir success.');
            done();
          },
          fail: function (data, code) {
            console.log(i + 'call fail rmdir fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        });
        setTimeout(function () {
          resolve('rmdir success!');
        }, 15);
      });
      promiseReadArrayBuffer.then(function () {
        console.log('rmdir success');
      });

    }
  });
});

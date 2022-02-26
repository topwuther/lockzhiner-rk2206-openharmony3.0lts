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

import file from '@system.file';
import fileio from '@ohos.fileio';
import {
  describe,
  it,
  expect
}
  from 'deccjsunit/index'
import {
  randomString
}
  from './Common'

describe('fileTest', function () {

  /**
   * @tc.number SUB_STORAGE_File_mkdir_1100
   * @tc.name File_mkdir_011
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_mkdir_011', 0, async function (done) {
    let start = new Date().getTime();
    for (let i = 0; i < 1000; i++) {
      file.mkdir({
        uri: 'internal://app/File_mkdir_011' + i,
        success: function () {
          let end = new Date().getTime();
          let time = end - start;
          console.log('-------File_mkdir_011 call mkdir success, time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.log('-------File_mkdir_011 call mkdir fail! code:' + code + ', data' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_access_1100
   * @tc.name File_access_011
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_access_011', 0, async function (done) {
    let start = new Date().getTime();
    for (let i = 0; i < 1000; i++) {
      file.access({
        uri: 'internal://app/File_mkdir_011' + i,
        success: function () {
          let end = new Date().getTime();
          let time = end - start;
          console.log('-------File_access_011 call access success, time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.log('-------File_access_011 call access fail! code:' + code + ', data' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_rmdir_1100
   * @tc.name File_rmdir_011
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_rmdir_011', 0, async function (done) {
    let start = new Date().getTime();
    for (let i = 0; i < 1000; i++) {
      file.rmdir({
        uri: 'internal://app/File_mkdir_011' + i,
        success: function () {
          let end = new Date().getTime();
          let time = end - start;
          console.log('-------File_rmdir_011 call rmdir success, time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.log('-------File_rmdir_011 call rmdir fail! code:' + code + ', data' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Copy_1500
   * @tc.name File_Copy_015
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_Copy_015', 0, function (done) {
    file.writeText({
      uri: 'internal://app/File_Copy_015',
      text: 'hello',
      success: function () {
        console.log('File_Copy_015 call writeText success');
        done();
      },
      fail: function (data, code) {
        console.log('File_Copy_015 call writeText fail! code:' + code + ', data' + data);
        expect(null).assertFail();
      },
    });
    let start = new Date().getTime();
    for (let i = 0; i < 1000; i++) {
      file.copy({
        srcUri: 'internal://app/File_Copy_015',
        dstUri: 'internal://app/File_Copy_015_1' + i,
        success: function () {
          let end = new Date().getTime();
          let time = end - start;
          console.log('-------File_Copy_015 call copy success, time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.log('-------File_Copy_015 call copy fail! code:' + code + ', data' + data);
          expect(null).assertFail();
        },
      });
    }
    file.delete({
      uri: 'internal://app/File_Copy_015',
      success: function () {
        console.log('-------File_Copy_015 call delete success.');
        done();
      },
      fail: function (data, code) {
        console.log('-------File_Copy_015 call delete fail! code:' + code + ', data' + data);
        expect(null).assertFail();
      },
    });
  });

  /**
   * @tc.number SUB_STORAGE_File_Delete_1100
   * @tc.name File_Delete_011
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_Delete_011', 0, function (done) {
    let start = new Date().getTime();
    for (let i = 0; i < 1000; i++) {
      file.delete({
        uri: 'internal://app/File_Copy_015_1' + i,
        success: function () {
          let end = new Date().getTime();
          let time = end - start;
          console.log('-------File_Delete_011 call delete success, time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.log('-------File_Delete_011 call delete fail! code:' + code + ', data' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_List_1300
   * @tc.name File_List_013
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_List_013', 0, async function (done) {
    file.writeText({
      uri: 'internal://app/File_List_013',
      text: 'Text that just for test.',
      success: function () {
        console.log('File_List_013 call writeText success.');
        done();
      },
      fail: function (data, code) {
        console.error('File_List_013 call writeText fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    let start = new Date().getTime();
    for (let i = 0; i <= 1000; i++) {
      file.list({
        uri: 'internal://app/',
        success: function () {
          let end = new Date().getTime();
          let time = end - start;
          console.log('File_List_013 Difference time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.error('File_List_013 call list fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_List_1400
   * @tc.name File_List_014
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_List_014', 0, async function (done) {
    for (let i = 0; i < 1000; i++) {
      file.writeText({
        uri: 'internal://app/File_List_014' + i,
        text: 'Text that just for test.',
        success: function () {
          console.log('File_List_014 call writeText success.');
          done();
        },
        fail: function (data, code) {
          console.error('File_List_014 call writeText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
    let start = new Date().getTime();
    for (let i = 0; i <= 1000; i++) {
      file.list({
        uri: 'internal://app/',
        success: function () {
          let end = new Date().getTime();
          let time = end - start;
          console.log('File_List_014 Difference time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.error('File_List_014 call list fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Get_1500
   * @tc.name File_Get_015
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_Get_015', 0, async function (done) {
    file.writeText({
      uri: 'internal://app/File_Get_015',
      text: 'Text that just for test.',
      success: function () {
        console.log('File_Get_015 call writeText success.');
        done();
      },
      fail: function (data, code) {
        console.error('File_Get_015 call writeText fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    let start = new Date().getTime();
    for (let i = 0; i <= 1000; i++) {
      file.get({
        uri: 'internal://app/File_Get_015',
        success: function () {
          let end = new Date().getTime();
          let time = end - start;
          console.log('File_Get_015 Difference time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.error('File_Get_015 call get fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Get_1600
   * @tc.name File_Get_016
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_Get_016', 0, async function (done) {
    for (let i = 0; i < 1000; i++) {
      file.writeText({
        uri: 'internal://app/File_Get_016' + i,
        text: 'Text that just for test.',
        success: function () {
          console.log('File_Get_016 call writeText success.');
          done();
        },
        fail: function (data, code) {
          console.error('File_Get_016 call writeText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
    let start = new Date().getTime();
    for (let i = 0; i <= 1000; i++) {
      file.get({
        uri: 'internal://app/File_Get_016' + i,
        success: function (data) {
          console.log(data.uri);
          let end = new Date().getTime();
          let time = end - start;
          console.log('File_Get_016 Difference time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.error('File_Get_016 call get fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_writeText_1400
   * @tc.name File_writeText_014
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_writeText_014', 0, async function (done) {
    let text = randomString(4096);
    let start = new Date().getTime();
    for (let i = 0; i <= 1000; i++) {
      file.writeText({
        uri: 'internal://app/File_writeText_014',
        text: text,
        success: function () {
          let end = new Date().getTime();
          let time = end - start;
          console.log('File_writeText_014 4K Difference time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.error('File_writeText_014 call writeText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_readText_1000
   * @tc.name File_readText_010
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_readText_010', 0, async function (done) {
    let start = new Date().getTime();
    for (let i = 0; i <= 1000; i++) {
      file.readText({
        uri: 'internal://app/File_writeText_014',
        success: function () {
          let end = new Date().getTime();
          let time = end - start;
          console.log('File_readText_010 4K Difference time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.log('File_readText_010 call readText fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_writeArrayBuffer_1300
   * @tc.name File_writeArrayBuffer_013
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_writeArrayBuffer_013', 0, async function (done) {
    let buf = new Uint8Array(4096);
    for (let i = 0; i < 4096; i++) {
      buf[i] = 100
    }
    let start = new Date().getTime();
    for (let i = 0; i <= 1000; i++) {
      file.writeArrayBuffer({
        uri: 'internal://app/File_writeArrayBuffer_013',
        buffer: buf,
        success: function () {
          console.log('call writeArrayBuffer success.');
          let end = new Date().getTime();
          let time = end - start;
          console.log('File_writeArrayBuffer_013 4K Difference time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.error('File_writeArrayBuffer_013 call writeArrayBuffer fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_readArrayBuffer_1200
   * @tc.name File_readArrayBuffer_012
   * @tc.desc Function of API, Run 1000 times to obtain each running time.
   */
  it('File_readArrayBuffer_012', 0, async function (done) {
    let start = new Date().getTime();
    for (let i = 0; i <= 1000; i++) {
      file.readArrayBuffer({
        uri: 'internal://app/File_writeArrayBuffer_013',
        success: function () {
          let end = new Date().getTime();
          let time = end - start;
          console.log('File_readArrayBuffer_012 4K Difference time:' + time + ',' + i);
          start = new Date().getTime();
          done();
        },
        fail: function (data, code) {
          console.error('File_readArrayBuffer_012 call readArrayBuffer fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  });
});

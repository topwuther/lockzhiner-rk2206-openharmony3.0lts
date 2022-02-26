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
  sleep
}
  from './Common'

describe('fileIOReliability', function () {

  /**
   * @tc.number SUB_STORAGE_fileIOReliability_dir
   * @tc.name fileIOReliability_dir
   * @tc.desc Dir Reliability Test
   */
  it('fileIOReliability_dir', 0, function () {
    for (let i = 0; i < 100000; i++) {
      try {
        let dpath = nextFileName('fileIOReliability') + 'd';
        fileio.mkdirSync(dpath);
        let fpath = dpath + '/f1';
        prepareFile(fpath, FILE_CONTENT);
        let dd = fileio.opendirSync(dpath);
        dd.readSync();
        dd.closeSync();
        fileio.unlinkSync(fpath);
        fileio.rmdirSync(dpath);
        sleep(300);
      } 
      catch (e) {
        console.log('fileIOReliability_dir has failed for ' + e);
        expect(null).assertFail();
      }
    }
  });

  /**
   * @tc.number SUB_STORAGE_fileIOReliability_dirent
   * @tc.name fileIOReliability_dirent
   * @tc.desc Dirent Reliability Test
   */
  it('fileIOReliability_dirent', 0, function () {
    for (let i = 0; i < 100000; i++) {
      try {
        let dpath = nextFileName('fileIOReliability') + 'd';
        fileio.mkdirSync(dpath);
        let fpath = dpath + '/f1';
        prepareFile(fpath, FILE_CONTENT);
        let dd = fileio.opendirSync(dpath);
        let dirent = dd.readSync();
        dirent.isBlockDevice();
        dirent.isCharacterDevice();
        dirent.isDirectory();
        dirent.isFIFO();
        dirent.isSocket();
        dirent.isSymbolicLink();
        dd.closeSync();
        fileio.unlinkSync(fpath);
        fileio.rmdirSync(dpath);
        sleep(300);
      } 
      catch (e) {
        console.log('fileIOReliability_dirent has failed for ' + e);
        expect(null).assertFail();
      }
    }
  });

  /**
   * @tc.number SUB_STORAGE_fileIOReliability_stat
   * @tc.name fileIOReliability_stat
   * @tc.desc Stat Reliability Test
   */
  it('fileIOReliability_stat', 0, function () {
    for (let i = 0; i < 100000; i++) {
      try {
        let dpath = nextFileName('fileIOReliability') + 'd';
        fileio.mkdirSync(dpath);
        let fpath = dpath + '/f1';
        prepareFile(fpath, FILE_CONTENT);
        let stat = fileio.statSync(fpath);
        stat.isBlockDevice();
        stat.isCharacterDevice();
        stat.isDirectory();
        stat.isFIFO();
        stat.isSocket();
        stat.isSymbolicLink();
        fileio.unlinkSync(fpath);
        fileio.rmdirSync(dpath);
        sleep(300);
      } 
      catch (e) {
        console.log('fileIOReliability_stat has failed for ' + e);
        expect(null).assertFail();
      }
    }
  });

  /**
   * @tc.number SUB_STORAGE_fileIOReliability_stream
   * @tc.name fileIOReliability_stream
   * @tc.desc Stream Reliability Test
   */
  it('fileIOReliability_stream', 0, function () {
    for (let i = 0; i < 100000; i++) {
      let fpath = nextFileName('fileIOReliability_stream');
      expect(prepareFile(fpath, FILE_CONTENT)).assertTrue();
      let ss;
      try {
        let fd = fileio.openSync(fpath, 0o2);
        ss = fileio.fdopenStreamSync(fd, 'r+');
        ss.writeSync(FILE_CONTENT);
        ss.flushSync();
        ss.closeSync();
        ss = fileio.createStreamSync(fpath, 'r+');
        ss.readSync(new ArrayBuffer(4096));
        ss.closeSync();
        fileio.unlinkSync(fpath);
        sleep(300);
      } 
      catch (e) {
        console.log('fileIOReliability_stream has failed for ' + e);
        expect(null).assertFail();
      }
    }
  });

  /**
   * @tc.number SUB_STORAGE_fileIOReliability_prop
   * @tc.name fileIOReliability_prop
   * @tc.desc Prop Reliability Test
   */
  it('fileIOReliability_prop', 0, function () {
    for (let i = 0; i < 100000; i++) {
      try {
        let dpath = nextFileName('fileIOReliability') + 'd';
        fileio.mkdirSync(dpath);
        let fpath = nextFileName('fileIOReliability');
        let fpathTarget = fpath + 'tgt';
        let fpathTarget1 = fpath + 'tgtt';
        let ff = fileio.openSync(fpath, 0o102, 0o666);
        fileio.accessSync(fpath);
        fileio.chmodSync(fpath, 0o660);
        let stat = fileio.statSync(fpath);
        fileio.chownSync(fpath, stat.uid, stat.gid);
        fileio.copyFileSync(fpath, fpathTarget);
        fileio.fchmodSync(ff, 0o660);
        fileio.fchownSync(ff, stat.uid, stat.gid);
        fileio.fstatSync(ff);
        fileio.ftruncateSync(ff);
        fileio.renameSync(fpath, fpathTarget1);
        fileio.fsyncSync(ff);
        fileio.truncateSync(fpathTarget1);
        fileio.writeSync(ff, FILE_CONTENT);
        fileio.closeSync(ff);
        fileio.unlinkSync(fpathTarget1);
        fileio.unlinkSync(fpathTarget);
        fileio.rmdirSync(dpath);
        sleep(300);
      } 
      catch (e) {
        console.log('fileIOReliability_prop has failed for ' + e);
        expect(null).assertFail();
      }
    }
  });
});

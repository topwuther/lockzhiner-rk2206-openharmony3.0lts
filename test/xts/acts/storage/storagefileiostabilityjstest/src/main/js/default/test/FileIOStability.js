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

describe('fileIOStability', function () {

  /**
   * @tc.number SUB_STORAGE_fileIOStability_dir
   * @tc.name fileIOStability_dir
   * @tc.desc Function of API, 7*24
   */
  it('fileIOStability_dir', 0, function () {
    for (let i = 0; i <= 100000; i++) {
      try {
        let dpath = nextFileName('fileIOStability') + 'd';
        fileio.mkdirSync(dpath);
        let fpath = dpath + '/f1';
        prepareFile(fpath, FILE_CONTENT);
        let dd = fileio.opendirSync(dpath);
        dd.readSync();
        dd.closeSync();
        fileio.unlinkSync(fpath);
        fileio.rmdirSync(dpath);
      } 
      catch (e) {
        console.log('fileIOStability_dir has failed for ' + e);
        expect(null).assertFail();
      }
    }
  });

  /**
   * @tc.number SUB_STORAGE_fileIOStability_dirent
   * @tc.name fileIOStability_dirent
   * @tc.desc Function of API, 7*24
   */
  it('fileIOStability_dirent', 0, function () {
    for (let i = 0; i <= 100000; i++) {
      try {
        let dpath = nextFileName('fileIOStability') + 'd';
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
      } 
      catch (e) {
        console.log('fileIOStability_dirent has failed for ' + e);
        expect(null).assertFail();
      }
    }
  });

  /**
   * @tc.number SUB_STORAGE_fileIOStability_stat
   * @tc.name fileIOStability_stat
   * @tc.desc Function of API, 7*24
   */
  it('fileIOStability_stat', 0, function () {
    for (let i = 0; i <= 100000; i++) {
      try {
        let dpath = nextFileName('fileIOStability') + 'd';
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
        stat.dev;
        stat.uid;
        stat.ino;
        stat.mode;
        stat.nlink;
        stat.gid;
        stat.rdev;
        stat.size;
        stat.blocks;
        stat.atime;
        stat.mtime;
        stat.ctime;
        fileio.unlinkSync(fpath);
        fileio.rmdirSync(dpath);
      } 
      catch (e) {
        console.log('fileIOStability_stat has failed for ' + e);
        expect(null).assertFail();
      }
    }
  });

  /**
   * @tc.number SUB_STORAGE_fileIOStability_stream
   * @tc.name fileIOStability_stream
   * @tc.desc Function of API, 7*24
   */
  it('fileIOStability_stream', 0, function () {
    for (let i = 0; i <= 100000; i++) {
      let fpath = nextFileName('fileIOStability_stream');
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
      } 
      catch (e) {
        console.log('fileIOStability_stream has failed for ' + e);
        expect(null).assertFail();
      }
    }
  });

  /**
   * @tc.number SUB_STORAGE_fileIOStability_prop
   * @tc.name fileIOStability_prop
   * @tc.desc Function of API, 7*24
   */
  it('fileIOStability_prop', 0, function () {
    for (let i = 0; i <= 100000; i++) {
      try {
        let dpath = nextFileName('fileIOStability') + 'd';
        fileio.mkdirSync(dpath);
        let fpath = nextFileName('fileIOStability');
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
        sleep(4000);
      } 
      catch (e) {
        console.log('fileIOStability_pop has failed for ' + e);
        expect(null).assertFail();
      }
    }
  });

});

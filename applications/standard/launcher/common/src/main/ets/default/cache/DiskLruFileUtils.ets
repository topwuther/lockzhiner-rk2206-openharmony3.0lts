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

import Fileio from '@ohos.fileio';

const writeFilePath = "/data/accounts/account_0/appdata/com.ohos.launcher/cache/";
const journalPath = "/data/accounts/account_0/appdata/com.ohos.launcher/cache/journal.txt";
const READ_DATA_SIZE = 4096;

/**
 * An util that provides io functionality which is used by DiskLruCache.
 */
export default class DiskLruFileUtils {

  /**
   * Read Json file from disk by bundleName.
   *
   * @param {string} bundleName - bundleName of the target file
   * @return {object} read object from file
   */
  static readJsonObj(bundleName) {
    console.info("Launcher FileUtil readJsonObj start execution");
    let filePath = writeFilePath + bundleName + ".json";
    return this.readJsonFile(filePath);
  }

  /**
   * Read Json file from disk by file path.
   *
   * @param {string} path - path of the target file.
   * @return {object} read object from file
   */
  static readJsonFile(path) {
    console.info("Launcher FileUtil readJsonFile start execution");
    let readStreamSync = null;
    try {
      readStreamSync = Fileio.createStreamSync(path, "r");
      let content = this.getContent(readStreamSync);
      console.info("Launcher FileUtil readJsonFile finish execution" + content);
      return JSON.parse(content);
    } catch (e) {
      console.info("Launcher FileUtil readJsonFile " + e);
    } finally {
      readStreamSync.closeSync();
    }
  }

  /**
   * Write Json object to a file.
   *
   * @param {object} jsonObj - target JSON object will be written
   * @param {string} bundleName - use bundleName as target file name
   */
  static writeJsonObj(jsonObj, bundleName) {
    console.info("Launcher FileUtil writeJsonObj start execution");
    let filePath = writeFilePath + bundleName + ".json";
    let content = JSON.stringify(jsonObj);
    let writeStreamSync = null;
    try {
      writeStreamSync = Fileio.createStreamSync(filePath, "w+");
      writeStreamSync.writeSync(content);
    } catch (e) {
      console.info("Launcher FileUtil writeJsonObj error: " + e);
    } finally {
      writeStreamSync.closeSync();
      console.info("Launcher FileUtil writeJsonObj close sync");
    }
  }

  /**
   * Record a key that maps the image as value.
   *
   * @param {string} content - the key maps the image file
   */
  static writeJournal(content) {
    let writeStreamSync = null;
    try {
      console.info("Launcher FileUtil writeJournal start");
      writeStreamSync = Fileio.createStreamSync(journalPath, "a+");
      writeStreamSync.writeSync(content + "\n");
    } catch (e) {
      console.info("Launcher FileUtil writeJournal error: " + e);
    } finally {
      writeStreamSync.closeSync();
      console.info("Launcher FileUtil writeJournal close sync");
    }
  }

  /**
   * Get the keys that map the images.
   *
   * @return {object} object read from file
   */
  static readJournal() {
    console.info("Launcher FileUtil readJournal start execution");
    let readStreamSync = null;
    try {
      readStreamSync = Fileio.createStreamSync(journalPath, "r");
      return this.getContent(readStreamSync);
    } catch (e) {
      console.info("Launcher FileUtil readJournal error: " + e);
    } finally {
      readStreamSync.closeSync();
      console.info("Launcher FileUtil readJournal closeSync");
    }
  }

  /**
   * Read JSON object from a file.
   *
   * @param {object} readStreamSync - stream of target file
   * @return {object} object read from file stream
   */
  static getContent(readStreamSync) {
    console.info("Launcher FileUtil getContent start");
    let bufArray = [];
    let totalLength = 0;
    let buf = new ArrayBuffer(READ_DATA_SIZE);
    let len = readStreamSync.readSync(buf);
    while (len != 0) {
      console.info("Launcher FileUtil getContent FileIO reading " + len);
      totalLength += len;
      if (len < READ_DATA_SIZE) {
        buf = buf.slice(0, len);
        bufArray.push(buf);
        break;
      }
      bufArray.push(buf);
      buf = new ArrayBuffer(READ_DATA_SIZE);
      len = readStreamSync.readSync(buf);
    }
    console.info("Launcher FileUtil getContent read finished " + totalLength);
    let contentBuf = new Uint8Array(totalLength);
    let offset = 0;
    for (let bufArr of bufArray) {
      console.info("Launcher FileUtil getContent collecting " + offset);
      let uInt8Arr = new Uint8Array(bufArr);
      contentBuf.set(uInt8Arr, offset);
      offset += uInt8Arr.byteLength;
    }
    let content = String.fromCharCode.apply(null, new Uint8Array(contentBuf));
    return content;
  }

  /**
   * Remove file.
   *
   * @param {string} bundleName - bundleName as target file name
   */
  static removeFile(bundleName) {
    try {
      console.info("Launcher FileUtil removeFile")
      //remove file,key : bundlename
      Fileio.unlinkSync(writeFilePath + bundleName + ".json")
    } catch (e) {
      console.error("Launcher FileUtil removeFile delete has failed for " + e)
    }
  }
}
 
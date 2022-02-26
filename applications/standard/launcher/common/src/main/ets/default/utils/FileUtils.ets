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

/**
 * An util that provides io functionality between file and JSON object.
 */
import Fileio from '@ohos.fileio';

const writeFilePath = "/data/accounts/account_0/appdata/com.ohos.launcher/cache/";
const READ_DATA_SIZE = 4096;

export default class FileUtils {

  /**
   * Read Json file from disk by file path.
   *
   * @param {string} path - path of the target file.
   * @return {object} - read object from file
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
   * Read String from disk by bundleName.
   *
   * @param {string} bundleName - bundleName as target file name
   * @return {string} - read string from file
   */
  static readStringFromFile(bundleName) {
    console.info("Launcher FileUtil readStringFromFile start execution");
    let filePath = writeFilePath + bundleName + ".json";
    let readStreamSync = null;
    try {
      readStreamSync = Fileio.createStreamSync(filePath, "r");
      let content = this.getContent(readStreamSync);
      console.info("Launcher FileUtil readStringFromFile finish execution" + content);
      return content;
    } catch (e) {
      console.info("Launcher FileUtil readStringFromFile " + e);
    } finally {
      readStreamSync.closeSync();
    }
  }

  /**
   * Write string to a file.
   *
   * @param {string} string - target string will be written to file
   * @param {string} bundleName - bundleName as target file name
   */
  static writeStringToFile(string, bundleName) {
    console.info("Launcher FileUtil writeStringToFile start execution");
    let filePath = writeFilePath + bundleName + ".json";
    let writeStreamSync = null;
    try {
      writeStreamSync = Fileio.createStreamSync(filePath, "w+");
      writeStreamSync.writeSync(string);
    } catch (e) {
      console.info("Launcher FileUtil writeStringToFile error: " + e);
    } finally {
      writeStreamSync.closeSync();
      console.info("Launcher FileUtil writeStringToFile close sync");
    }
  }

  /**
   * Read JSON object from a file.
   *
   * @param {object} readStreamSync - stream of target file
   * @return {object} - object read from file stream
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
}
 
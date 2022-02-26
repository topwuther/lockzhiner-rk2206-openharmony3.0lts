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
import Log from '../../../../../../../../common/src/main/ets/default/Log.ets'

const DEFAULT_LAYOUT_INFO_FILE_PATH = "/data/accounts/account_0/applications/com.ohos.systemui/" +
  "com.ohos.systemui.navigationbar/assets/navigationBar/resources/rawfile/navigationBarLayoutConfig.json";
const READ_DATA_SIZE = 4096;
const TAG = "ConfigReader";

export default class ConfigReader {

  public readLayoutConfig() {
    Log.showInfo(TAG, "navigationbar ConfigReader readLayoutConfig start");
    var mLayoutConfig = AppStorage.SetAndLink("navigationLayoutConfig", {});
    var configObj = this.readFileToJson(DEFAULT_LAYOUT_INFO_FILE_PATH);
    mLayoutConfig.set(configObj)
  }

  public readFileToJson(path : string) {
    let readStreamSync = null;
    try {
      readStreamSync = Fileio.createStreamSync(path, "r");
      let content = this.getContent(readStreamSync);
      Log.showInfo(TAG, "navigationbar ConfigReader readJsonFile finish execution" + content);
      return JSON.parse(content);
    } catch (e) {
      Log.showError(TAG, "navigationbar ConfigReader readJsonFile " + e);
    } finally {
      readStreamSync.closeSync();
    }
  }

  public getContent(readStreamSync) {
    Log.showInfo(TAG, "navigationbar ConfigReader getContent start");
    let buf = new ArrayBuffer(READ_DATA_SIZE);
    let len = readStreamSync.readSync(buf);
    let arr = new Uint8Array(buf)
    let charAt = ' '.charCodeAt(0)
    for (let i = len; i < READ_DATA_SIZE; i++) {
      arr[i] = charAt
    }
    let contentBuf = String.fromCharCode.apply(null, arr)
    Log.showInfo(TAG, "navigationbar ConfigReader getContent read finished " + contentBuf);
    return contentBuf;
  }
}
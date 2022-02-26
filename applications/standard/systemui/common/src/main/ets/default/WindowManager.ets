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

import Window from '@ohos.window';
import display from '@ohos.display'
import Log from './Log.ets';

const TAG = 'WindowManager';
const STATUS_BAR_H = 0.07;

var maxWidth;
var maxHeight;
var minHeight;
var quicklySettingH;
var notificationH;

/**
 * Manage window size changes.
 */
export default class WindowManager {

  initWindowManager(){
    Log.showInfo(TAG, 'initWindowManager ');
    maxWidth = AppStorage.SetAndLink("maxWidth", 0);
    maxHeight = AppStorage.SetAndLink("maxHeight", 0);
    minHeight = AppStorage.SetAndLink("minHeight", 0);
    quicklySettingH = AppStorage.SetAndLink("quicklySettingH", 0);
    notificationH = AppStorage.SetAndLink("notificationH", 0);

    display.getDefaultDisplay().then(dis => {
      maxWidth.set(dis.width);
      maxHeight.set(dis.height);
      minHeight.set(parseInt((dis.height*STATUS_BAR_H).toString()));
      quicklySettingH.set((dis.height-(minHeight.get()*2))*0.15);
      notificationH.set(dis.height-(minHeight.get()*2)-quicklySettingH.get());

      Log.showInfo(TAG, `initWindowManager maxWidth ${maxWidth.get()} maxHeight ${maxHeight.get()} minHeight ${minHeight.get()}`);
    })
  }

  /**
     * Set the window to the maximum size.
     *
     * @param {Object} callback - Callback function.
     */
  setWindowMax(callback) {
    Log.showInfo(TAG, 'enter setWindowMax =================');
    Window.getTopWindow().then((windowData)=> {
      windowData.resetSize(maxWidth.get(), maxHeight.get()).then((result)=> {
        Log.showInfo(TAG, result);
        callback(result);
      });
    });
  }

  /**
     * Set the window to the minimum size.
     *
     * @param {Object} callback - Callback function.
     */
  setWindowMin(callback) {
    Log.showInfo(TAG, 'enter setWindowMin =================');
    Window.getTopWindow().then((windowData)=> {
      windowData.resetSize(maxWidth.get(), minHeight.get()).then((result)=> {
        Log.showInfo(TAG, result);
        callback(result);
      });
    });
  }
}
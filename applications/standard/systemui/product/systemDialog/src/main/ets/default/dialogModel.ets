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

import FeatureAbilityManager from '../../../../../../common/src/main/ets/default/abilitymanager/featureAbilityManager.ets';
import Log from '../../../../../../common/src/main/ets/default/Log.ets';

const TAG = 'SystemDialog_SystemDialogCenter';

let listener;
let mFeatureAbilityManager = new FeatureAbilityManager();

export class DialogModel {

  /**
     * Page listener registration
     *
     * @param {Object} callback - Function callback
     */
  registerListener(callback) {
    if (callback == null && callback == undefined) {
      Log.showInfo(TAG, 'registerListener callback is null or undefined');
      return;
    }
    listener = callback;
    mFeatureAbilityManager.getAbilityWant(listener)
  }

  /**
     * Page listener unregistration
     */
  unregisterListener() {
    listener = null;
  }

  /**
     * Return of the authority information selected by the user
     *
     * @param {Object} abilityResult - information selected by the user
     */
  finishResult(abilityResult) {
    if (Object.keys(abilityResult).length !== 0) {
      Log.showInfo(TAG, `finishResult parameter ${JSON.stringify(abilityResult)}`);
      mFeatureAbilityManager.finishAbilityWithResult(abilityResult);
    } else {
      Log.showInfo(TAG, 'finishResult abilityResult is a empty object');
    }
  }
}

let mDialogModel = new DialogModel();
export default mDialogModel as DialogModel;
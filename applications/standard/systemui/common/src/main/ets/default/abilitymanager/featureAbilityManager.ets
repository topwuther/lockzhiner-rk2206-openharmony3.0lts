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

import FeatureAbility from '@ohos.ability.featureAbility';
import Log from '../Log.ets';

const TAG = 'FeatureAbilityManager';

export default class FeatureAbilityManager {

  openAbility(tag, want) {
    Log.showInfo(TAG, `openAbility from: ${tag}`));
    let result = FeatureAbility.startAbility(want)
      .then(data =>
    Log.showInfo(TAG, `tag: ${tag} promise then: ${JSON.stringify(data)}`))
      .catch(error =>
    Log.showError(TAG, `tag: ${tag} promise catch: ${JSON.stringify(error)}`));
    Log.showInfo(TAG, `tag: ${tag} openAbility result: ${result}`);
  }

  getAbilityWant(listener) {
    FeatureAbility.getWant((err, data) => {
      Log.showInfo(TAG, `getAbilityWant callBack err: ${JSON.stringify(err)} data: ${JSON.stringify(data)}`);
      if (err.code !== 0) {
        Log.showError(TAG, `failed to getAbilityWant because ${err.message}`);
        return;
      } else {
        if(listener != null && listener != undefined) {
          listener(data);
        }
      }
    });
  }

  finishAbilityWithResult(abilityResult) {
    FeatureAbility.finishWithResult(abilityResult, (err, data) => {
      if (err.code !== 0) {
        Log.showError(TAG, `failed to finishWithResult because ${JSON.stringify(err)}`);
        return;
      }
      FeatureAbilityManager.finishAbility();
    });
  }

  static finishAbility() {
    FeatureAbility.terminateAbility((err, data) => {
      if (err.code !== 0) {
        Log.showError(TAG, `failed to finishAbility because ${JSON.stringify(err)}`);
        return;
      }
      Log.showInfo(TAG, ` finishAbility callback err: ${JSON.stringify(err)} data:${data}`);
    });
  }
}
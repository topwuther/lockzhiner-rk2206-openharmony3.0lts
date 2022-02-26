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

import Log from '../../../../../../../../common/src/main/ets/default/Log.ets';
import Constants from './common/constants.ets';
import FeatureAbilityManager from '../../../../../../../../common/src/main/ets/default/abilitymanager/featureAbilityManager.ets';
import Input from '@ohos.injectEventHandler';

const TAG = 'KeyCodeEvent'

let res;
let mFeatureAbilityManager = new FeatureAbilityManager();

export class KeyCodeEvent{

  public sendKeyEvent(keyCode: number, eventType: number) {
    Log.showInfo(TAG, `sendKeyEvent keycode: ${keyCode} type: ${eventType}`);
    switch (keyCode) {
      case Constants.KEYCODE_BACK:
        Log.showInfo(TAG, `sendKeyEvent case back type: ${eventType}`);
        if (eventType === Constants.KEY_DOWN) {
          Log.showInfo(TAG, 'sendKeyEvent, if');
          this.sendBackKeyEventStart();//down
        } else if (eventType === Constants.KEY_UP) {
          Log.showInfo(TAG, `sendKeyEvent, else type: ${eventType}`);
          this.sendBackKeyEventEnd();//up
        }
        break;
      case Constants.KEYCODE_HOME:
        Log.showInfo(TAG, `sendKeyEvent case home type: ${eventType}`);
        if (eventType === Constants.KEY_UP) {
          Log.showInfo(TAG, 'sendKeyEvent, if');
          this.sendHomeKeyEvent();
        }
        break;
      case Constants.KEYCODE_RECENT:
        Log.showInfo(TAG, `sendKeyEvent case recent type: ${eventType}`);
        if (eventType === Constants.KEY_UP) {
          Log.showInfo(TAG, 'sendKeyEvent, if');
          this.sendRecentKeyEvent();
        }
        break;
      default:
        Log.showInfo(TAG, `sendKeyEvent case default keycode: ${keyCode} type: ${eventType}`);
        break;
    }
  }

  private sendBackKeyEventStart() {
    Log.showInfo(TAG, 'sendBackKeyEventStart');
    res = Input.injectEventSync({
      isPressed: true,
      keyCode: 2,
      keyDownDuration: 1
    });
    Log.showInfo(TAG, `injectEventHandler injectEventSync down res: ${res}`);
  }

  private sendBackKeyEventEnd() {
    Log.showInfo(TAG, 'sendBackKeyEventEnd');
    res = Input.injectEventSync({
      isPressed: false,
      keyCode: 2,
      keyDownDuration: 1
    });
    Log.showInfo(TAG, `injectEventHandler injectEventSync up res: ${res}`);
  }

  private sendHomeKeyEvent() {
    Log.showInfo(TAG, 'sendHomeKeyEvent');
    Log.showInfo(TAG, 'home click and start ability launcher');
    mFeatureAbilityManager.openAbility(TAG, {
      want: {
        bundleName: Constants.LAUNCHER_BUNDLE_NAME,
        abilityName: Constants.LAUNCHER_ABILITY_NAME
      }
    });
  }

  private sendRecentKeyEvent() {
    Log.showInfo(TAG, 'sendRecentKeyEvent');
    mFeatureAbilityManager.openAbility(TAG, {
      want: {
        bundleName: Constants.RECENT_BUNDLE_NAME,
        abilityName: Constants.RECENT_ABILITY_NAME
      }
    });
  }
}

let keyCodeEvent = new KeyCodeEvent();
export default keyCodeEvent as KeyCodeEvent;
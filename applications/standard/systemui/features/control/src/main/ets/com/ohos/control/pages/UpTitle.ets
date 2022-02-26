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

import FeatureAbilityManager from '../../../../../../../../../common/src/main/ets/default/abilitymanager/featureAbilityManager.ets';
import WindowManager from '../../../../../../../../../common/src/main/ets/default/WindowManager.ets';
import Log from '../../../../../../../../../common/src/main/ets/default/Log.ets';
const TAG = 'Control-UpTitle'

let mWindowManager;
let mFeatureAbilityManager;

@Component
export
struct UpTitle {
  @Link showStatusBar: boolean

  aboutToAppear() {
    Log.showInfo(TAG,'aboutToAppear');
    mWindowManager = new WindowManager();
    mFeatureAbilityManager = new FeatureAbilityManager();
  }

  aboutToDisappear () {
    Log.showInfo(TAG,'aboutToDisappear')
  }

  build() {
    Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.End, justifyContent: FlexAlign.SpaceBetween }) {
      Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Start }) {
        Text($r('app.string.control_center'))
          .fontColor($r('app.color.title_font_color'))
          .fontSize($r('app.float.title_font_size'))
      }
      .height('100%')

      Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.End }) {
        Image($r('app.media.ic_notification_settings'))
          .objectFit(ImageFit.Contain)
          .size({ width: $r('app.float.notification_settings_width'),
          height: $r('app.float.notification_settings_height') })
          .onClick(this.settingClick.bind(this))
      }
      .height('100%')

    }
  }

  settingClick() {
    this.showStatusBar = true;
    mWindowManager.setWindowMin((result)=>{
      mFeatureAbilityManager.openAbility(TAG, {
        want: {
          bundleName: 'com.ohos.settings',
          abilityName: 'com.ohos.settings.MainAbility',
        }
      });
    });
  }
}
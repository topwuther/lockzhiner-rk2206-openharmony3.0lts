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

import Constants from '../common/constants.ets'
import mWifiModel from '../WifiModel.ets'
import Log from '../../../../../../../common/src/main/ets/default/Log.ets'
const TAG = 'WifiComponent-WifiIcon'

@Component
export default struct WifiIcon {
  @StorageLink('wifiInfo') wifiInfo : number = Constants.DEFAULT_WIFI_INFO
  @StorageLink('wifiStatus') wifiStatus : boolean = Constants.DEFAULT_WIFI_STATUS

  aboutToAppear() {
    Log.showInfo(TAG, `aboutToAppear`)
    mWifiModel.initWifiModel()
  }

  aboutToDisappear() {
    Log.showInfo(TAG, `aboutToDisappear`)
    //    mWifiModel.uninitWifiModel()
  }

  build() {
    Flex({ alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      if (this.wifiStatus) {
        Image(this.getImage(this.wifiInfo)).objectFit(ImageFit.Contain)
      }
    }
    .width(this.wifiStatus ? $r('app.float.WIFISTATUS_MAXWIDTH') : 0)
    .height(this.wifiStatus ? '50%' : $r('app.float.WIFISTATUS_HIDDEN_HEIGHT'))
  }

  private getImage(wifiInfo: number) {
    Log.showInfo(TAG, `getImage, wifiInfo: ${ JSON.stringify(wifiInfo) }`)
    let wifiImage ;
    switch (wifiInfo) {
      case Constants.WIFI_SIGNAL_NO:
        wifiImage = $r('app.media.ic_statusbar_wifi_no');
        break;
      case Constants.WIFI_SIGNAL_LOW:
        wifiImage = $r('app.media.ic_statusbar_wifi_1');
        break;
      case Constants.WIFI_SIGNAL_MID:
        wifiImage = $r('app.media.ic_statusbar_wifi_2');
        break;
      case Constants.WIFI_SIGNAL_HIGH:
        wifiImage = $r('app.media.ic_statusbar_wifi_3');
        break;
      case Constants.WIFI_SIGNAL_FULL:
        wifiImage =$r('app.media.ic_statusbar_wifi_full');
        break;
      default:
        wifiImage = $r('app.media.ic_statusbar_wifi_no');
        break;
    }
    return wifiImage;
  }
}

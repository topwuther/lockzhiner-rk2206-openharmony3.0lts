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

import Constants from '../common/constants.ets';
import mSignalModel from '../SignalModel.ets';
import Log from '../../../../../../../common/src/main/ets/default/Log.ets'
const TAG = 'SignalComponent-SignalIcon'

@Component
export default
struct SignalIcon {
  @StorageLink('cellularLevel') cellularLevel : number = Constants.CELLULAR_NO_SIM_CARD
  @StorageLink('cellularType') cellularType : number = Constants.NETWORK_TYPE_UNKNOWN
  @StorageLink('networkState') networkState : string = Constants.NET_NULL
  aboutToAppear() {
    Log.showInfo(TAG, 'aboutToAppear');
    mSignalModel.initSignalModel();
  }

  aboutToDisappear() {
    Log.showInfo(TAG, 'aboutToDisappear');
    mSignalModel.uninitSignalModel();
  }

  build() {
    Row() {
      Text(this.updateNetworkState(this.networkState))
        .fontSize($r('app.float.fontSize_network_state'))
        .fontColor($r('app.color.font_color_white'))
        .textOverflow({overflow: TextOverflow.Ellipsis})
        .constraintSize({ maxWidth: $r('app.float.signalicon_netstatus_width')})
        .flexShrink(0)
        .maxLines(1)
        .textAlign(TextAlign.Center)

      Stack({ alignContent: Alignment.TopStart }) {
        Text(this.updateCellularType(this.cellularType))
          .fontSize($r('app.float.fontSize_cellular_type'))
          .fontColor($r('app.color.font_color_white'))
          .margin({ top: $r('app.float.margin_cellular_type') })
        Image(this.updateCellularImage(this.cellularLevel))
          .objectFit(ImageFit.ScaleDown)
      }.flexShrink(1).width(42)
    }
    .margin({ left: 5})
    .height('50%')
  }

/**
     * Get the string of cellular type
     *
     * @param {number} type - number of cellular type
     * @return {string} typeString type of cellular type
     */
  private updateCellularType(signalType) : string {
    Log.showInfo(TAG, `updateCellularType, signalType: ${ signalType }`);
    let typeString;
    switch (signalType) {
      case Constants.NETWORK_TYPE_UNKNOWN:
        typeString = '';
        break;
      case Constants.NETWORK_TYPE_GSM:
      case Constants.NETWORK_TYPE_CDMA:
        typeString = $r('app.string.2G');
        break;
      case Constants.NETWORK_TYPE_W_CDMA:
      case Constants.NETWORK_TYPE_TDS_CDMA:
        typeString = $r('app.string.3G');
        break;
      case Constants.NETWORK_TYPE_LTE:
        typeString = $r('app.string.4G');
        break;
      case Constants.NETWORK_TYPE_NR:
      default:
        typeString = $r('app.string.5G');
        break;
    }
    return typeString;
  }

/**
     * Get the cellular signal image
     *
     * @param {number} level - signal level from signalModel
     * @return {string} cellularImage image of cellular signal
     */
  private updateCellularImage(level) {
    Log.showInfo(TAG, `updateCellularImage, level: ${level}`);
    let cellularImage;
    switch (level) {
      case Constants.CELLULAR_SIGNAL_NO:
        cellularImage = $r('app.media.ic_statusbar_signal_no');
        break;
      case Constants.CELLULAR_SIGNAL_MIN:
        cellularImage = $r('app.media.ic_statusbar_signal_1');
        break;
      case Constants.CELLULAR_SIGNAL_LOW:
        cellularImage = $r('app.media.ic_statusbar_signal_2');
        break;
      case Constants.CELLULAR_SIGNAL_HALF:
        cellularImage = $r('app.media.ic_statusbar_signal_3');
        break;
      case Constants.CELLULAR_SIGNAL_HIGH:
        cellularImage = $r('app.media.ic_statusbar_signal_4');
        break;
      case Constants.CELLULAR_SIGNAL_FULL:
        cellularImage = $r('app.media.ic_statusbar_signal_full');
        break;
      case Constants.CELLULAR_NO_SIM_CARD:
      default:
        cellularImage = $r('app.media.ic_statusbar_signal_null');
        break;
    }
    console.info('systemui updateCellularImage = ' + cellularImage);
    return cellularImage;
  }

/**
     * Get the NetworkState signal name
     *
     * @param {string} netWorkState - network state from signal model
     * @return {string} vendor's name or signal state
     */
  private updateNetworkState(netWorkState) {
    Log.showInfo(TAG, `updateNetworkState, netWorkState: ${netWorkState}`);
    let networkStateName;
    if(netWorkState == Constants.NET_NULL){
      networkStateName = $r('app.string.net_null');
    }else {
      networkStateName = netWorkState;
    }
    return networkStateName;
  }
}
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

import Constants from './common/constants.ets'
import WifiInfo from '@ohos.wifi_native_js';
import Subscriber from '@ohos.commonEvent';
import Log from "../../../../../../common/src/main/ets/default/Log.ets";

const TAG = 'WifiComponent-WifiModel';

var mCommonEventSubscribeInfo = {
  events: [Constants.EVENT_CONN_STATE, Constants.EVENT_RSSI_VALUE,
  Constants.EVENT_POWER_STATE, Constants.Event_CONN_NAME]
};
var mCommonEventSubscriber = null;
var mWifiInfo;
var mWifiStatus;
var mWifiOpenStatus;
var mWifiName;

export class WifiModel {

  initWifiModel() {
    Log.showInfo(TAG, `initWifiModel`)
    mWifiInfo = AppStorage.SetAndLink("wifiInfo", 0);
    mWifiStatus = AppStorage.SetAndLink("wifiStatus", false);
    mWifiName = AppStorage.SetAndLink("wifiName", 'WLAN')
    mWifiOpenStatus = AppStorage.SetAndLink("wifiOpenStatus", false);
    if(mCommonEventSubscriber == null){
      this.registerWiFiStatusListener();
    }
    //    this.getWifiMessage();
  }

  uninitWifiModel() {
    Log.showInfo(TAG, `uninitWifiModel`)
    this.unregisterWiFiStatusListener()
  }

  /**
   * Get Wifi value for the first time when restart device
   *
   */
  getWifiMessage() {
    Log.showInfo(TAG, `wifiModel getWifiMessage ===========`);
    this.changeWifiStatus(WifiInfo.isWifiActive());

  }

  /**
   * Subscribe wifi events
   *
   */
  registerWiFiStatusListener() : void {
    Log.showInfo(TAG, `register Wifi status listener ===========`);
    Subscriber.createSubscriber(
      mCommonEventSubscribeInfo,
      this.createWifiStatusSubscriberCallBack.bind(this)
    );
  }

  /**
   * Callback of the subscriber
   *
   * @param {Object} err - error returns from the caller
   * @param {Object} data - data returns from the caller
   */
  createWifiStatusSubscriberCallBack(err, data) {
    Log.showInfo(TAG, `createWifiStatusSubscriberCallBack start err: ${JSON.stringify(err)} data: ${JSON.stringify(data)}`);
    mCommonEventSubscriber = data;
    Subscriber.subscribe(mCommonEventSubscriber, this.wifiStatusSubscriberCallBack.bind(this));
  }

  /**
   * Callback of the events
   *
   * @param {Object} err - error returns from the caller
   * @param {Object} data - data returns from the caller
   */
  private wifiStatusSubscriberCallBack(err, data) {
    Log.showInfo(TAG, `createWifiStatusSubscriberCallBack start err:${ JSON.stringify(err)} data: ${ JSON.stringify(data) }`);
    if (err.code == Constants.DEFAULT_ERR_CODE) {
      Log.showInfo(TAG, `wifi data == ${JSON.stringify(data)}`);
      if (data.event == Constants.EVENT_CONN_STATE && data.code == Constants.WIFI_STATE_AP_CONNECTING) {
        this.updateWifiInfo();
        this.changeWifiStatus(true);
      }
      if (data.event == Constants.EVENT_POWER_STATE && data.code == Constants.WIFI_POWER_OFF) {
        this.updateWifiInfo();
        this.changeWifiStatus(false);
        mWifiOpenStatus.set(false);
        mWifiName.set('WLAN');
      }
      if (data.event == Constants.EVENT_POWER_STATE && data.code == Constants.WIFI_POWER_ON) {
        mWifiOpenStatus.set(true);
      }
      if(data.event == Constants.EVENT_CONN_STATE && data.code == Constants.WIFI_Disconnected){
        this.updateWifiInfo();
        this.changeWifiStatus(false);
        mWifiName.set('WLAN');
      }
      if (data.event == Constants.EVENT_RSSI_VALUE) {
        this.updateWifiInfo();
      }
      if(data.event == 'WIFI_CONNECT_NAME'){
        this.updateWifiName(JSON.parse(data.data).name)
        this.changeWifiStatus(true)
        let rssi = JSON.parse(data.data).rssi
        let band = JSON.parse(data.data).band
        Log.showInfo(TAG, `WIFI_CONNECT_NAME ================ ${rssi}`)
        this.updateWifiInfo(rssi,band);
      }
    } else {
      Log.showError(TAG, `error when subscribing ========`);
    }
  }

  /**
   * Unsubscribe wifi events
   *
   */
  private unregisterWiFiStatusListener() {
    Subscriber.unsubscribe(mCommonEventSubscriber, () => {
      Log.showInfo(TAG, `unregister Wifi Status Listener ===============`);
    });
  }

  /**
   * Check the connection status of Wifi
   *
   * @param {boolean} status - if display wifi icon
   */
  private changeWifiStatus(status) {
    Log.showInfo(TAG, `enter changeWifiStatus ================ ${status}`);
    mWifiStatus.set(status)
  }

  private updateWifiInfo(rssi=Constants.UPDATE_WIFI_INFO_DEFAULT_PARAM,band=Constants.SIGN_LEVEL) {
    Log.showInfo(TAG, `enter changeWifiInfos ================ ${rssi}+${band}`);
    mWifiInfo.set(this.getWifiInfo(rssi,band))
    Log.showInfo(TAG, `mWifiInfo wifiInfo ${mWifiInfo.get()}`);
  }

  private updateWifiName(name) {
    Log.showInfo(TAG, `enter changeWifiNames ================ ${name}`);
    mWifiName.set(name);
  }


  /**
   * Update the image of wifi status
   *
   * @param {number} rssi - the rssi number of the connected wifi
   * @return {string} image used to show wifi status
   */
  private getWifiInfo(rssi,band) {
    //The current version keeps return 0 as result, set the level as 4(full level) by hand.
    Log.showInfo(TAG, `getWifiImage enter =========`);
    //Fake number of band and rssi for wifi signal level temporarily
    let level = WifiInfo.getSignalLevel(rssi, band);
    Log.showInfo(TAG, `wifi level = ${level}`);
    return level;
  }

  enableWifi() {
    Log.showInfo(TAG, 'enableWifi ing');
    WifiInfo.enableWifi();
  }

  disableWifi() {
    Log.showInfo(TAG, 'disableWifi ing');
    WifiInfo.disableWifi();
  }
}

let mWifiModel = new WifiModel();
export default mWifiModel as WifiModel;

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

import Radio from '@ohos.telephony_radio';
import Sim from '@ohos.telephony_sim'
import Observer from '@ohos.telephony_observer';
import Log from "../../../../../../common/src/main/ets/default/Log.ets";
import CheckEmpty from '../../../../../../common/src/main/ets/default/CheckEmptyUtils.ets';
import Constants from './common/constants.ets'

const TAG = 'SignalStatus-SignalModel';
const EMPTY_LEVEL = 0;

let mSignalCallback;
let signalValue = {
  cellularLevel: '',
  cellularType: '',
  networkState: ''
};

var mLevelLink;
var mTypeLink;
var mStateLink;

export class SignalModel

{

  initSignalModel() {
    Log.showInfo(TAG, 'initSignalModel');
    mLevelLink = AppStorage.SetAndLink("cellularLevel", Constants.CELLULAR_NO_SIM_CARD);
    mTypeLink = AppStorage.SetAndLink("cellularType", Constants.NETWORK_TYPE_UNKNOWN);
    mStateLink = AppStorage.SetAndLink("networkState", Constants.NET_NULL);
    this.initObserver();
    this.checkCellularStatus();
  }

  uninitSignalModel() {
    Log.showInfo(TAG, 'uninitSignalModel');
    this.unInitObserver();
  }

  /**
     * Check the connection type and signal level of cellular network
     */
  checkCellularStatus() {
    let cellularStatus;
    let slotId = 0;
    Log.showInfo(TAG, 'enter checkCellularStatus ============');

    Sim.hasSimCard(slotId, (err, value) => {
      if (value === true) {
        //The interface of getting the cellular signal status is unavailable temporarily
        Radio.getSignalInformation(slotId, (err, value) => {
          if (err) {
            // Failed to call the interface，error is not null
            Log.showError(TAG, `failed to getSimState because ${err.message}`);
            // When failed to call the interface, set the result as no signal
            mLevelLink.set(Constants.CELLULAR_NO_SIM_CARD);
            mTypeLink.set(Constants.NETWORK_TYPE_UNKNOWN);
          } else {
            // Call interface succeed，error is null
            Log.showInfo(TAG, `success to getSignalInformation: ${JSON.stringify(value)}`);
            // Since the value might be empty, set it as no signal by hand
            if (CheckEmpty.isEmpty(value)) {
              Log.showError(TAG, 'value from api is empty, set 0');
              mLevelLink.set(Constants.CELLULAR_NO_SIM_CARD);
              mTypeLink.set(Constants.NETWORK_TYPE_UNKNOWN);
            }else{
              mLevelLink.set(value[0].signalLevel);
              mTypeLink.set(value[0].signalType);
            }
          }

          Log.showInfo(TAG, 'enter checknetworkState ============');
          //The interface of getting the cellular signal status is unavailable temporarily
          Radio.getnetworkState(slotId,(err, value) => {
            if (err) {
              // Failed to call the interface，error is not null
              Log.showError(TAG, `failed to getnetworkState because ${err.message}`);
              // When failed to call the interface, set the result as no signal
              mStateLink.set(Constants.NET_NULL);
            } else {
              // Call interface succeed，error is null
              Log.showInfo(TAG, `success to getnetworkState: ${JSON.stringify(value)}`);
              // Since the value might be empty, set it as no signal by hand
              if (CheckEmpty.isEmpty(value)) {
                Log.showError(TAG, 'value from api is empty, set 0');
                mStateLink.set(Constants.NET_NULL);
              }else{
                mStateLink.set(value.longOperatorName);
              }
            }
          });
        });
      } else {
        mLevelLink.set(Constants.CELLULAR_NO_SIM_CARD);
        mTypeLink.set(Constants.NETWORK_TYPE_UNKNOWN);
        mStateLink.set(Constants.NET_NULL);
      }
    });


  }

    /**
     * init the observer of the cellular and signal
     */
  initObserver() {
    Log.showInfo(TAG, 'initObserver');
    Observer.on('signalInfoChange', { slotId: 0 }, (err, value) => {
      if (err.code != 0) {
        this.checkCellularStatus();
      } else {
        Log.showError(TAG, `failed, because ${JSON.stringify(err)}`);
      }
    });
  }

    /**
     * Uninit the observer of the cellular and signal
     */
  unInitObserver() {
    Log.showInfo(TAG, 'unInitObserver');
    Observer.off('signalInfoChange', (err, value) => {
      if (err.code != 0) {
        Log.showInfo(TAG, `success: ${JSON.stringify(value)}`);
      } else {
        Log.showError(TAG, `failed, because ${JSON.stringify(err)}`);
      }
    });
  }
}

  let mSignalModel = new SignalModel();
  export default mSignalModel as SignalModel;
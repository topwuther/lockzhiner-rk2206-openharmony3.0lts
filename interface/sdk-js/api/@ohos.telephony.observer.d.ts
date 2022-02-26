/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

import {AsyncCallback} from "./basic";
import radio from "./@ohos.telephony.radio";
import data from "./@ohos.telephony.data";
import call from "./@ohos.telephony.call";

/**
 * Monitors telephony state updates of a device, including updates of the network state,
 * signal strength, call state, the data link connection state and others.
 *
 * @since 6
 */
declare namespace observer {
  export import NetworkState = radio.NetworkState;
  export import SignalInformation = radio.SignalInformation;
  export import CellInformation = radio.CellInformation;
  export import DataConnectState = data.DataConnectState;
  export import RatType = radio.RatType;
  export import DataFlowType = data.DataFlowType;
  export import CallState = call.CallState;

  /**
   * Called when the network state corresponding to a monitored {@code slotId} updates.
   *
   * <p>Applications must have the {@code ohos.permission.GET_NETWORK_INFO} permission
   * to register this event.
   *
   * @param type networkStateChange
   * @param options including slotId Indicates the ID of the target card slot.
   *   The value {@code 0} indicates card 1, and the value {@code 1} indicates card 2.
   * @param callback including an instance of the {@code NetworkState} class.
   */
  function on(type: 'networkStateChange', callback: AsyncCallback<NetworkState>): void;
  function on(type: 'networkStateChange', options: { slotId: number }, callback: AsyncCallback<NetworkState>): void;

  function once(type: 'networkStateChange', callback: AsyncCallback<NetworkState>): void;
  function once(type: 'networkStateChange', options: { slotId: number }, callback: AsyncCallback<NetworkState>): void;

  function off(type: 'networkStateChange', callback?: AsyncCallback<NetworkState>): void;

  /**
   * Called when the signal strength corresponding to a monitored {@code slotId} updates.
   *
   * @param type signalInfoChange
   * @param options including slotId Indicates the ID of the target card slot.
   *   The value {@code 0} indicates card 1, and the value {@code 1} indicates card 2.
   * @param callback including an array of instances of the classes derived from {@link SignalInformation}.
   */
  function on(type: 'signalInfoChange', callback: AsyncCallback<Array<SignalInformation>>): void;
  function on(type: 'signalInfoChange', options: { slotId: number },
    callback: AsyncCallback<Array<SignalInformation>>): void;

  function once(type: 'signalInfoChange', callback: AsyncCallback<Array<SignalInformation>>): void;
  function once(type: 'signalInfoChange', options: { slotId: number },
    callback: AsyncCallback<Array<SignalInformation>>): void;

  function off(type: 'signalInfoChange', callback?: AsyncCallback<Array<SignalInformation>>): void;

  /**
   * Called when the cellular data link connection state updates.
   *
   * @param type cellularDataConnectionStateChange
   * @param options including slotId Indicates the ID of the target card slot.
   *   The value {@code 0} indicates card 1, and the value {@code 1} indicates card 2.
   * @param callback including state Indicates the cellular data link connection state,
   *   and networkType Indicates the radio access technology for cellular data services.
   */
  function on(type: 'cellularDataConnectionStateChange',
    callback: AsyncCallback<{ state: DataConnectState, network: RatType }>): void;
  function on(type: 'cellularDataConnectionStateChange', options: { slotId: number },
    callback: AsyncCallback<{ state: DataConnectState, network: RatType }>): void;

  function once(type: 'cellularDataConnectionStateChange',
    callback: AsyncCallback<{ state: DataConnectState, network: RatType }>): void;
  function once(type: 'cellularDataConnectionStateChange', options: { slotId: number },
    callback: AsyncCallback<{ state: DataConnectState, network: RatType }>): void;

  function off(type: 'cellularDataConnectionStateChange',
    callback?: AsyncCallback<{ state: DataConnectState, network: RatType }>): void;

  /**
   * Receives a call state change. This callback is invoked when the call state of a specified card updates
   * and the observer is added to monitor the updates.
   *
   * @param type callStateChange
   * @param options including slotId Indicates the ID of the target card slot.
   *   The value {@code 0} indicates card 1, and the value {@code 1} indicates card 2.
   * @param callback including state Indicates the call state, and number Indicates the called number.
   *   The value of number is an empty string if the application does not have
   *     the {@code ohos.permission#READ_CALL_LOG READ_CALL_LOG} permission.
   */
  function on(type: 'callStateChange', callback: AsyncCallback<{ state: CallState, number: String }>): void;
  function on(type: 'callStateChange', options: { slotId: number },
    callback: AsyncCallback<{ state: CallState, number: String }>): void;

  function once(type: 'callStateChange', callback: AsyncCallback<{ state: CallState, number: String }>): void;
  function once(type: 'callStateChange', options: { slotId: number },
    callback: AsyncCallback<{ state: CallState, number: String }>): void;

  function off(type: 'callStateChange', callback?: AsyncCallback<{ state: CallState, number: String }>): void;
}

export default observer;
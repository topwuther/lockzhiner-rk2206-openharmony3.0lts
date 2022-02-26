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

/**
 * Provides methods related to cellular data services.
 *
 * @since 6
 */
declare namespace data {
  /**
   * Obtains the connection state of the PS domain.
   *
   * @param slotId Indicates the ID of a card slot.
   *      The value {@code 0} indicates card 1, and the value {@code 1} indicates card 2.
   * @param callback Returns the connection state, which can be any of the following:
   * <ul>
   * <li>{@code DataConnectState#DATA_STATE_UNKNOWN}
   * <li>{@code DataConnectState#DATA_STATE_DISCONNECTED}
   * <li>{@code DataConnectState#DATA_STATE_CONNECTING}
   * <li>{@code DataConnectState#DATA_STATE_CONNECTED}
   * <li>{@code DataConnectState#DATA_STATE_SUSPENDED}
   * </ul>
   */
  function getCellularDataState(callback: AsyncCallback<DataConnectState>): void;
  function getCellularDataState(): Promise<DataConnectState>;

  /**
   * Checks whether cellular data services are enabled.
   *
   * <p>Requires Permission: {@code ohos.permission.GET_NETWORK_INFO}.
   *
   * @param callback Returns {@code true} if cellular data services are enabled; returns {@code false} otherwise.
   */
  function isCellularDataEnabled(callback: AsyncCallback<boolean>): void;
  function isCellularDataEnabled(): Promise<boolean>;

  /**
   * Enables cellular data services.
   *
   * @hide Used for system app.
   */
  function enableCellularData(callback: AsyncCallback<void>): void;
  function enableCellularData(): Promise<void>;

  /**
   * Diables cellular data services.
   *
   * @hide Used for system app.
   */
  function disableCellularData(callback: AsyncCallback<void>): void;
  function disableCellularData(): Promise<void>;

  /**
   * Describes the cellular data link connection state.
   */
  export enum DataConnectState {
    /**
     * Indicates that a cellular data link is unknown.
     */
    DATA_STATE_UNKNOWN = -1,

    /**
     * Indicates that a cellular data link is disconnected.
     */
    DATA_STATE_DISCONNECTED = 0,

    /**
     * Indicates that a cellular data link is being connected.
     */
    DATA_STATE_CONNECTING = 1,

    /**
     * Indicates that a cellular data link is connected.
     */
    DATA_STATE_CONNECTED = 2,

    /**
     * Indicates that a cellular data link is suspended.
     */
    DATA_STATE_SUSPENDED = 3
  }
}

export default data;
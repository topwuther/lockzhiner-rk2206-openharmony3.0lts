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

export default class constants {
  static DEFAULT_WIFI_INFO: number = 0
  static DEFAULT_WIFI_STATUS: boolean = false
  static WIFI_STATUS_DISPLAY_HEIGHT: string = '50%'
  static WIFI_STATUS_HIDDEN_HEIGHT: number = 0
  static WIFI_STATUS_MAX_WIDTH: string = '50'
  static EVENT_CONN_STATE: string = 'usual.event.wifi.CONN_STATE'
  static EVENT_RSSI_VALUE: string = 'usual.event.wifi.RSSI_VALUE'
  static EVENT_POWER_STATE: string = 'usual.event.wifi.POWER_STATE'
  static Event_CONN_NAME:string ='WIFI_CONNECT_NAME'

  static WIFI_SIGNAL_NO: number = 0
  static WIFI_SIGNAL_LOW: number = 1
  static WIFI_SIGNAL_MID: number = 2
  static WIFI_SIGNAL_HIGH: number = 3
  static WIFI_SIGNAL_FULL: number = 4
  static WIFI_POWER_OFF: number = 1
  static WIFI_POWER_ON: number = 3
  static WIFI_STATE_AP_CONNECTING: number = 1
  static WIFI_Disconnected: number = 9
  static DEFAULT_ERR_CODE: number = 0
  static UPDATE_WIFI_INFO_DEFAULT_PARAM: number = -60
  static WIFI_SIGNAL_DEFAULT: number = -1
  static SIGN_LEVEL: number = 2

}

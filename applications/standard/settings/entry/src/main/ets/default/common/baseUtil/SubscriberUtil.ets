/**
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
import BrightnessSettingModel from '../../model/brightnessImpl/BrightnessSettingModel.ets';
import ConfigData from './ConfigData.ets';
import CommonEvent from '@ohos.commonevent';

let mCommonEventSubscribeInfo = {
  events: ['usual.event.wifi.CONN_STATE', 'BRIGHTNESS_VALUE_SYSTEM_UI']
};

let mCommonEventSubscriber = null;
let eventSubscriber = null;

export class SubscriberUtils {
  sendEvent(value, broadcastName) {
    console.log(`settings settingList sendEvent ets in value:` + value + "|broadcastName:" + broadcastName);
    let commonEventPublishData = {
      data: value
    };
    CommonEvent.publish(broadcastName, commonEventPublishData, (err, data) => {
      console.log(`settings settingList sendEvent ets ${JSON.stringify(err)}`);
      console.log(`settings settingList sendEvent ets ${JSON.stringify(data)}`);
    });
    console.log(`settings settingList sendEvent ets out`);
  }

  registerStatusListener() {
    console.info(`settings settingList Subscriberregister status listener in`);
    CommonEvent.createSubscriber(
      mCommonEventSubscribeInfo,
      this.createStatusSubscriberCallBack.bind(this)
    );
    console.info(`settings settingList Subscriberregister status listener out`);
  }

  createStatusSubscriberCallBack(err, data) {
    console.info(`settings settingList Subscriberregister in ets ${JSON.stringify(err)} ${JSON.stringify(data)}`);
    mCommonEventSubscriber = data;
    CommonEvent.subscribe(mCommonEventSubscriber, this.statusSubscriberCallBack.bind(this));
    console.info(`settings settingList Subscriberregister in`);
  }

  statusSubscriberCallBack(err, data) {
    console.info(`settings settingList statusSubscriberCallBack data ets : ${JSON.stringify(data)}`);
    console.info(`settings settingList statusSubscriberCallBack err ets : ${JSON.stringify(err)}`);
    console.log(`settings settingList statusSubscriberCallBack data.event ets : ${data.event} `);
    if ("BRIGHTNESS_VALUE_SYSTEM_UI" === data.event) {
      console.log(`settings settingList statusSubscriberCallBack data.data ets : ${data.data} `);
      let mBrightnessSetValue = AppStorage.SetAndLink('BrightnessSetValue', 150);
      mBrightnessSetValue.set(parseInt(data.data));
      BrightnessSettingModel.setBrightnessListener(parseInt(data.data));
      BrightnessSettingModel.setStorageValue(ConfigData.BRIGHTNESS_SAVE_VALUE_KEY,parseInt(data.data));
    }
    if (err) {
      console.info(`settings settingList statusSubscriberCallBack error : ${err}`)
      return;
    }
    console.log(`settings settingList statusSubscriberCallBack out`);
  }

  unSubscriberListener() {
    CommonEvent.unsubscribe(eventSubscriber, () => {
      console.info('settings settingList unSubscriberListener');
    });
  }
}

let subscriberUtils = new SubscriberUtils();
export default subscriberUtils as SubscriberUtils
;
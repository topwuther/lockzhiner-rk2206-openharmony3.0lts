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
import SubscriberUtils from '../common/baseUtil/SubscriberUtil.ets';
import WifiModel from '../model/wifiImpl/WifiModel.ets';
import HeadComponent from './component/headComponent.ets';
import EntryComponent from './component/entryComponent.ets';
import ImageAnimatorComponent from './component/imageAnimatorComponent.ets';
import ConfigData from '../common/baseUtil/ConfigData.ets';
import LogUtil from '../common/baseUtil/LogUtil.ets';
import Subscriber from '@ohos.commonevent';
import router from '@system.router';


@Entry
@Component
struct Index {
  @StorageLink('WifiList') wifiList: any[] = [];
  @StorageLink('wifiPsd') password: string = '';
  @State isOn: boolean = true;
  @State isShow: boolean = true;
  private connectPsd: string = '';
  private mIndex: number = -1;
  private timeoutFlag;
  private timeoutMark;
  @State connectRssi: number = -1;
  @State connectBand: number = -1;
  @State connectImage: string = '';
  @State connectName: string = '';
  @State connectBssid: string = '';
  @State connectSecurityType: number = -1;
  private mCommonEventSubscriber = null;
  private mCommonEventSubscribeInfo = {
    events: ["usual.event.wifi.CONN_STATE"]
  };

  build() {
    Column() {
      HeadComponent({ headName: $r('app.string.wifiTab'), isActive: true });
      Flex({ justifyContent: FlexAlign.SpaceBetween }) {
        Row() {
          Text($r('app.string.wifiTab'))
            .fontSize($r('app.float.font_22'))
            .fontColor(Color.Black)
            .textAlign(TextAlign.Start);
        }.alignItems(VerticalAlign.Center);
        Toggle({ type: ToggleType.Switch, isOn: this.isOn })
          .width($r('app.float.wh_value_50'))
          .height($r('app.float.wh_value_40'))
          .onChange((isOn: boolean) => {
          this.isOn = !this.isOn;
          if (this.isOn) {
            this.switchOn();
          } else {
            this.switchOff();
          }
        });
      }
      .height($r('app.float.wh_value_70'))
      .margin({ left: $r('app.float.wh_value_15'), right: $r('app.float.wh_value_15') });
      Flex({ alignItems: ItemAlign.Start}) {
        Text($r('app.string.tipsContent'))
          .fontSize($r('app.float.font_20'));
      }
        .visibility(this.isOn ? Visibility.None : Visibility.Visible)
        .margin({ left: $r('app.float.wh_value_15'), right: $r('app.float.wh_value_15') });
      Column() {
        Flex({ justifyContent: FlexAlign.SpaceBetween }) {
          Text($r('app.string.wifiList'))
            .fontSize($r('app.float.font_24'))
            .align(Alignment.Start);
          Row() {
            ImageAnimatorComponent({ imageWidth: $r('app.float.wh_value_40'),
              imageHeight: $r('app.float.wh_value_40') })
          }
          .width($r('app.float.wh_value_40'))
          .visibility(this.isShow ? Visibility.Visible : Visibility.None);
        }
        .width(ConfigData.WH_100_100)
        .margin({ left: $r('app.float.wh_value_15'), right: $r('app.float.wh_value_15') });

        List() {
          ForEach(this.wifiList, (item) => {
            ListItem() {
              EntryComponent({
                settingIcon: '',
                settingTitle: item.settingTitle,
                settingSummary: item.settingSummary,
                settingValue: '',
                settingArrow: item.settingArrow,
                settingArrowStyle: 'wifi',
                settingUri: ''
              });
            }.onClick(() => {
              this.connectName = item.settingTitle
              this.connectImage = item.settingArrow
              this.connectBssid = item.bssid
              this.connectSecurityType = item.securityType
              this.connectRssi = item.settingRssi
              this.connectBand = item.settingBand
              this.connectWifi();
            })
            .height($r('app.float.wh_value_60'))
          }, item => JSON.stringify(item));
        }
        .divider({
          strokeWidth: $r('app.float.wh_value_1'),
          color: $r('app.color.color_E3E3E3_grey'),
          startMargin: $r('app.float.wh_value_15'),
          endMargin: $r('app.float.wh_value_15')
        })
        .width(ConfigData.WH_100_100)
        .height(ConfigData.WH_100_100)
        .align(Alignment.Top)
        .margin({ bottom: $r('app.float.wh_value_70'), top: $r('app.float.wh_value_20') });
      }
      .width(ConfigData.WH_100_100)
      .height(ConfigData.WH_100_100)
      .visibility(this.isOn ? Visibility.Visible : Visibility.None);
    }
    .width(ConfigData.WH_100_100)
    .height(ConfigData.WH_100_100);
  }

  private aboutToAppear(): void {
    this.wifiStatusListener();
    this.getSwitchStatus();
    this.clearScanInfo();
    LogUtil.info('init wifi get switch status');
    if (this.isOn) {
      LogUtil.info('init wifi isOn is true');
      let that = this
      this.timeoutFlag = setTimeout(() => {
        if (WifiModel.getScanWifi()) {
          LogUtil.info('init wifi start scan');
          that.timeoutMark = setTimeout(() => {
            WifiModel.getScanInfoCallBack();
            that.isShow = false;
            clearTimeout(this.timeoutFlag);
            clearTimeout(that.timeoutMark);
            LogUtil.info('init wifi wifiList' + this.wifiList);
          }, 3000);
        } else {
          LogUtil.info('init scan wifiList fail');
        }
      }, 3000);
    } else {
      LogUtil.info('init wifi isOn is false');
    }
  }

  connectWifi() {
    LogUtil.info('[wifi_js_test] connect to wifi come in');
    let index = WifiModel.getItemPosition(this.wifiList, this.connectName);
    if (this.mIndex !== index) {
      this.connectPsd = '';
      this.mIndex = index;
    }
    let obj = {
      "ssid": this.connectName,
      "bssid": this.connectBssid,
      "preSharedKey": this.connectPsd,
      "isHiddenSsid": false,
      "securityType": this.connectSecurityType
    }

    LogUtil.info('[wifi_js_test] connectSecurityType' + this.connectSecurityType);
    LogUtil.info('[wifi_js_test] connectPsd' + this.connectPsd);
    LogUtil.info('[wifi_js_test] connectName' + this.connectName);
    if (this.connectSecurityType !== 1 && '' == this.connectPsd) {
      LogUtil.info('click to detail wifiPsd');
      let name = { 'connectName': this.connectName };
      this.mIndex = index;
      router.push({
        uri: 'pages/wifiPsd',
        params: name,
      });
      LogUtil.info('jump to detail end');
    }
    if (WifiModel.connectToDevice(obj)) {
      LogUtil.info('[wifi_js_test] connect to come in');
      LogUtil.info('[wifi_js_test] connect to come in obj' + JSON.stringify(obj));
      LogUtil.info('[wifi_js_test] connect to wifiList ' + JSON.stringify(this.wifiList));
    }
    LogUtil.info('clickToSecret end:');
  }

  getSwitchStatus() {
    if (WifiModel.getWifiStatus()) {
      this.isOn = true;
    } else {
      this.isOn = false;
    }
  }

  private sleep(delay) {
    let start = (new Date()).getTime();
    while ((new Date()).getTime() - start < delay) {
      continue;
    }
  }

  private switchOn() {
    LogUtil.info('switchClick wifi start ---->');
    this.clearScanInfo();
    this.isShow = true;
    this.sleep(1000);
    if (WifiModel.getEnableWifi()) {
      LogUtil.info('switchClick wifi enableWifi enter ---->');
      let that = this;
      this.timeoutFlag = setTimeout(() => {
        if (WifiModel.getScanWifi()) {
          that.timeoutMark = setTimeout(() => {
            WifiModel.getScanInfoCallBack();
            that.isShow = false;
            clearTimeout(this.timeoutFlag);
            clearTimeout(that.timeoutMark);
            LogUtil.info('switchClick wifi wifiList' + JSON.stringify(this.wifiList));
          }, 3000);
        } else {
          LogUtil.info('switchClick wifi fail to scan ---->');
        }
      }, 3000);
    } else {
      LogUtil.info('switchClick wifi fail to enableWifi ---->');
    }
  }

  private switchOff() {
    LogUtil.info('switchClick wifi off start ---->');
    if (WifiModel.disableWifi()) {
      LogUtil.info('switchClick wifi enter to disableWifi ---->');
      this.clearScanInfo();
    } else {
      LogUtil.info('switchClick wifi off end ---->');
    }
  }

  wifiStatusListener() {
    LogUtil.info('wifi status listener');
    Subscriber.createSubscriber(this.mCommonEventSubscribeInfo,
      this.CreateSubscriberCallBack.bind(this));
  }

  CreateSubscriberCallBack(err, data) {
    LogUtil.info('subscriber subscribe');
    this.mCommonEventSubscriber = data;
    Subscriber.subscribe(this.mCommonEventSubscriber, this.SubscriberCallBack.bind(this));
  }

  SubscriberCallBack(err, data) {
    LogUtil.info('subscriber call back');
    LogUtil.info('==========================>SubscriberCallBack  event = ' + data.event);
    LogUtil.info('==========================>SubscriberCallBack  data = ' + JSON.stringify(data));
    LogUtil.info('==========================>SubscriberCallBack  data code = ' + data.code);

    if (data.code === 1) {
      let wifi = { 'name': '', 'rssi': -1, 'band': -1 };
      wifi.name = this.connectName;
      wifi.rssi = this.connectRssi;
      wifi.band = this.connectBand;
      SubscriberUtils.sendEvent(JSON.stringify(wifi), ConfigData.SENT_EVENT_WIFI_CONNECT_NAME);
      let index = WifiModel.getItemPosition(this.wifiList, this.connectName);
      this.wifiList.splice(index, 1);
      this.wifiList.unshift({
        settingSummary: $r('app.string.connected'),
        settingTitle: this.connectName,
        settingValue: '',
        settingArrow: this.connectImage,
        settingArrowStyle: 'wifi',
        settingRssi: this.connectRssi,
        settingBand: this.connectBand,
        bssid: this.connectBssid,
        securityType: this.connectSecurityType,
      });
      this.wifiList[1].settingSummary = '';
      LogUtil.info('subscriber connected' + JSON.stringify(this.wifiList));
    }
    ;
    if (data.code === 9) {
      LogUtil.info("subscriber code nine disconnected");
      this.wifiList.unshift({
        settingSummary: '',
        settingTitle: this.wifiList[0].settingTitle,
        settingValue: '',
        settingArrow: this.wifiList[0].settingArrow,
        settingArrowStyle: 'wifi',
        settingRssi: this.wifiList[0].settingRssi,
        bssid: this.wifiList[0].bssid,
        securityType: this.wifiList[0].securityType,
      });
      this.wifiList.splice(1, 1);
    }
  }

  unSubscriberListener() {
    Subscriber.unsubscribe(this.mCommonEventSubscriber, () => {
      LogUtil.info('wifi unsubscribe');
    });
  }

  clearScanInfo() {
    LogUtil.info('clear scan wifiList start---->');
    this.wifiList = [];
    LogUtil.info('clear scan wifiList end---->');
  }

  private aboutToDisappear(): void {
    this.unSubscriberListener();
    this.connectPsd = '';
  }

  onPageShow() {
    LogUtil.info('wifi onPageShow this.password:' + this.password);
    this.connectPsd = this.password;
    if (this.connectPsd !== '') {
      this.connectWifi();
    }
  }

  onBackPress() {
    Navigator().active(true);
  }
}





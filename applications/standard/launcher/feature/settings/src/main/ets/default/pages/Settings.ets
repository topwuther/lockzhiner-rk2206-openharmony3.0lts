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

import DefaultConstants from '../common/constants/DefaultConstants.ets'
import SettingsPresenter from '../common/presenter/SettingsPresenter.ets'

let mSettingsPresenter: SettingsPresenter;

@Entry
@Component
struct Index {
  private aboutToAppear(): void  {
    mSettingsPresenter = SettingsPresenter.getInstance();
  }

  build() {
    Flex({ alignItems: ItemAlign.Center, direction: FlexDirection.Column }) {
      Row() {
        top_bar()
      }

      Column() {
        Text($r('app.string.layout'))
          .fontSize(DefaultConstants.DEFAULT_PIXEL_25)
          .fontColor(DefaultConstants.DEFAULT_LAYOUT_FONT_COLOR)
          .margin({ right: DefaultConstants.DEFAULT_PIXEL_340 })
      }

      Column() {
        SettingPage()
      }
    }.width(DefaultConstants.PERCENTAGE_100)
    .backgroundColor(DefaultConstants.DEFAULT_BACKGROUND_COLOR)
  }
}

@Component
struct top_bar {
  build() {
    Flex({ justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
      Image($r('app.media.ic_back'))
        .margin({ right: DefaultConstants.DEFAULT_PIXEL_20,
        left: DefaultConstants.DEFAULT_PIXEL_20 })
        .width(DefaultConstants.DEFAULT_PIXEL_75)
        .height(DefaultConstants.DEFAULT_PIXEL_75)
        .objectFit(ImageFit.Contain)
        .onClick(() => {
        mSettingsPresenter.backToTheDesktop();
      })
      Text($r('app.string.intoSettings'))
        .fontSize(DefaultConstants.DEFAULT_PIXEL_35)
    }
    .margin({ top: DefaultConstants.DEFAULT_PIXEL_10 })
    .height(DefaultConstants.DEFAULT_PIXEL_80)
    .width(DefaultConstants.PERCENTAGE_90)
    .backgroundColor(DefaultConstants.DEFAULT_BACKGROUND_COLOR)
    .padding({ bottom: DefaultConstants.DEFAULT_PIXEL_10 })
  }
}

@Component
struct SettingPage {
  @State SettingList: any = [];

  private aboutToAppear(): void  {
    this.SettingList = mSettingsPresenter.getSettingList();
  }

  private onPageShow(): void {
    this.SettingList = mSettingsPresenter.getSettingList();
  }

  build() {
    Column() {
      ForEach(this.SettingList, (item: any) => {
        SettingItem({
          ida: item.ida,
          settingName: item.settingName,
          settingValue: item.settingValue,
          valueList: item.valueList,
        })
      })
    }
    .margin({ top: DefaultConstants.DEFAULT_PIXEL_10,
      bottom: DefaultConstants.DEFAULT_PIXEL_10,
      left: DefaultConstants.DEFAULT_PIXEL_30,
      right: DefaultConstants.DEFAULT_PIXEL_30 })
    .height(DefaultConstants.PERCENTAGE_100)
    .padding(DefaultConstants.DEFAULT_PIXEL_20)
    .borderRadius(DefaultConstants.DEFAULT_PIXEL_30)
    .backgroundColor(DefaultConstants.DEFAULT_SETTING_PAGE_COLOR)
  }
}

@Component
struct SettingItem {
  @State ida: number = 0;
  @State settingValue: string = ' ';
  @State settingName: string = ' ';
  private valueList: any;
  dialogController: CustomDialogController = new CustomDialogController({
    builder: SettingsDialog(),
    cancel: this.cancelDialog,
    autoCancel: true
  });

  cancelDialog() {
    console.info('cancelDialog');
  }

  private aboutToAppear(): void  {
    mSettingsPresenter.registerValueCallback(this.ida, this.callback.bind(this));
  }

  callback(data) {
    this.settingValue = data;
  }

  build() {
    Column() {
      Row() {
        Text(this.settingName)
          .lineHeight(DefaultConstants.DEFAULT_PIXEL_60)
          .height(DefaultConstants.DEFAULT_PIXEL_60)
          .fontSize(DefaultConstants.DEFAULT_PIXEL_30)
          .width(DefaultConstants.PERCENTAGE_60)
          .align(Alignment.Start)
        Text(this.settingValue)
          .lineHeight(DefaultConstants.DEFAULT_PIXEL_60)
          .height(DefaultConstants.DEFAULT_PIXEL_60)
          .fontSize(DefaultConstants.DEFAULT_PIXEL_30)
          .width(DefaultConstants.DEFAULT_PIXEL_60)
          .align(Alignment.Start)
        Image($r('app.media.ic_settings_arrow'))
          .margin({ left: DefaultConstants.DEFAULT_PIXEL_80 })
          .height(DefaultConstants.DEFAULT_PIXEL_120)
          .width(DefaultConstants.DEFAULT_PIXEL_100)
          .align(Alignment.End)
      }.onClick(() => {
        AppStorage.SetOrCreate('ida', this.ida);
        AppStorage.SetOrCreate('valueList', this.valueList);
        this.dialogController.open();
      }).margin({ top: DefaultConstants.DEFAULT_PIXEL_10,
        bottom: DefaultConstants.DEFAULT_PIXEL_10 })
      .width(DefaultConstants.PERCENTAGE_100)
      .height(DefaultConstants.DEFAULT_PIXEL_75)
    }
  }
}

@CustomDialog
@Component
struct SettingsDialog {
  controller: CustomDialogController;
  action: () => void;
  cancel: () => void;
  @StorageLink('valueList') valueList: any = [];
  @StorageLink('ida') ida: number = 0;

  build() {
    Column() {
      ForEach(this.valueList, (item: any) => {
        Row() {
          Text(item.name)
            .margin({ left: DefaultConstants.DEFAULT_PIXEL_10 })
            .align(Alignment.Start)
            .width(DefaultConstants.PERCENTAGE_85)
            .fontSize(DefaultConstants.DEFAULT_PIXEL_30)
            .fontColor(DefaultConstants.DEFAULT_DIALOG_FONT_COLOR)
          Radio({ value: item.value })
            .enabled(false)
            .checked(item.checked)
            .width(DefaultConstants.DEFAULT_PIXEL_30)
            .height(DefaultConstants.DEFAULT_PIXEL_30)
            .onChange((isChecked) => {})
        }.width(DefaultConstants.PERCENTAGE_100)
        .height(DefaultConstants.DEFAULT_PIXEL_80)
        .onClick(() => {
          mSettingsPresenter.changeSettingValue(this.ida, item.name);
          mSettingsPresenter.setSettingsValue(this.ida, item.value);
          this.controller.close();
          this.action();
        })
      })
      Text($r('app.string.cancel'))
        .textAlign(TextAlign.Center)
        .height(DefaultConstants.DEFAULT_PIXEL_80)
        .width(DefaultConstants.PERCENTAGE_100)
        .fontSize(DefaultConstants.DEFAULT_PIXEL_30)
        .fontColor(Color.Blue)
        .onClick(() => {
        this.controller.close();
        this.action();
      })
    }.padding(DefaultConstants.DEFAULT_PIXEL_20)
    .backgroundColor(DefaultConstants.DEFAULT_SETTING_PAGE_COLOR)
    .borderRadius(DefaultConstants.DEFAULT_PIXEL_30)
  }
}

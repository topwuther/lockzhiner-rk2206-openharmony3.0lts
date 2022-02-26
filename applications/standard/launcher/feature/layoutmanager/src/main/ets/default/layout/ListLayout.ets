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

import AppListPresenter from '../common/presenter/AppListPresenter.ets'
import ListName from '../../../../../../../common/src/main/ets/default/uicomponents/ListName.ets'
import AppIcon from '../../../../../../../common/src/main/ets/default/uicomponents/AppIcon.ets'
import DefaultConstants from '../common/constants/DefaultConstants.ets'

let mAppListPresenter: AppListPresenter;

@Component
export default
struct ListLayout {
  @StorageLink('listInfo') AppListInfo: [] = [];

  private aboutToAppear(): void  {
    mAppListPresenter = AppListPresenter.getInstance();
    mAppListPresenter.getAppList();
    mAppListPresenter.registerAppListChangeCallback()
  }

  dialogController: CustomDialogController = new CustomDialogController({
    builder: ShowDialog({ action: this.openUninstallDialog }),
    cancel: this.cancelDialog,
    autoCancel: true
  });
  uninstallDialogController: CustomDialogController = new CustomDialogController({
    builder: UninstallDialog(),
    cancel: this.cancelDialog,
    autoCancel: true
  });

  openUninstallDialog() {
    this.dialogController.close();
    this.uninstallDialogController.open();
  }

  cancelDialog() {
    console.info('Launcher Grid Cancel Dialog');
  }

  build() {
    List({ space: 5, initialIndex: 1 }) {
      ForEach(this.AppListInfo, (item) => {
        ListItem() {
          Row() {
            AppIcon({
              iconSize: DefaultConstants.DEFAULT_APP_ITEM_WIDTH,
              appIcon: item.AppIcon,
              bundleName: item.bundleName,
              labelId: item.labelId
            })
            ListName({
              appName: item.AppName,
              nameSize: DefaultConstants.DEFAULT_APP_NAME_SIZE,
              bundleName: item.bundleName,
              labelId: item.labelId
            })
          }
          .gesture(
            LongPressGesture({ repeat: false })
              .onAction((event: GestureEvent) => {
              AppStorage.SetOrCreate('uninstallAppInfo', item);
              this.dialogController.open();
            })
          )
          .onClick(() => {
            mAppListPresenter.openApplication(item.abilityName, item.bundleName);
          })
          .width(DefaultConstants.PERCENTAGE_100)
          .height(DefaultConstants.DEFAULT_80)
          .backgroundColor(DefaultConstants.LIGHT_BLACK)
          .borderRadius(DefaultConstants.DEFAULT_20)
          .padding({ left: DefaultConstants.DEFAULT_5 })
          .margin({
            top: DefaultConstants.DEFAULT_5,
            left: DefaultConstants.DEFAULT_30,
            bottom: DefaultConstants.DEFAULT_5,
            right: DefaultConstants.DEFAULT_30
          })
        }
      })
    }
    .gesture(
      LongPressGesture({ repeat: false })
        .onAction((event: GestureEvent) => {
        mAppListPresenter.intoSetting();
      })
    )
    .width(DefaultConstants.PERCENTAGE_100)
    .height(DefaultConstants.PERCENTAGE_100)
  }
}

@CustomDialog
struct ShowDialog {
  @StorageLink('uninstallAppInfo') AppInfo: any = {};
  controller: CustomDialogController;
  cancel: () => void;
  action: () => void;

  build() {
    Row() {
      Button() {
        Text($r('app.string.intoSettings'))
          .height(DefaultConstants.PERCENTAGE_100)
          .fontColor(Color.White)
          .fontSize(DefaultConstants.DEFAULT_APP_NAME_SIZE)
      }
      .width(DefaultConstants.PIXEL_200)
      .height(DefaultConstants.PIXEL_50)
      .backgroundColor(Color.Blue)
      .onClick(() => {
        mAppListPresenter.intoSetting();
        this.controller.close();
      })

      Button() {
        Text($r('app.string.uninstall'))
          .height(DefaultConstants.PERCENTAGE_100)
          .fontColor(Color.White)
          .fontSize(DefaultConstants.DEFAULT_APP_NAME_SIZE)
      }
      .width(DefaultConstants.PIXEL_200)
      .height(DefaultConstants.PIXEL_50)
      .backgroundColor(Color.Blue)
      .onClick(() => {
        mAppListPresenter.uninstallApp(this.AppInfo.bundleName, this.AppInfo.System);
        this.controller.close();
      })
    }.padding(DefaultConstants.DEFAULT_20)
  }
}

@CustomDialog
struct UninstallDialog {
  controller: CustomDialogController;
  cancel: () => void;
  action: () => void;
  @StorageLink('uninstallAppInfo') AppInfo: any = {};

  build() {
    Column() {
      Row() {
        Text($r('app.string.uninstall'))
        Text(this.AppInfo.AppName + ' ?')
      }

      Row() {
        Button() {
          Text($r('app.string.submit'))
            .fontColor(Color.White)
            .fontSize(DefaultConstants.DEFAULT_APP_NAME_SIZE)
        }
        .margin({ right: DefaultConstants.DEFAULT_20 })
        .width(DefaultConstants.PIXEL_150)
        .height(DefaultConstants.PIXEL_50)
        .backgroundColor(Color.Blue)
        .onClick(() => {
          mAppListPresenter.uninstallApp(this.AppInfo.bundleName, this.AppInfo.System);
          this.controller.close();
        })

        Button() {
          Text($r('app.string.cancel'))
            .fontColor(Color.White)
            .fontSize(DefaultConstants.DEFAULT_APP_NAME_SIZE)
        }
        .margin({ left: DefaultConstants.DEFAULT_20 })
        .width(DefaultConstants.PIXEL_150)
        .height(DefaultConstants.PIXEL_50)
        .backgroundColor(Color.Blue)
        .onClick(() => {
          this.controller.close();
        })
      }
      .margin({ top: DefaultConstants.DEFAULT_20 })
      .padding(DefaultConstants.DEFAULT_20)
    }.padding(DefaultConstants.DEFAULT_20)
  }
}
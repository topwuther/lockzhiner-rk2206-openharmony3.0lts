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

import AppGridPresenter from '../common/presenter/AppGridPresenter.ets'
import GridName from '../../../../../../../common/src/main/ets/default/uicomponents/GridName.ets'
import AppIcon from '../../../../../../../common/src/main/ets/default/uicomponents/AppIcon.ets'
import DefaultConstants from '../common/constants/DefaultConstants.ets'

let mAppGridPresenter: AppGridPresenter;
const DESIGN_WIDTH = 720;
const APP_INFO_REFRESH_DELAY = 500;
const CELLWIDTH = DefaultConstants.DEFAULT_APP_ITEM_WIDTH;
const PROPORTION = 0.85;
let mSHeight = 0;
let mScreenMagnification = 0;

@Component
export default
struct GridLayout {
  @State AppItemHeight: number = DefaultConstants.DEFAULT_APP_ITEM_HEIGHT;
  @State AppItemWidth: number = DefaultConstants.DEFAULT_APP_ITEM_WIDTH;
  @State AppNameSize: number = DefaultConstants.DEFAULT_APP_NAME_SIZE;
  @StorageLink('appListInfo') AppListInfo: {
    appGridInfo: [[]],
    appBottomBarInfo: []
  } = { appGridInfo: [[]], appBottomBarInfo: [] };
  @State mScreenHeight: number = 0;
  @State mScreenWidth: number = 0;
  @State moveAppX: number = 0;
  @State moveAppY: number = 0;
  @State moveAppShow: boolean = false;
  @State moveAppInfo: any = {};
  @Prop @Watch('changeGridConfig') gridConfig: string;
  @Prop equipment: string;
  @State ScreenMagnification: number = 1.0;
  @Prop SwiperProportion: string;
  @Prop BottomBarProportion: string;
  dialogController: CustomDialogController = new CustomDialogController({
    builder: ShowDialog(),
    cancel: this.cancelDialog,
    autoCancel: true
  });

  cancelDialog() {
    console.info('Launcher Grid Cancel Dialog');
  }

  private aboutToAppear(): void  {
    mSHeight = this.mScreenHeight * PROPORTION;
    mScreenMagnification = this.ScreenMagnification;
    mAppGridPresenter = AppGridPresenter.getInstance();
    mAppGridPresenter.setEquipment(this.equipment);
    mAppGridPresenter.setScreenHeight(this.mScreenHeight);
    mAppGridPresenter.setScreenWidth(this.mScreenWidth);
    mAppGridPresenter.calculateCoordinate();
    mAppGridPresenter.getGridList();
    mAppGridPresenter.registerAppListChangeCallback()
  }

  public changeGridConfig() {
    mAppGridPresenter.calculateCoordinate();
    mAppGridPresenter.getGridList();
  }

  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      Column() {
        GridSwiper({
          mAppGridInfo: this.AppListInfo.appGridInfo,
          mAppBottomBarInfo: this.AppListInfo.appBottomBarInfo,
          moveAppX: $moveAppX,
          moveAppY: $moveAppY,
          moveAppShow: $moveAppShow,
          moveAppInfo: $moveAppInfo,
          gridConfig: this.gridConfig
        })
      }
      .width(DefaultConstants.PERCENTAGE_100)
      .height(this.SwiperProportion)

      Column() {
        BottomBar({
          mAppBottomBarInfo: this.AppListInfo.appBottomBarInfo,
          moveAppX: $moveAppX,
          moveAppY: $moveAppY,
          moveAppShow: $moveAppShow,
          moveAppInfo: $moveAppInfo
        })
      }
      .width(DefaultConstants.PERCENTAGE_100)
      .height(this.BottomBarProportion)

      if (this.moveAppShow) {
        Stack() {
          Column() {
            AppIcon({
              iconSize: this.AppItemWidth,
              appIcon: this.moveAppInfo.AppIcon,
              bundleName: this.moveAppInfo.bundleName,
              labelId: this.moveAppInfo.labelId
            })
          }
        }
        .height(this.AppItemHeight)
        .width(this.AppItemWidth)
        .position({
          x: this.moveAppX - (DefaultConstants.DEFAULT_APP_ITEM_WIDTH / 2),
          y: this.moveAppY - (DefaultConstants.DEFAULT_APP_ITEM_WIDTH / 2)
        })
      }

    }
    .gesture(
      LongPressGesture({ repeat: false })
        .onAction((event: GestureEvent) => {
        mAppGridPresenter.onPageLongPress();
        this.dialogController.open();
      })
    )
    .width(DefaultConstants.PERCENTAGE_100)
    .height(DefaultConstants.PERCENTAGE_100)
  }
}

@Component
struct GridSwiper {
  @Link moveAppX: number;
  @Link moveAppY: number;
  @Link moveAppShow: boolean;
  @Link moveAppInfo: any;
  @StorageLink('pageIndex') PageIndex: number = 0;
  @Prop gridConfig: string;
  private mAppGridInfo;
  private mAppBottomBarInfo;

  private aboutToAppear(): void  {
  }

  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      Swiper() {
        ForEach(this.mAppGridInfo, (item) => {
          SwiperPage({
            mAppListInfo: item,
            mAppBottomBarInfo: this.mAppBottomBarInfo,
            moveAppX: $moveAppX,
            moveAppY: $moveAppY,
            moveAppShow: $moveAppShow,
            moveAppInfo: $moveAppInfo,
            gridConfig: this.gridConfig
          })
        })
      }
      .height(DefaultConstants.PERCENTAGE_100)
      .width(DefaultConstants.PERCENTAGE_100)
      .loop(false)
      .index(this.PageIndex)
      .onChange((index) => {
        mAppGridPresenter.changeIndex(index);
      })
    }
    .height(DefaultConstants.PERCENTAGE_100)
    .width(DefaultConstants.PERCENTAGE_100)
  }
}

@Component
struct SwiperPage {
  @State ColumnsTemplate: string = '';
  @State RowsTemplate: string = ''
  @Link moveAppX: number;
  @Link moveAppY: number;
  @Link moveAppShow: boolean;
  @Link moveAppInfo: any;
  @Prop @Watch('changeColumnAndRow') gridConfig: string;
  private mAppBottomBarInfo;
  private mAppListInfo;

  private aboutToAppear(): void  {
    this.changeConfig();
  }

  public changeColumnAndRow() {
    this.changeConfig();
  }

  public changeConfig() {
    let mGridConfig = mAppGridPresenter.getGridConfig();
    let column = mGridConfig.column;
    let row = mGridConfig.row;
    this.ColumnsTemplate = '';
    this.RowsTemplate = '';
    for (let i = 0;i < column; i++) {
      this.ColumnsTemplate += '1fr '
    }
    for (let i = 0;i < row; i++) {
      this.RowsTemplate += '1fr '
    }
  }

  build() {
    Grid() {
      ForEach(this.mAppListInfo, (item) => {
        GridItem() {
          AppItem({
            mAppBottomBarInfo: this.mAppBottomBarInfo
            item: item,
            moveAppX: $moveAppX,
            moveAppY: $moveAppY,
            moveAppShow: $moveAppShow,
            moveAppInfo: $moveAppInfo
          })
        }
        .rowStart(item.row)
        .columnStart(item.column)
        .rowEnd(item.row)
        .columnEnd(item.column)
      })
    }
    .columnsTemplate(this.ColumnsTemplate)
    .rowsTemplate(this.RowsTemplate)
  }
}


@Component
struct AppItem {
  @State AppItemHeight: number = DefaultConstants.DEFAULT_APP_ITEM_HEIGHT;
  @State AppItemWidth: number = DefaultConstants.DEFAULT_APP_ITEM_WIDTH;
  @State AppNameSize: number = DefaultConstants.DEFAULT_APP_NAME_SIZE;
  @State isShow: boolean = true;
  @State isLongPress: boolean = false;
  @Link moveAppX: number;
  @Link moveAppY: number;
  @Link moveAppShow: boolean;
  @Link moveAppInfo: any;
  private mAppBottomBarInfo;
  private startPosition;
  private endPosition;
  private item: any;
  private isSwappingPage = false;
  dialogController: CustomDialogController = new CustomDialogController({
    builder: UninstallDialog(),
    cancel: this.cancelDialog,
    autoCancel: true
  });

  cancelDialog() {
    console.info('Launcher Grid Cancel Dialog');
  }

  movingIconSwapPageDelay() {
    this.isSwappingPage = true;
    setTimeout(() => {
      this.isSwappingPage = false;
    }, APP_INFO_REFRESH_DELAY);
  }

  build() {
    Flex({
      direction: FlexDirection.Column,
      alignItems: ItemAlign.Center,
      justifyContent: FlexAlign.SpaceAround
    }) {
      if (this.isShow) {
        Column() {
          AppIcon({
            iconSize: this.AppItemWidth,
            appIcon: this.item.AppIcon,
            bundleName: this.item.bundleName,
            labelId: this.item.labelId
          })
          GridName({
            nameHeight: this.AppItemHeight - this.AppItemWidth,
            nameWidth: this.AppItemWidth,
            appName: this.item.AppName,
            nameSize: this.AppNameSize,
            bundleName: this.item.bundleName,
            labelId: this.item.labelId
          })
        }
        .onClick(() => {
          mAppGridPresenter.openApplication(this.item.abilityName, this.item.bundleName);
        })
      }
    }
    .gesture(
      LongPressGesture({ repeat: false })
        .onAction((event: GestureEvent) => {
        let appName = mAppGridPresenter.getAppName(this.item.bundleName);
        if(appName != null){
          this.item.AppName = appName;
        }
        AppStorage.SetOrCreate('uninstallAppInfo', this.item)
        this.dialogController.open();
        this.isLongPress = true;
      })
    )
    .onTouch((event: TouchEvent) => {
      if (event.type == 0) {
        this.moveAppX = event.touches[0].screenX;
        this.moveAppY = event.touches[0].screenY;
        this.moveAppInfo = this.item;
        this.startPosition = mAppGridPresenter.getTouchPosition(event.touches[0].screenX * mScreenMagnification, event.touches[0].screenY * mScreenMagnification);
      }

      if (event.type == 1 && this.isLongPress) {
        this.moveAppX = event.touches[0].screenX;
        this.moveAppY = event.touches[0].screenY;
        this.moveAppShow = false;
        this.isShow = true;
        this.isLongPress = false;
        this.endPosition = mAppGridPresenter.getTouchPosition(event.touches[0].screenX * mScreenMagnification, event.touches[0].screenY * mScreenMagnification);
        mAppGridPresenter.layoutAdjustment(this.startPosition, this.endPosition, this.mAppBottomBarInfo);
      }
      if (event.type == 2 && this.isLongPress) {

        this.dialogController.close();
        this.moveAppShow = true;
        this.isShow = false;
        this.moveAppX = event.touches[0].screenX;
        this.moveAppY = event.touches[0].screenY;
        let moveX = this.moveAppX * mScreenMagnification;
        let moveY = this.moveAppY * mScreenMagnification;


        if ((moveX - CELLWIDTH / 2) < 0 && !this.isSwappingPage && mAppGridPresenter.getIndex() > 0 && moveY < mSHeight) {
          mAppGridPresenter.changeIndex(mAppGridPresenter.getIndex() - 1);
          this.movingIconSwapPageDelay();
        } else if ((moveX + CELLWIDTH / 2) > DESIGN_WIDTH && !this.isSwappingPage && moveY < mSHeight) {
          if (mAppGridPresenter.getIndex() == mAppGridPresenter.getGridPageCount() - 1) {
            mAppGridPresenter.addBlankPage();
          } else {
            mAppGridPresenter.changeIndex(mAppGridPresenter.getIndex() + 1);
          }
          this.movingIconSwapPageDelay();
        }
      }
    })
    .width(DefaultConstants.PERCENTAGE_100)
    .height(DefaultConstants.PERCENTAGE_100)
  }
}

@Component
struct BottomBar {
  @State AppItemHeight: number = DefaultConstants.DEFAULT_APP_ITEM_HEIGHT;
  @State AppItemWidth: number = DefaultConstants.DEFAULT_APP_ITEM_WIDTH;
  @State AppNameSize: number = DefaultConstants.DEFAULT_APP_NAME_SIZE;
  @StorageLink('BottomBarItemWidth') ItemWidth: string = DefaultConstants.PERCENTAGE_25;
  @Link moveAppX: number;
  @Link moveAppY: number;
  @Link moveAppShow: boolean;
  @Link moveAppInfo: any;
  private mAppBottomBarInfo;

  private aboutToAppear(): void  {
  }

  build() {
    Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.SpaceAround }) {
      Row() {
        ForEach(this.mAppBottomBarInfo, (item) => {
          Column() {
            AppItem({
              mAppBottomBarInfo: this.mAppBottomBarInfo
              item: item,
              moveAppX: $moveAppX,
              moveAppY: $moveAppY,
              moveAppShow: $moveAppShow,
              moveAppInfo: $moveAppInfo
            })
          }
          .height(DefaultConstants.PERCENTAGE_100)
          .width(this.ItemWidth)
        })
      }
      .height(DefaultConstants.PERCENTAGE_100)
    }
    .height(DefaultConstants.PERCENTAGE_100)
    .width(DefaultConstants.PERCENTAGE_100)
  }
}

@CustomDialog
struct ShowDialog {
  controller: CustomDialogController;
  cancel: () => void;
  action: () => void;
  @StorageLink('blankPageBtnText') buttonText: string = '';

  build() {
    Row() {
      Button() {
        Text($r('app.string.intoSettings'))
          .height(DefaultConstants.PERCENTAGE_100)
          .fontSize(DefaultConstants.DEFAULT_APP_NAME_SIZE)
          .fontColor(Color.White)
      }
      .width(DefaultConstants.PIXEL_200)
      .height(DefaultConstants.PIXEL_50)
      .backgroundColor(Color.Blue)
      .onClick(() => {
        mAppGridPresenter.intoSetting();
        this.controller.close();
      })

      Button() {
        Text(this.buttonText)
          .height(DefaultConstants.PERCENTAGE_100)
          .fontSize(DefaultConstants.DEFAULT_APP_NAME_SIZE)
          .fontColor(Color.White)
      }
      .width(DefaultConstants.PIXEL_200)
      .height(DefaultConstants.PIXEL_50)
      .backgroundColor(Color.Blue)
      .onClick(() => {
        mAppGridPresenter.addOrDeleteBlankPage();
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
            .fontSize(DefaultConstants.DEFAULT_APP_NAME_SIZE)
            .fontColor(Color.White)
        }
        .margin({ right: DefaultConstants.DEFAULT_20 })
        .width(DefaultConstants.PIXEL_150)
        .height(DefaultConstants.PIXEL_50)
        .backgroundColor(Color.Blue)
        .onClick(() => {
          mAppGridPresenter.uninstallApp(this.AppInfo.bundleName, this.AppInfo.System);
          this.controller.close();
        })

        Button() {
          Text($r('app.string.cancel'))
            .fontSize(DefaultConstants.DEFAULT_APP_NAME_SIZE)
            .fontColor(Color.White)
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
    }
    .padding(DefaultConstants.DEFAULT_20)
  }
}

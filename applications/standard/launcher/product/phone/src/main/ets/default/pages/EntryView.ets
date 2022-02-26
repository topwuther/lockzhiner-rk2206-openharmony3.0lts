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

import display from '@ohos.display';
import DefaultConstants from '../common/constants/DefaultConstants.ets'
import GridLayout from '../../../../../../../feature/layoutmanager/src/main/ets/default/layout/GridLayout.ets'
import ListLayout from '../../../../../../../feature/layoutmanager/src/main/ets/default/layout/ListLayout.ets'
import AppModel from '../../../../../../../common/src/main/ets/default/common/model/AppModel.ets'
import SettingsModel from '../../../../../../../common/src/main/ets/default/common/model/SettingsModel.ets'

const SYSTEM_UI_HEIGHT = 134;
const DESIGN_WIDTH = 720.0;
const HORIZONTAL = 0.7;

@Entry
@Component
struct LayoutManager {
  @State mAppPageStartConfig: string = '';
  @State proportion: number = 0;
  @State mScreenHeight: number = 0;
  @State mScreenWidth: number = 0;
  @State gridConfig: string = '';
  @State equipment: string = 'phone';
  @State show: boolean = false;
  @State SwiperProportion: string = '85%';
  @State BottomBarProportion: string = '15%';
  private mAppModel: AppModel;
  private mSettingsModel: SettingsModel;

  private aboutToAppear(): void  {
    this.mAppModel = AppModel.getInstance()
    this.mSettingsModel = new SettingsModel();
    this.mAppPageStartConfig = this.mSettingsModel.getAppPageStartConfig();
    this.mAppModel.registerAppListEvent();
    display.getDefaultDisplay().then(dis => {
      this.proportion = DESIGN_WIDTH / dis.width;
      this.mScreenHeight = (dis.height - SYSTEM_UI_HEIGHT) * this.proportion;
      this.mScreenWidth = DESIGN_WIDTH;
      if (this.mScreenHeight < this.mScreenWidth) {
        this.equipment = 'smartVision';
        this.SwiperProportion = '75%';
        this.BottomBarProportion = '25%';
      }
      this.show = true;
      console.info("Launcher EntryView onShow end");
    })
  }

  onPageShow() {
    if (this.mSettingsModel != undefined) {
        this.mAppPageStartConfig = this.mSettingsModel.getAppPageStartConfig();
        this.gridConfig = this.mSettingsModel.getGridConfig().layout;
    }
  }

  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      if (this.show == true) {
        if (this.mAppPageStartConfig === 'Grid') {
          GridLayout({
            equipment: this.equipment,
            mScreenHeight: this.mScreenHeight,
            mScreenWidth: this.mScreenWidth,
            ScreenMagnification: this.proportion,
            SwiperProportion: this.SwiperProportion,
            BottomBarProportion: this.BottomBarProportion,
            gridConfig: this.gridConfig
          });
        } else {
          ListLayout();
        }
      }
    }
    .backgroundImage(DefaultConstants.DEFAULT_BACKGROUND_IMAGE)
    .backgroundImageSize(ImageSize.Cover)
  }
}
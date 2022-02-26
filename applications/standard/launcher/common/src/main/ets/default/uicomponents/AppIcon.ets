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

import ResourceManager from '../manager/ResourceManager.ets'
import DefaultConstants from '../common/constants/DefaultConstants.ets'

const LOOP_TIME = 500;

@Component
export default
struct AppIcon {
  @Prop iconSize: number;
  @Prop @Watch("updateIcon") appIcon: string;
  @Prop bundleName: string;
  @Prop labelId: string;
  @State icon: string = ' ';
  @State useCache: boolean = true;
  private mResourceManager;
  private mDefaultAppIcon;

  public aboutToAppear(): void  {
    this.mDefaultAppIcon = DefaultConstants.DEFAULT_ICON;
    this.icon = this.mDefaultAppIcon;
    this.mResourceManager = ResourceManager.getInstance();
    if (this.useCache) {
      this.mResourceManager.getAppIconWithCache(this.appIcon, this.bundleName, this.iconLoadCallback.bind(this), this.mDefaultAppIcon);
    } else {
      this.mResourceManager.getAppIcon(this.appIcon, this.bundleName, this.iconLoadCallback.bind(this), this.mDefaultAppIcon);
    }
    setInterval(() => {
      this.updateIcon();
    }, LOOP_TIME);
  }

  public iconLoadCallback(image) {
    this.icon = image;
  }

  public updateIcon() {
    if (this.useCache) {
      this.mResourceManager.getAppIconWithCache(this.appIcon, this.bundleName, this.iconLoadCallback.bind(this), this.mDefaultAppIcon);
    } else {
      this.mResourceManager.getAppIcon(this.appIcon, this.bundleName, this.iconLoadCallback.bind(this), this.mDefaultAppIcon);
    }
  }

  build() {
    Column() {
      Image(this.icon)
    }
    .width(this.iconSize)
    .height(this.iconSize)
  }
}
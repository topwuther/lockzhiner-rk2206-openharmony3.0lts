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
struct ListName {
  @State nameSize: number = 20;
  @Prop @Watch("updateName") appName: string;
  @State name: string = ''
  @Prop bundleName: string;
  @Prop labelId: string;
  private mResourceManager;

  public aboutToAppear(): void  {
    this.mResourceManager = ResourceManager.getInstance();
    this.mResourceManager.getAppName(this.labelId, this.bundleName, this.appName, this.appNameLoadCallback.bind(this));
    setInterval(() => {
      this.updateName();
    }, LOOP_TIME);
  }

  public appNameLoadCallback(name: string) {
    this.name = name;
  }

  public updateName() {
    this.mResourceManager.getAppName(this.labelId, this.bundleName, this.appName, this.appNameLoadCallback.bind(this));
  }

  build() {
    Column() {
      Text(this.name)
        .fontSize(this.nameSize)
        .fontColor(DefaultConstants.DEFAULT_FONT_COLOR)
        .margin({ left: DefaultConstants.DEFAULT_NUMBER })
    }
  }
}
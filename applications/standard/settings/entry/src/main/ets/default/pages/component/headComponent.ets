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
import ConfigData from '../../common/baseUtil/ConfigData.ets';
import Router from '@system.router';

@Component
export default
struct HeadComponent {
  private isActive: boolean= true;
  private icBackIsVisibility: boolean= true;
  private headName: string= '';
  @State isTouch: boolean= false;
  @StorageLink('wifiPsd') wifiPsd: string= ''

  build() {
    Row() {
      Image($r('app.media.ic_back'))
        .width($r('app.float.wh_value_50'))
        .height($r('app.float.wh_value_50'))
        .borderRadius($r('app.float.wh_value_15'))
        .margin({
        left: $r('app.float.wh_value_15'),
        top: $r('app.float.wh_value_5'),
        bottom: $r('app.float.wh_value_5')
      })
        .backgroundColor(this.isTouch ? $r('app.color.color_E3E3E3_grey') : $r('app.color.color_00000000_transparent'))
        .visibility(this.icBackIsVisibility ? Visibility.Visible : Visibility.None)
        .onTouch(() => {
        this.isTouch = true;
        Router.back();
      })
      Text(this.headName)
        .fontSize($r('app.float.font_24'))
        .fontWeight(FontWeight.Bold)
        .fontColor(Color.Black)
        .width(ConfigData.WH_90_100)
        .maxLines(ConfigData.MAX_LINES_1)
        .textOverflow({ overflow: TextOverflow.Ellipsis })
        .margin({ left: $r('app.float.wh_value_15'), right: $r('app.float.wh_value_15') });
    }
    .width(ConfigData.WH_100_100)
    .height($r('app.float.wh_value_70'))
    .alignItems(VerticalAlign.Center)
    .align(Alignment.Start);
  }
}
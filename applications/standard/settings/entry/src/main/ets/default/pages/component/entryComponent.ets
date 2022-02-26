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

@Component
export default
struct EntryComponent {
  @State isShow: Boolean = true;
  @Prop settingIcon: string;
  @State endTextIsShow: Boolean= true;
  @Prop settingTitle: string;
  @Prop settingSummary: string;
  @Prop settingValue: string;
  @Prop settingArrow: string;
  @Prop settingArrowStyle: string;
  @Prop settingUri: string;

  build() {
    Flex({ justifyContent: FlexAlign.SpaceBetween }) {
      Row() {
        Image(this.settingIcon)
          .width($r('app.float.wh_value_50'))
          .height($r('app.float.wh_value_50'))
          .margin({ right: $r('app.float.wh_value_10') })
          .visibility('' === this.settingIcon ? Visibility.None : Visibility.Visible)
          .objectFit(ImageFit.Contain);
        Column() {
          Text(this.settingTitle)
            .fontColor($r('app.color.color_333333_grey'))
            .fontSize($r('app.float.font_20'))
            .textAlign(TextAlign.Start)
            .maxLines(ConfigData.MAX_LINES_3)
            .textOverflow({ overflow: TextOverflow.Ellipsis })
            .margin({ top: $r('app.float.wh_value_10'), bottom: $r('app.float.wh_value_5') });
          Row() {
            Text($r('app.string.version'))
              .fontColor($r('app.color.color_999999_grey'))
              .fontSize($r('app.float.font_18'))
              .textAlign(TextAlign.Start)
              .maxLines(ConfigData.MAX_LINES_1)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
              .visibility('pages/applicationInfo' === this.settingUri ? Visibility.Visible : Visibility.None)
              .margin({ bottom: $r('app.float.wh_value_10') });
            Text(this.settingSummary)
              .fontColor($r('app.color.color_999999_grey'))
              .fontSize($r('app.float.font_18'))
              .textAlign(TextAlign.Start)
              .maxLines(ConfigData.MAX_LINES_1)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
              .visibility('' === this.settingSummary ? Visibility.None : Visibility.Visible)
              .margin({ bottom: $r('app.float.wh_value_10') });
          }
        }
        .alignItems(HorizontalAlign.Start);
      }
      .flexShrink(0)
      .alignItems(VerticalAlign.Center)
      .align(Alignment.Start)

      Row() {
        Text(this.settingValue)
          .fontSize($r('app.float.font_18'))
          .fontColor($r('app.color.color_999999_grey'))
          .height($r('app.float.wh_value_40'))
          .margin({ right: $r('app.float.wh_value_10') })
          .align(Alignment.End);
        Image(this.settingArrow)
          .width(this.settingArrowStyle === '' ? $r('app.float.wh_value_20') : $r('app.float.wh_value_40'))
          .height($r('app.float.wh_value_40'))
          .visibility('' === this.settingArrow ? Visibility.None : Visibility.Visible);
      }
      .margin({ right: $r('app.float.wh_value_15') })
      .align(Alignment.End);
    }
    .margin({ left: $r('app.float.wh_value_15') })
    .width(ConfigData.WH_100_100);
  }
}
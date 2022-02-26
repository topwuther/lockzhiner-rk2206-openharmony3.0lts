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
import HeadComponent from './component/headComponent.ets';
import EntryComponent from './component/entryComponent.ets';
import AboutDeviceModel from '../model/dateAndTimeImpl/DateAndTimeModel.ets'
import ConfigData from '../common/baseUtil/ConfigData.ets';
import DateAndTime from '../common/baseUtil/DateAndTimeUtil.ets';
import DateAndTimeModel from '../model/dateAndTimeImpl/DateAndTimeModel.ets'
import LogUtil from '../common/baseUtil/LogUtil.ets';

@Entry
@Component
struct dateAndTime {
  private date: string = '';
  private time: string = '';
  private image: string = '';
  private dateMark: string = 'date';
  private timeMark: string = 'time';
  private intervalFlag;
  @State dateAndTimeList: any[] = [];
  timeDialogController: CustomDialogController = new CustomDialogController({
    builder: TimeDialog({ action: this.onAccept }),
    cancel: this.existApp,
    autoCancel: true
  });
  dateDialogController: CustomDialogController = new CustomDialogController({
    builder: DateDialog({ action: this.onAccept }),
    cancel: this.existApp,
    autoCancel: true
  });

  build() {
    Column() {
      HeadComponent({ headName: $r('app.string.dateAndTimeTab'), isActive: true })
      List() {
        ForEach(this.dateAndTimeList, (item) => {
          ListItem() {
            Row() {
              Text(item.settingTitle)
                .fontSize($r('app.float.font_22'))
                .fontColor($r('app.color.color_333333_grey'))
                .width(ConfigData.WH_45_100)
                .textAlign(TextAlign.Start);
              Text(item.settingValue)
                .fontSize($r('app.float.font_20'))
                .fontColor($r('app.color.color_666666_grey'))
                .width(ConfigData.WH_45_100)
                .textAlign(TextAlign.End);
              Image(item.settingArrow)
                .width($r('app.float.wh_value_40'))
                .height($r('app.float.wh_value_40'));
            }
          }
          .width(ConfigData.WH_100_100)
          .height($r('app.float.wh_value_80'))

          .onClick(() => {
            LogUtil.info('dialog come in' + JSON.stringify(item))
            if (item.settingAlias === 'date') {
              LogUtil.info('date dialog come in')
              this.dateDialogController.open()
            }
            if (item.settingAlias === 'time') {
              LogUtil.info('time dialog come in')
              this.timeDialogController.open()
            }
          })
          .width(ConfigData.WH_100_100)
          .height($r('app.float.wh_value_70'));
        }, item => JSON.stringify(item))
      }
      .divider({ strokeWidth: $r('app.float.wh_value_1'), color: $r('app.color.color_E3E3E3_grey'),
        startMargin: $r('app.float.wh_value_15'), endMargin: $r('app.float.wh_value_15') })
      .width(ConfigData.WH_100_100)
      .height(ConfigData.WH_100_100)
      .margin({ left: $r('app.float.wh_value_15'), right: $r('app.float.wh_value_15') })
      .align(Alignment.TopStart);
    }
  }

  onAccept() {
    console.log("onAccept");
  }

  existApp() {
    console.log("Cancel dialog!");
  }

  changeValue() {
    this.dateAndTimeList = [];
    this.dateAndTimeList.push(
      {
        settingSummary: '',
        settingTitle: $r('app.string.date'),
        settingValue: this.date,
        settingAlias: this.dateMark,
        settingArrow: this.image,
      },
      {
        settingSummary: '',
        settingTitle: $r('app.string.time'),
        settingValue: this.time,
        settingAlias: this.timeMark,
        settingArrow: this.image,
      }
    );

  }

  private aboutToAppear(): void {
    this.dateAndTimeList = AboutDeviceModel.dateAndTimeListener();
    this.image = this.dateAndTimeList[0].settingArrow;
    this.time = DateAndTime.getSystemTime();
    this.date = DateAndTime.getSystemDate();
    this.changeValue();
    this.getNowTime();
  }

  private getNowTime(): void {
    this.intervalFlag = setInterval(() => {
      console.log('get time come in');
      this.time = DateAndTime.getSystemTime();
      this.date = DateAndTime.getSystemDate();
      console.log('get time end in' + this.time);
      this.changeValue();
    }, 1000);
  }

  private aboutToDisappear(): void {
    clearInterval(this.intervalFlag);
  }

  onBackPress() {
    Navigator().active(true);
  }
}

@CustomDialog
@Component
struct TimeDialog {
  controller: CustomDialogController;
  action: () => void;
  private currentTime;
  private formatTime;

  build() {
    Column() {
      Column() {
        Text($r('app.string.time'))
          .height($r('app.float.wh_value_60'))
          .margin({ left: $r('app.float.wh_value_30') })
          .width(ConfigData.WH_100_100)
          .fontSize($r('app.float.font_28'));
        DatePicker({ selected: this.currentTime, type: DatePickerType.Time })
          .useMilitaryTime(true)
          .onChange((date: DatePickerResult) => {
          this.currentTime = DateAndTime.concatTime(date.hour, date.minute);
          console.info("onchange currentTime" + this.currentTime);
        })
          .width(ConfigData.WH_100_100)
          .height($r('app.float.wh_value_280'))
      }

      Row() {
        Column() {
          Text($r('app.string.cancel'))
            .fontSize($r('app.float.font_24'))
            .fontColor(Color.Blue)
            .onClick(() => {
            this.controller.close();
            this.action();
          })
        }
        .alignItems(HorizontalAlign.Center)
        .width(ConfigData.WH_50_100);
        Column() {
          Text($r('app.string.confirm'))
            .fontSize($r('app.float.font_24'))
            .fontColor(Color.Blue)
            .onClick(() => {
            LogUtil.info("button confirm");
            let datetime = new Date();
            let y = datetime.getFullYear();
            let m = datetime.getMonth() + 1;
            let d = datetime.getDate();
            this.formatTime = y + '-' + DateAndTime.fill(m) + '-' + DateAndTime.fill(d)
            + 'T' + this.currentTime + ':' + '00';
            LogUtil.info("onchange format time" + this.formatTime);
            let s = (new Date(this.formatTime)).getTime();
            LogUtil.info("onchange time second" + s);
            DateAndTimeModel.setTime(s);
            this.controller.close();
            this.action();
          })
        }
        .alignItems(HorizontalAlign.Center)
        .width(ConfigData.WH_50_100)
      }
      .width(ConfigData.WH_100_100)
      .height($r('app.float.wh_value_60'))
    }
    .height($r('app.float.wh_value_400'))
    .width(ConfigData.WH_100_100)
  }
}

@CustomDialog
@Component
struct DateDialog {
  controller: CustomDialogController;
  action: () => void;
  private currentDate;
  private formatDate;
  @State date: string = '';
  private day;
  @StorageLink('Title') title: string= '';

  build() {
    Column() {
      Column() {
        Row() {
          Text(this.date)
            .fontSize($r('app.float.font_28'))
          Text(this.day)
            .fontSize($r('app.float.font_28'))
        }
        .height($r('app.float.wh_value_60'))
        .margin({ left: $r('app.float.wh_value_15') })
        .width(ConfigData.WH_100_100)

        DatePicker({ start: new Date('2008-12-21'), end: new Date('2030-12-12'),
          selected: this.currentDate, type: DatePickerType.Date })
          .onChange((date: DatePickerResult) => {
          let datetime = new Date();
          let h = datetime.getHours();
          let min = datetime.getMinutes();
          let s = datetime.getSeconds();
          this.formatDate = date.year + '-' + DateAndTime.fill(date.month + 1) + '-' + DateAndTime.fill(date.day) + 'T' +
          DateAndTime.fill(h) + ':' + DateAndTime.fill(min) + ':' + DateAndTime.fill(s);
          this.date = DateAndTime.concatDate(date.year, date.month + 1, date.day);
          this.day = DateAndTime.convert(date.year, date.month + 1, date.day);
          LogUtil.info("onchange formatDate" + this.formatDate);
          LogUtil.info("onchange currentDate" + this.currentDate);
          LogUtil.info("onchange date" + this.date);
        })
          .width(ConfigData.WH_100_100)
          .height($r('app.float.wh_value_280'));
      }

      Row() {
        Column() {
          Text($r('app.string.cancel'))
            .fontSize($r('app.float.font_24'))
            .fontColor(Color.Blue)
            .onClick(() => {
            this.controller.close();
            this.action();
          });
        }
        .alignItems(HorizontalAlign.Center)
        .width(ConfigData.WH_50_100);
        Column() {
          Text($r('app.string.confirm'))
            .fontSize($r('app.float.font_24'))
            .fontColor(Color.Blue)
            .onClick(() => {
            let s = (new Date(this.formatDate)).getTime();
            DateAndTimeModel.setTime(s);
            this.controller.close();
            this.action();
          });
        }
        .alignItems(HorizontalAlign.Center)
        .width(ConfigData.WH_50_100);
      }
      .width(ConfigData.WH_100_100)
      .height($r('app.float.wh_value_60'));
    }
    .height($r('app.float.wh_value_400'))
    .width(ConfigData.WH_100_100);
  }

  private aboutToAppear() {
    let datetime = new Date();
    let y = datetime.getFullYear();
    let m = datetime.getMonth() + 1;
    let d = datetime.getDate();
    this.date = DateAndTime.getSystemDate();
    this.day = DateAndTime.convert(y, m, d);
  }
}






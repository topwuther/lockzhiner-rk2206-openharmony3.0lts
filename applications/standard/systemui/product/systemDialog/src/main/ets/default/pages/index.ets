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

import DialogModel from '../dialogModel.ets';
import Log from '../../../../../../../common/src/main/ets/default/Log.ets';

const TAG = 'SystemDialog_Index';

let tempWorkArray = [];
let tempUnuseList = [];

@Entry
@Component
struct Index {
  @State useList: any = []
  @State workDataArray: any = []
  @State unUseList: any = []
  private getData: any;

  aboutToAppear() {
    Log.showInfo(TAG, 'system dialog about to appear');
    DialogModel.registerListener(this.getMessage.bind(this));
  }

  aboutToDisappear() {
    Log.showInfo(TAG, 'system dialog about to disappear');
    DialogModel.unregisterListener();
  }

  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center }) {
      Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
        Text($r('app.string.access_request'))
          .fontSize(22)
          .fontWeight(FontWeight.Bold)
          .fontColor($r('app.color.title_font_color'))
          .height(50)
      }.width('100%')

      List() {
        ForEach(this.workDataArray, (item) => {
          ListItem() {
            Flex({
              direction: FlexDirection.Row,
              alignItems: ItemAlign.Center,
              justifyContent: FlexAlign.SpaceAround
            }) {
              Text(item)
                .fontSize(18)
                .fontColor(Color.Black)
                .width(250)

              Toggle({ type: ToggleType.Button, isOn: false })
                .width(60)
                .height(40)
                .onChange((isOn) => {
                let index = this.useList.indexOf(item);
                if (isOn) {
                  if (index === -1) {
                    this.useList.push(item)
                    this.unUseList.splice(index, 1)
                  }
                } else {
                  if (index !== -1) {
                    this.useList.splice(index, 1)
                    this.unUseList.push(item)
                  }
                }
              })
            }
            .width('100%')
            .height(75)
          }.height(75)
        }, item => item);
      }.divider({ strokeWidth: 1, color: $r('app.color.divider_line'), startMargin: 2, endMargin: 2 })
      .width('100%')
      .height(200)

      Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.SpaceAround }) {
        Button() {
          Text($r('app.string.access_sure')).fontSize(20).fontColor($r('app.color.button_font_color'))
        }
        .width(100)
        .height(50)
        .onClick(() => this.makeSure())

        Button() {
          Text($r('app.string.access_cancel')).fontSize(20).fontColor($r('app.color.button_font_color'))
        }
        .width(100)
        .height(50)
        .onClick(() => this.cancelButton())
      }.width('100%')
    }.width('100%')
  }

/**
     * Get permission data
     * @param {Object} resultData  permission data Object
     *
     */
  getMessage(resultData) {
    Log.showInfo(TAG, 'index getMessage result');
    let dataList = resultData?.parameters?.OHOS_REQUEST_PERMISSIONS_LIST;
    if (dataList?.length !== 0) {
      this.getData = resultData;
      Log.showInfo(TAG, ` ge tMessageWant result Data ${JSON.stringify(resultData)}`);
      // handle permission Data to show
      for (let item of dataList) {
        tempWorkArray.push(item);
        tempUnuseList.push(item);
      }
      this.workDataArray = tempWorkArray;
      this.unUseList = tempUnuseList;
      Log.showInfo(TAG, ` getMessageWant work with data  ${JSON.stringify(this.workDataArray)}`);
    } else {
      Log.showInfo(TAG, ' getMessageWant result Data is a empty object');
    }
  }


/**
     * User confirmation
     */
  makeSure() {
    Log.showInfo(TAG, ' makeSure Click');
    let want = {
      parameters: {
        OHOS_RESULT_PERMISSION_KEY: 1,
        OHOS_RESULT_PERMISSIONS_LIST: this.getData.parameters.OHOS_REQUEST_PERMISSIONS_LIST,
        OHOS_RESULT_PERMISSIONS_LIST_YES: this.useList,
        OHOS_RESULT_PERMISSIONS_LIST_NO: this.unUseList,
        OHOS_RESULT_CALLER_BUNDLERNAME: this.getData.parameters.OHOS_REQUEST_CALLER_BUNDLERNAME
      }
    };
    let AbilityResult = {
      resultCode: 1,
      want: want
    };
    DialogModel.finishResult(AbilityResult);
  }

/**
     * User canceled
     */
  cancelButton() {
    Log.showInfo(TAG, ' cancelButton Click');
    let want = {
      parameters: {
        OHOS_RESULT_PERMISSION_KEY: 1,
        OHOS_RESULT_PERMISSIONS_LIST: this.getData.parameters.OHOS_REQUEST_PERMISSIONS_LIST,
        OHOS_RESULT_CALLER_BUNDLERNAME: this.getData.parameters.OHOS_REQUEST_CALLER_BUNDLERNAME
      }
    };
    let abilityResult = {
      resultCode: 0,
      want: want
    };
    DialogModel.finishResult(abilityResult);
  }
}
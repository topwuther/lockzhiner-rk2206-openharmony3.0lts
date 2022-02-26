/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file expected in compliance with the License.
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

#include "buffer_manager_test.h"

void UtestBufferManagerTest::SetUpTestCase(void)
{}
void UtestBufferManagerTest::TearDownTestCase(void)
{}
void UtestBufferManagerTest::SetUp(void)
{
    if (display_ == nullptr)
    display_ = std::make_shared<TestDisplay>();
    display_->FBInit();
    display_->Init();
}
void UtestBufferManagerTest::TearDown(void)
{
    display_->Close();
}

/**
  * @tc.name: OpenCamera cameraID input error
  * @tc.desc: 请求GBM/ION buffer命令，参数正常，通过Display模块申请buffer申请buffer成功。
  * @tc.level: level0
  * @tc.size: MediumTest
  * @tc.type: Function
  */
TEST_F(UtestBufferManagerTest, camera_buffer_0001)
{
    std::cout << "==========[test log] Preview stream, expected success." << std::endl;
    // 获取流管理器
    display_->AchieveStreamOperator();
    // 启流
    display_->intents = {Camera::PREVIEW};
    display_->StartStream(display_->intents);
    // 获取预览图
    display_->StartCapture(display_->streamId_preview, display_->captureId_preview, false, true);
    // 释放流
    display_->captureIds = {display_->captureId_preview};
    display_->streamIds = {display_->streamId_preview};
    display_->StopStream(display_->captureIds, display_->streamIds);
}
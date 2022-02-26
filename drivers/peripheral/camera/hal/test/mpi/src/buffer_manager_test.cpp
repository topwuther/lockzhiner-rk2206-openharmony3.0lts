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

using namespace OHOS;
using namespace std;
using namespace testing::ext;
using namespace OHOS::Camera;

void BufferManagerTest::SetUpTestCase(void) {}
void BufferManagerTest::TearDownTestCase(void) {}
void BufferManagerTest::SetUp(void)
{
    Test_ = std::make_shared<OHOS::Camera::Test>();
    Test_->Init();
    Test_->Open();
}
void BufferManagerTest::TearDown(void)
{
    Test_->Close();

}

/**
  * @tc.name: OpenCamera cameraID input error
  * @tc.desc: 请求GBM/ION buffer命令，参数正常，通过Display模块申请buffer申请buffer成功。
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(BufferManagerTest, Camera_Buffer_0001, TestSize.Level0)
{
    std::cout << "==========[test log]Preview stream, 640*480, expected success." << std::endl;
    // 启动流
    Test_->intents = {Camera::PREVIEW};
    Test_->StartStream(Test_->intents);
    // 获取预览图
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 释放流
    Test_->captureIds = {Test_->captureId_preview};
    Test_->streamIds = {Test_->streamId_preview};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

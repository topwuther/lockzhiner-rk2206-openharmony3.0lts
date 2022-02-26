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

#include "camera_manager.h"
#include "ic_callback.h"
#include "ic_constants.h"
#include "ic_retcode.h"
#include "ic_sdk.h"
#include "picture_utils.h"

using namespace std;
using namespace IC;
using namespace OHOS;
using namespace OHOS::AI;
using namespace OHOS::Media;

namespace {
const auto DELETE_PIC_BUFFER = [](uint8_t *&buffer) {
    if (buffer != nullptr) {
        delete[] buffer;
        buffer = nullptr;
    }
};
}

class MyIcCallback : public IcCallback {
public:
    MyIcCallback() {}
    ~MyIcCallback() {}

    void OnError(const IcRetCode errorCode)
    {
        printf("Executing error, error code: %d\n", errorCode);
    }

    void OnResult(const IcOutput &result)
    {
        if (result.size % 2 == 0) {
            int numClass = result.size / 2;
            for (size_t i = 0; i < numClass; ++i) {
                printf("class index [%d] score: %d\n", result.data[i], result.data[i + numClass]);
            }
        }
    }
};

static int CameraCapture()
{
    printf("Camera sample begin.\n");
    CameraKit *camKit = CameraKit::GetInstance();
    if (camKit == nullptr) {
        printf("Can not get CameraKit instance\n");
        return IC_RETCODE_FAILURE;
    }
    list<string> camList = camKit->GetCameraIds();
    string camId;
    for (auto &cam : camList) {
        printf("camera name: %s\n", cam.c_str());
        const CameraAbility *ability = camKit->GetCameraAbility(cam);
        list<CameraPicSize> sizeList = ability->GetSupportedSizes(0);
        for (auto &pic : sizeList) {
            if (pic.width == CAMERA_PIC_WIDTH && pic.height == CAMERA_PIC_HEIGHT) {
                camId = cam;
                break;
            }
        }
        if (!camId.empty()) {
            break;
        }
    }
    if (camId.empty()) {
        printf("No available camera.(1080p wanted)\n");
        return IC_RETCODE_FAILURE;
    }

    EventHandler eventHdlr;
    SampleCameraStateMng camStateMng(eventHdlr);
    camKit->CreateCamera(camId, camStateMng, eventHdlr);
    std::unique_lock<std::mutex> cameraCreateLock(g_mutex);
    g_cv.wait(cameraCreateLock);
    cameraCreateLock.unlock();
    camStateMng.Capture();
    std::unique_lock<std::mutex> captureLock(g_mutex);
    g_cv.wait(captureLock);
    captureLock.unlock();
    printf("Camera sample end.\n");
    return IC_RETCODE_SUCCESS;
}

static int ImageClassificationExecute(const IcInput &picData)
{
    if (picData.data == nullptr || picData.size == 0) {
        printf("ImageClassificationExecute: picData is empty.\n");
        return IC_RETCODE_FAILURE;
    }
    shared_ptr<IcSdk> detector = make_shared<IcSdk>();
    shared_ptr<MyIcCallback> callback = make_shared<MyIcCallback>();
    if (detector == nullptr || callback == nullptr) {
        return IC_RETCODE_FAILURE;
    }
    detector->SetCallback(callback);
    int retcode = detector->Create();
    if (retcode != IC_RETCODE_SUCCESS) {
        printf("ImageClassificationExecute: IcSdk Create failed.\n");
        return IC_RETCODE_FAILURE;
    }
    retcode = detector->SyncExecute(picData);
    if (retcode != IC_RETCODE_SUCCESS) {
        printf("ImageClassificationExecute: IcSdk SyncExecute failed.\n");
    }
    (void)detector->Destroy();
    return retcode;
}

static int GetPictureData(const string &sourcePath, Array<uint8_t> &picData)
{
    int srcWidth = 0;
    int srcHeight = 0;
    uint8_t *rawData = ReadJpegFile(sourcePath, srcWidth, srcHeight);
    if (rawData == nullptr) {
        printf("ReadJpegFile failed.\n");
        return IC_RETCODE_FAILURE;
    }
    uint8_t *srcData = Resize(WIDTH_DEST, HEIGHT_DEST, rawData, srcWidth, srcHeight);
    if (srcData == nullptr) {
        printf("Resize failed.\n");
        DELETE_PIC_BUFFER(rawData);
        return IC_RETCODE_FAILURE;
    }
    int srcDataSize = WIDTH_DEST * HEIGHT_DEST * NUM_CHANNELS;
    uint8_t *input = ConvertToCaffeInput(srcData, srcDataSize);
    if (input == nullptr) {
        printf("Convert to caffe input failed.\n");
        DELETE_PIC_BUFFER(rawData);
        DELETE_PIC_BUFFER(srcData);
        return IC_RETCODE_FAILURE;
    }
    picData.data = input;
    picData.size = srcDataSize;
    DELETE_PIC_BUFFER(rawData);
    DELETE_PIC_BUFFER(srcData);
    return IC_RETCODE_SUCCESS;
}

static int GetPicFromCamera(IcInput &picData)
{
    int retcode = CameraCapture();
    if (retcode != IC_RETCODE_SUCCESS) {
        printf("Capture picture failed.\n");
        return IC_RETCODE_FAILURE;
    }
    retcode = GetPictureData(CAMERA_SAVE_PATH, picData);
    if (retcode != IC_RETCODE_SUCCESS) {
        printf("GetPictureData failed.\n");
        return IC_RETCODE_FAILURE;
    }
    return IC_RETCODE_SUCCESS;
}

static int GetPicFromLocal(IcInput &picData)
{
    int retcode = GetPictureData(JPEG_SRC_PATH, picData);
    if (retcode != IC_RETCODE_SUCCESS) {
        printf("GetPictureData failed.\n");
        return IC_RETCODE_FAILURE;
    }
    return IC_RETCODE_SUCCESS;
}

static void SampleHelp()
{
    printf("****************************************\n");
    printf("Select the image source.\n");
    printf("1: Camera capture\n");
    printf("2: Local\n");
    printf("****************************************\n");
}

int main()
{
    IcInput picData = {
        .data = nullptr,
        .size = 0
    };
    int retcode = IC_RETCODE_SUCCESS;
    char input = ' ';
    SampleHelp();
    cin >> input;
    switch (input) {
        case '1':
            retcode = GetPicFromCamera(picData);
            break;
        case '2':
            retcode = GetPicFromLocal(picData);
            break;
        default:
            SampleHelp();
            break;
    }
    if (retcode != IC_RETCODE_SUCCESS) {
        printf("GetPicture failed.\n");
        return -1;
    }
    if (retcode != IC_RETCODE_SUCCESS) {
        printf("WriteBgrFile failed.\n");
        DELETE_PIC_BUFFER(picData.data);
        return -1;
    }
    retcode = ImageClassificationExecute(picData);
    if (retcode != IC_RETCODE_SUCCESS) {
        printf("ImageClassification failed.\n");
    } else {
        printf("ImageClassification successed.\n");
    }
    DELETE_PIC_BUFFER(picData.data);
    return 0;
}
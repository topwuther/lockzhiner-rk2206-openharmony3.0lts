/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "camera_manager.h"
#include "securec.h"
#include "ui_config.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;

static constexpr int TEMP_BUF_LEN = 8;
static constexpr int MAX_THM_SIZE = (64 * PAGE_SIZE);
static constexpr int FILE_NAME_LEN = 128;
static constexpr int MILLI_SECONDS = 1000;
static constexpr int PIC_WIDTH = 1920;
static constexpr int PIC_HEIGHT = 1080;

char* g_dstBuf = nullptr;

static int32_t SampleDealThumb(char* psrc, uint32_t srcSize, uint32_t* dstSize, uint16_t u16THMLen)
{
    int32_t endpos = 0;
    uint32_t s32I = 0;
    int32_t startpos = 0;
    char tempbuf[TEMP_BUF_LEN] = { 0 };
    int32_t bufpos = 0;
    char startflag[2] = { 0xff, 0xd8 };
    char endflag[2] = { 0xff, 0xd9 };

    while (s32I < srcSize) {
        if (bufpos >= TEMP_BUF_LEN) {
            break;
        }
        tempbuf[bufpos] = psrc[s32I++];
        if (bufpos > 0) {
            if (0 == memcmp(tempbuf + bufpos - 1, startflag, sizeof(startflag))) {
                startpos = s32I - 2;    /* 2 thumb head offset */
                if (startpos < 0) {
                    startpos = 0;
                }
            }
            if (0 == memcmp(tempbuf + bufpos - 1, endflag, sizeof(endflag))) {
                if (u16THMLen == s32I) {
                    endpos = s32I;
                } else {
                    endpos = s32I;
                }
                break;
            }
        }
        if (++bufpos == (TEMP_BUF_LEN - 1)) {
            if (tempbuf[bufpos - 1] != 0xFF) {
                bufpos = 0;
            }
        } else if (bufpos > (TEMP_BUF_LEN - 1)) {
            bufpos = 0;
        }
    }

    if ((endpos - startpos <= 0) || (static_cast<uint32_t>(endpos - startpos) >= srcSize)) {
        return -1;
    }

    char* temp = psrc + startpos;
    if (MAX_THM_SIZE < (endpos - startpos)) {
        return -1;
    }
    char* cDstBuf = (char*)malloc(endpos - startpos);
    if (cDstBuf == nullptr) {
        return -1;
    }

    (void)memcpy_s(cDstBuf, endpos - startpos, temp, endpos - startpos);

    g_dstBuf = cDstBuf;
    *dstSize = endpos - startpos;

    return 0;
}

static int32_t SampleGetThmFromJpg(const char* jpegPath, uint32_t* dstSize)
{
    FILE* fpJpg = nullptr;
    fpJpg = fopen(jpegPath, "rb");
    char* pszFile = nullptr;
    int32_t fd = 0;
    struct stat stStat;
    (void)memset_s(&stStat, sizeof(struct stat), 0, sizeof(struct stat));
    if (fpJpg == nullptr) {
        printf("file %s not exist!\n", jpegPath);
        return -1;
    }
    fd = fileno(fpJpg);
    fstat(fd, &stStat);
    pszFile = (char*)malloc(stStat.st_size);
    if ((pszFile == nullptr) || (stStat.st_size < 6)) {    /* 6 min size of thumb head  */
        fflush(fpJpg);
        fsync(fileno(fpJpg));
        fclose(fpJpg);
        if (pszFile) {
            free(pszFile);
            pszFile = nullptr;
        }
        printf("memory malloc fail!\n");
        return -1;
    }

    if (fread(pszFile, stStat.st_size, 1, fpJpg) <= 0) {
        fflush(fpJpg);
        fsync(fileno(fpJpg));
        fclose(fpJpg);
        free(pszFile);
        printf("fread jpeg src fail!\n");
        return -1;
    }
    fclose(fpJpg);

    // The fourth byte is shifted to the left by eight bits then the fifth byte is added.
    // the 4 byte is the length high 8 bit, and byte 5 is low 8bit;
    uint16_t u16THMLen = (static_cast<uint16_t>(pszFile[4]) << 8) + pszFile[5];
    if (SampleDealThumb(pszFile, stStat.st_size, dstSize, u16THMLen) < 0) {
        printf("get jpg thumb failed! \n");
        free(pszFile);
        return -1;
    }
    free(pszFile);
    return 0;
}

int32_t SampleGetdcfinfo(const char* srcJpgPath, const char* dstThmPath)
{
    int32_t s32RtnVal = 0;
    char jpegSrcPath[FILE_NAME_LEN] = { 0 };
    char jpegDesPath[FILE_NAME_LEN] = { 0 };
    uint32_t dstSize = 0;
    if (sprintf_s(jpegSrcPath, sizeof(jpegSrcPath), "%s", srcJpgPath) < 0) {
        return -1;
    }
    if (sprintf_s(jpegDesPath, sizeof(jpegDesPath), "%s", dstThmPath) < 0) {
        return -1;
    }
    s32RtnVal = SampleGetThmFromJpg(static_cast<const char *>(jpegSrcPath), &dstSize);
    if ((s32RtnVal != 0) || (dstSize == 0)) {
        printf("fail to get thm\n");
        return -1;
    }
    FILE* fpTHM = fopen(jpegDesPath, "w");
    if (fpTHM == 0) {
        printf("file to create file %s\n", jpegDesPath);
        return -1;
    }
    uint32_t u32WritenSize = 0;
    while (u32WritenSize < dstSize) {
        s32RtnVal = fwrite(g_dstBuf + u32WritenSize, 1, dstSize, fpTHM);
        if (s32RtnVal <= 0) {
            printf("fail to wirte file, rtn=%d\n", s32RtnVal);
            break;
        }
        u32WritenSize += s32RtnVal;
    }
    if (fpTHM != nullptr) {
        fflush(fpTHM);
        fsync(fileno(fpTHM));
        fclose(fpTHM);
        fpTHM = 0;
    }
    if (g_dstBuf != nullptr) {
        free(g_dstBuf);
        g_dstBuf = nullptr;
    }
    return 0;
}

static void SampleSaveCapture(const char* p, uint32_t size, int type, const char *timeStamp, int length)
{
    char acFileDcf[FILE_NAME_LEN] = {0};
    FILE *fp = nullptr;
    char acFile[FILE_NAME_LEN] = { 0 };

    if (type == 0) {
        char tmpFile[FILE_NAME_LEN] = {0};
        if (sprintf_s(tmpFile, sizeof(tmpFile), "%s/photo%s.jpg", PHOTO_PATH, timeStamp) < 0) {
            return;
        }
        fp = fopen(tmpFile, "w+");
        if (fp) {
            fwrite(p, 1, size, fp);
            fflush(fp);
            fsync(fileno(fp));
            fclose(fp);
        }
    }

        if (sprintf_s(acFile, sizeof(acFile), "%s/tmp.jpg", PHOTO_PATH) < 0) {
            return;
        }
        remove(acFile);

        fp = fopen(acFile, "w+");
        if (fp == NULL) {
            return;
        }
        fwrite(p, 1, size, fp);
        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);

    if (type == 0) {
        if (sprintf_s(acFileDcf, sizeof(acFileDcf), "%s/photo%s.jpg", THUMB_PATH, timeStamp) < 0) {
            return;
        }
    } else if (type == 1) {
        if (sprintf_s(acFileDcf, sizeof(acFileDcf), "%s/video%s.jpg", THUMB_PATH, timeStamp) < 0) {
            return;
        }
    } else {
        return;
    }

    SampleGetdcfinfo(static_cast<char *>(acFile), static_cast<char *>(acFileDcf));
}

static int CameraGetRecordFd(const char* p)
{
    int fd = -1;
    char pname[128] = {0};
    char *ptr = const_cast<char *>(p);
    char *pe = strrchr(ptr, '.');
    char *ps = strrchr(ptr, '/');
    if (pe == nullptr || ps == nullptr) {
        return -1;
    }

    if (strcpy_s(static_cast<char *>(pname), sizeof(pname), VIDEO_PATH) != 0) {
        return -1;
    }

    if (strncat_s(pname, sizeof(pname), ps, pe - ps) != 0) {
        return -1;
    }

    if (strcat_s(pname, sizeof(pname), ".mp4") < 0) {
        return -1;
    }
    fd = open(pname, O_RDWR | O_CREAT | O_CLOEXEC | O_TRUNC, S_IROTH | S_IRUSR | S_IWUSR);
    if (fd < 0) {
        return -1;
    }

    return fd;
}

Recorder *SampleCreateRecorder(int w, int h)
{
    int ret = 0;
    AudioCodecFormat audioFormat = AAC_LC;
    AudioSourceType inputSource = AUDIO_MIC;
    VideoSourceType source = VIDEO_SOURCE_SURFACE_ES;
    int32_t sourceId = 0;
    int32_t audioSourceId = 0;

    VideoCodecFormat encoder = HEVC;

    Recorder *recorder = new Recorder();
    if ((ret = recorder->SetVideoSource(source, sourceId)) != SUCCESS) {
        goto EXIT_;
    }
    if ((ret = recorder->SetVideoEncoder(sourceId, encoder)) != SUCCESS) {
        goto EXIT_;
    }
    if ((ret = recorder->SetVideoSize(sourceId, w, h)) != SUCCESS) {
        goto EXIT_;
    }
    if ((ret = recorder->SetVideoFrameRate(sourceId, FRAME_RATE)) != SUCCESS) {
        goto EXIT_;
    }
    if ((ret = recorder->SetVideoEncodingBitRate(sourceId, BIT_RATE)) != SUCCESS) {
        goto EXIT_;
    }
    if ((ret = recorder->SetCaptureRate(sourceId, FRAME_RATE)) != SUCCESS) {
        goto EXIT_;
    }
    if ((ret = recorder->SetAudioSource(inputSource, audioSourceId)) != SUCCESS) {
        goto EXIT_;
    }
    if ((ret = recorder->SetAudioEncoder(audioSourceId, audioFormat)) != SUCCESS) {
        goto EXIT_;
    }
    if ((ret = recorder->SetAudioSampleRate(audioSourceId, SAMPLE_RATE)) != SUCCESS) {
        goto EXIT_;
    }
    if ((ret = recorder->SetAudioChannels(audioSourceId, CHANNEL_COUNT)) != SUCCESS) {
        goto EXIT_;
    }
    if ((ret = recorder->SetAudioEncodingBitRate(audioSourceId, AUDIO_ENCODING_BITRATE)) != SUCCESS) {
        goto EXIT_;
    }

    if ((ret = recorder->SetMaxDuration(36000)) != SUCCESS) {    // 36000=10h
        goto EXIT_;
    }
EXIT_:
    if (ret != SUCCESS) {
        delete recorder;
        recorder = nullptr;
    }
    return recorder;
}

// TestFrameStateCallback
void TestFrameStateCallback::OnFrameFinished(Camera &camera, FrameConfig &fc, FrameResult &result)
{
    if (fc.GetFrameConfigType() == FRAME_CONFIG_CAPTURE) {
        cout << "Capture frame received." << endl;
        list<Surface *> surfaceList = fc.GetSurfaces();

        for (Surface *surface : surfaceList) {
            SurfaceBuffer *buffer = surface->AcquireBuffer();
            if (buffer != nullptr) {
                char *virtAddr = static_cast<char *>(buffer->GetVirAddr());
                if (virtAddr != nullptr) {
                    SampleSaveCapture(virtAddr, buffer->GetSize(),
                        gPhotoType_, static_cast<const char *>(timeStamp_), sizeof(timeStamp_));
                }
                surface->ReleaseBuffer(buffer);
            } else {
                printf("ERROR:surface buffer is NULL!! \n");
            }
            delete surface;

            gIsFinished_ = true;
        }
        delete &fc;
    } else if (fc.GetFrameConfigType() == FRAME_CONFIG_RECORD || fc.GetFrameConfigType() == FRAME_CONFIG_PREVIEW) {
        delete &fc;
    }
}

void TestFrameStateCallback::SetPhotoType(int type)
{
    gPhotoType_ = type;
    gIsFinished_ = false;
}

bool TestFrameStateCallback::IsFinish(void)
{
    return gIsFinished_;
}

void TestFrameStateCallback::GetVideoName(char *pName, size_t length) const
{
    if (strlen(videoName_) <= 0)
        return;
    if (pName == nullptr || length < strlen(videoName_))
        return;
    if (strcpy_s(pName, length, videoName_) != 0) {
        return;
    }
}

void TestFrameStateCallback::InitTimeStamp()
{
    struct timeval tv;
    struct tm *ltm = nullptr;
    gettimeofday(&tv, nullptr);
    ltm = localtime(&tv.tv_sec);
    if (ltm != nullptr) {
        if (sprintf_s(timeStamp_, sizeof(timeStamp_), "%02d-%02d-%02d-%lld",
            ltm->tm_hour, ltm->tm_min, ltm->tm_sec, tv.tv_usec / MILLI_SECONDS) < 0) {
            return;
        }
    }
}

void TestFrameStateCallback::InitVideoName()
{
    if (sprintf_s(videoName_, sizeof(videoName_), "%s/video%s.jpg", THUMB_PATH, timeStamp_) < 0) {
        return;
    }
}

// SampleCameraStateMng class
SampleCameraStateMng::~SampleCameraStateMng()
{
    if (recorder_ != nullptr) {
        if (gRecordSta_ != MEDIA_STATE_IDLE) {
            recorder_->Stop(false);
        }
        recorder_->Release();
        delete recorder_;
    }
    if (gRecFd_ >= 0) {
        FILE *fp = fdopen(gRecFd_, "w+");
        if (fp) {
            fflush(fp);
            fsync(gRecFd_);
            fclose(fp);
            close(gRecFd_);
        }
    }
    if (fc_) {
        delete fc_;
        fc_ = nullptr;
    }
}

void SampleCameraStateMng::OnCreated(Camera &c)
{
    cout << "Sample recv OnCreate camera." << endl;
    auto config = CameraConfig::CreateCameraConfig();
    if (config == nullptr) {
        cout << "New object failed." << endl;
        return;
    }
    config->SetFrameStateCallback(&fsCb_, &eventHdlr_);
    c.Configure(*config);
    cam_ = &c;
}

void SampleCameraStateMng::StartRecord(Surface *mSurface)
{
    int ret = 0;
    if (gRecordSta_ == MEDIA_STATE_START) {
        return;
    }
    if (recorder_ == nullptr) {
        recorder_ = SampleCreateRecorder(IMAGE_WIDTH, IMAGE_HEIGHT);
    }
    if (recorder_ == nullptr) {
        return;
    }
    if (gRecFd_ > 0) {
        FILE *fp = fdopen(gRecFd_, "w+");
        if (fp) {
            fflush(fp);
            fsync(gRecFd_);
            fclose(fp);
            close(gRecFd_);
        }
        gRecFd_ = -1;
    }

    char vName[MAX_NAME_LEN] = {0};
    fsCb_.GetVideoName(vName, sizeof(vName));
    gRecFd_ = CameraGetRecordFd(static_cast<char *>(vName));
    if (gRecFd_ < 0) {
        return;
    }

    if (gRecordSta_ == MEDIA_STATE_PAUSE) {
        recorder_->SetNextOutputFile(gRecFd_);
        recorder_->Resume();
    } else {
        ret = recorder_->SetOutputFile(gRecFd_);
        if (ret != SUCCESS || recorder_->Prepare() != SUCCESS || recorder_->Start() != SUCCESS) {
            return;
        }
    }

    auto surface = recorder_->GetSurface(0);
    if (surface == nullptr) {
        return;
    }

    surface->SetWidthAndHeight(IMAGE_WIDTH, IMAGE_HEIGHT);
    surface->SetQueueSize(3);    /* queueSize is 3 */
    surface->SetSize(PAGE_SIZE * PAGE_SIZE);

    FrameConfig *fc = new FrameConfig(FRAME_CONFIG_RECORD);
    fc->AddSurface(*surface);
    if (cam_->TriggerLoopingCapture(*fc) != 0) {
        delete fc;
        return;
    }
    gRecordSta_ = MEDIA_STATE_START;
}

void SampleCameraStateMng::StartPreview(Surface *surface)
{
    printf("enter StartPreview ##################################### \n");
    if (cam_ == nullptr) {
        cout << "Camera is not ready." << endl;
        return;
    }
    if (gPreviewSta_ == MEDIA_STATE_START) {
        cout << "Camera is already previewing." << endl;
        return;
    }
    if (surface == nullptr) {
        cout << "surface is NULL." << endl;
        return;
    }

    FrameConfig *fc = new FrameConfig(FRAME_CONFIG_PREVIEW);
    fc->AddSurface(*surface);
    int32_t ret = cam_->TriggerLoopingCapture(*fc);
    if (ret != 0) {
        delete fc;
        cout << "camera start preview failed. ret=" << ret << endl;
        return;
    }

    gPreviewSta_ =  MEDIA_STATE_START;
    cout << "camera start preview succeed." << endl;
}

void SampleCameraStateMng::Capture(int type)
{
    printf("camera start Capture  ##################################### \n");
    if (cam_ == nullptr) {
        cout << "Camera is not ready." << endl;
        return;
    }

    fsCb_.SetPhotoType(type);
    fsCb_.InitTimeStamp();
    fsCb_.InitVideoName();

    FrameConfig *fc = new FrameConfig(FRAME_CONFIG_CAPTURE);
    Surface *surface = Surface::CreateSurface();
    if (surface == nullptr) {
        delete fc;
        return;
    }

    surface->SetWidthAndHeight(IMAGE_WIDTH, IMAGE_HEIGHT);
    fc->AddSurface(*surface);
    printf("Capture before TriggerSingleCapture. ##################################### \n");
    cam_->TriggerSingleCapture(*fc);
    printf("camera start Capture over. ##################################### \n");
}

bool SampleCameraStateMng::IsCaptureOver(void)
{
    return fsCb_.IsFinish();
}

void SampleCameraStateMng::SetPause() { }

void SampleCameraStateMng::SetResume(Surface *mSurface) { }

void SampleCameraStateMng::SetStop(int s)
{
    if (cam_ == nullptr) {
        cout << "Camera is not ready." << endl;
        return;
    }

    cam_->StopLoopingCapture();
    if (gRecordSta_ == MEDIA_STATE_START) {
        if (s) {
            recorder_->Stop(false);
            gRecordSta_ = MEDIA_STATE_IDLE;
        } else {
            recorder_->Pause();
            gRecordSta_ = MEDIA_STATE_PAUSE;
        }
    }

    gPreviewSta_ = MEDIA_STATE_IDLE;
}

bool SampleCameraStateMng::RecordState()
{
    return (gRecordSta_ == MEDIA_STATE_START);
}

bool SampleCameraStateMng::CameraIsReady()
{
    return (cam_ == nullptr) ? false : true;
}

// SampleCameraManager class
SampleCameraManager::~SampleCameraManager()
{
    if (CamStateMng) {
        CamStateMng->SetStop(1);
        delete CamStateMng;
        CamStateMng = NULL;
    }
}

int SampleCameraManager::SampleCameraCreate()
{
    int retval = 0;
    printf("camera start init!!! \n");
    camKit = CameraKit::GetInstance();
    if (camKit == nullptr) {
        cout << "Can not get CameraKit instance" << endl;
        return -1;
    }

    list<string> camList = camKit->GetCameraIds();
    for (auto &cam : camList) {
        camId = cam;
        break;
    }

    if (camId.empty()) {
        cout << "No available camera.(1080p wanted)" << endl;
        printf("No available camera.(1080p wanted)####################### \n");
        return -1;
    }

    CamStateMng = new SampleCameraStateMng(eventHdlr_);
    if (CamStateMng == NULL) {
        printf("create SampleCameraStateMng failed! \n");
        return -1;
    }

    printf("before CreateCamera \n");
    camKit->CreateCamera(camId, *CamStateMng, eventHdlr_);
    printf("after CreateCamera \n");
    if (!access("/userdata/", F_OK | R_OK | W_OK)) {
        if (access(PHOTO_PATH, F_OK) != 0) {
            mkdir(PHOTO_PATH, FILE_MODE);
        }
        if (access(THUMB_PATH, F_OK) != 0) {
            mkdir(THUMB_PATH, FILE_MODE);
        }
        if (access(VIDEO_PATH, F_OK) != 0) {
            mkdir(VIDEO_PATH, FILE_MODE);
        }
    }
    printf("camera init ok! \n");
    return retval;
}

bool SampleCameraManager::SampleCameraExist(void)
{
    return camId.empty() ? false : true;
}

int SampleCameraManager::SampleCameraStart(Surface *surface)
{
    if (CamStateMng == NULL) {
        return -1;
    }
    CamStateMng->StartPreview(surface);
    return 0;
}

int SampleCameraManager::SampleCameraStop(void)
{
    if (CamStateMng == nullptr) {
        return -1;
    }
    CamStateMng->SetStop(1);
    return 0;
}

int SampleCameraManager::SampleCameraCaptrue(int type)
{
    if (CamStateMng == NULL) {
        return -1;
    }
    CamStateMng->Capture(type);
    return 0;
}

int SampleCameraManager::SampleCameraStartRecord(Surface *surface)
{
    if (CamStateMng == NULL) {
        return -1;
    }
    CamStateMng->StartRecord(surface);
    return 0;
}

int SampleCameraManager::SampleCameraPauseRecord(void)
{
    if (CamStateMng == NULL) {
        return -1;
    }
    CamStateMng->SetPause();
    return 0;
}

int SampleCameraManager::SampleCameraResumeRecord(Surface *mSurface)
{
    if (CamStateMng == NULL) {
        return -1;
    }
    CamStateMng->SetResume(mSurface);
    return 0;
}

int SampleCameraManager::SampleCameraStopRecord(void)
{
    if (CamStateMng == NULL) {
        return -1;
    }
    CamStateMng->SetStop(0);
    return 0;
}
bool SampleCameraManager::SampleCameraGetRecord(void)
{
    if (CamStateMng == NULL) {
        return false;
    }
    return CamStateMng->RecordState();
}

bool SampleCameraManager::SampleCameraCaptrueIsFinish(void)
{
    return CamStateMng->IsCaptureOver();
}

bool SampleCameraManager::SampleCameraIsReady(void)
{
    return CamStateMng->CameraIsReady();
}

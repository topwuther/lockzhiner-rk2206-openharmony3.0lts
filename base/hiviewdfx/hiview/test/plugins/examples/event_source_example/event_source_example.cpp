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
#include "event_source_example.h"

#include <fstream>
#include <iostream>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/inotify.h>

#include "event_loop.h"
#include "plugin_factory.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(EventSourceExample);
void EventSourceExample::OnLoad()
{
    printf("EventSourceExample::OnLoad.\n");
    SetName("EventSourceExample");

    std::string folderPath = "/data/log/dropbox";

    int isCreate = ::mkdir(folderPath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    if (!isCreate) {
        printf("create path:%s \n", folderPath.c_str());
    }

    CreateWatchFile("/data/log/dropbox/aaa");
    CreateWatchFile("/data/log/dropbox/bbb");
    CreateWatchFile("/data/log/dropbox/ccc");
}

void EventSourceExample::CreateWatchFile(const std::string& path)
{
    std::ofstream file(path);
    if (!file.good()) {
        printf("Fail to create watch file:%s.\n", path.c_str());
        return;
    }
    file << "" << std::endl;
    file.close();
}

void EventSourceExample::OnUnload()
{
    printf("EventSourceExample::OnUnload.\n");
}

void EventSourceExample::StartEventSource()
{
    printf("EventSourceExample::StartEventSource.\n");
    SetName("EventSourceExample");
    GetWorkLoop()->AddFileDescriptorEventCallback("Dropbox",
        std::static_pointer_cast<EventSourceExample>(shared_from_this()));
}

bool EventSourceExample::OnFileDescriptorEvent(int fd, int type)
{
    printf("EventSourceExample::OnEvent fd:%d, type:%d, inotifyFd_:%d.\n", fd, type, inotifyFd_);
    const int bufSize = 2048;
    char buffer[bufSize] = {0};
    char *offset = nullptr;
    struct inotify_event *event = nullptr;
    if (inotifyFd_ < 0) {
        printf("EventSourceExample Invalid inotify fd:%d", inotifyFd_);
        return false;
    }

    int len = read(inotifyFd_, buffer, bufSize);
    if (len < 0) {
        printf("EventSourceExample failed to read event");
        return false;
    }

    offset = buffer;
    event = (struct inotify_event *)buffer;
    while ((reinterpret_cast<char *>(event) - buffer) < len) {
        for (const auto &it : fileMap_) {
            if (it.second != event->wd) {
                continue;
            }

            if (event->name[event->len - 1] != '\0') {
                event->name[event->len - 1] = '\0';
            }
            std::string filePath = it.first + "/" + std::string(event->name);
            printf("handle file event in %s \n", filePath.c_str());
            CreateAndPublishEvent(filePath);
        }
        int tmpLen = sizeof(struct inotify_event) + event->len;
        event = (struct inotify_event *)(offset + tmpLen);
        offset += tmpLen;
    }
    return true;
}

int32_t EventSourceExample::GetPollFd()
{
    printf("EventSourceExample::GetPollFd.\n");
    if (inotifyFd_ > 0) {
        return inotifyFd_;
    }

    inotifyFd_ = inotify_init();
    if (inotifyFd_ == -1) {
        printf("failed to init inotify: %s.\n", strerror(errno));
        return -1;
    }

    int wd = inotify_add_watch(inotifyFd_, SYSTEM_FAULT_LOG_PATH.c_str(), IN_CLOSE_WRITE | IN_MOVED_TO);
    if (wd < 0) {
        printf("failed to add watch entry : %s.\n", strerror(errno));
        close(inotifyFd_);
        inotifyFd_ = -1;
        return -1;
    }

    printf("GetPollFd %d \n", inotifyFd_);
    fileMap_[SYSTEM_FAULT_LOG_PATH] = wd;
    return inotifyFd_;
}

int32_t EventSourceExample::GetPollType()
{
    printf("EventSourceExample::GetPollType.\n");
    return EPOLLIN;
}

void EventSourceExample::CreateAndPublishEvent(const std::string &file)
{
    // create a pipeline event
    auto event = std::make_shared<EventSourceExampleEvent>(file, static_cast<PipelineEventProducer *>(this));

    // add general information
    const int demoBufSize = 128;
    auto bufPtr = reinterpret_cast<char *>(malloc(demoBufSize));
    if (bufPtr == nullptr) {
        return;
    }
    event->data_ = bufPtr;
    event->addon_ = file;

    // add special information
    event->messageType_ = Event::MessageType::FAULT_EVENT;
    if (file == "/data/log/dropbox/aaa") {
        event->eventId_ = PIPELINE_EVENT_ID_AAA;
    } else if (file == "/data/log/dropbox/bbb") {
        event->eventId_ = PIPELINE_EVENT_ID_BBB;
    } else if (file == "/data/log/dropbox/ccc")  {
        event->eventId_ = PIPELINE_EVENT_ID_CCC;
        event->SetValue("Pipeline", "Repack");
    } else {
        return;
    }
    PublishPipelineEvent(std::dynamic_pointer_cast<PipelineEvent>(event));
}

void EventSourceExample::Recycle(PipelineEvent *event)
{
    printf("EventSourceExample::Recycle.\n");
    auto eventPtr = static_cast<EventSourceExampleEvent *>(event);
    if (eventPtr == nullptr || eventPtr->data_ == nullptr) {
        return;
    }
    free(eventPtr->data_);
    eventPtr->data_ = nullptr;
    printf("Recycle event:%s.\n", eventPtr->addon_.c_str());
}

void EventSourceExample::PauseDispatch(std::weak_ptr<Plugin> plugin)
{
    auto requester = plugin.lock();
    if (requester != nullptr) {
        printf("process pause dispatch event from plugin:%s.\n", requester->GetName().c_str());
    }
}
} // namespace HiviewDFX
} // namespace OHOS

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

#ifndef SYS_EVENT_SERVER_H
#define SYS_EVENT_SERVER_H

#include <memory>
#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class EventReceiver {
public:
    EventReceiver() {};
    virtual ~EventReceiver() {};
    virtual void HandlerEvent(const std::string& rawMsg) = 0;
};

class EventServer {
public:
    EventServer(): isStart_(false), socketId_(-1) {};
    ~EventServer() {}
    void Start();
    void Stop();
    void AddReceiver(std::shared_ptr<EventReceiver> receiver);
private:
    void InitSocket(int &socketId);
    std::vector<std::shared_ptr<EventReceiver>> receivers_;
    bool isStart_;
    int socketId_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // SYS_EVENT_SERVER_H
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

#include "event_processor_example2.h"

#include "event_source_example.h"
#include "pipeline.h"
#include "plugin_factory.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(EventProcessorExample2)
bool EventProcessorExample2::CanProcessEvent(std::shared_ptr<Event> event)
{
    return false;
}

bool EventProcessorExample2::OnEvent(std::shared_ptr<Event>& event)
{
    printf("EventProcessorExample2 OnEvent \n");
    if (event->eventId_ == EventSourceExample::PIPELINE_EVENT_ID_BBB) {
        printf("EventProcessorExample2 process bbb event.\n");
    }

    if (event->eventId_ == EventSourceExample::PIPELINE_EVENT_ID_AAA) {
        printf("EventProcessorExample2 process aaa event.\n");
    }

    if (event->GetValue("Finish") == "EventProcessorExample2") {
        printf("EventProcessorExample2 OnFinish.\n");
        event->OnFinish();
    }

    if (event->eventId_ == EventSourceExample::PIPELINE_EVENT_ID_CCC) {
        auto repackEvent = PipelineEvent::RepackPipelineEvent<Event, PipelineEvent>(shared_from_this(),
                                                                                    "NormalPipeline", event, false);
        repackEvent->eventId_ = EventSourceExample::PIPELINE_EVENT_ID_AAA;
        repackEvent->SetValue("Unload", "True");
        printf("EventProcessorExample2 repack ccc event.\n");
        repackEvent->OnContinue();
        return true;
    }

    event->SetValue("EventProcessorExample2", "Done");
    return true;
}
void EventProcessorExample2::OnLoad()
{
    SetName("EventProcessorExample2");
    SetVersion("EventProcessorExample2.0");
    printf("EventProcessorExample2 OnLoad \n");
}

void EventProcessorExample2::OnUnload()
{
    printf("EventProcessorExample2 OnUnload \n");
}
} // namespace HiviewDFX
} // namespace OHOS
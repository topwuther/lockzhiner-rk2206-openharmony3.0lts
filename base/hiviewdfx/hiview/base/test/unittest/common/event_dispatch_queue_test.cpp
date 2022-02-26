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

#include "event_dispatch_queue_test.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

void EventDispatchQueueTest::SetUp()
{
    /**
     * @tc.setup: create order and unordered event dispatch queue
     */
    printf("SetUp.\n");
    if (order_ == nullptr) {
        order_ = std::make_shared<EventDispatchQueue>("disp-order", Event::ManageType::ORDERED);
        order_->Start();
    }

    if (unorder_ == nullptr) {
        unorder_ = std::make_shared<EventDispatchQueue>("disp-unorder", Event::ManageType::UNORDERED);
        unorder_->Start();
    }
}

void EventDispatchQueueTest::TearDown()
{
    /**
     * @tc.teardown: destroy the event dispatch queue we have created
     */
    printf("TearDown.\n");
    if (order_ != nullptr) {
        order_->Stop();
        order_ = nullptr;
    }

    if (unorder_ != nullptr) {
        unorder_->Stop();
        unorder_ = nullptr;
    }
}

std::shared_ptr<Event> EventDispatchQueueTest::CreateEvent(const std::string& name, int32_t id,
                                                           const std::string& message, Event::MessageType type)
{
    auto event = std::make_shared<Event>(name);
    event->messageType_ = type;
    event->eventId_ = id;
    event->SetValue("message", message);
    return event;
}

bool ExtendEventListener::OnOrderedEvent(Event& msg)
{
    printf("cur listener:%s OnOrderedEvent eventId_:%u \n", name_.c_str(), msg.eventId_);
    orderEventCount_++;
    auto message = msg.GetValue("message");
    processedOrderedEvents_[message] = msg.sender_;
    if (msg.GetValue("Finish") == name_) {
        return true;
    }
    return false;
}

void ExtendEventListener::OnUnorderedEvent(const Event& msg)
{
    printf("cur listener:%s OnUnorderedEvent eventId_:%u \n", name_.c_str(), msg.eventId_);
    unorderEventCount_++;
    auto message = msg.GetValue("message");
    processedUnorderedEvents_[message] = msg.sender_;
}

std::string ExtendEventListener::GetListenerName()
{
    return name_;
}

/**
 * @tc.name: EventDispatchQueueCreateTest001
 * @tc.desc: create and init an event dispatch queue
 * @tc.type: FUNC
 * @tc.require: AR000DPTSU
 */
HWTEST_F(EventDispatchQueueTest, EventDispatchQueueCreateTest001, TestSize.Level3)
{
    printf("EventDispatchQueueTest.\n");
    auto orderQueue = std::make_shared<EventDispatchQueue>("test", Event::ManageType::ORDERED);
    ASSERT_EQ(false, orderQueue->IsRunning());
    ASSERT_EQ(0, orderQueue->GetWaitQueueSize());
    orderQueue->Start();
    sleep(1);
    ASSERT_EQ(true, orderQueue->IsRunning());
    orderQueue->Stop();
    ASSERT_EQ(false, orderQueue->IsRunning());
    auto unorderQueue = std::make_shared<EventDispatchQueue>("test1", Event::ManageType::UNORDERED);
    ASSERT_EQ(false, unorderQueue->IsRunning());
    unorderQueue->Start();
    sleep(1);
    ASSERT_EQ(true, unorderQueue->IsRunning());
    unorderQueue->Stop();
    ASSERT_EQ(false, unorderQueue->IsRunning());
}

/**
 * @tc.name: UnorderEventDispatchTest001
 * @tc.desc: create event and send it to a unorder dispatch queue
 * @tc.type: FUNC
 * @tc.require: AR000DPTSU
 */
HWTEST_F(EventDispatchQueueTest, UnorderEventDispatchTest001, TestSize.Level3)
{
    printf("EventDispatchQueueTest.\n");
    if (unorder_ == nullptr) {
        FAIL();
    }
    auto listener1 = std::make_shared<ExtendEventListener>("listener1");
    listener1->AddListenerInfo(Event::MessageType::RAW_EVENT, EventListener::EventIdRange(EVENT_ID_0, EVENT_ID_2));
    unorder_->RegisterListener(listener1);
    auto listener2 = std::make_shared<ExtendEventListener>("listener2");
    listener2->AddListenerInfo(Event::MessageType::RAW_EVENT, EVENT_ID_2);
    unorder_->RegisterListener(listener2);
    auto event1 = CreateEvent("testEvent1", EVENT_ID_0, "test", Event::MessageType::RAW_EVENT);
    unorder_->Enqueue(event1);
    sleep(1);
    ASSERT_EQ(listener1->unorderEventCount_, 1ul);
    ASSERT_EQ(listener2->unorderEventCount_, 0ul);
    auto event2 = CreateEvent("testEvent1", EVENT_ID_2, "test", Event::MessageType::RAW_EVENT);
    unorder_->Enqueue(event2);
    sleep(1);
    ASSERT_EQ(listener1->unorderEventCount_, 2ul);
    ASSERT_EQ(listener2->unorderEventCount_, 1ul);
    auto event3 = CreateEvent("testEvent1", EVENT_ID_3, "test", Event::MessageType::RAW_EVENT);
    unorder_->Enqueue(event3);
    sleep(1);
    ASSERT_EQ(listener1->unorderEventCount_, 2ul);
    ASSERT_EQ(listener2->unorderEventCount_, 1ul);
    auto event4 = CreateEvent("testEvent1", EVENT_ID_3, "test", Event::MessageType::FAULT_EVENT);
    unorder_->Enqueue(event4);
    sleep(1);
    ASSERT_EQ(listener1->unorderEventCount_, 2ul);
    ASSERT_EQ(listener2->unorderEventCount_, 1ul);
}

/**
 * @tc.name: OrderEventDispatchTest001
 * @tc.desc: create event and send it to a order dispatch queue
 * @tc.type: FUNC
 * @tc.require: AR000DPTSU
 */
HWTEST_F(EventDispatchQueueTest, OrderEventDispatchTest001, TestSize.Level3)
{
    printf("EventDispatchQueueTest.\n");
    if (order_ == nullptr) {
        FAIL();
    }

    auto listener1 = std::make_shared<ExtendEventListener>("listener1");
    listener1->AddListenerInfo(Event::MessageType::RAW_EVENT, EventListener::EventIdRange(EVENT_ID_0, EVENT_ID_2));
    order_->RegisterListener(listener1);
    auto listener2 = std::make_shared<ExtendEventListener>("listener2");
    listener2->AddListenerInfo(Event::MessageType::RAW_EVENT, EVENT_ID_2);
    order_->RegisterListener(listener2);
    auto event1 = CreateEvent("", EVENT_ID_0, "test", Event::MessageType::RAW_EVENT);
    order_->Enqueue(event1);
    sleep(1);
    ASSERT_EQ(listener1->orderEventCount_, 1ul);
    ASSERT_EQ(listener2->orderEventCount_, 0ul);
    auto listener3 = std::make_shared<ExtendEventListener>("listener3");
    listener3->AddListenerInfo(Event::MessageType::RAW_EVENT, EVENT_ID_2);
    order_->RegisterListener(listener3);
    auto listener4 = std::make_shared<ExtendEventListener>("listener4");
    listener4->AddListenerInfo(Event::MessageType::RAW_EVENT, EVENT_ID_2);
    order_->RegisterListener(listener4);
    auto event2 = CreateEvent("listener1", EVENT_ID_2, "test", Event::MessageType::RAW_EVENT);
    order_->Enqueue(event2);
    sleep(1);
    ASSERT_EQ(listener1->orderEventCount_, 1ul);
    ASSERT_EQ(listener2->orderEventCount_, 1ul);
    ASSERT_EQ(listener3->orderEventCount_, 1ul);
    ASSERT_EQ(listener4->orderEventCount_, 1ul);

    auto event3 = CreateEvent("listener1", EVENT_ID_2, "test", Event::MessageType::RAW_EVENT);
    event3->SetValue("Finish", "listener2");
    order_->Enqueue(event3);
    sleep(1);
    ASSERT_EQ(listener1->orderEventCount_, 1ul);
    ASSERT_EQ(listener2->orderEventCount_, 2ul);
    ASSERT_EQ(listener3->orderEventCount_, 1ul);
    ASSERT_EQ(listener4->orderEventCount_, 1ul);

    auto event4 = CreateEvent("listener3", EVENT_ID_2, "test", Event::MessageType::RAW_EVENT);
    order_->Enqueue(event4);
    sleep(1);
    ASSERT_EQ(listener1->orderEventCount_, 1ul);
    ASSERT_EQ(listener2->orderEventCount_, 2ul);
    ASSERT_EQ(listener3->orderEventCount_, 1ul);
    ASSERT_EQ(listener4->orderEventCount_, 2ul);
}

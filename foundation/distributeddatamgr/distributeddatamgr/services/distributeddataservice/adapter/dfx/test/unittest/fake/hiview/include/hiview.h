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

#ifndef HIVIEW_HIVIEW_H
#define HIVIEW_HIVIEW_H

#include <mutex>
#include <iostream>

#include "hievent.h"

class HiView {
public:

    /**
     * A abstract class for which classes can be report by HiView.
     */
    class Reportable {
    public:
        virtual ~Reportable() {}
        virtual HiEvent& ToHiEvent() = 0;
    };

    /**
     * Report a HiEvent.
     * @param event reference of l-value.
     */
    static void Report(HiEvent& event)
    {
    }

    /**
     * Report a HiEvent.
     * @param event reference of r-value.
     */
    static void Report(HiEvent&& event);

    /**
     * Report a Reportable.
     * @param reportable self-defined class which implement Reportable.
     */
    static void Report(HiView::Reportable& reportable);

    /**
     * Create a HiEvent by single bool value.
     * @param eventID ID of event.
     * @param key single payload's key.
     * @param value single payload's value.
     *
     * @return HiEvent
     */
    static HiEvent ByPair(int eventID, const std::string& key, bool value);

    /**
     * Create a HiEvent by single byte value.
     * @param eventID ID of event.
     * @param key single payload's key.
     * @param value single payload's value.
     *
     * @return HiEvent
     */
    static HiEvent ByPair(int eventID, const std::string& key, unsigned char value);

    /**
     * Create a HiEvent by single byte value.
     * @param eventID ID of event.
     * @param key single payload's key.
     * @param value single payload's value.
     *
     * @return HiEvent
     */
    static HiEvent ByPair(int eventID, const std::string& key, int value);

    /**
     * Create a HiEvent by single byte value.
     * @param eventID ID of event.
     * @param key single payload's key.
     * @param value single payload's value.
     *
     * @return HiEvent
     */
    static HiEvent ByPair(int eventID, const std::string& key, long value);

    /**
     * Create a HiEvent by single byte value.
     * @param eventID ID of event.
     * @param key single payload's key.
     * @param value single payload's value.
     *
     * @return HiEvent
     */
    static HiEvent ByPair(int eventID, const std::string& key, float value);

    /**
     * Create a HiEvent by single byte value.
     * @param eventID ID of event.
     * @param key single payload's key.
     * @param value single payload's value.
     *
     * @return HiEvent
     */
    static HiEvent ByPair(int eventID, const std::string& key, const std::string& value);

    /**
     * Create a HiEvent by json formatten string.
     * @param eventID ID of event.
     * @param key single payload's key.
     * @param value json formatten string.
     *
     * @return HiEvent
     */
    static HiEvent ByJson(int eventID, const std::string& json);
private:
    static void ReportImpl(HiEvent&& event);
};

#endif

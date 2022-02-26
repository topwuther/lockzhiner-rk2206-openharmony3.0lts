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

#ifndef HIVIEW_HIEVENT_H
#define HIVIEW_HIEVENT_H

#include <string>
#include <mutex>
#include <set>
#include <any>
#include <map>

#include "fake_hiview.h"
/**
 * HiEvent is common event that can be uploaded to cloud by hiview.
 */
class HiEvent {
public:
    explicit HiEvent(int id)
    {
        id_ = id;
    }

    virtual ~HiEvent() {};

    HiEvent(const HiEvent& other)
    {
    }

    HiEvent& operator=(const HiEvent& other)
    {
        return *this;
    }

    HiEvent(HiEvent&& other) {};
    HiEvent& operator=(HiEvent&& other)
    {
        return *this;
    }

    /**
     * Max length of payload's key.
     */
    static const size_t MAX_KEY_LEN = 32;

    /**
     * Max length of payload's value which type is string.
     */
    static const size_t MAX_VALUE_LEN = 128;

    /**
     * Max number of payload's <k,v > pairs.
     */
    static const size_t MAX_PAIR_NUM = 256;

    /**
     * Max size of payload's array.
     */
    static const size_t MAX_ARR_SIZE = 100;

    /**
     * Max length of payload's path of file.
     */
    static const size_t MAX_PATH_LEN = 256;

    /**
     * Max number of payload's file.
     */
    static const size_t MAX_FILE_NUM = 10;

    /**
     * Put bool or array of bool as payload of this Hievent.
     */
    HiEvent& PutBool(const std::string& key, bool value)
    {
        FakeHivew::Put(key, value);
        return *this;
    }
    HiEvent& PutBoolArray(const std::string& key, const std::vector<bool>& value);

    /**
     * Put byte or array of byte as payload of this Hievent.
     */
    HiEvent& PutByte(const std::string& key, unsigned char value)
    {
        FakeHivew::Put(key, value);
        return *this;
    }
    HiEvent& PutByteArray(const std::string& key, const std::vector<unsigned char>& value);

    /**
     * Put short or array of short as payload of this Hievent.
     */
    HiEvent& PutShort(const std::string& key, short value)
    {
        FakeHivew::Put(key, value);
        return *this;
    }

    HiEvent& PutShortArray(const std::string& key, const std::vector<short>& value);

    /**
     * Put int or array of int as payload of this Hievent.
     */
    HiEvent& PutInt(const std::string& key, int value)
    {
        FakeHivew::Put(key, value);
        return *this;
    }
    HiEvent& PutIntArray(const std::string& key, const std::vector<int>& value);

    /**
     * Put long or array of long as payload of this Hievent.
     */
    HiEvent& PutLong(const std::string& key, long value)
    {
        FakeHivew::Put(key, value);
        return *this;
    }
    HiEvent& PutLongArray(const std::string& key, const std::vector<long>& value);

    /**
     * Put float or array of float as payload of this Hievent.
     */
    HiEvent& PutFloat(const std::string& key, float value)
    {
        FakeHivew::Put(key, value);
        return *this;
    }
    HiEvent& PutFloatArray(const std::string& key, const std::vector<float>& value);

    /**
     * Put string or array of string as payload of this Hievent.
     */
    HiEvent& PutString(const std::string& key, const std::string& value)
    {
        FakeHivew::Put(key, value);
        return *this;
    }
    HiEvent& PutStringArray(const std::string& key, const std::vector<std::string>& value);

    /**
     * Put sub HiEvent or array of HiEvent as payload of this Hievent.
     */
    HiEvent& PutHiEvent(const std::string& key, HiEvent& value);
    HiEvent& PutHiEventArray(const std::string& key, const std::vector<HiEvent>& value)
    {
        return *this;
    }

    /**
     * Add file path to HiEvent.
     */
    HiEvent& AddFilePath(const std::string& path);

    /**
     * Reset the HiEvent.
     */
    HiEvent& Reset();

    int GetId() const;
    long GetTime() const;

    /**
     * Flatten the HiEvent as a string.
     */
    const std::string& Flatten();

    /**
     * Set happen time of the HiEvent, if not, default one will be set.
     */
    HiEvent& SetTime(long second);

private:
    int id_{};
};

#endif

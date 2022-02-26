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

#ifndef HI_SYS_EVENT_H
#define HI_SYS_EVENT_H
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class HiSysEvent {
public:
    // system event domain list
    class Domain {
    public:
        static constexpr char AAFWK[] = "AAFWK";
        static constexpr char APPEXECFWK[] = "APPEXECFWK";
        static constexpr char ACCOUNT[] = "ACCOUNT";
        static constexpr char ACE[] = "ACE";
        static constexpr char AI[] = "AI";
        static constexpr char BARRIER_FREE[] = "BARRIERFREE";
        static constexpr char BIOMETRICS[] = "BIOMETRICS";
        static constexpr char CCRUNTIME[] = "CCRUNTIME";
        static constexpr char COMMUNICATION[] = "COMMUNICATION";
        static constexpr char DEVELOPTOOLS[] = "DEVELOPTOOLS";
        static constexpr char DISTRIBUTED_DATAMGR[] = "DISTDATAMGR";
        static constexpr char DISTRIBUTED_SCHEDULE[] = "DISTSCHEDULE";
        static constexpr char GLOBAL[] = "GLOBAL";
        static constexpr char GRAPHIC[] = "GRAPHIC";
        static constexpr char HIVIEWDFX[] = "HIVIEWDFX";
        static constexpr char IAWARE[] = "IAWARE";
        static constexpr char INTELLI_ACCESSORIES[] = "INTELLIACC";
        static constexpr char INTELLI_TV[] = "INTELLITV";
        static constexpr char IVI_HARDWARE[] = "IVIHARDWARE";
        static constexpr char LOCATION[] = "LOCATION";
        static constexpr char MSDP[] = "MSDP";
        static constexpr char MULTI_MEDIA[] = "MULTIMEDIA";
        static constexpr char MULTI_MODAL_INPUT[] = "MULTIMODALINPUT";
        static constexpr char NOTIFICATION[] = "NOTIFICATION";
        static constexpr char POWERMGR[] = "POWERMGR";
        static constexpr char ROUTER[] = "ROUTER";
        static constexpr char SECURITY[] = "SECURITY";
        static constexpr char SENSORS[] = "SENSORS";
        static constexpr char SOURCE_CODE_TRANSFORMER[] = "SRCTRANSFORMER";
        static constexpr char STARTUP[] = "STARTUP";
        static constexpr char TELEPHONY[] = "TELEPHONY";
        static constexpr char UPDATE[] = "UPDATE";
        static constexpr char USB[] = "USB";
        static constexpr char WEARABLE_HARDWARE[] = "WEARABLEHW";
        static constexpr char WEARABLE[] = "WEARABLE";
        static constexpr char OTHERS[] = "OTHERS";
    };

public:
    enum EventType {
        FAULT     = 1,    // system fault event
        STATISTIC = 2,    // system statistic event
        SECURITY  = 3,    // system security event
        BEHAVIOR  = 4     // system behavior event
    };

    /**
     * @brief write system event
     * @param domain    system event domain name
     * @param eventName system event name
     * @param type      system event type
     * @param keyValues system event parameter name or value
     * @return 0 success, other fail
     */
    template<typename... Types> static int Write(const std::string &domain, const std::string &eventName,
        EventType type, Types... keyValues)
    {
        EventBase eventBase(domain, eventName, type);
        eventBase.jsonStr_ << "{";
        WritebaseInfo(eventBase);
        if (IsError(eventBase)) {
            ExplainRetCode(eventBase);
            return eventBase.retCode_;
        }

        InnerWrite(eventBase, keyValues...);
        if (IsError(eventBase)) {
            ExplainRetCode(eventBase);
            return eventBase.retCode_;
        }
        eventBase.jsonStr_ << "}";

        if (IsWarnAndUpdate(SendSysEvent(eventBase), eventBase)) {
            ExplainRetCode(eventBase);
            return eventBase.retCode_;
        }
        ExplainRetCode(eventBase);
        return eventBase.retCode_;
    }

private:
    class EventBase {
    public:
        EventBase(const std::string &domain, const std::string &eventName, const EventType &type)
            : retCode_(0), keyCnt_(0), domain_(domain), eventName_(eventName), type_(type)
            {};
        ~EventBase() {}
    public:
        int retCode_;
        unsigned int keyCnt_;
        std::stringstream jsonStr_;
        const std::string &domain_;
        const std::string &eventName_;
        const EventType &type_;
    };
private:
    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, bool value, Types... keyValues)
    {
        AppendData<bool>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const char value, Types... keyValues)
    {
        AppendData<short>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const unsigned char value, Types... keyValues)
    {
        AppendData<unsigned short>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const short value, Types... keyValues)
    {
        AppendData<short>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const unsigned short value, Types... keyValues)
    {
        AppendData<unsigned short>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const int value, Types... keyValues)
    {
        AppendData<int>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const unsigned int value, Types... keyValues)
    {
        AppendData<unsigned int>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const long value, Types... keyValues)
    {
        AppendData<long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const unsigned long value, Types... keyValues)
    {
        AppendData<unsigned long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const long long value, Types... keyValues)
    {
        AppendData<long long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const unsigned long long value, Types... keyValues)
    {
        AppendData<unsigned long long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const float value, Types... keyValues)
    {
        AppendData<float>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const double value, Types... keyValues)
    {
        AppendData<double>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const std::string &value, Types... keyValues)
    {
        AppendData(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const char *value, Types... keyValues)
    {
        AppendData(eventBase, key, std::string(value));
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<bool> &value, Types... keyValues)
    {
        AppendArrayData<bool>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<char> &value, Types... keyValues)
    {
        AppendArrayData(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<unsigned char> &value, Types... keyValues)
    {
        AppendArrayData(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<short> &value, Types... keyValues)
    {
        AppendArrayData<short>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<unsigned short> &value, Types... keyValues)
    {
        AppendArrayData<unsigned short>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<int> &value, Types... keyValues)
    {
        AppendArrayData<int>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<unsigned int> &value, Types... keyValues)
    {
        AppendArrayData<unsigned int>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<long> &value, Types... keyValues)
    {
        AppendArrayData<long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<unsigned long> &value, Types... keyValues)
    {
        AppendArrayData<unsigned long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<long long> &value, Types... keyValues)
    {
        AppendArrayData<long long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<unsigned long long> &value, Types... keyValues)
    {
        AppendArrayData<unsigned long long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<float> &value, Types... keyValues)
    {
        AppendArrayData<float>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<double> &value, Types... keyValues)
    {
        AppendArrayData<double>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<std::string> &value, Types... keyValues)
    {
        AppendArrayData(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename T>
    static void AppendData(EventBase &eventBase, const std::string &key, T value, bool isDefKey = true)
    {
        if (IsWarnAndUpdate(CheckKey(key), eventBase)) {
            return;
        }
        if (UpdateAndCheckKeyNumIsOver(eventBase, isDefKey)) {
            return;
        }
        eventBase.jsonStr_ << "\"" << key << "\":" << value << ",";
    }

    template<typename T>
    static void AppendArrayData(EventBase &eventBase, const std::string &key, const std::vector<T> &value,
        bool isDefKey = true)
    {
        if (IsWarnAndUpdate(CheckKey(key), eventBase)) {
            return;
        }

        if (UpdateAndCheckKeyNumIsOver(eventBase, isDefKey)) {
            return;
        }

        if (value.empty()) {
            eventBase.jsonStr_ << "\"" << key << "\":[]";
            return;
        }

        IsWarnAndUpdate(CheckArraySize(value.size()), eventBase);

        unsigned int index = 0;
        unsigned int arrayMax = GetArrayMax();
        eventBase.jsonStr_ << "\"" << key << "\":[";
        for (auto item = value.begin(); item != value.end(); item++) {
            index++;
            if (index > arrayMax) {
                break;
            }
            eventBase.jsonStr_ << (*item) << ",";
        }
        if (eventBase.jsonStr_.tellp() != 0) {
            eventBase.jsonStr_.seekp(-1, std::ios_base::end);
        }
        eventBase.jsonStr_ << "],";
    }

    static void AppendArrayData(EventBase &eventBase,
        const std::string &key, const std::vector<char> &value, bool isDefKey = true);
    static void AppendArrayData(EventBase &eventBase,
        const std::string &key, const std::vector<unsigned char> &value, bool isDefKey = true);
    static void AppendArrayData(EventBase &eventBase,
        const std::string &key, const std::vector<std::string> &value, bool isDefKey = true);
    static void AppendData(EventBase &eventBase,
        const std::string &key, const std::string &value, bool isDefKey = true);
    static void AppendHexData(EventBase &eventBase, const std::string &key, uint64_t value);
    static int CheckArraySize(unsigned long size);
    static int CheckDomain(EventBase &eventBase);
    static int CheckEventName(EventBase &eventBase);
    static int CheckKey(const std::string &key);
    static int CheckValue(const std::string &value);
    static void ExplainRetCode(EventBase &eventBase);
    static void InnerWrite(EventBase &eventBase);
    static bool IsError(EventBase &eventBase);
    static bool IsErrorAndUpdate(int retCode, EventBase &eventBase);
    static bool IsWarnAndUpdate(int retCode, EventBase &eventBase);
    static int SendSysEvent(EventBase &eventBase);
    static int WritebaseInfo(EventBase &eventBase);
    static bool UpdateAndCheckKeyNumIsOver(EventBase &eventBase, bool isDefKey);
    static unsigned int GetArrayMax();
}; // HiSysEvent
} // HiviewDFX
} // OHOS
#endif // HI_SYS_EVENT_H
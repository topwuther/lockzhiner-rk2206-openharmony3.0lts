/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_TIME_SERVICE_INTERFACE_H
#define SERVICES_INCLUDE_TIME_SERVICE_INTERFACE_H

#include "iremote_broker.h"
#include "want_agent.h"

namespace OHOS {
namespace MiscServices {
class ITimeService : public IRemoteBroker {
public:
    // remote method code
    enum {
        SET_TIME = 0,
        SET_TIME_ZONE = 1,
        GET_TIME_ZONE = 2,
        GET_WALL_TIME_MILLI = 3,
        GET_WALL_TIME_NANO = 4,
        GET_BOOT_TIME_MILLI = 5,
        GET_BOOT_TIME_NANO = 6,
        GET_MONO_TIME_MILLI = 7,
        GET_MONO_TIME_NANO = 8,
        GET_THREAD_TIME_MILLI = 9,
        GET_THREAD_TIME_NANO = 10,
        CREATE_TIMER = 11,
        START_TIMER = 12,
        STOP_TIMER = 13,
        DESTORY_TIMER = 14
    };
      /**
     * SetTime
     *
     * @param time int64_t set milliseconds
     * @return int32_t ERR_OK on success, other on failure.
     */
    virtual int32_t SetTime(const int64_t time) = 0;

    /**
     * SetTimeZone
     *
     * @param timezoneId std::string &timezoneId string
     * @return int32_t ERR_OK on success, other on failure.
     */
    virtual int32_t SetTimeZone(const std::string timezoneId) = 0;
    
    /**
     * GetTimeZone
     *
     * @param timezoneId std::string &timezoneId string
     * @return int32_t ERR_OK on success, other on failure.
     */
    virtual int32_t GetTimeZone(std::string &timezoneId) = 0;
    
     /**
     * GetWallTimeMs
     *
     * @param times result of times ,unit: millisecond
     * @return int32_t ERR_OK on success, other on failure.
     */
    virtual int32_t GetWallTimeMs(int64_t &times) = 0;

     /**
     * GetWallTimeNs
     *
     * @param times result of times ,unit: Nanosecond
     * @return int32_t ERR_OK on success, other on failure.
     */
    virtual int32_t GetWallTimeNs(int64_t &times) = 0;

     /**
     * GetBootTimeMs
     *
     * @param times result of times ,unit: millisecond
     * @return int32_t ERR_OK on success, other on failure.
     */
    virtual int32_t GetBootTimeMs(int64_t &times) = 0;

     /**
     * GetBootTimeNs
     *
     * @param times result of times ,unit: millisecond
     * @return int32_t ERR_OK on success, other on failure.
     */
    virtual int32_t GetBootTimeNs(int64_t &times) = 0;

     /**
     * GetMonotonicTimeMs
     *
     * @param times result of times ,unit: millisecond
     * @return int32_t ERR_OK on success, other on failure.
     */
    virtual int32_t GetMonotonicTimeMs(int64_t &times) = 0;

     /**
     * GetMonotonicTimeNs
     *
     * @param times result of times ,unit: Nanosecond
     * @return int32_t ERR_OK on success, other on failure.
     */
    virtual int32_t GetMonotonicTimeNs(int64_t &times) = 0;

     /**
     * GetThreadTimeMs
     *
     * @param times result of times ,unit: millisecond
     * @return int32_t ERR_OK on success, other on failure.
     */
    virtual int32_t GetThreadTimeMs(int64_t &times) = 0;

     /**
     * GetThreadTimeNs
     * 
     * @param times result of times ,unit: Nanosecond
     * @return int32_t ERR_OK on success, other on failure.
     */
    virtual int32_t GetThreadTimeNs(int64_t &times) = 0;
    
    /**
     * CreateTimer
     * 
     * @param type    timer type
     * @param repeat  is repeat or not
     * @param timerCallback remoteobject
     * @return uint64_t > 0 on success, == 0 failure.
     */
    virtual uint64_t CreateTimer(int32_t type, bool repeat, uint64_t interval, sptr<IRemoteObject> &timerCallback) = 0;

     /**
     * StartTimer
     * 
     * @param timerId indicate timerId
     * @param treggerTime  trigger times
     * @return bool true on success, false on failure.
     */
    virtual bool StartTimer(uint64_t timerId, uint64_t treggerTime) = 0;

     /**
     * StopTimer
     * 
     * @param timerId indicate timerId
     * @return bool true on success, false on failure.
     */
    virtual bool StopTimer(uint64_t  timerId) = 0;

     /**
     * DestroyTimer
     * 
     * @param timerId indicate timerId
     * @return bool true on success, false on failure.
     */
    virtual bool DestroyTimer(uint64_t  timerId) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.time.ITimeService");
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_TIME_SERVICE_INTERFACE_H
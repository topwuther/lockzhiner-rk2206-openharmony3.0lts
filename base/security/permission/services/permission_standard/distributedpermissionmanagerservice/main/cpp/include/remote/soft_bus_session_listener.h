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

#ifndef SOFT_BUS_SESSION_LISTENER_H
#define SOFT_BUS_SESSION_LISTENER_H

#include <string>
#include <map>
#include <mutex>
#include "session.h"

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
class SoftBusSessionListener final {
public:
    /**
     * @brief Called when a session is opened.
     *
     * This function can be used to verify the session or initialize resources related to the session.
     *
     * @param sessionId Indicates the session ID.
     * @param result 0 if the session is opened successfully, returns an error code otherwise.
     * @return Returns <b>0</b> if the session connection is accepted; returns a non-zero value
     * otherwise (you do not need to call {@link CloseSession} to close the session).
     * @since 1.0
     * @version 1.0
     */
    static int32_t OnSessionOpened(int32_t sessionId, int32_t result);

    /**
     * @brief Called when a session is closed.
     *
     * This function can be used to release resources related to the session.
     * You do not need to call {@link CloseSession}.
     *
     * @param sessionId Indicates the session ID.
     * @since 1.0
     * @version 1.0
     */
    static void OnSessionClosed(int32_t sessionId);

    /**
     * @brief Called when data is received.
     *
     * This function is used to notify that data is received.
     *
     * @param sessionId Indicates the session ID.
     * @param data Indicates the pointer to the data received.
     * @param dataLen Indicates the length of the data received.
     * @since 1.0
     * @version 1.0
     */
    static void OnMessageReceived(int32_t sessionId, const void *data, uint32_t dataLen);

    /**
     * @brief Called when message is received.
     *
     * This function is used to notify that message is received.
     *
     * @param sessionId Indicates the session ID.
     * @param data Indicates the pointer to the message data received.
     * @param dataLen Indicates the length of the message received.
     * @since 1.0
     * @version 1.0
     */
    static void OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen);

    /**
     * @brief get the state of a session.
     *
     * This function is used to verify that session is opened.
     *
     * @param sessionId Indicates the session ID.
     * @return -2: sessionId not used, -1: session is in opening, greater than 0: session is opened.
     * @since 1.0
     * @version 1.0
     */
    static int64_t GetSessionState(int32_t sessionId);

    static const int64_t STATE_OPENING = -1;
    static const int64_t STATE_NOTFOUND = -2;

private:
    /**
     * key: sessionId, value: status.
     * status: -1: opening, >0: opened timestamp
     */
    static std::map<int32_t, int64_t> g_SessionOpenedMap_;
    /**
     * mutex for map
     */
    static std::mutex g_SessionMutex_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // SOFT_BUS_SESSION_LISTENER_H

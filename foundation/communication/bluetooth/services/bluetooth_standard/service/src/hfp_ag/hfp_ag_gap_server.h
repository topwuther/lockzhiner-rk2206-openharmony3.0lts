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

#ifndef HFP_AG_GAP_SERVER_H
#define HFP_AG_GAP_SERVER_H

#include <cstdint>

#include "base_def.h"
#include "gap_if.h"

namespace bluetooth {
/**
 * @brief Class for HFP AG GAP server.
 */
class HfpAgGapServer {
public:
    /**
     * @brief Register Service Security.
     *
     * @param scn Server channel number.
     * @return Returns the error code of register service security.
     */
    static int RegisterServiceSecurity(uint8_t scn);

    /**
     * @brief Deregister Service Security
     *
     * @return Returns the error code of deregister service security.
     */
    static int DeregisterServiceSecurity();

private:
    /**
     * @brief Construct a new HfpAgGapServer object.
     */
    HfpAgGapServer() = delete;

    /**
     * @brief Destroy the HfpAgGapServer object.
     */
    ~HfpAgGapServer() = delete;

    static GapServiceSecurityInfo g_securityInfo;
    DISALLOW_COPY_AND_ASSIGN(HfpAgGapServer);
};
}  // namespace bluetooth
#endif // HFP_AG_GAP_SERVER_H

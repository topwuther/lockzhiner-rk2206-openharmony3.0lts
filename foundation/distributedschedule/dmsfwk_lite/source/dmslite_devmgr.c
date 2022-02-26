/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#include "dmslite_devmgr.h"

#include "dmslite_session.h"
#include "securec.h"
#include "softbus_bus_center.h"

#define DMSLITE_BUNDLE_NAME "dmslite"

static char g_peerDevId[NETWORK_ID_BUF_LEN] = {0};

static void onNodeOnline(NodeBasicInfo *info);
static void onNodeOffline(NodeBasicInfo *info);
static void onNodeBasicInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info);

static INodeStateCb g_networkListener = {
    .events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE,
    .onNodeOnline = onNodeOnline,
    .onNodeOffline = onNodeOffline,
    .onNodeBasicInfoChanged = onNodeBasicInfoChanged
};

void onNodeOnline(NodeBasicInfo *info)
{
    if (info == NULL) {
        return;
    }
    (void)strncpy_s(g_peerDevId, NETWORK_ID_BUF_LEN, info->networkId, sizeof(info->networkId));
    CreateDMSSessionServer();
}

void onNodeOffline(NodeBasicInfo *info)
{
    if (info == NULL) {
        return;
    }
    (void)memset_s(g_peerDevId, NETWORK_ID_BUF_LEN, 0x00, NETWORK_ID_BUF_LEN);
    CloseDMSSessionServer();
}

void onNodeBasicInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info)
{
    return;
}

int32_t AddDevMgrListener()
{
    return RegNodeDeviceStateCb(DMSLITE_BUNDLE_NAME, &g_networkListener);
}

int32_t UnRegisterDevMgrListener()
{
    return UnregNodeDeviceStateCb(&g_networkListener);
}

char* GetPeerId()
{
    return g_peerDevId;
}

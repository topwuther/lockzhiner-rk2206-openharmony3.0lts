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

#include "common_event_permission_manager.h"
#include "common_event_support.h"
#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {
const int REVERSE = 3;

CommonEventPermissionManager::CommonEventPermissionManager()
{
    Init();
}

void CommonEventPermissionManager::Init()
{
    EVENT_LOGI("enter");

    Permission per;
    per.names.reserve(REVERSE);

    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.RECEIVER_STARTUP_COMPLETED");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.RECEIVER_STARTUP_COMPLETED");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_LOCKED_BOOT_COMPLETED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.MANAGE_USERS");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_USER_SWITCHED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.INTERACT_ACROSS_USERS");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_USER_STARTING, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.INTERACT_ACROSS_USERS");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_USER_STOPPING, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.LOCATION");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_WIFI_SCAN_FINISHED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.GET_WIFI_INFO");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_WIFI_RSSI_VALUE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.GET_WIFI_INFO");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_JOIN, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.GET_WIFI_INFO");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_LEAVE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.MPLINK_CHANGE_STATE");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_WIFI_MPLINK_STATE_CHANGE, per));

    per.names.clear();
    per.state = Permission::AND;
    per.names.emplace_back("ohos.permission.GET_WIFI_INFO");
    per.names.emplace_back("ohos.permission.LOCATION");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_WIFI_P2P_CONN_STATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.GET_WIFI_INFO");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_WIFI_P2P_STATE_CHANGED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.GET_WIFI_INFO");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_WIFI_P2P_PEERS_STATE_CHANGED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.GET_WIFI_INFO");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_WIFI_P2P_PEERS_DISCOVERY_STATE_CHANGED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.GET_WIFI_INFO");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_WIFI_P2P_CURRENT_DEVICE_STATE_CHANGED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.GET_WIFI_INFO");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_WIFI_P2P_GROUP_STATE_CHANGED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CONNECT_STATE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(
        std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CURRENT_DEVICE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_AUDIO_STATE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CONNECT_STATE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CURRENT_DEVICE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_PLAYING_STATE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CODEC_VALUE_UPDATE, per));

    per.names.clear();
    per.state = Permission::AND;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    per.names.emplace_back("ohos.permission.LOCATION");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_DISCOVERED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CLASS_VALUE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_CONNECTED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_DISCONNECTED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_NAME_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIR_STATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_BATTERY_VALUE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.DISCOVER_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_UUID_VALUE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.DISCOVER_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIRING_REQ, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_REQ_ENABLE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_REQ_DISABLE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_SCAN_MODE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_STARTED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_FINISHED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_NAME_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSINK_CONNECT_STATE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSINK_PLAYING_STATE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.USE_BLUETOOTH");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_BLUETOOTH_A2DPSINK_AUDIO_STATE_UPDATE, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.MANAGE_SECURE_SETTINGS");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_NFC_ACTION_RF_FIELD_ON_DETECTED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.MANAGE_SECURE_SETTINGS");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_NFC_ACTION_RF_FIELD_OFF_DETECTED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.MANAGE_USERS");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_USER_ADDED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.MANAGE_USERS");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_USER_REMOVED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.LISTEN_BUNDLE_CHANGE");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_ABILITY_ADDED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.LISTEN_BUNDLE_CHANGE");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_ABILITY_REMOVED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.LISTEN_BUNDLE_CHANGE");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED, per));

    per.names.clear();
    per.state = Permission::OR;
    per.names.emplace_back("ohos.permission.WRITE_USER_STORAGE");
    per.names.emplace_back("ohos.permission.READ_USER_STORAGE");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_DISK_REMOVED, per));

    per.names.clear();
    per.state = Permission::OR;
    per.names.emplace_back("ohos.permission.WRITE_USER_STORAGE");
    per.names.emplace_back("ohos.permission.READ_USER_STORAGE");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_DISK_UNMOUNTED, per));

    per.names.clear();
    per.state = Permission::OR;
    per.names.emplace_back("ohos.permission.WRITE_USER_STORAGE");
    per.names.emplace_back("ohos.permission.READ_USER_STORAGE");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_DISK_MOUNTED, per));

    per.names.clear();
    per.state = Permission::OR;
    per.names.emplace_back("ohos.permission.WRITE_USER_STORAGE");
    per.names.emplace_back("ohos.permission.READ_USER_STORAGE");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_DISK_BAD_REMOVAL, per));

    per.names.clear();
    per.state = Permission::OR;
    per.names.emplace_back("ohos.permission.WRITE_USER_STORAGE");
    per.names.emplace_back("ohos.permission.READ_USER_STORAGE");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_DISK_UNMOUNTABLE, per));

    per.names.clear();
    per.state = Permission::OR;
    per.names.emplace_back("ohos.permission.WRITE_USER_STORAGE");
    per.names.emplace_back("ohos.permission.READ_USER_STORAGE");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_DISK_EJECT, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.GET_APP_ACCOUNTS");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_VISIBLE_ACCOUNTS_UPDATED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_ACCOUNT_DELETED, per));

    per.names.clear();
    per.state = Permission::DEFAULT;
    per.names.emplace_back("ohos.permission.RECEIVER_STARTUP_COMPLETED");
    multimap_.insert(std::make_pair(CommonEventSupport::COMMON_EVENT_FOUNDATION_READY, per));
}

Permission CommonEventPermissionManager::GetEventPermission(std::string event)
{
    EVENT_LOGI("enter");
    Permission per;

    auto iter = multimap_.begin();
    for (; iter != multimap_.end(); ++iter) {
        if (!event.compare(iter->first)) {
            break;
        }
    }

    if (iter == multimap_.end()) {
        return per;
    }

    return iter->second;
}
}  // namespace EventFwk
}  // namespace OHOS
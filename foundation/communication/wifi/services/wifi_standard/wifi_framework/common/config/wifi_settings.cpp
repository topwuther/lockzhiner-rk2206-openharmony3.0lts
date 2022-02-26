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
#include "wifi_settings.h"
#include <algorithm>
#include "define.h"
#include "wifi_global_func.h"

namespace OHOS {
namespace Wifi {
WifiSettings &WifiSettings::GetInstance()
{
    static WifiSettings gWifiSettings;
    return gWifiSettings;
}

WifiSettings::WifiSettings()
    : mWifiStaCapabilities(0),
      mWifiState(0),
      mScanAlwaysActive(false),
      mHotspotState(static_cast<int>(ApState::AP_STATE_CLOSED)),
      mP2pState(static_cast<int>(P2pState::P2P_STATE_CLOSED)),
      mP2pDiscoverState(0),
      mP2pConnectState(0),
      mApMaxConnNum(0),
      mLastSelectedNetworkId(-1),
      mLastSelectedTimeVal(0),
      mScreenState(1),
      mAirplaneModeState(MODE_STATE_CLOSE),
      mAppRunningModeState(1),
      mPowerSavingModeState(MODE_STATE_CLOSE)
{}

WifiSettings::~WifiSettings()
{
    SyncDeviceConfig();
    SyncHotspotConfig();
    SyncBlockList();
    SyncWifiConfig();
    SyncWifiP2pGroupInfoConfig();
    SyncP2pVendorConfig();
}

void WifiSettings::InitWifiConfig()
{
    if (mSavedWifiConfig.LoadConfig() < 0) {
        return;
    }
    std::vector<WifiConfig> tmp;
    mSavedWifiConfig.GetValue(tmp);
    if (tmp.size() > 0) {
        mWifiConfig = tmp[0];
        mScanAlwaysActive = mWifiConfig.scanAlwaysSwitch;
    }
    return;
}

void WifiSettings::InitHotspotConfig()
{
    /* init hotspot config */
    if (mSavedHotspotConfig.LoadConfig() >= 0) {
        std::vector<HotspotConfig> tmp;
        mSavedHotspotConfig.GetValue(tmp);
        if (tmp.size() > 0) {
            mHotspotConfig = tmp[0];
        } else {
            InitDefaultHotspotConfig();
        }
    } else {
        InitDefaultHotspotConfig();
    }
    /* init block list info */
    if (mSavedBlockInfo.LoadConfig() >= 0) {
        std::vector<StationInfo> tmp;
        mSavedBlockInfo.GetValue(tmp);
        for (std::size_t i = 0; i < tmp.size(); ++i) {
            StationInfo &item = tmp[i];
            mBlockListInfo.emplace(item.bssid, item);
        }
    }
    return;
}

void WifiSettings::InitP2pVendorConfig()
{
    if (mSavedWifiP2pVendorConfig.LoadConfig() >= 0) {
        std::vector<P2pVendorConfig> tmp;
        mSavedWifiP2pVendorConfig.GetValue(tmp);
        if (tmp.size() > 0) {
            mP2pVendorConfig = tmp[0];
        } else {
            InitDefaultP2pVendorConfig();
        }
    } else {
        InitDefaultP2pVendorConfig();
    }
    return;
}

int WifiSettings::Init()
{
    mCountryCode = "CN";
    InitGetApMaxConnNum();

    /* read ini config */
    mSavedDeviceConfig.SetConfigFilePath(DEVICE_CONFIG_FILE_PATH);
    mSavedHotspotConfig.SetConfigFilePath(HOTSPOT_CONFIG_FILE_PATH);
    mSavedBlockInfo.SetConfigFilePath(BLOCK_LIST_FILE_PATH);
    mSavedWifiConfig.SetConfigFilePath(WIFI_CONFIG_FILE_PATH);
    mSavedWifiP2pGroupInfo.SetConfigFilePath(WIFI_P2P_GROUP_INFO_FILE_PATH);
    mSavedWifiP2pVendorConfig.SetConfigFilePath(WIFI_P2P_VENDOR_CONFIG_FILE_PATH);
    InitWifiConfig();
    ReloadDeviceConfig();
    InitHotspotConfig();
    InitScanControlInfo();
    InitP2pVendorConfig();
    ReloadWifiP2pGroupInfoConfig();
    return 0;
}

int WifiSettings::GetWifiStaCapabilities() const
{
    return mWifiStaCapabilities;
}

int WifiSettings::SetWifiStaCapabilities(int capabilities)
{
    mWifiStaCapabilities = capabilities;
    return 0;
}

int WifiSettings::GetWifiState() const
{
    return mWifiState.load();
}

int WifiSettings::SetWifiState(int state)
{
    mWifiState = state;
    return 0;
}

bool WifiSettings::GetScanAlwaysState() const
{
    return mScanAlwaysActive.load();
}

int WifiSettings::SetScanAlwaysState(bool isActive)
{
    mScanAlwaysActive = isActive;
    mWifiConfig.scanAlwaysSwitch = isActive;
    return 0;
}

int WifiSettings::SaveScanInfoList(const std::vector<WifiScanInfo> &results)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    mWifiScanInfoList = results;
    return 0;
}

int WifiSettings::GetScanInfoList(std::vector<WifiScanInfo> &results)
{
    struct timespec clkTime = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &clkTime);
    int64_t curr = static_cast<int64_t>(clkTime.tv_sec) * MSEC * MSEC + clkTime.tv_nsec / MSEC; /* us */
    std::unique_lock<std::mutex> lock(mInfoMutex);
    for (auto iter = mWifiScanInfoList.begin(); iter != mWifiScanInfoList.end(); ++iter) {
        if (iter->timestamp + WIFI_GET_SCAN_INFO_VALID_TIMESTAMP * MSEC * MSEC < curr) {
            continue;
        }
        results.push_back(*iter);
    }
    return 0;
}

int WifiSettings::GetScanControlInfo(ScanControlInfo &info)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    info = mScanControlInfo;
    return 0;
}

int WifiSettings::GetP2pInfo(WifiP2pInfo &connInfo)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    connInfo = mWifiP2pInfo;
    return 0;
}

int WifiSettings::SaveP2pInfo(WifiP2pInfo &connInfo)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    mWifiP2pInfo = connInfo;
    return 0;
}

int WifiSettings::SetScanControlInfo(const ScanControlInfo &info)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    mScanControlInfo = info;
    return 0;
}

int WifiSettings::AddDeviceConfig(const WifiDeviceConfig &config)
{
    std::unique_lock<std::mutex> lock(mConfigMutex);
    auto iter = mWifiDeviceConfig.find(config.networkId);
    if (iter != mWifiDeviceConfig.end()) {
        iter->second = config;
    } else {
        mWifiDeviceConfig.emplace(std::make_pair(config.networkId, config));
    }
    return config.networkId;
}

int WifiSettings::RemoveDevice(int networkId)
{
    std::unique_lock<std::mutex> lock(mConfigMutex);
    auto iter = mWifiDeviceConfig.find(networkId);
    if (iter != mWifiDeviceConfig.end()) {
        mWifiDeviceConfig.erase(iter);
    }
    return 0;
}

void WifiSettings::ClearDeviceConfig(void)
{
    std::unique_lock<std::mutex> lock(mConfigMutex);
    mWifiDeviceConfig.clear();
    return;
}

int WifiSettings::GetDeviceConfig(std::vector<WifiDeviceConfig> &results)
{
    std::unique_lock<std::mutex> lock(mConfigMutex);
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
        results.push_back(iter->second);
    }
    return 0;
}

int WifiSettings::GetDeviceConfig(const int &networkId, WifiDeviceConfig &config)
{
    std::unique_lock<std::mutex> lock(mConfigMutex);
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
        if (iter->second.networkId == networkId) {
            config = iter->second;
            return 0;
        }
    }
    return -1;
}

int WifiSettings::GetDeviceConfig(const std::string &index, const int &indexType, WifiDeviceConfig &config)
{
    std::unique_lock<std::mutex> lock(mConfigMutex);
    if (indexType == DEVICE_CONFIG_INDEX_SSID) {
        for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
            if (iter->second.ssid == index) {
                config = iter->second;
                return 0;
            }
        }
    } else {
        for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
            if (iter->second.bssid == index) {
                config = iter->second;
                return 0;
            }
        }
    }
    return -1;
}

int WifiSettings::GetDeviceConfig(const std::string &ssid, const std::string &keymgmt, WifiDeviceConfig &config)
{
    std::unique_lock<std::mutex> lock(mConfigMutex);
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
        if ((iter->second.ssid == ssid) && (iter->second.keyMgmt == keymgmt)) {
            config = iter->second;
            return 0;
        }
    }
    return -1;
}

int WifiSettings::GetHiddenDeviceConfig(std::vector<WifiDeviceConfig> &results)
{
    std::unique_lock<std::mutex> lock(mConfigMutex);
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
        if (iter->second.hiddenSSID) {
            results.push_back(iter->second);
        }
    }
    return 0;
}

int WifiSettings::SetDeviceState(int networkId, int state, bool bSetOther)
{
    if (state < 0 || state >= (int)WifiDeviceConfigStatus::UNKNOWN) {
        return -1;
    }
    std::unique_lock<std::mutex> lock(mConfigMutex);
    auto iter = mWifiDeviceConfig.find(networkId);
    if (iter == mWifiDeviceConfig.end()) {
        return -1;
    }
    iter->second.status = state;
    if (bSetOther && state == (int)WifiDeviceConfigStatus::ENABLED) {
        for (iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); ++iter) {
            if (iter->first != networkId && iter->second.status == state) {
                iter->second.status = 1;
            }
        }
    }
    return 0;
}

int WifiSettings::SyncWifiP2pGroupInfoConfig()
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    mSavedWifiP2pGroupInfo.SetValue(mGroupInfoList);
    return mSavedWifiP2pGroupInfo.SaveConfig();
}

int WifiSettings::ReloadWifiP2pGroupInfoConfig()
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    if (mSavedWifiP2pGroupInfo.LoadConfig()) {
        return -1;
    }
    mSavedWifiP2pGroupInfo.GetValue(mGroupInfoList);
    return 0;
}

int WifiSettings::SetWifiP2pGroupInfo(const std::vector<WifiP2pGroupInfo> &groups)
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    mGroupInfoList = groups;
    return 0;
}

int WifiSettings::RemoveWifiP2pGroupInfo()
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    mGroupInfoList.clear();
    return 0;
}

int WifiSettings::GetWifiP2pGroupInfo(std::vector<WifiP2pGroupInfo> &groups)
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    groups = mGroupInfoList;
    return 0;
}

int WifiSettings::SyncDeviceConfig()
{
#ifndef CONFIG_NO_CONFIG_WRITE
    std::unique_lock<std::mutex> lock(mConfigMutex);
    std::vector<WifiDeviceConfig> tmp;
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); ++iter) {
        tmp.push_back(iter->second);
    }
    mSavedDeviceConfig.SetValue(tmp);
    return mSavedDeviceConfig.SaveConfig();
#else
    return 0;
#endif
}

int WifiSettings::ReloadDeviceConfig()
{
#ifndef CONFIG_NO_CONFIG_WRITE
    int ret = mSavedDeviceConfig.LoadConfig();
    if (ret < 0) {
        return -1;
    }
    std::vector<WifiDeviceConfig> tmp;
    mSavedDeviceConfig.GetValue(tmp);
    std::unique_lock<std::mutex> lock(mConfigMutex);
    mWifiDeviceConfig.clear();
    for (std::size_t i = 0; i < tmp.size(); ++i) {
        WifiDeviceConfig &item = tmp[i];
        item.networkId = i;
        mWifiDeviceConfig.emplace(item.networkId, item);
    }
    return 0;
#else
    std::unique_lock<std::mutex> lock(mConfigMutex);
    mWifiDeviceConfig.clear();
    return 0;
#endif
}

int WifiSettings::AddWpsDeviceConfig(const WifiDeviceConfig &config)
{
    int ret = mSavedDeviceConfig.LoadConfig();
    if (ret < 0) {
        return -1;
    }
    std::vector<WifiDeviceConfig> tmp;
    mSavedDeviceConfig.GetValue(tmp);
    std::unique_lock<std::mutex> lock(mConfigMutex);
    mWifiDeviceConfig.clear();
    mWifiDeviceConfig.emplace(0, config);
    for (std::size_t i = 0; i < tmp.size(); ++i) {
        WifiDeviceConfig &item = tmp[i];
        item.networkId = i + 1;
        mWifiDeviceConfig.emplace(item.networkId, item);
    }
    return 0;
}

int WifiSettings::GetIpInfo(IpInfo &info)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    info = mWifiIpInfo;
    return 0;
}

int WifiSettings::SaveIpInfo(const IpInfo &info)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    mWifiIpInfo = info;
    return 0;
}

int WifiSettings::GetLinkedInfo(WifiLinkedInfo &info)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    info = mWifiLinkedInfo;
    return 0;
}

int WifiSettings::SaveLinkedInfo(const WifiLinkedInfo &info)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    mWifiLinkedInfo = info;
    return 0;
}

int WifiSettings::SetMacAddress(const std::string &macAddress)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    mMacAddress = macAddress;
    return 0;
}

int WifiSettings::GetMacAddress(std::string &macAddress)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    macAddress = mMacAddress;
    return 0;
}

int WifiSettings::SetCountryCode(const std::string &countryCode)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    std::string tmpCode = countryCode;
    std::transform(countryCode.begin(), countryCode.end(), tmpCode.begin(), ::toupper);
    mCountryCode = tmpCode;
    return 0;
}

int WifiSettings::GetCountryCode(std::string &countryCode)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    countryCode = mCountryCode;
    return 0;
}

int WifiSettings::GetHotspotState()
{
    return mHotspotState.load();
}

int WifiSettings::SetHotspotState(int state)
{
    mHotspotState = state;
    return 0;
}

int WifiSettings::SetHotspotConfig(const HotspotConfig &config)
{
    std::unique_lock<std::mutex> lock(mApMutex);
    mHotspotConfig = config;
    return 0;
}

int WifiSettings::GetHotspotConfig(HotspotConfig &config)
{
    std::unique_lock<std::mutex> lock(mApMutex);
    config = mHotspotConfig;
    return 0;
}

int WifiSettings::SyncHotspotConfig()
{
    std::unique_lock<std::mutex> lock(mApMutex);
    std::vector<HotspotConfig> tmp;
    tmp.push_back(mHotspotConfig);
    mSavedHotspotConfig.SetValue(tmp);
    return mSavedHotspotConfig.SaveConfig();
}

int WifiSettings::SetP2pVendorConfig(const P2pVendorConfig &config)
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    mP2pVendorConfig = config;
    return 0;
}

int WifiSettings::GetP2pVendorConfig(P2pVendorConfig &config)
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    config = mP2pVendorConfig;
    return 0;
}

int WifiSettings::SyncP2pVendorConfig()
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    std::vector<P2pVendorConfig> tmp;
    tmp.push_back(mP2pVendorConfig);
    mSavedWifiP2pVendorConfig.SetValue(tmp);
    return mSavedWifiP2pVendorConfig.SaveConfig();
}

int WifiSettings::GetStationList(std::vector<StationInfo> &results)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    for (auto iter = mConnectStationInfo.begin(); iter != mConnectStationInfo.end(); iter++) {
        results.push_back(iter->second);
    }
    return 0;
}

int WifiSettings::ManageStation(const StationInfo &info, int mode)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    auto iter = mConnectStationInfo.find(info.bssid);
    if (MODE_ADD == mode || MODE_UPDATE == mode) {
        if (iter != mConnectStationInfo.end()) {
            iter->second = info;
        } else {
            mConnectStationInfo.emplace(std::make_pair(info.bssid, info));
        }
    } else if (MODE_DEL == mode) {
        if (iter != mConnectStationInfo.end()) {
            mConnectStationInfo.erase(iter);
        }
    } else {
        return -1;
    }
    return 0;
}

int WifiSettings::FindConnStation(const StationInfo &info)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    auto iter = mConnectStationInfo.find(info.bssid);
    if (iter == mConnectStationInfo.end()) {
        return -1;
    }
    return 0;
}

int WifiSettings::ClearStationList()
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    mConnectStationInfo.clear();
    return 0;
}

int WifiSettings::GetBlockList(std::vector<StationInfo> &results)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    for (auto iter = mBlockListInfo.begin(); iter != mBlockListInfo.end(); iter++) {
        results.push_back(iter->second);
    }
    return 0;
}

int WifiSettings::ManageBlockList(const StationInfo &info, int mode)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    auto iter = mBlockListInfo.find(info.bssid);
    if (MODE_ADD == mode || MODE_UPDATE == mode) {
        if (iter != mBlockListInfo.end()) {
            iter->second = info;
        } else {
            mBlockListInfo.emplace(std::make_pair(info.bssid, info));
        }
    } else if (MODE_DEL == mode) {
        if (iter != mBlockListInfo.end()) {
            mBlockListInfo.erase(iter);
        }
    } else {
        return -1;
    }
    return 0;
}

int WifiSettings::SyncBlockList()
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    std::vector<StationInfo> tmp;
    for (auto iter = mBlockListInfo.begin(); iter != mBlockListInfo.end(); ++iter) {
        tmp.push_back(iter->second);
    }
    mSavedBlockInfo.SetValue(tmp);
    return mSavedBlockInfo.SaveConfig();
}

int WifiSettings::GetValidBands(std::vector<BandType> &bands)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);

    auto it = mValidChannels.find(BandType::BAND_2GHZ);
    if (it != mValidChannels.end() && it->second.size() > 0) {
        bands.push_back(BandType::BAND_2GHZ);
    }
    it = mValidChannels.find(BandType::BAND_5GHZ);
    if (it != mValidChannels.end() && it->second.size() > 0) {
        bands.push_back(BandType::BAND_5GHZ);
    }
    return 0;
}

int WifiSettings::SetValidChannels(const ChannelsTable &channelsInfo)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    mValidChannels = channelsInfo;

    return 0;
}

int WifiSettings::GetValidChannels(ChannelsTable &channelsInfo)
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    channelsInfo = mValidChannels;

    return 0;
}

int WifiSettings::ClearValidChannels()
{
    std::unique_lock<std::mutex> lock(mInfoMutex);
    mValidChannels.clear();
    return 0;
}

int WifiSettings::SetP2pState(int state)
{
    mP2pState = state;
    return 0;
}

int WifiSettings::GetP2pState()
{
    return mP2pState.load();
}

int WifiSettings::SetP2pDiscoverState(int state)
{
    mP2pDiscoverState = state;
    return 0;
}

int WifiSettings::GetP2pDiscoverState()
{
    return mP2pDiscoverState.load();
}

int WifiSettings::SetP2pConnectedState(int state)
{
    mP2pConnectState = state;
    return 0;
}

int WifiSettings::GetP2pConnectedState()
{
    return mP2pConnectState.load();
}

int WifiSettings::GetSignalLevel(const int &rssi, const int &band)
{
    int level = 0;
    do {
        if (band == static_cast<int>(BandType::BAND_2GHZ)) {
            if (rssi < mWifiConfig.firstRssiLevel2G) {
                break;
            }
            ++level;
            if (rssi < mWifiConfig.secondRssiLevel2G) {
                break;
            }
            ++level;
            if (rssi < mWifiConfig.thirdRssiLevel2G) {
                break;
            }
            ++level;
            if (rssi < mWifiConfig.fourthRssiLevel2G) {
                break;
            }
            ++level;
        }
        if (band == static_cast<int>(BandType::BAND_5GHZ)) {
            if (rssi < mWifiConfig.firstRssiLevel5G) {
                break;
            }
            ++level;
            if (rssi < mWifiConfig.secondRssiLevel5G) {
                break;
            }
            ++level;
            if (rssi < mWifiConfig.thirdRssiLevel5G) {
                break;
            }
            ++level;
            if (rssi < mWifiConfig.fourthRssiLevel5G) {
                break;
            }
            ++level;
        }
    } while (0);
    return level;
}

int WifiSettings::GetApMaxConnNum()
{
    return mApMaxConnNum;
}

void WifiSettings::InitDefaultHotspotConfig()
{
    mHotspotConfig.SetSecurityType(KeyMgmt::WPA_PSK);
    mHotspotConfig.SetBand(BandType::BAND_2GHZ);
    mHotspotConfig.SetChannel(AP_CHANNEL_DEFAULT);
    mHotspotConfig.SetMaxConn(GetApMaxConnNum());
    mHotspotConfig.SetSsid("OHOS_" + GetRandomStr(RANDOM_STR_LEN));
    mHotspotConfig.SetPreSharedKey("12345678");
}

void WifiSettings::InitDefaultP2pVendorConfig()
{
    mP2pVendorConfig.SetRandomMacSupport(false);
    mP2pVendorConfig.SetIsAutoListen(true);
    mP2pVendorConfig.SetDeviceName("");
    mP2pVendorConfig.SetPrimaryDeviceType("");
    mP2pVendorConfig.SetSecondaryDeviceType("");
}

void WifiSettings::InitGetApMaxConnNum()
{
    /* query drivers capability, support max connection num. */
    mApMaxConnNum = MAX_AP_CONN;
}

void WifiSettings::InitScanControlForbidList(void)
{
    /* Disable external scanning during scanning. */
    std::vector<ScanForbidMode> forbidModeList;
    ScanForbidMode forbidMode;
    forbidMode.scanMode = ScanMode::ALL_EXTERN_SCAN;
    forbidModeList.push_back(forbidMode);
    mScanControlInfo.scanForbidMap[SCAN_SCENE_SCANNING] = forbidModeList;

    /* Disable external scanning when the screen is shut down. */
    mScanControlInfo.scanForbidMap[SCAN_SCENE_SCREEN_OFF] = forbidModeList;

    /* Disable all scans in connection */
    forbidMode.scanMode = ScanMode::PNO_SCAN;
    forbidModeList.push_back(forbidMode);
    forbidMode.scanMode = ScanMode::SYSTEM_TIMER_SCAN;
    forbidModeList.push_back(forbidMode);
    mScanControlInfo.scanForbidMap[SCAN_SCENE_CONNECTING] = forbidModeList;

    /* Deep sleep disables all scans. */
    mScanControlInfo.scanForbidMap[SCAN_SCENE_DEEP_SLEEP] = forbidModeList;

    /* PNO scanning disabled */
    forbidModeList.clear();
    forbidMode.scanMode = ScanMode::PNO_SCAN;
    forbidModeList.push_back(forbidMode);
    mScanControlInfo.scanForbidMap[SCAN_SCENE_CONNECTED] = forbidModeList;
    return;
}

void WifiSettings::InitScanControlIntervalList(void)
{
    /* Foreground app: 4 times in 2 minutes for a single application */
    ScanIntervalMode scanIntervalMode;
    scanIntervalMode.scanScene = SCAN_SCENE_ALL;
    scanIntervalMode.scanMode = ScanMode::APP_FOREGROUND_SCAN;
    scanIntervalMode.isSingle = true;
    scanIntervalMode.intervalMode = IntervalMode::INTERVAL_FIXED;
    scanIntervalMode.interval = FOREGROUND_SCAN_CONTROL_INTERVAL;
    scanIntervalMode.count = FOREGROUND_SCAN_CONTROL_TIMES;
    mScanControlInfo.scanIntervalList.push_back(scanIntervalMode);

    /* Backend apps: once every 30 minutes */
    scanIntervalMode.scanScene = SCAN_SCENE_ALL;
    scanIntervalMode.scanMode = ScanMode::APP_BACKGROUND_SCAN;
    scanIntervalMode.isSingle = false;
    scanIntervalMode.intervalMode = IntervalMode::INTERVAL_FIXED;
    scanIntervalMode.interval = BACKGROUND_SCAN_CONTROL_INTERVAL;
    scanIntervalMode.count = BACKGROUND_SCAN_CONTROL_TIMES;
    mScanControlInfo.scanIntervalList.push_back(scanIntervalMode);

    /* PNO scanning every 20 seconds */
    scanIntervalMode.scanScene = SCAN_SCENE_ALL;
    scanIntervalMode.scanMode = ScanMode::PNO_SCAN;
    scanIntervalMode.isSingle = false;
    scanIntervalMode.intervalMode = IntervalMode::INTERVAL_FIXED;
    scanIntervalMode.interval = PNO_SCAN_CONTROL_INTERVAL;
    scanIntervalMode.count = PNO_SCAN_CONTROL_TIMES;
    mScanControlInfo.scanIntervalList.push_back(scanIntervalMode);

    /*
     * The system scans for 20 seconds, multiplies 2 each time,
     * and performs scanning every 160 seconds.
     */
    scanIntervalMode.scanScene = SCAN_SCENE_ALL;
    scanIntervalMode.scanMode = ScanMode::SYSTEM_TIMER_SCAN;
    scanIntervalMode.isSingle = false;
    scanIntervalMode.intervalMode = IntervalMode::INTERVAL_EXP;
    scanIntervalMode.interval = SYSTEM_TIMER_SCAN_CONTROL_INTERVAL;
    scanIntervalMode.count = SYSTEM_TIMER_SCAN_CONTROL_TIMES;
    mScanControlInfo.scanIntervalList.push_back(scanIntervalMode);
    return;
}

void WifiSettings::InitScanControlInfo()
{
    InitScanControlForbidList();
    InitScanControlIntervalList();
}

bool WifiSettings::EnableNetwork(int networkId, bool disableOthers)
{
    if (disableOthers) {
        SetUserLastSelectedNetworkId(networkId);
    }
    return true;
}

void WifiSettings::SetUserLastSelectedNetworkId(int networkId)
{
    mLastSelectedNetworkId = networkId;
    mLastSelectedTimeVal = time(NULL);
}

int WifiSettings::GetUserLastSelectedNetworkId()
{
    return mLastSelectedNetworkId;
}

time_t WifiSettings::GetUserLastSelectedNetworkTimeVal()
{
    return mLastSelectedTimeVal;
}

int WifiSettings::SyncWifiConfig()
{
    std::vector<WifiConfig> tmp;
    tmp.push_back(mWifiConfig);
    mSavedWifiConfig.SetValue(tmp);
    return mSavedWifiConfig.SaveConfig();
}

bool WifiSettings::GetCanUseStaWhenAirplaneMode()
{
    return mWifiConfig.staAirplaneMode;
}

int WifiSettings::SetCanUseStaWhenAirplaneMode(bool bCan)
{
    mWifiConfig.staAirplaneMode = bCan;
    return 0;
}

bool WifiSettings::GetCanOpenStaWhenAirplaneMode()
{
    return mWifiConfig.canOpenStaWhenAirplane;
}

bool WifiSettings::GetStaLastRunState()
{
    return mWifiConfig.staLastState;
}

int WifiSettings::SetStaLastRunState(bool bRun)
{
    mWifiConfig.staLastState = bRun;
    return 0;
}

int WifiSettings::GetDhcpIpType()
{
    return mWifiConfig.dhcpIpType;
}

int WifiSettings::SetDhcpIpType(int dhcpIpType)
{
    mWifiConfig.dhcpIpType = dhcpIpType;
    return 0;
}

std::string WifiSettings::GetDefaultWifiInterface()
{
    return mWifiConfig.defaultWifiInterface;
}

void WifiSettings::SetScreenState(const int &state)
{
    mScreenState = state;
}

int WifiSettings::GetScreenState()
{
    return mScreenState;
}

void WifiSettings::SetAirplaneModeState(const int &state)
{
    mAirplaneModeState = state;
}

int WifiSettings::GetAirplaneModeState()
{
    return mAirplaneModeState;
}

void WifiSettings::SetAppRunningState(const int &state)
{
    mAppRunningModeState = state;
}

int WifiSettings::GetAppRunningState()
{
    return mAppRunningModeState;
}

void WifiSettings::SetPowerSavingModeState(const int &state)
{
    mPowerSavingModeState = state;
}

int WifiSettings::GetPowerSavingModeState()
{
    return mPowerSavingModeState;
}

int WifiSettings::SetWhetherToAllowNetworkSwitchover(bool bSwitch)
{
    mWifiConfig.whetherToAllowNetworkSwitchover = bSwitch;
    return 0;
}

bool WifiSettings::GetWhetherToAllowNetworkSwitchover()
{
    return mWifiConfig.whetherToAllowNetworkSwitchover;
}

int WifiSettings::SetScoretacticsScoreSlope(const int &score)
{
    mWifiConfig.scoretacticsScoreSlope = score;
    return 0;
}

int WifiSettings::GetScoretacticsScoreSlope()
{
    return mWifiConfig.scoretacticsScoreSlope;
}

int WifiSettings::SetScoretacticsInitScore(const int &score)
{
    mWifiConfig.scoretacticsInitScore = score;
    return 0;
}

int WifiSettings::GetScoretacticsInitScore()
{
    return mWifiConfig.scoretacticsInitScore;
}

int WifiSettings::SetScoretacticsSameBssidScore(const int &score)
{
    mWifiConfig.scoretacticsSameBssidScore = score;
    return 0;
}

int WifiSettings::GetScoretacticsSameBssidScore()
{
    return mWifiConfig.scoretacticsSameBssidScore;
}

int WifiSettings::SetScoretacticsSameNetworkScore(const int &score)
{
    mWifiConfig.scoretacticsSameNetworkScore = score;
    return 0;
}

int WifiSettings::GetScoretacticsSameNetworkScore()
{
    return mWifiConfig.scoretacticsSameNetworkScore;
}

int WifiSettings::SetScoretacticsFrequency5GHzScore(const int &score)
{
    mWifiConfig.scoretacticsFrequency5GHzScore = score;
    return 0;
}

int WifiSettings::GetScoretacticsFrequency5GHzScore()
{
    return mWifiConfig.scoretacticsFrequency5GHzScore;
}

int WifiSettings::SetScoretacticsLastSelectionScore(const int &score)
{
    mWifiConfig.scoretacticsLastSelectionScore = score;
    return 0;
}

int WifiSettings::GetScoretacticsLastSelectionScore()
{
    return mWifiConfig.scoretacticsLastSelectionScore;
}

int WifiSettings::SetScoretacticsSecurityScore(const int &score)
{
    mWifiConfig.scoretacticsSecurityScore = score;
    return 0;
}

int WifiSettings::GetScoretacticsSecurityScore()
{
    return mWifiConfig.scoretacticsSecurityScore;
}

int WifiSettings::SetSavedDeviceAppraisalPriority(const int &priority)
{
    mWifiConfig.savedDeviceAppraisalPriority = priority;
    return 0;
}

int WifiSettings::GetSavedDeviceAppraisalPriority()
{
    return mWifiConfig.savedDeviceAppraisalPriority;
}

bool WifiSettings::IsModulePreLoad(const std::string &name)
{
    if (name == WIFI_SERVICE_STA) {
        return mWifiConfig.preLoadSta;
    } else if (name == WIFI_SERVICE_SCAN) {
        return mWifiConfig.preLoadScan;
    } else if (name == WIFI_SERVICE_AP) {
        return mWifiConfig.preLoadAp;
    } else if (name == WIFI_SERVICE_P2P) {
        return mWifiConfig.preLoadP2p;
    } else if (name == WIFI_SERVICE_AWARE) {
        return mWifiConfig.preLoadAware;
    } else {
        return false;
    }
}

bool WifiSettings::GetSupportHwPnoFlag()
{
    return mWifiConfig.supportHwPnoFlag;
}

int WifiSettings::GetMinRssi2Dot4Ghz()
{
    return mWifiConfig.minRssi2Dot4Ghz;
}

int WifiSettings::GetMinRssi5Ghz()
{
    return mWifiConfig.minRssi5Ghz;
}

int WifiSettings::SetP2pDeviceName(const std::string &deviceName)
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    mP2pVendorConfig.SetDeviceName(deviceName);
    return mSavedWifiP2pVendorConfig.SaveConfig();
}
}  // namespace Wifi
}  // namespace OHOS
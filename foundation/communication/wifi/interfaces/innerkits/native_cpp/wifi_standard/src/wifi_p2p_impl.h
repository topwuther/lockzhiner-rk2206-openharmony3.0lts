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
#ifndef OHOS_WIFI_P2P_IMPL_H
#define OHOS_WIFI_P2P_IMPL_H

#include "wifi_p2p.h"
#include "wifi_p2p_proxy.h"

namespace OHOS {
namespace Wifi {
class WifiP2pImpl : public WifiP2p {
public:
    explicit WifiP2pImpl(int systemAbilityId);
    ~WifiP2pImpl();

    bool Init(void);

    /**
     * @Description Enabling the P2P Mode
     *
     * @return ErrCode - operate result
     */
    ErrCode EnableP2p(void) override;

    /**
     * @Description Disable the P2P mode
     *
     * @return ErrCode - operate result
     */
    ErrCode DisableP2p(void) override;

    /**
     * @Description Start Wi-Fi P2P device search
     *
     * @return ErrCode - operate result
     */
    ErrCode DiscoverDevices(void) override;

    /**
     * @Description Stop Wi-Fi P2P device search
     *
     * @return ErrCode - operate result
     */
    ErrCode StopDiscoverDevices(void) override;

    /**
     * @Description Start the search for the Wi-Fi P2P service
     *
     * @return ErrCode - operate result
     */
    ErrCode DiscoverServices(void) override;

    /**
     * @Description Stop the search for the Wi-Fi P2P service
     *
     * @return ErrCode - operate result
     */
    ErrCode StopDiscoverServices(void) override;

    /**
     * @Description request the P2P service
     *
     * @param device - WifiP2pDevice object
     * @param request - WifiP2pServiceRequest object
     * @return ErrCode - operate result
     */
    ErrCode RequestService(const WifiP2pDevice &device, const WifiP2pServiceRequest &request) override;

    /**
     * @Description Register the local P2P service
     *
     * @param srvInfo - WifiP2pServiceInfo object
     * @return ErrCode - operate result
     */
    ErrCode PutLocalP2pService(const WifiP2pServiceInfo &srvInfo) override;

    /**
     * @Description Delete the local P2P service
     *
     * @param srvInfo - WifiP2pServiceInfo object
     * @return ErrCode - operate result
     */
    ErrCode DeleteLocalP2pService(const WifiP2pServiceInfo &srvInfo) override;

    /**
     * @Description Enable Wi-Fi P2P listening
     *
     * @param period - period
     * @param interval - interval
     * @return ErrCode - operate result
     */
    ErrCode StartP2pListen(int period, int interval) override;

    /**
     * @Description Disable Wi-Fi P2P listening
     *
     * @return ErrCode - operate result
     */
    ErrCode StopP2pListen(void) override;

    /**
     * @Description Creating a P2P Group
     *
     * @param config - WifiP2pGroupInfo object
     * @return ErrCode - operate result
     */
    ErrCode FormGroup(const WifiP2pConfig &config) override;

    /**
     * @Description Remove a P2P Group
     *
     *
     * @return ErrCode - operate result
     */
    ErrCode RemoveGroup(void) override;

    /**
     * @Description Delete a p2p Group
     *
     * @param group - WifiP2pGroupInfo object
     * @return ErrCode - operate result
     */
    ErrCode DeleteGroup(const WifiP2pGroupInfo &group) override;

    /**
     * @Description P2P connection
     *
     * @param config - WifiP2pConfig object
     * @return ErrCode - operate result
     */
    ErrCode P2pConnect(const WifiP2pConfig &config) override;

    /**
     * @Description P2P disconnection
     *
     * @return ErrCode - operate result
     */
    ErrCode P2pDisConnect(void) override;

    /**
     * @Description Querying Wi-Fi P2P Connection Information
     *
     * @param connInfo - Get the WifiP2pInfo msg
     * @return ErrCode - operate result
     */
    ErrCode QueryP2pInfo(WifiP2pInfo &connInfo) override;

    /**
     * @Description Get the Current Group object
     *
     * @param group - the WifiP2pGroupInfo object
     * @return ErrCode - operate result
     */
    ErrCode GetCurrentGroup(WifiP2pGroupInfo &group) override;

    /**
     * @Description Obtains the P2P switch status
     *
     * @param status - the P2P switch status
     * @return ErrCode - operate result
     */
    ErrCode GetP2pEnableStatus(int &status) override;

    /**
     * @Description Obtains the P2P discovery status
     *
     * @param status - the P2P discovery status
     * @return ErrCode
     */
    ErrCode GetP2pDiscoverStatus(int &status) override;

    /**
     * @Description Obtains the P2P connection status
     *
     * @param status - the P2P connection status
     * @return ErrCode - operate result
     */
    ErrCode GetP2pConnectedStatus(int &status) override;

    /**
     * @Description Query the information about the found devices
     *
     * @param devives - Get result vector of WifiP2pDevice
     * @return ErrCode - operate result
     */
    ErrCode QueryP2pDevices(std::vector<WifiP2pDevice> &devives) override;

    /**
     * @Description Query the information about the found groups
     *
     * @param groups - Get result vector of WifiP2pGroupInfo
     * @return ErrCode - operate result
     */
    ErrCode QueryP2pGroups(std::vector<WifiP2pGroupInfo> &groups) override;

    /**
     * @Description Query the service information
     *
     * @param services - Get result vector of Device
     * @return ErrCode - operate result
     */
    ErrCode QueryP2pServices(std::vector<WifiP2pServiceInfo> &services) override;

    /**
     * @Description Register callback function
     *
     * @param callback - IWifiP2pCallback object
     * @return ErrCode - operate result
     */
    ErrCode RegisterCallBack(const sptr<IWifiP2pCallback> &callback) override;

    /**
     * @Description Get supported features
     *
     * @param features - return supported features
     * @return ErrCode - operation result
     */
    ErrCode GetSupportedFeatures(long &features) override;

    /**
     * @Description Check if supported input feature
     *
     * @param feature - input feature
     * @return true - supported
     * @return false - unsupported
     */
    bool IsFeatureSupported(long feature) override;

    /**
     * @Description set the device name
     *
     * @param deviceName - device name
     * @return ErrCode - operate result
     */
    ErrCode SetP2pDeviceName(const std::string &deviceName) override;

    /**
     * @Description set p2p wifi display info
     *
     * @param wfdInfo - wifi display info
     * @return ErrCode - operate result
     */
    ErrCode SetP2pWfdInfo(const WifiP2pWfdInfo &wfdInfo) override;

private:
    int systemAbilityId_;
    sptr<IWifiP2p> client_;
};
}  // namespace Wifi
}  // namespace OHOS
#endif
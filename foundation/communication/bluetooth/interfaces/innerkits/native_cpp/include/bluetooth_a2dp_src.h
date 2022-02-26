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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief The framework interface and callback function of a2dp source are defined.
 *
 * @since 6
 */

#ifndef BLUETOOTH_A2DP_SRC_H
#define BLUETOOTH_A2DP_SRC_H

#include <vector>

#include "bluetooth_def.h"
#include "bluetooth_types.h"
#include "bluetooth_remote_device.h"

namespace OHOS {
namespace Bluetooth {
/**
 * @brief A2dp source API callback function.
 *
 * @since 6.0
 */
class BluetoothA2dpSrcObserver {
public:
    /**
     * @brief A destructor used to delete the a2dp source Observer instance.
     *
     * @since 6.0
     */
    virtual ~BluetoothA2dpSrcObserver() = default;

    /**
     * @brief The callback function after device's playing state changed.
     *
     * @param device  the remote bluetooth device.
     * @param playingState  the playing state after changing.
     * @param error  the error information.
     * @since 6.0
     */
    virtual void OnPlayingStateChanged(const BluetoothRemoteDevice &device, int playingState, int error)
    {}
};

/**
 * @brief A2dp source API.
 *
 * @since 6.0
 */
class BLUETOOTH_API BluetoothA2dpSrc {
public:
    /**
     * @brief Get a2dp source instance.
     *
     * @return Returns an instance of a2dp source.
     * @since 6.0
     */
    static BluetoothA2dpSrc &GetProfile();

    /**
     * @brief Get devices by connection states.
     *
     * @param states The connection states of the bluetooth device.
     * @return Returns devices that match the connection states.
     * @since 6.0
     */
    std::vector<BluetoothRemoteDevice> GetDevicesByStates(std::vector<int> states) const;

    /**
     * @brief Get device connection state by address.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>A2DP_DISCONNECTED</b> if device connect state is disconnected;
     *         Returns <b>A2DP_DISCONNECTING</b> if device connect state is disconnecting;
     *         Returns <b>A2DP_CONNECTED</b> if device connect state is connected;
     *         Returns <b>A2DP_CONNECTING</b> if device connect state is connecting;
     *         Returns <b>A2DP_INVALID_STATUS</b> if can not find peer device.
     * @since 6.0
     */
    int GetDeviceState(const BluetoothRemoteDevice &device) const;

    /**
     * @brief Get device playing state by address when peer device is on connected.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>1</b> if device is on playing;
     *         Returns <b>0</b> if device is not on playing.
     * @since 6.0
     */
    int GetPlayingState(const BluetoothRemoteDevice &device) const;

    /**
     * @brief Connect to the peer bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>true</b> Perform normal connection processing.
     *         Returns <b>false</b> Target device is on connected,or connecting,
                                    or device is not allowed to connect,or the connection fails.
     * @since 6.0
     */
    bool Connect(const BluetoothRemoteDevice &device);

    /**
     * @brief Disconnect with the peer bluetooth service.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>true</b> if perform normal disconnection processing.
     *         Returns <b>false</b> if target device is on disconnected,or disconnecting,or disconnection fails.
     * @since 6.0
     */
    bool Disconnect(const BluetoothRemoteDevice &device);

    /**
     * @brief Set target device as active device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> Target device has already been active, or perform normal setting processing.
     *         Returns <b>RET_BAD_PARAM</b> Input error.
     *         Returns <b>RET_BAD_STATUS</b> Target device is not on connected, or set fails.
     * @since 6.0
     */
    int SetActiveSinkDevice(const BluetoothRemoteDevice &device);

    /**
     * @brief Get active device.
     * @return Returns active device.
     * @since 6.0
     */
    const BluetoothRemoteDevice &GetActiveSinkDevice() const;

    /**
     * @brief Set connection strategy for peer bluetooth device.
     *        If peer device is connected and the policy is set not allowed,then perform disconnect operation.
     *        If peer device is disconnected and the policy is set allowed,then perform connect operation.
     *
     * @param device The address of the peer bluetooth device.
     * @param strategy The device connect strategy.
     * @return Returns <b>true</b> if the operation is successful.
     *         Returns <b>false</b> if the operation fails.
     * @since 6.0
     */
    bool SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy);

    /**
     * @brief Get connection strategy of peer bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>CONNECTION_ALLOWED</b> if the peer device is allowed to connect.
     *         Returns <b>CONNECTION_FORBIDDEN</b> if the peer device is not allowed to connect.
     *         Returns <b>CONNECTION_UNKNOWN</b> if the connection policy is unknown.
     * @since 6.0
     */
    int GetConnectStrategy(const BluetoothRemoteDevice &device) const;

    /**
     * @brief Set whether enables the optional codec.
     *
     * @param device The address of the bluetooth device.
     * @param isEnable Set true if enables the optional codec and set optional codec's priority high.
     *                 Set false if disables the optional codec and set optional codec's priority low.
     * @since 6.0
     */
    void SwitchOptionalCodecs(const BluetoothRemoteDevice &device, bool isEnable);

    /**
     * @brief Get whether the peer bluetooth device supports optional codec.
     *
     * @param device The address of the bluetooth device.
     * @return Returns <b>A2DP_OPTIONAL_SUPPORT</b> The device supports optional codec.
     *         Returns <b>A2DP_OPTIONAL_NOT_SUPPORT</b> The device dosn't support optional codec.
     *         Returns <b>A2DP_OPTIONAL_SUPPORT_UNKNOWN</b> Don't know if the device support optional codec.
     * @since 6.0
     */
    int GetOptionalCodecsSupportState(const BluetoothRemoteDevice &device) const;

    /**
     * @brief Audio start streaming.
     *
     * @param device The address of the bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_PARAM</b> Input error.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     * @since 6.0
     */
    int StartPlaying(const BluetoothRemoteDevice &device);

    /**
     * @brief Audio suspend streaming.
     *
     * @param device The address of the bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_PARAM</b> Input error.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     * @since 6.0
     */
    int SuspendPlaying(const BluetoothRemoteDevice &device);

    /**
     * @brief Audio stop streaming.
     *
     * @param device The address of the bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_PARAM</b> Input error.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     * @since 6.0
     */
    int StopPlaying(const BluetoothRemoteDevice &device);

    /**
     * @brief Register callback function of framework.
     *
     * @param observer Reference to the a2dp source observer.
     * @since 6.0
     */
    void RegisterObserver(std::shared_ptr<BluetoothA2dpSrcObserver> &observer);

    /**
     * @brief Deregister callback function of framework.
     *
     * @param observer Reference to the a2dp source observer.
     * @since 6.0
     */
    void DeregisterObserver(std::shared_ptr<BluetoothA2dpSrcObserver> &observer);

    /**
     * @brief Set audio configure.
     * @param[in] addr: The address of peer device
     * @param[in] sampleRate: Audio pcm sample rate
     * @param[in] bits: Audio pcm bits
     * @param[in] channel: Number of audio pcm channel
     * @since 6.0
     */
    void SetAudioConfigure(const BluetoothRemoteDevice &addr, uint32_t sampleRate, uint32_t bits, uint8_t channel);

private:
    static BluetoothA2dpSrc a2dpSrcProfile_;
    /**
     * @brief A constructor used to create a a2dp source instance.
     *
     * @since 6.0
     */
    BluetoothA2dpSrc();

    /**
     * @brief A destructor used to delete the a2dp source instance.
     *
     * @since 6.0
     */
    ~BluetoothA2dpSrc();

    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothA2dpSrc);
    BLUETOOTH_DECLARE_IMPL();
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_A2DP_SRC_H

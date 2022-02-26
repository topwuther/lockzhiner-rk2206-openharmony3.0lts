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

#include <sys/socket.h>
#include <string>
#include "securec.h"
#include <unistd.h>
#include "bluetooth_log.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_socket_proxy.h"
#include "bluetooth_socket.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "raw_address.h"

namespace OHOS {
namespace Bluetooth {
struct SppClientSocket::impl {
    impl(const BluetoothRemoteDevice &addr, UUID uuid, SppSocketType type, bool auth);
    impl(int fd, std::string address);
    ~impl()
    {
        if (!proxy_) {
            HILOGI("SppClientSocket::impl:～impl() failed: no proxy_");
            return;
        }

        proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);

        if (fd_ > 0) {
            shutdown(fd_, SHUT_RD);
            shutdown(fd_, SHUT_WR);

            if (isServer_) {
                HILOGE("accept fd closed");
                close(fd_);
                fd_ = -1;
            } else {
                HILOGE("accept fd closed");
            }
        }
    }

    void Close()
    {
        HILOGI("SppClientSocket::Close starts");
        if (socketStatus_ == SOCKET_CLOSED) {
            HILOGE("socket closed");
            return;
        } else {
            socketStatus_ = SOCKET_CLOSED;
            if (fd_ > 0) {
                shutdown(fd_, SHUT_RD);
                shutdown(fd_, SHUT_WR);

                if (isServer_) {
                    HILOGE("accept fd closed");
                    close(fd_);
                    fd_ = -1;
                } else {
                    HILOGE("accept fd closed");
                }
            } else {
                HILOGE("socket not created");
                return;
            }
        }
    }

    bool RecvSocketSignal()
    {
        uint8_t recvStateBuf[1];
#ifdef DARWIN_PLATFORM
        int recvBufSize = recv(fd_, recvStateBuf, sizeof(recvStateBuf), 0);
#else
        int recvBufSize = recv(fd_, recvStateBuf, sizeof(recvStateBuf), MSG_WAITALL);
#endif
        if (recvBufSize <= 0) {
            HILOGE("service closed");
            return false;
        }
        bool state = recvStateBuf[0];

        uint8_t buf[6] = {0};
#ifdef DARWIN_PLATFORM
        int recvAddrSize = recv(fd_, buf, sizeof(buf), 0);
#else
        int recvAddrSize = recv(fd_, buf, sizeof(buf), MSG_WAITALL);
#endif
        if (recvAddrSize <= 0) {
            HILOGE("service closed");
            return false;
        }
        char token[18] = {0};
        BluetoothRawAddress rawAddr {
            token
        };
        std::string address = rawAddr.GetAddress().c_str();
        return state;
    }

    int getSecurityFlags()
    {
        int flags = 0;
        if (auth_) {
            flags |= FLAG_AUTH;
            flags |= FLAG_ENCRYPT;
        }
        return flags;
    }

    InputStream &GetInputStream()
    {
        HILOGI("SppClientSocket::GetInputStream starts");
        if (inputStream_ == nullptr) {
            HILOGE("inputStream is NULL, failed. please Connect");
        }
        return *inputStream_;
    }

    OutputStream &GetOutputStream()
    {
        HILOGI("SppClientSocket::GetOutputStream starts");
        if (outputStream_ == nullptr) {
            HILOGE("outputStream is NULL, failed. please Connect");
        }
        return *outputStream_;
    }

    BluetoothRemoteDevice &GetRemoteDevice()
    {
        HILOGI("SppClientSocket::GetRemoteDevice starts");
        return remoteDevice_;
    }

    bool IsConnected()
    {
        HILOGI("SppClientSocket::IsConnected starts");
        return socketStatus_ == SOCKET_CONNECTED;
    }

    class BluetoothClientSocketDeathRecipient;
    sptr<BluetoothClientSocketDeathRecipient> deathRecipient_;

    sptr<IBluetoothSocket> proxy_;
    std::unique_ptr<InputStream> inputStream_ {
        nullptr
    };
    std::unique_ptr<OutputStream> outputStream_ {
        nullptr
    };
    BluetoothRemoteDevice remoteDevice_;
    UUID uuid_;
    SppSocketType type_;
    std::string address_;
    bool isServer_;
    int fd_;
    bool auth_;
    int socketStatus_;
};

class SppClientSocket::impl::BluetoothClientSocketDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    BluetoothClientSocketDeathRecipient(SppClientSocket::impl &host) : host_(host){};
    ~BluetoothClientSocketDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothClientSocketDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("SppClientSocket::impl::BluetoothClientSocketDeathRecipient::OnRemoteDied starts");
        host_.proxy_ = nullptr;
    }

private:
    SppClientSocket::impl &host_;
};

SppClientSocket::impl::impl(const BluetoothRemoteDevice &addr, UUID uuid, SppSocketType type, bool auth)
    : inputStream_(nullptr),
      outputStream_(nullptr),
      remoteDevice_(addr),
      uuid_(uuid),
      type_(type),
      isServer_(false),
      fd_(-1),
      auth_(auth),
      socketStatus_(SOCKET_INIT)
{
    HILOGI("SppClientSocket::impl:impl(4 parameters) starts");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

    if (!hostRemote) {
        int try_time = 5;
        while (!hostRemote && try_time-- > 0) {
            sleep(1);
            hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
        }
    }

    if (!hostRemote) {
        HILOGI("SppClientSocket::impl:impl() failed: no hostRemote");
        return;
    }

    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_SPP);

    if (!remote) {
        int try_time = 5;
        while (!remote && try_time-- > 0) {
            sleep(1);
            remote = hostProxy->GetProfile(PROFILE_SPP);
        }
    }

    if (!remote) {
        HILOGI("SppClientSocket::impl:impl() failed: no remote");
        return;
    }
    HILOGI("SppClientSocket::impl:impl() remote obtained");

    proxy_ = iface_cast<IBluetoothSocket>(remote);

    deathRecipient_ = new BluetoothClientSocketDeathRecipient(*this);

    if (!proxy_) {
        HILOGE("SppClientSocket:impl: proxy_ is nullptr");
        return;
    }

    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
}

SppClientSocket::impl::impl(int fd, std::string address)
    : inputStream_(std::make_unique<InputStream>(fd)),
      outputStream_(std::make_unique<OutputStream>(fd)),
      remoteDevice_(BluetoothRemoteDevice(address, 0)),
      type_(TYPE_RFCOMM),
      address_(address),
      isServer_(true),
      fd_(fd),
      auth_(false),
      socketStatus_(SOCKET_CONNECTED)
{
    HILOGI("SppClientSocket::impl:impl(2 parameters) starts");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

    if (!hostRemote) {
        HILOGI("SppClientSocket::impl:impl() failed: no hostRemote");
        return;
    }

    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_SPP);

    if (!remote) {
        HILOGI("SppClientSocket::impl:impl() failed: no remote");
        return;
    }
    HILOGI("SppClientSocket::impl:impl() remote obtained");

    proxy_ = iface_cast<IBluetoothSocket>(remote);

    deathRecipient_ = new BluetoothClientSocketDeathRecipient(*this);

    if (!proxy_) {
        HILOGE("SppClientSocket:impl: proxy_ is nullptr");
        return;
    }

    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
}

SppClientSocket::SppClientSocket(const BluetoothRemoteDevice &bda, UUID uuid, SppSocketType type, bool auth)
    : pimpl(new SppClientSocket::impl(bda, uuid, type, auth))
{}

SppClientSocket::SppClientSocket(int fd, std::string address) : pimpl(new SppClientSocket::impl(fd, address))
{}

SppClientSocket::~SppClientSocket()
{}

int SppClientSocket::Connect()
{
    HILOGI("SppClientSocket::Connect starts");
    pimpl->address_ = pimpl->remoteDevice_.GetDeviceAddr();

    std::string tempAddress = pimpl->address_;
    if (!tempAddress.size()) {
        return SPPStatus::SPP_FAILURE;
    }
    if (pimpl->socketStatus_ == SOCKET_CLOSED) {
        HILOGE("socket closed");
        return SPPStatus::SPP_FAILURE;
    }

    bluetooth::Uuid serverUuid = bluetooth::Uuid::ConvertFrom128Bits(pimpl->uuid_.ConvertTo128Bits());

    if (!pimpl->proxy_) {
        HILOGE("SppClientSocket:Connect: proxy_ is nullptr");
        return SPPStatus::SPP_FAILURE;
    }

    pimpl->fd_ =
        pimpl->proxy_->Connect(tempAddress, serverUuid, (int32_t)pimpl->getSecurityFlags(), (int32_t)pimpl->type_);

    if (pimpl->fd_ == -1) {
        HILOGE("connect failed!");
        return SPPStatus::SPP_FAILURE;
    }

    bool recvret = pimpl->RecvSocketSignal();

    pimpl->inputStream_ = std::make_unique<InputStream>(pimpl->fd_);
    pimpl->outputStream_ = std::make_unique<OutputStream>(pimpl->fd_);

    if (!recvret) {
        HILOGE("connect failed!");
        return SPPStatus::SPP_FAILURE;
    }
    pimpl->socketStatus_ = SOCKET_CONNECTED;
    return SPPStatus::SPP_SUCCESS;
}

void SppClientSocket::Close()
{
    HILOGI("SppClientSocket::Close starts");
    return pimpl->Close();
}

InputStream &SppClientSocket::GetInputStream()
{
    HILOGI("SppClientSocket::GetInputStream starts");
    return pimpl->GetInputStream();
}

OutputStream &SppClientSocket::GetOutputStream()
{
    HILOGI("SppClientSocket::GetOutputStream starts");
    return pimpl->GetOutputStream();
}

BluetoothRemoteDevice &SppClientSocket::GetRemoteDevice()
{
    HILOGI("SppClientSocket::GetRemoteDevice starts");
    return pimpl->GetRemoteDevice();
}

bool SppClientSocket::IsConnected() const
{
    HILOGI("SppClientSocket::IsConnected starts");
    return pimpl->IsConnected();
}

struct SppServerSocket::impl {
    impl(const std::string &name, UUID uuid, SppSocketType type, bool auth);
    ~impl()
    {
        if (!proxy_) {
            HILOGE("SppServerSocket:impl: proxy_ is nullptr");
            return;
        }

        proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);

        if (fd_ > 0) {
            shutdown(fd_, SHUT_RD);
            shutdown(fd_, SHUT_WR);
        }
    }

    int Listen()
    {
        HILOGI("SppServerSocket::Listen() starts");
        if (socketStatus_ == SOCKET_CLOSED) {
            HILOGE("Listen is failed, bluetooth is off!");
            return SPPStatus::SPP_FAILURE;
        }

        bluetooth::Uuid serverUuid = bluetooth::Uuid::ConvertFrom128Bits(uuid_.ConvertTo128Bits());

        if (!proxy_) {
            HILOGE("SppServerSocket:Listen: proxy_ is nullptr");
            return SPPStatus::SPP_FAILURE;
        }

        fd_ = proxy_->Listen(name_, serverUuid, (int32_t)getSecurityFlags(), (int32_t)type_);
        if (fd_ == -1) {
            HILOGE("Listen is failed, bluetooth is off!");
            return SPPStatus::SPP_FAILURE;
        }

        if (socketStatus_ == SOCKET_INIT) {
            socketStatus_ = SOCKET_LISTENING;
        } else {
            HILOGE("Listen is failed, bluetooth is off!");
            close(fd_);
        }

        return SPPStatus::SPP_SUCCESS;
    }

    int getSecurityFlags()
    {
        int flags = 0;
        if (auth_) {
            flags |= FLAG_AUTH;
            flags |= FLAG_ENCRYPT;
        }
        return flags;
    }

    std::unique_ptr<SppClientSocket> Accept(int timeout)
    {
        HILOGI("SppServerSocket::Accept() starts");
        if (socketStatus_ != SOCKET_LISTENING) {
            HILOGE("socket is not in listen state");
            return nullptr;
        }
        struct timeval time = {timeout, 0};
        setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, (const char *)&time, sizeof(time));
        setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&time, sizeof(time));

        acceptFd_ = RecvSocketFd();
        HILOGE("RecvSocketFd acceptFd: %{public}d", acceptFd_);
        if (acceptFd_ <= 0) {
            return nullptr;
        }
        if (timeout > 0) {
            time = {0, 0};
            setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, (const char *)&time, sizeof(time));
            setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&time, sizeof(time));
        }

        std::unique_ptr<SppClientSocket> clientSocket = std::make_unique<SppClientSocket>(acceptFd_, acceptAddress_);

        return clientSocket;
    }

    int RecvSocketFd()
    {
        HILOGI("SppServerSocket::RecvSocketFd() starts");
        int rv = 0;
        int cfd = -1;
        int clientFd = -1;
        char ccmsg[CMSG_SPACE(sizeof(cfd))];
        char buffer[10];
        struct iovec io = {.iov_base = buffer, .iov_len = sizeof(buffer)};
        struct msghdr msg;
        memset_s(&msg, sizeof(msg), 0, sizeof(msg));
        msg.msg_control = ccmsg;
        msg.msg_controllen = sizeof(ccmsg);
        msg.msg_iov = &io;
        msg.msg_iovlen = 1;

#ifdef DARWIN_PLATFORM
        rv = recvmsg(fd_, &msg, 0);
#else
        rv = recvmsg(fd_, &msg, MSG_NOSIGNAL);
#endif
        if (rv == -1) {
            HILOGE("[sock] recvmsg error  %{public}d,  fd: %{public}d", errno, fd_);
            return SPPStatus::SPP_FAILURE;
        }
        struct cmsghdr *cmptr = CMSG_FIRSTHDR(&msg);
        if ((cmptr != NULL) && (cmptr->cmsg_len == CMSG_LEN(sizeof(int)))) {
            if (cmptr->cmsg_level != SOL_SOCKET || cmptr->cmsg_type != SCM_RIGHTS) {
                HILOGE("[sock] control level: %{public}d", cmptr->cmsg_level);
                HILOGE("[sock] control type: %{public}d", cmptr->cmsg_type);
                return SPPStatus::SPP_FAILURE;
            }
            clientFd = *((int *)CMSG_DATA(cmptr));
        } else {
            return SPPStatus::SPP_FAILURE;
        }
        uint8_t recvBuf[rv];
        memset_s(&recvBuf, sizeof(recvBuf),0, sizeof(recvBuf));
        memcpy_s(recvBuf, rv, (uint8_t *)msg.msg_iov[0].iov_base, rv);

        uint8_t buf[6] = {0};
        memcpy_s(buf, sizeof(buf), &recvBuf[1], sizeof(buf));

        char token[18] = {0};
        BluetoothRawAddress rawAddr {
            token
        };
        acceptAddress_ = rawAddr.GetAddress().c_str();
        return clientFd;
    }

    void Close()
    {
        HILOGI("SppServerSocket::Close() starts");
        if (socketStatus_ == SOCKET_CLOSED) {
            return;
        } else {
            socketStatus_ = SOCKET_CLOSED;
            if (fd_ > 0) {
                shutdown(fd_, SHUT_RD);
                shutdown(fd_, SHUT_WR);
                fd_ = -1;
                return;
            } else {
                HILOGE("socket not created");
                return;
            }
        }
    }

    const std::string &GetStringTag()
    {
        HILOGI("SppServerSocket::GetStringTag() starts");
        socketServiceType_ = "ServerSocket: Type: TYPE_RFCOMM";
        socketServiceType_.append(" ServerName: ").append(name_);
        return socketServiceType_;
    }

    class BluetoothServerSocketDeathRecipient;
    sptr<BluetoothServerSocketDeathRecipient> deathRecipient_;

    sptr<IBluetoothSocket> proxy_;
    UUID uuid_;
    SppSocketType type_;
    bool auth_;
    int fd_;
    int socketStatus_;
    std::string name_ {
        ""
        };
    int acceptFd_;
    std::string acceptAddress_;
    std::string socketServiceType_ {
        ""
    };
};

class SppServerSocket::impl::BluetoothServerSocketDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    BluetoothServerSocketDeathRecipient(SppServerSocket::impl &host) : host_(host){};
    ~BluetoothServerSocketDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothServerSocketDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("SppServerSocket::impl::BluetoothServerSocketDeathRecipient::OnRemoteDied starts");
        host_.proxy_ = nullptr;
    }

private:
    SppServerSocket::impl &host_;
};

SppServerSocket::impl::impl(const std::string &name, UUID uuid, SppSocketType type, bool auth)
    : uuid_(uuid), type_(type), auth_(auth), fd_(-1), socketStatus_(SOCKET_INIT), name_(name)
{
    HILOGI("SppServerSocket::impl::impl(4 parameters) starts");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_SPP);

    if (!remote) {
        HILOGI("SppServerSocket::impl:impl() failed: no remote");
    }
    HILOGI("SppServerSocket::impl:impl() remote obtained");

    proxy_ = iface_cast<IBluetoothSocket>(remote);

    if (!proxy_) {
        HILOGI("SppServerSocket::impl:impl() failed: no proxy_");
        return;
    }

    deathRecipient_ = new BluetoothServerSocketDeathRecipient(*this);
    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
}

SppServerSocket::SppServerSocket(const std::string &name, UUID uuid, SppSocketType type, bool auth)
    : pimpl(new SppServerSocket::impl(name, uuid, type, auth))
{
    HILOGI("SppServerSocket::SppServerSocket(4 parameters) starts");
    int ret = pimpl->Listen();
    if (ret < 0) {
        HILOGE("bind listen failed!");
    }
}

SppServerSocket::~SppServerSocket()
{}

std::unique_ptr<SppClientSocket> SppServerSocket::Accept(int timeout)
{
    HILOGI("SppServerSocket::Accept starts");
    return pimpl->Accept(timeout);
}

void SppServerSocket::Close()
{
    HILOGI("SppServerSocket::Close()) starts");
    return pimpl->Close();
}

const std::string &SppServerSocket::GetStringTag()
{
    HILOGI("SppServerSocket::GetStringTag:() starts");
    return pimpl->GetStringTag();
}

SppClientSocket *SocketFactory::BuildInsecureRfcommDataSocketByServiceRecord(
    const BluetoothRemoteDevice &device, const UUID &uuid)
{
    HILOGI("SocketFactory::BuildInsecureRfcommDataSocketByServiceRecord starts");
    if (device.IsValidBluetoothRemoteDevice()) {
        return new SppClientSocket(device, uuid, TYPE_RFCOMM, false);
    } else {
        HILOGE("[sock] Device is not valid.");
        return nullptr;
    }
}

SppClientSocket *SocketFactory::BuildRfcommDataSocketByServiceRecord(
    const BluetoothRemoteDevice &device, const UUID &uuid)
{
    HILOGI("SocketFactory::BuildRfcommDataSocketByServiceRecord starts");
    if (device.IsValidBluetoothRemoteDevice()) {
        return new SppClientSocket(device, uuid, TYPE_RFCOMM, true);
    } else {
        HILOGE("[sock] Device is not valid.");
        return nullptr;
    }
}

SppServerSocket *SocketFactory::DataListenInsecureRfcommByServiceRecord(const std::string &name, const UUID &uuid)
{
    HILOGI("SocketFactory::DataListenInsecureRfcommByServiceRecord starts");
    return new SppServerSocket(name, uuid, TYPE_RFCOMM, false);
}

SppServerSocket *SocketFactory::DataListenRfcommByServiceRecord(const std::string &name, const UUID &uuid)
{
    HILOGI("SocketFactory::DataListenRfcommByServiceRecord starts");
    return new SppServerSocket(name, uuid, TYPE_RFCOMM, true);
}
}  // namespace Bluetooth
}  // namespace OHOS
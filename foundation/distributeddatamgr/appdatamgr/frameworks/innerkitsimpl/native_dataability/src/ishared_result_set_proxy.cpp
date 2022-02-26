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

#include "ishared_result_set_proxy.h"
#include "rdb_errno.h"
#include "iremote_proxy.h"
#include "logger.h"
namespace OHOS::NativeRdb {
std::function<std::shared_ptr<AbsSharedResultSet>(
    MessageParcel &parcel)> ISharedResultSet::consumerCreator_ = ISharedResultSetProxy::CreateProxy;
BrokerDelegator<ISharedResultSetProxy> ISharedResultSetProxy::delegator_;
ISharedResultSetProxy::ISharedResultSetProxy(const sptr<OHOS::IRemoteObject> &impl)
    : IRemoteProxy<ISharedResultSet>(impl)
{
}

std::shared_ptr<AbsSharedResultSet> ISharedResultSetProxy::CreateProxy(MessageParcel &parcel)
{
    sptr<IRemoteObject> remoter = parcel.ReadRemoteObject();
    if (remoter == nullptr) {
        return nullptr;
    }
    sptr<ISharedResultSet> result = iface_cast<ISharedResultSet>(remoter);
    return std::make_shared<ISharedResultSetClient>(result, parcel);
}

int ISharedResultSetProxy::GetAllColumnNames(std::vector<std::string> &columnNames)
{
    LOG_DEBUG("GetAllColumnNames Begin");
    MessageParcel request;
    request.WriteInterfaceToken(GetDescriptor());
    MessageParcel reply;
    MessageOption msgOption;
    int errCode = Remote()->SendRequest(FUNC_GET_ALL_COLUMN_NAMES, request, reply, msgOption);
    if (errCode != 0) {
        LOG_ERROR("GetAllColumnNames IPC Error %{public}x", errCode);
        return -errCode;
    }
    errCode = reply.ReadInt32();
    if (errCode != E_OK) {
        LOG_ERROR("GetAllColumnNames Reply Error %{public}d", errCode);
        return errCode;
    }
    if (!reply.ReadStringVector(&columnNames)) {
        return E_INVALID_PARCEL;
    }
    return E_OK;
}

int ISharedResultSetProxy::GetRowCount(int &count)
{
    LOG_DEBUG("GetRowCount Begin");
    MessageParcel request;
    request.WriteInterfaceToken(GetDescriptor());
    MessageParcel reply;
    MessageOption msgOption;
    int errCode = Remote()->SendRequest(FUNC_GET_ROW_COUNT, request, reply, msgOption);
    if (errCode != 0) {
        LOG_ERROR("GetRowCount IPC Error %{public}x", errCode);
        return -errCode;
    }
    errCode = reply.ReadInt32();
    if (errCode != E_OK) {
        LOG_ERROR("GetRowCount Reply Error %{public}d", errCode);
        return errCode;
    }
    count = reply.ReadInt32();
    LOG_DEBUG("GetRowCount count %{public}d", count);
    return E_OK;
}

bool ISharedResultSetProxy::OnGo(int oldRowIndex, int newRowIndex)
{
    LOG_DEBUG("OnGo Begin");
    MessageParcel request;
    request.WriteInterfaceToken(GetDescriptor());
    request.WriteInt32(oldRowIndex);
    request.WriteInt32(newRowIndex);
    MessageParcel reply;
    MessageOption msgOption;
    int errCode = Remote()->SendRequest(FUNC_ON_GO, request, reply, msgOption);
    if (errCode != 0) {
        LOG_ERROR("OnGo IPC Error %{public}x", errCode);
        return -errCode;
    }
    return reply.ReadBool();
}

int ISharedResultSetProxy::Close()
{
    LOG_DEBUG("Close Begin");
    MessageParcel request;
    request.WriteInterfaceToken(GetDescriptor());
    MessageParcel reply;
    MessageOption msgOption;
    int errCode = Remote()->SendRequest(FUNC_CLOSE, request, reply, msgOption);
    if (errCode != 0) {
        LOG_ERROR("Close IPC Error %{public}x", errCode);
        return -errCode;
    }
    return reply.ReadInt32();
}

ISharedResultSetProxy::ISharedResultSetClient::ISharedResultSetClient(sptr<ISharedResultSet> remote,
    MessageParcel &parcel) : remote_(remote)
{
    Unmarshalling(parcel);
}
}
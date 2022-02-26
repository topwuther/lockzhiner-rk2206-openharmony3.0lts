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

#ifndef DATAABILITY_I_SHARED_RESULT_SET_PROXY_H
#define DATAABILITY_I_SHARED_RESULT_SET_PROXY_H
#include "ishared_result_set.h"
#include "iremote_proxy.h"
#include "rdb_errno.h"

namespace OHOS::NativeRdb {
class ISharedResultSetProxy : public IRemoteProxy<ISharedResultSet> {
public:
    static std::shared_ptr<AbsSharedResultSet> CreateProxy(MessageParcel &parcel);
    explicit ISharedResultSetProxy(const sptr<IRemoteObject> &impl);
    virtual ~ISharedResultSetProxy() = default;
    int GetAllColumnNames(std::vector<std::string> &columnNames) override;
    int GetRowCount(int &count) override;
    bool OnGo(int oldRowIndex, int newRowIndex) override;
    int Close() override;
private:
    class ISharedResultSetClient : public AbsSharedResultSet {
    public:
        ISharedResultSetClient(sptr<ISharedResultSet> remote, MessageParcel &parcel);
        int GetAllColumnNames(std::vector<std::string> &columnNames) override
        {
            int errCode = E_OK;
            if (columnNames_.empty()) {
                errCode = remote_->GetAllColumnNames(columnNames_);
            }
            columnNames = columnNames_;
            return errCode;
        }

        int GetRowCount(int &count) override
        {
            int errCode = E_OK;
            if (rowCount_ < 0) {
                errCode = remote_->GetRowCount(rowCount_);
            }
            count = rowCount_;
            return errCode;
        }

        bool OnGo(int oldRowIndex, int newRowIndex) override
        {
            return remote_->OnGo(oldRowIndex, newRowIndex);
        }

        int Close() override
        {
            AbsSharedResultSet::Close();
            return remote_->Close();
        }
    private:
        sptr<ISharedResultSet> remote_;
        std::vector<std::string> columnNames_;
        int32_t rowCount_ = -1;
    };
    static BrokerDelegator<ISharedResultSetProxy> delegator_;
};
}
#endif // DATAABILITY_I_SHARED_RESULT_SET_PROXY_H

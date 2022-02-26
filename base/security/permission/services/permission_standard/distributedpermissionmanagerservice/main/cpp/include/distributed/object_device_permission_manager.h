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
#ifndef OBJECT_DEVICE_PERMISSON_MANAGER_H
#define OBJECT_DEVICE_PERMISSON_MANAGER_H
#include <string>
#include <set>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "permission_log.h"
#include "distributed_data_validator.h"
#include "object_device_permission_repository.h"
#include "monitor_manager.h"
#include "data_validator.h"
#include "constant.h"
#include "external_deps.h"
#include "device_info_manager.h"
#include "device_info.h"
#include "permission_bms_manager.h"
#include "user_handle_manager.h"
#include "permission_log.h"
#include "base_remote_command.h"
#include "sync_uid_permission_command.h"
#include "remote_command_manager.h"
#include "delete_uid_permission_command.h"

namespace OHOS {
namespace Security {
namespace Permission {
/**
 * Object device permission manager.<br/>
 * This class manage the subscription information between which device copied local uid's permission.
 */
class ObjectDevicePermissionManager {
public:
    virtual ~ObjectDevicePermissionManager();
    /**
     * Get singleton instance.
     *
     * @return This singleton instance.
     */
    static ObjectDevicePermissionManager &GetInstance();
    /**
     * If not successfully recovered, recover from file.
     */
    void Init();
    // clear object device's caches
    void Clear();

    /**
     * Operation function. Ask RemoteExecutorManager to execute SyncUidPermissionCommand or DeleteUidPermissionCommand
     * for all deviceIds that has a relationship with given uid.
     *
     * @param uid The operation uid in local device.
     * @return SUCCESS: 0; FAILURE: -1.
     */
    int32_t NotifyPermissionChanged(const int32_t uid);
    /**
     * Operation function. Add a new uid to deviceId relationship, store in the repository.
     * <p>
     * Check input parameter first. If:
     * <ol>
     * <li>DeviceId or uid is invalid, will return <b>FAILURE</b>, and do not add.
     * <li>Uid is the special ruid, which is defined by DuidTranslator are 0 or 1000, will return <b>SUCCESS</b>, and do
     * not add.
     * </ol>
     * The survivor will be saved to repository.
     *
     * @param deviceId The operation device id, unique, for remote device.
     * @param uid The operation uid, in local device.
     * @return SUCCESS: 0; FAILURE: -1
     */
    int32_t AddNotifyPermissionMonitorUid(const std::string &deviceId, const int32_t uid);
    /**
     * Notify current device sync {@code uid}'s permissions to object device while distributed business is scheduling.
     *
     * @param deviceId The object device's deviceId, for remote device.
     * @param uid The application uid in local device.
     * @param packageName The package name.
     * @return Result code indicates sync permission successfully, SUCCESS: 0, FAILURE: -1, INVALID_DEVICE_ID: -3.
     */
    int32_t NotifySyncPermission(std::string &deviceId, int32_t uid, std::string &packageName);
    /**
     * verify device whether has been granted permission with the given name to access the remote device.
     *
     * @param permission permission name.
     * @param deviceId Remote device id, not null.
     * @param pid The Caller Pid
     * @param uid The Caller Uid
     * @return 0 Granted, -1 Denied.
     */
    int32_t VerifyPermissionFromRemote(
        const std::string &permission, const std::string &deviceId, const int32_t pid, const int32_t uid);
    /**
     * Reset the sensitive permission that have been granted by object device
     *
     * @param deviceId Remote device id.
     * @param uid local uid.
     * @param permissions the sensitive permission that have been granted by object device
     */
    void ResetGrantSensitivePermission(std::string &deviceId, int32_t uid, std::set<std::string> &permissions);
    /**
     * Delete all uid relationship with this deviceId.
     *
     * @param deviceId The operation deviceId, unique, for remote device.
     */
    void RemoveObject(const std::string &deviceId);
    /**
     * Operation function. Ask RemoteExecutorManager to asynchronous execute DeleteUidPermissionCommand for all
     * deviceIds that has a relationship with given uid which has a relationship with given userId, remove them from
     * repository anyway.
     *
     * @param userId The operation userId in local device.
     * @return SUCCESS: 0; FAILURE: -1
     */
    int32_t RemoveNotifyPermissionMonitorUserId(const int32_t userId);

private:
    std::mutex mutex_;
    std::atomic<bool> recovered_;
    std::atomic<bool> registered_;
    std::recursive_mutex remove_notify_permission_monitor_user_id_locl_;
    sptr<Permission::IPermissionManager> iPermissionManager_;
    sptr<AppExecFwk::IBundleMgr> iBundleManager_;

    /**
     * Rom may forward a special uid which is -1, we will ignore this.
     */
    static const int32_t SPECIAL_INVALID_UID = -1;
    /**
     * Max timeout limit when notify permission changed.
     */
    static const int32_t MAX_NOTIFY_PERMISSION_CHANGED_TIMEOUT = 1000;

    static const int32_t NOTIFY_PERMISSION_CHANGED_TIMEOUT = 200;

private:
    ObjectDevicePermissionManager();
    int32_t ProcessDeviceCommandImmediately(const int32_t uid, const std::set<std::string> &devicesToSync);
    /**
     * Operation function. Ask RemoteExecutorManager to asynchronous execute DeleteUidPermissionCommand for all
     * deviceIds that has a relationship with given uid, remove them from repository anyway.
     *
     * @param uid The operation uid in local device.
     * @return SUCCESS: 0; FAILURE: -1
     */
    int32_t RemoveNotifyPermissionMonitorUid(const int32_t uid);
    int32_t VerifyPermissionFromRemoteInner(
        const std::string &permission, const std::string &deviceId, const int32_t pid, const int32_t uid);
    bool IsValidVerificationParams(const std::string &permission, const std::string &deviceId, const int32_t uid);
};
class CountDownLatch {
public:
    CountDownLatch(uint32_t count) : count_(count)
    {}
    uint32_t GetCount()
    {
        std::lock_guard<std::mutex> guard(mutex_);
        return count_;
    }
    void CountDown() noexcept
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (count_ == 0) {
            return;
        }
        --count_;
        if (count_ == 0) {
            cv_.notify_all();
        }
    }

    void Await(long time) noexcept
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, std::chrono::milliseconds(time), [this] { return count_ == 0; });
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    uint32_t count_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // OBJECT_DEVICE_PERMISSON_MANAGER_H
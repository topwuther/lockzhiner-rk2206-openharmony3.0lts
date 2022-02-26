# samgr<a name="EN-US_TOPIC_0000001162068341"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section161941989596)
-   [Usage](#section1312121216216)
-   [Repositories Involved](#section1371113476307)

## Introduction<a name="section11660541593"></a>

The  **samgr**  module is a core module of OpenHarmony. It provides functions related to system abilities \(also called system services\), including system ability \(SA\) startup, registration, and query.

![](figures/en-us_image_0000001115820566.png)

## Directory Structure<a name="section161941989596"></a>

```
/foundation/distributedschedule/services/samgr/
├── native             
│   ├── BUILD.gn  # Compilation script
│   ├── include   # Header files
│   ├── samgr.rc  # RC file for starting samgr
│   ├── source    # Source code
│   ├── test      # Test code
```

## Usage<a name="section1312121216216"></a>

1.  After receiving the registration message from the SA framework, the samgr service saves information about the particular SA in the local cache.

    ```
    int32_t SystemAbilityManager::AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp)
    {
        if (!CheckInputSysAbilityId(systemAbilityId) || ability == nullptr || (!CheckCapability(extraProp.capability))) {
            HILOGE("AddSystemAbilityExtra input params is invalid.");
            return ERR_INVALID_VALUE;
        }
        {
            unique_lock<shared_mutex> writeLock(abilityMapLock_);
            auto saSize = abilityMap_.size();
            if (saSize >= MAX_SERVICES) {
                HILOGE("map size error, (Has been greater than %zu)", saSize);
                return ERR_INVALID_VALUE;
            }
            SAInfo saInfo;
            saInfo.remoteObj = ability;
            saInfo.isDistributed = extraProp.isDistributed;
            saInfo.capability = extraProp.capability;
            saInfo.permission = Str16ToStr8(extraProp.permission);
            abilityMap_[systemAbilityId] = std::move(saInfo);
            HILOGI("insert %{public}d. size : %zu,", systemAbilityId, abilityMap_.size());
        }
        if (abilityDeath_ != nullptr) {
            ability->AddDeathRecipient(abilityDeath_);
        }
        FindSystemAbilityManagerNotify(systemAbilityId, ADD_SYSTEM_ABILITY_TRANSACTION);
        u16string strName = Str8ToStr16(to_string(systemAbilityId));
        if (extraProp.isDistributed && dBinderService_ != nullptr) {
            dBinderService_->RegisterRemoteProxy(strName, ability);
            HILOGD("AddSystemAbility RegisterRemoteProxy, serviceId is %{public}d", systemAbilityId);
        }
        if (systemAbilityDataStorage_ == nullptr) {
            HILOGE("AddSystemAbility systemAbilityDataStorage not init!");
            return ERR_NO_INIT;
        }
        if (extraProp.isDistributed) {
            systemAbilityDataStorage_->ClearSyncRecords();
            DoInsertSaData(strName, ability, extraProp);
        }
    }
    ```

2.  If the SA requested by the SA framework is a local SA, the samgr service searches for the proxy object of the SA based on the SA ID and then returns the proxy object to the SA framework.

    ```
    sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId)
    {
        if (!CheckInputSysAbilityId(systemAbilityId)) {
            return nullptr;
        }
    
        std::string selectedDeviceId;
        if (CheckRemoteSa(to_string(systemAbilityId), selectedDeviceId)) {
           return CheckSystemAbility(systemAbilityId, selectedDeviceId);
        }
    
        shared_lock<shared_mutex> readLock(abilityMapLock_);
        auto iter = abilityMap_.find(systemAbilityId);
        if (iter != abilityMap_.end()) {
            auto callingUid = IPCSkeleton::GetCallingUid();
            if (IsSystemApp(callingUid) || CheckPermission(iter->second.permission)) {
                 return iter->second.remoteObj;
            }
            return nullptr;
        }
        return nullptr;
    }
    ```


## Repositories Involved<a name="section1371113476307"></a>

Distributed Scheduler subsystem

distributedschedule\_dms\_fwk

distributedschedule\_safwk

**distributedschedule\_samgr**

distributedschedule\_safwk\_lite

hdistributedschedule\_samgr\_lite

distributedschedule\_dms\_fwk\_lite


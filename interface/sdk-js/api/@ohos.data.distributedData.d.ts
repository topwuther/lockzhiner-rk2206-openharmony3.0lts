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

import { AsyncCallback, Callback } from './basic';

declare namespace distributedData {
    /**
     * Provides configuration information for {@link KVManager} instances,
     * including the caller's package name and distributed network type.
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @devices phone, tablet
     * @since 7
     */
    interface KVManagerConfig {
        /**
         * Indicates the user information
         */
        userInfo: UserInfo;

        /**
         * Indicates the bundleName
         */
        bundleName: string;
    }

    /**
     * Manages user information.
     *
     * <p>This class provides methods for obtaining the user ID and type, setting the user ID and type,
     * and checking whether two users are the same.
     *
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @since 7
     */
    interface UserInfo {
        /** Indicates the user ID to set */
        userId?: string;

        /** Indicates the user type to set */
        userType?: UserType;
    }

    /**
     * Enumerates user types.
     *
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @since 7
     */
    enum UserType {
        /** Indicates a user that logs in to different devices using the same account. */
        SAME_USER_ID = 0
    }

    /**
     * KVStore constants
     */
    namespace Constants {
        /**
        * max key length.
        */
        const MAX_KEY_LENGTH = 1024;

        /**
         * max value length.
         */
        const MAX_VALUE_LENGTH = 4194303;

        /**
         * max device coordinate key length.
         */
        const MAX_KEY_LENGTH_DEVICE = 896;

        /**
         * max store id length.
         */
        const MAX_STORE_ID_LENGTH = 128;

        /**
         * max query length.
         */
        const MAX_QUERY_LENGTH = 512000;

        /**
         * max batch operation size.
         */
        const MAX_BATCH_SIZE = 128;
    }

    /**
     * Indicates the {@code ValueType}.
     *
     * <p>{@code ValueType} is obtained based on the value.
     *
     * @since 7
     */
    enum ValueType {
        /** Indicates that the value type is string. */
        STRING = 0,

        /** Indicates that the value type is int. */
        INTEGER = 1,

        /** Indicates that the value type is float. */
        FLOAT = 2,

        /** Indicates that the value type is byte array. */
        BYTE_ARRAY = 3,

        /** Indicates that the value type is boolean. */
        BOOLEAN = 4,

        /** Indicates that the value type is double. */
        DOUBLE = 5
    }

    /**
     * Obtains {@code Value} objects stored in a {@link KVStore} database.
     *
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @devices phone, tablet
     * @since 7
     */
    interface Value {
        /**
         * Indicates value type
         *
         * @see ValueType
         * @type {number}
         * @memberof Value
         */
        type: ValueType;
        value: Uint8Array | string | number | boolean;
    }

    /**
     * Provides key-value pairs stored in the distributed database.
     *
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @devices phone, tablet
     * @since 7
     */
    interface Entry {
        key: string;
        value: Value;
    }

    /**
     * Receives notifications of all data changes, including data insertion, update, and deletion.
     *
     * <p>If you have subscribed to {@code KVStore}, you will receive data change notifications and obtain the changed data
     * from the parameters in callback methods upon data insertion, update, or deletion.
     *
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @devices phone, tablet
     * @since 7
     */
    interface ChangeNotification {
        /** Indicates data addition records. */
        insertEntries: Entry[];
        /** Indicates data update records. */
        updateEntries: Entry[];
        /** Indicates data deletion records. */
        deleteEntries: Entry[];
        /** Indicates from device id. */
        deviceId: string;
    }

    /**
     * Indicates the database synchronization mode.
     *
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @since 7
     */
    enum SyncMode {
        /** Indicates that data is only pulled from the remote end. */
        PULL_ONLY = 0,
        /** Indicates that data is only pushed from the local end. */
        PUSH_ONLY = 1,
        /** Indicates that data is pushed from the local end, and then pulled from the remote end. */
        PUSH_PULL = 2
    }

    /**
     * Describes the subscription type.
     *
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @devices phone, tablet
     * @since 7
     */
    enum SubscribeType {
        /** Subscription to local data changes */
        SUBSCRIBE_TYPE_LOCAL = 0,

        /** Subscription to remote data changes */
        SUBSCRIBE_TYPE_REMOTE = 1,

        /** Subscription to both local and remote data changes */
        SUBSCRIBE_TYPE_ALL = 2,
    }

    /**
     * Describes the {@code KVStore} type.
     *
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @since 7
     */
    enum KVStoreType {
        /** Device-collaborated database, as specified by {@code DeviceKVStore} */
        DEVICE_COLLABORATION = 0,

        /** Single-version database, as specified by {@code SingleKVStore} */
        SINGLE_VERSION = 1,

        /** Multi-version database, as specified by {@code MultiKVStore} */
        MULTI_VERSION = 2,
    }

    /**
     * Describes the {@code KVStore} type.
     *
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @since 7
     */
    enum SecurityLevel {
        /**
         * NO_LEVEL: mains not set the security level.
         *
         * @since 7
         */
        NO_LEVEL = 0,

        /**
         * S0: mains the db is public.
         * There is no impact even if the data is leaked.
         *
         * @since 7
         */
        S0 = 1,

        /**
         * S1: mains the db is low level security
         * There are some low impact, when the data is leaked.
         *
         * @since 7
         */
        S1 = 2,

        /**
         * S2: mains the db is middle level security
         * There are some major impact, when the data is leaked.
         *
         * @since 7
         */
        S2 = 3,

        /**
         * S3: mains the db is high level security
         * There are some severity impact, when the data is leaked.
         *
         * @since 7
         */
        S3 = 5,

        /**
         * S4: mains the db is critical level security
         * There are some critical impact, when the data is leaked.
         *
         * @since 7
         */
        S4 = 6,
    }

    /**
     * Provides configuration options for creating a {@code KVStore}.
     *
     * <p>You can determine whether to create another database if a {@code KVStore} database is missing,
     * whether to encrypt the database, and the database type.
     *
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @since 7
     */
    interface Options {
        createIfMissing?: boolean;
        encrypt?: boolean;
        backup?: boolean;
        autoSync?: boolean;
        kvStoreType?: KVStoreType;
        securityLevel?: SecurityLevel;
    }

    /**
     * Represents a key-value distributed database and provides methods for adding, deleting, modifying, querying,
     * and subscribing to distributed data.
     *
     * <p>You can create distributed databases of different types by {@link KVManager#getKVStore (Options, String)}
     * with input parameter {@code Options}. Distributed database types are defined in {@code KVStoreType},
     * including {@code SingleKVStore}.
     *
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @devices phone, tablet
     * @since 7
     * @version 1
     */
    interface KVStore {
        /**
         * Writes a key-value pair of the byte array type into the {@code KVStore} database.
         *
         * @param key Indicates the key. The length must be less than {@code MAX_KEY_LENGTH}.
         * Spaces before and after the key will be cleared.
         * @param value Indicates the byte array, which must be less than 4 MB.
         * @return Returns the error code of databases.
         * @since 7
         */
        put(key: string, value: Uint8Array | string | number | boolean, callback: AsyncCallback<void>): void;
        put(key: string, value: Uint8Array | string | number | boolean): Promise<void>;

        /**
         * Deletes the key-value pair based on a specified key.
         *
         * @param key Indicates the key. The length must be less than {@code MAX_KEY_LENGTH}.
         * Spaces before and after the key will be cleared.
         * @return Returns the error code of databases.
         * @since 7
         */
        delete(key: string, callback: AsyncCallback<void>): void;
        delete(key: string): Promise<void>;

        /**
         * turn on from the data change notify based on the specified {@code subscribeType}.
         *
         * @param type Indicates the subscription type, which is defined in {@code SubscribeType}.
         * @param observer Indicates the observer of data change events in the distributed database.
         * @return Returns the error code of databases.
         * @since 7
         */
        on(event: 'dataChange', type: SubscribeType, observer: Callback<ChangeNotification>): void;

        /**
         * Subscribe to the notification of store synchronization completion.
         *
         * <p>Sync result is returned through asynchronous callback.
         *
         * @param syncCallback Indicates the callback used to send the synchronization result to the caller.
         * @return Returns the {@code number} object.
         * @since 7
         */
        on(event: 'syncComplete', syncCallback: Callback<Array<[string, number]>>): void;
    }

    /**
     * Provides methods related to single-version distributed databases.
     *
     * <p>To create a {@code SingleKVStore} database,
     * you can use the {@link data.distributed.common.KVManager#getKVStore​(Options, String)} method
     * with {@code KVStoreType} set to {@code SINGLE_VERSION} for the input parameter {@code Options}.
     * This database synchronizes data to other databases in time sequence.
     * The {@code SingleKVStore} database does not support
     * synchronous transactions, or data search using snapshots.
     *
     * @devices phone, tablet
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @since 7
     * @version 1
     */
    interface SingleKVStore extends KVStore {
        /**
         * Obtains the value of a specified key.
         *
         * @param key Indicates the key of the value to be queried.
         * @return Returns the value matching the given criteria.
         * @throws Throws exception if any of the following errors occurs: {@code INVALID_ARGUMENT},
         * {@code SERVER_UNAVAILABLE}, {@code IPC_ERROR}, {@code DB_ERROR}, and {@code KEY_NOT_FOUND}.
         * @since 7
         */
        get(key: string, callback: AsyncCallback<Uint8Array | string | boolean | number>): void;
        get(key: string): Promise<Uint8Array | string | boolean | number>;

        /**
         * Synchronizing KVStore Data Between Devices.
         *
         * @param deviceIdList Indicates the {@code string}.
         * @param mode Indicates the {@code SyncMode} object.
         * @param allowedDelayMs Indicates the number type parameter.
         * @return Returns the {@code number} object.
         * @since 7
         */
        sync(deviceIdList: string[], mode: SyncMode, allowedDelayMs?: number): void;
    }
    
    /**
     * Creates a {@link KVManager} instance based on the configuration information.
     *
     * <p>You must pass {@link KVManagerConfig} to provide configuration information
     * for creating the {@link KVManager} instance.
     *
     * @param config Indicates the {@link KVStore} configuration information,
     * including the user information and package name.
     * @return Returns the {@code KVManager} instance.
     * @throws Throws exception if input is invalid.
     * @since 7
     */
    function createKVManager(config: KVManagerConfig, callback: AsyncCallback<KVManager>): void;
    function createKVManager(config: KVManagerConfig): Promise<KVManager>;

    /**
     * Provides interfaces to manage a {@code KVStore} database,
     * including obtaining, closing, and deleting the {@code KVStore}.
     *
     * @devices phone, tablet
     * @Syscap SystemCapability.Data.DATA_DISTRIBUTEDDATAMGR
     * @since 7
     * @version 1
     */
    interface KVManager {
        /**
         * Creates and obtains a {@code KVStore} database by specifying {@code Options} and {@code storeId}.
         *
         * @param options Indicates the options used for creating and obtaining the {@code KVStore} database,
         * including {@code isCreateIfMissing}, {@code isEncrypt}, and {@code KVStoreType}.
         * @param storeId Identifies the {@code KVStore} database.
         * The value of this parameter must be unique for the same application,
         * and different applications can share the same value.
         * @return Returns a {@code KVStore}, or {@code SingleKVStore},
         * @since 7
         */
        getKVStore<T extends KVStore>(storeId: string, options: Options): Promise<T>;
        getKVStore<T extends KVStore>(storeId: string, options: Options, callback: AsyncCallback<T>): void;
    }
}

export default distributedData;
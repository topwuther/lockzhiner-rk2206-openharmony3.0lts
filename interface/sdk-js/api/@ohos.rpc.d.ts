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

declare namespace rpc {
    class MessageParcel {
        static create(): MessageParcel;
        reclaim(): void;
        writeRemoteObject(object: IRemoteObject): boolean;
        readRemoteObject(): IRemoteObject;
        writeInterfaceToken(token: string): boolean;
        readInterfaceToken(): string;
        getSize(): number;
        getCapacity(): number;
        setSize(size: number): boolean;
        setCapacity(size: number): boolean;
        getWritableBytes(): number;
        getReadableBytes(): number;
        getReadPosition(): number;
        getWritePosition(): number;
        rewindRead(pos: number): boolean;
        rewindWrite(pos: number): boolean;
        writeByte(val: number): boolean;
        writeShort(val: number): boolean;
        writeInt(val: number): boolean;
        writeLong(val: number): boolean;
        writeFloat(val: number): boolean;
        writeDouble(val: number): boolean;
        writeBoolean(val: boolean): boolean;
        writeChar(val: number): boolean;
        writeString(val: string): boolean;
        writeSequenceable(val: Sequenceable): boolean;
        writeByteArray(byteArray: number[]): boolean;
        writeShortArray(shortArray: number[]): boolean;
        writeIntArray(intArray: number[]): boolean;
        writeLongArray(longArray: number[]): boolean;
        writeFloatArray(floatArray: number[]): boolean;
        writeDoubleArray(doubleArray: number[]): boolean;
        writeBooleanArray(booleanArray: boolean[]): boolean;
        writeCharArray(charArray: number[]): boolean;
        writeStringArray(stringArray: string[]): boolean;
        writeSequenceableArray(sequenceableArray: Sequenceable[]): boolean;
        readByte(): number;
        readShort(): number;
        readInt(): number;
        readLong(): number;
        readFloat(): number;
        readDouble(): number;
        readBoolean(): boolean;
        readChar(): number;
        readString(): string;
        readSequenceable(dataIn: Sequenceable) : boolean;
        readByteArray(dataIn: number[]) : void;
        readByteArray(): number[];
        readShortArray(dataIn: number[]) : void;
        readShortArray(): number[];
        readIntArray(dataIn: number[]) : void;
        readIntArray(): number[];
        readLongArray(dataIn: number[]) : void;
        readLongArray(): number[];
        readFloatArray(dataIn: number[]) : void;
        readFloatArray(): number[];
        readDoubleArray(dataIn: number[]) : void;
        readDoubleArray(): number[];
        readBooleanArray(dataIn: boolean[]) : void;
        readBooleanArray(): boolean[];
        readCharArray(dataIn: boolean[]) : void;
        readCharArray(): boolean[];
        readStringArray(dataIn: string[]) : void;
        readStringArray(): string[];
    }

    interface Sequenceable {
        hasFileDescriptor(): boolean;
        marshalling(dataOut: MessageParcel): boolean;
        unmarshalling(dataIn: MessageParcel) : boolean;
    }

    enum IRemoteObject {
        /**
         * Indicates the message code for a Ping operation.
         */
        PING_TRANSACTION = ('_' << 24) | ('P' << 16) | ('N' << 8) | 'G',

        /**
         * Indicates the message code for a dump operation.
         */
        DUMP_TRANSACTION = ('_' << 24) | ('D' << 16) | ('M' << 8) | 'P',

        /**
         * Indicates the message code for a transmission.
         */
        INTERFACE_TRANSACTION = ('_' << 24) | ('N' << 16) | ('T' << 8) | 'F',

        /**
         * Indicates the minimum value of a valid message code.
         *
         * <p>This constant is used to check the validity of an operation.
         */
        MIN_TRANSACTION_ID = 0x1,

        /**
         * Indicates the maximum value of a valid message code.
         *
         * <p>This constant is used to check the validity of an operation.
         */
        MAX_TRANSACTION_ID = 0x00FFFFFF,
    }

    interface IRemoteObject {
        /**
         * Queries the description of an interface.
         *
         * <p>A valid {@link IRemoteBroker} object is returned for an interface used by the service provider;
         * {@code null} is returned for an interface used by the service user,
         * indicating that the interface is not a local one.
         *
         * @param descriptor Indicates a string of the interface descriptor.
         * @return Returns the {@link IRemoteBroker} object bound to the specified interface descriptor.
         * @since 7
         */
        queryLocalInterface(descriptor: string): IRemoteBroker;

        /**
         * Sends a {@link MessageParcel} message to the peer process in synchronous or asynchronous mode.
         *
         * <p>If asynchronous mode is set for {@code option}, a response is returned immediately.
         * If synchronous mode is set for {@code option}, the interface will wait for a response from the peer process
         * until the request times out. The user must prepare {@code reply} for receiving the execution result
         * given by the peer process.
         *
         * @param code Indicates the message code, which is determined by both sides of the communication.
         * If the interface is generated by the IDL tool, the message code is automatically generated by IDL.
         * @param data Indicates the {@link MessageParcel} object sent to the peer process.
         * @param reply Indicates the {@link MessageParcel} object returned by the peer process.
         * @param options Indicates the synchronous or asynchronous mode to send messages.
         * @return Returns {@code true} if the method is called successfully; returns {@code false} otherwise.
         * @throws RemoteException Throws this exception if the method fails to be called.
         * @since 7
         */
        sendRequest(code: number, data: MessageParcel, reply: MessageParcel, options: MessageOption): boolean;

        /**
         * Registers a callback used to receive notifications of the death of a remote object.
         *
         * <p>This method is called if the remote object process matching the {@link RemoteProxy} object dies.
         *
         * @param recipient Indicates the callback to be registered.
         * @param flags Indicates the flag of the death notification.
         * @return Returns {@code true} if the callback is registered successfully; returns {@code false} otherwise.
         * @since 7
         */
        addDeathRecipient(recipient: DeathRecipient, flags: number): boolean;

        /**
         * Deregisters a callback used to receive notifications of the death of a remote object.
         *
         * @param recipient Indicates the callback to be deregistered.
         * @param flags Indicates the flag of the death notification.
         * @return Returns {@code true} if the callback is deregistered successfully; returns {@code false} otherwise.
         * @since 7
         */
        removeDeathRecipient(recipient: DeathRecipient, flags: number): boolean;

        /**
         * Obtains the interface descriptor of an object.
         *
         * <p>The interface descriptor is a character string.
         *
         * @return Returns the interface descriptor.
         * @since 7
         */
        getInterfaceDescriptor(): string;

        /**
         * Checks whether an object is dead.
         *
         * @return Returns {@code true} if the object is dead; returns {@code false} otherwise.
         * @since 7
         */
        isObjectDead(): boolean;
    }

    interface IRemoteBroker {
        asObject(): IRemoteObject;
    }

    interface DeathRecipient {
        onRemoteDied(): void;
    }

    enum  MessageOption {
        TF_SYNC = 0,
        TF_ASYNC = 1,
        TF_ACCEPT_FDS = 0x10,
        TF_WAIT_TIME  = 4,
        MAX_WAIT_TIME  = 3000,
    }

    interface MessageOption {
        syncflags: number;
        waitTime: number;
    }

    class RemoteObject implements IRemoteObject {
        descriptor: string;
        interface: IRemoteObject;

        /**
         * Queries a remote object using an interface descriptor.
         *
         * @param descriptor Indicates the interface descriptor used to query the remote object.
         * @return Returns the remote object matching the interface descriptor; returns null
         * if no such remote object is found.
         * @since 7
         */
        queryLocalInterface(interface: string): IRemoteObject;

        /**
         * Queries an interface descriptor.
         *
         * @return Returns the interface descriptor.
         * @since 7
         */
        getInterfaceDescriptor(): string;

        /**
         * Sets an entry for receiving requests.
         *
         * <p>This method is implemented by the remote service provider. You need to override this method with
         * your own service logic when you are using IPC.
         *
         * @param code Indicates the service request code sent from the peer end.
         * @param data Indicates the {@link MessageParcel} object sent from the peer end.
         * @param reply Indicates the response message object sent from the remote service.
         * The local service writes the response data to the {@link MessageParcel} object.
         * @param options Indicates whether the operation is synchronous or asynchronous.
         * @return Returns {@code true} if the operation succeeds; returns {@code false} otherwise.
         * @throws RemoteException Throws this exception if a remote service error occurs.
         * @since 7
         */
        onRemoteRequest(code: number, data: MessageParcel, reply: MessageParcel, options: MessageOption): boolean;

        /**
         * Sends a request to the peer object.
         *
         * <p>If the peer object and {@code RemoteObject} are on the same device, the request is sent by the IPC driver.
         * If they are on different devices, the request is sent by the socket driver.
         *
         * @param code Indicates the message code of the request.
         * @param data Indicates the {@link MessageParcel} object storing the data to be sent.
         * @param reply Indicates the {@link MessageParcel} object receiving the response data.
         * @param options Indicates a synchronous (default) or asynchronous request.
         * @return Returns {@code true} if the operation succeeds; returns {@code false} otherwise.
         * @since 7
         */
        sendRequest(code: number, data: MessageParcel, reply: MessageParcel, options: MessageOption): boolean;

        /**
         * Obtains the PID of the {@link RemoteProxy} object.
         *
         * @return Returns the PID of the {@link RemoteProxy} object.
         * @since 7
         */
        getCallingPid(): number;

        /**
         * Obtains the UID of the {@link RemoteProxy} object.
         *
         * @return Returns the UID of the {@link RemoteProxy} object.
         * @since 7
         */
        getCallingUid(): number;

        /**
         * Modifies the description of the current {@code RemoteObject}.
         *
         * <p>This method is used to change the default descriptor specified during the creation of {@code RemoteObject}.
         *
         * @param localInterface Indicates the {@code RemoteObject} whose descriptor is to be changed.
         * @param descriptor Indicates the new descriptor of the {@code RemoteObject}.
         * @since 7
         */
        attachLocalInterface(localInterface: IRemoteBroker, descriptor: string): void;
    }

    class RemoteProxy implements IRemoteObject {
        /**
         * Queries a local interface with a specified descriptor.
         *
         * @param descriptor Indicates the descriptor of the interface to query.
         * @return Returns null by default, indicating a proxy interface.
         * @since 7
         */
        queryLocalInterface(interface: string): IRemoteObject;

        /**
         * Registers a callback used to receive death notifications of a remote object.
         *
         * @param recipient Indicates the callback to be registered.
         * @param flags Indicates the flag of the death notification. This is a reserved parameter. Set it to {@code 0}.
         * @return Returns {@code true} if the callback is registered successfully; returns {@code false} otherwise.
         * @since 7
         */
        addDeathRecipient(recipient: DeathRecipient, flags: number): boolean;

        /**
         * Deregisters a callback used to receive death notifications of a remote object.
         *
         * @param recipient Indicates the callback to be deregistered.
         * @param flags Indicates the flag of the death notification. This is a reserved parameter. Set it to {@code 0}.
         * @return Returns {@code true} if the callback is deregistered successfully; returns {@code false} otherwise.
         * @since 7
         */
        removeDeathRecipient(recipient: DeathRecipient, flags: number): boolean;

        /**
         * Queries the interface descriptor of remote object.
         *
         * @return Returns the interface descriptor.
         * @since 7
         */
        getInterfaceDescriptor(): string;

        /**
         * Sends a request to the peer object.
         *
         * <p>If the peer object and {@code RemoteProxy} are on the same device, the request is sent by the IPC driver.
         * If they are on different devices, the request is sent by the socket driver.
         *
         * @param code Indicates the message code of the request.
         * @param data Indicates the {@link MessageParcel} object storing the data to be sent.
         * @param reply Indicates the {@link MessageParcel} object receiving the response data.
         * @param options Indicates a synchronous (default) or asynchronous request.
         * @return Returns {@code true} if the operation succeeds; returns {@code false} otherwise.
         * @throws RemoteException Throws this exception if a remote object exception occurs.
         * @since 7
         */
        sendRequest(code: number, data: MessageParcel, reply: MessageParcel, options: MessageOption): boolean;

        /**
         * Checks whether the {@code RemoteObject} corresponding to a {@code RemoteProxy} is dead.
         *
         * @return Returns {@code true} if the {@code RemoteObject} is dead; returns {@code false} otherwise.
         * @since 7
         */
        isObjectDead(): boolean;
    }

    class IPCSkeleton {
        /**
         * Obtains a local {@link IRemoteObject} reference of a registered service.
         *
         * <p>This method is static.
         *
         * @return Returns an {@link IRemoteObject} reference of the registered service.
         * @since 7
         */
        static getContextObject(): IRemoteObject;

        /**
         * Obtains the PID of a proxy.
         *
         * <p>This method is static. The PID is a positive integer during the communication between
         * the {@link RemoteProxy} object and {@link RemoteObject} object, and resumes to {@code 0}
         * when the communication ends. If this method is called from the {@link RemoteProxy} object,
         * {@code 0} is returned; if this method is called from the {@link RemoteObject} object,
         * the PID of the corresponding {@link RemoteProxy} object is returned.
         *
         * @return Returns the PID of the proxy.
         * @since 7
         */
        static getCallingPid(): number;

        /**
         * Obtains the UID of a proxy.
         *
         * <p>This method is static. The UID is a positive integer during the communication between
         * the {@link RemoteProxy} object and {@link RemoteObject} object, and resumes to {@code 0}
         * when the communication ends. If this method is called from the {@link RemoteProxy} object,
         * {@code 0} is returned; if this method is called from the {@link RemoteObject} object,
         * the UID of the corresponding {@link RemoteProxy} object is returned.
         *
         * @return Returns the UID of the proxy.
         * @since 7
         */
        static getCallingUid(): number;

        /**
         * Obtains the ID of the device where the peer process resides.
         *
         * <p>This method is static.
         *
         * @return Returns the ID of the device where the peer process resides.
         * @since 7
         */
        static getCallingDeviceID(): string;

        /**
         * Obtains the ID of the local device.
         *
         * <p>This method is static.
         *
         * @return Returns the ID of the local device.
         * @since 7
         */
        static getLocalDeviceID(): string;

        /**
         * Checks whether a call is made on the same device.
         *
         * <p>This method is static.
         *
         * @return Returns {@code true} if the call is made on the same device; returns {@code false} otherwise.
         * @since 7
         */
        static isLocalCalling(): boolean;

        /**
         * Flushes all pending commands from a specified {@link RemoteProxy} to the corresponding {@link RemoteObject}.
         *
         * <p>This method is static. You are advised to call this method before performing any time-sensitive operations.
         *
         * @param object Indicates the specified {@link RemoteProxy}.
         * @return Returns {@code 0} if the operation succeeds; returns an error code if the input object is empty
         * or {@link RemoteObject}, or the operation fails.
         * @since 7
         */
        static flushCommands(object: IRemoteObject): number;

        /**
         * Replaces the UID and PID of the remote user with those of the local user.
         *
         * <p>This method is static. It can be used in scenarios like authentication.
         *
         * @return Returns a string containing the UID and PID of the remote user.
         * @since 7
         */
        static resetCallingIdentity(): string;

        /**
         * Restores the UID and PID to those of the remote user.
         *
         * <p>This method is static. It is usually called after {@code resetCallingIdentity} is used
         * and requires the UID and PID of the remote user returned by {@code resetCallingIdentity}.
         *
         * @param identity Indicates the string containing the UID and PID of the remote user,
         * which is returned by {@code resetCallingIdentity}.
         * @return Returns {@code true} if the operation succeeds; returns {@code false} otherwise.
         * @since 7
         */
        static setCallingIdentity(identity: string): boolean;
    }
}

export default rpc;
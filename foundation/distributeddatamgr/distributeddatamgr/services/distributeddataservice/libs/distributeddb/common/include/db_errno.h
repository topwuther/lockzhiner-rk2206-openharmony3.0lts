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

#ifndef DISTRIBUTEDDB_ERRNO_H
#define DISTRIBUTEDDB_ERRNO_H

#include <errno.h>

namespace DistributedDB {
constexpr int E_OK = 0;
constexpr int E_BASE = 1000; // different from the other errno.
constexpr int E_NOT_SUPPORT = (E_BASE + 1); // not support currently.
constexpr int E_INVALID_DB = (E_BASE + 2); // invalid db or connection.
constexpr int E_NOT_FOUND = (E_BASE + 3); // not found the resource.
constexpr int E_BUSY = (E_BASE + 4); // the db is busy
constexpr int E_UNEXPECTED_DATA = (E_BASE + 5); // Data does not match expectation.
constexpr int E_STALE = (E_BASE + 6); // Resource has been stopped, killed or destroyed.
constexpr int E_INVALID_ARGS = (E_BASE + 7); // the input args is invalid.
constexpr int E_REGISTER_OBSERVER = (E_BASE + 8); // error in register observer related function.
constexpr int E_TRANSACT_STATE = (E_BASE + 9); // transaction state error.
constexpr int E_SECUREC_ERROR = (E_BASE + 10); // security interface returns error
constexpr int E_OUT_OF_MEMORY = (E_BASE + 11); // out of memory
constexpr int E_NOT_PERMIT = (E_BASE + 12); // operation is not permitted
constexpr int E_ALREADY_REGISTER = (E_BASE + 13); // function or handle already registered and not allowed replace
constexpr int E_ALREADY_ALLOC = (E_BASE + 14); // Object had already been allocated
constexpr int E_ALREADY_RELEASE = (E_BASE + 15); // Object had already been released
constexpr int E_CONTAINER_FULL = (E_BASE + 16); // container full
constexpr int E_CONTAINER_EMPTY = (E_BASE + 17); // container empty
constexpr int E_CONTAINER_FULL_TO_NOTFULL = (E_BASE + 18); // container status changed from full to not full
constexpr int E_CONTAINER_NOTEMPTY_TO_EMPTY = (E_BASE + 19); // container status changed from full to not full
constexpr int E_WAIT_RETRY = (E_BASE + 20); // wait and retry later
constexpr int E_PARSE_FAIL = (E_BASE + 21); // parse packet or frame fail
constexpr int E_TIMEOUT = (E_BASE + 22); // time out
constexpr int E_SERIALIZE_ERROR = (E_BASE + 23); // serialize error
constexpr int E_DESERIALIZE_ERROR = (E_BASE + 24); // deserialize error
constexpr int E_NOT_REGISTER = (E_BASE + 25); // handler or function not registered
constexpr int E_LENGTH_ERROR = (E_BASE + 26); // error relative to length
constexpr int E_UNFINISHED = (E_BASE + 27); // get sync data unfinished.
constexpr int E_FINISHED = (E_BASE + 28); // get sync data unfinished.
constexpr int E_INVALID_MESSAGE_ID = (E_BASE + 29); // invalid messageId error
constexpr int E_MESSAGE_ID_ERROR = (E_BASE + 30); // messageId is not expected
constexpr int E_MESSAGE_TYPE_ERROR = (E_BASE + 31); // messageType is not expected
constexpr int E_PERIPHERAL_INTERFACE_FAIL = (E_BASE + 32); // peripheral interface fail
constexpr int E_NOT_INIT = (E_BASE + 33); // module may not init
constexpr int E_MAX_LIMITS = (E_BASE + 34); // over max limits.
constexpr int E_INVALID_CONNECTION = (E_BASE + 35); // invalid db connection.
constexpr int E_NO_SUCH_ENTRY = (E_BASE + 36); // invalid db connection.
constexpr int E_INTERNAL_ERROR = (E_BASE + 37); // an error due to code logic that is a bug
constexpr int E_CONTAINER_ONLY_DELAY_TASK = (E_BASE + 38); // only delay task left in the container
constexpr int E_SUM_CALCULATE_FAIL = (E_BASE + 39); // only delay task left in the container
constexpr int E_SUM_MISMATCH = (E_BASE + 40); // check sum mismatch
constexpr int E_OUT_OF_DATE = (E_BASE + 41); // things is out of date
constexpr int E_OBJ_IS_KILLED = (E_BASE + 42); // the refObject has been killed.
constexpr int E_SYSTEM_API_FAIL = (E_BASE + 43); // call the system api failed
constexpr int E_INVALID_DATA = (E_BASE + 44); // invalid data
constexpr int E_OUT_OF_IDS = (E_BASE + 45); // out of ids.
constexpr int E_SEND_DATA = (E_BASE + 46); // need send data
constexpr int E_NEED_TIMER = (E_BASE + 47); // timer is still need
constexpr int E_NO_NEED_TIMER = (E_BASE + 48); // timer no longer need
constexpr int E_COMBINE_FAIL = (E_BASE + 49); // fail in combining a frame
constexpr int E_END_TIMER = (E_BASE + 50); // timer no longer needed
constexpr int E_CALC_HASH = (E_BASE + 51); // calc hash error
constexpr int E_REMOVE_FILE = (E_BASE + 52); // remove file failed
constexpr int E_STATE_MACHINE_ERROR = (E_BASE + 53); // sync state machine error
constexpr int E_NO_DATA_SEND = (E_BASE + 54); // no data to send
constexpr int E_RECV_FINISHED = (E_BASE + 55); // recv finished
constexpr int E_NEED_PULL_REPONSE = (E_BASE + 56); // need to response pull request
constexpr int E_NO_SYNC_TASK = (E_BASE + 57); // no sync task to do
constexpr int E_INVALID_PASSWD_OR_CORRUPTED_DB = (E_BASE + 58); // invalid password or corrupted database.
constexpr int E_RESULT_SET_STATUS_INVALID = (E_BASE + 59); // status of result set is invalid.
constexpr int E_RESULT_SET_EMPTY = (E_BASE + 60); // the result set is empty.
constexpr int E_UPGRADE_FAILED = (E_BASE + 61); // the upgrade failed.
constexpr int E_INVALID_FILE = (E_BASE + 62); // import invalid file.
constexpr int E_INVALID_PATH = (E_BASE + 63); // the path is invalid.
constexpr int E_EMPTY_PATH = (E_BASE + 64); // the path is empty.
constexpr int E_TASK_BREAK_OFF = (E_BASE + 65); // task quit due to normal break off or error happen
constexpr int E_INCORRECT_DATA = (E_BASE + 66); // data in the database is incorrect
constexpr int E_NO_RESOURCE_FOR_USE = (E_BASE + 67); // no resource such as dbhandle for use
constexpr int E_LAST_SYNC_FRAME = (E_BASE + 68); // this frame is the last frame for this sync
constexpr int E_VERSION_NOT_SUPPORT = (E_BASE + 69); // version not support in any layer
constexpr int E_FRAME_TYPE_NOT_SUPPORT = (E_BASE + 70); // frame type not support
constexpr int E_INVALID_TIME = (E_BASE + 71); // the time is invalid
constexpr int E_INVALID_VERSION = (E_BASE + 72); // sqlite storage version is invalid
constexpr int E_SCHEMA_NOTEXIST = (E_BASE + 73); // schema does not exist
constexpr int E_INVALID_SCHEMA = (E_BASE + 74); // the schema is invalid
constexpr int E_SCHEMA_MISMATCH = (E_BASE + 75); // the schema is mismatch
constexpr int E_INVALID_FORMAT = (E_BASE + 76); // the value is invalid json or mismatch with the schema.
constexpr int E_READ_ONLY = (E_BASE + 77); // only have the read permission.
constexpr int E_NEED_ABILITY_SYNC = (E_BASE + 78); // ability sync has not done
constexpr int E_WAIT_NEXT_MESSAGE = (E_BASE + 79); // need remote device send a next message.
constexpr int E_LOCAL_DELETED = (E_BASE + 80); // local data is deleted by the unpublish.
constexpr int E_LOCAL_DEFEAT = (E_BASE + 81); // local data defeat the sync data while unpublish.
constexpr int E_LOCAL_COVERED = (E_BASE + 82); // local data is covered by the sync data while unpublish.
constexpr int E_INVALID_QUERY_FORMAT = (E_BASE + 83); // query format is not valid.
constexpr int E_INVALID_QUERY_FIELD = (E_BASE + 84); // query field is not valid.
constexpr int E_ALREADY_OPENED = (E_BASE + 85); // the database is already opened.
constexpr int E_ALREADY_SET = (E_BASE + 86); // already set.
constexpr int E_SAVE_DATA_NOTIFY = (E_BASE + 87); // notify remote device to keep alive, don't timeout
constexpr int E_RE_SEND_DATA = (E_BASE + 88); // need re send data
constexpr int E_EKEYREVOKED = (E_BASE + 89); // the EKEYREVOKED error
constexpr int E_SECURITY_OPTION_CHECK_ERROR = (E_BASE + 90); // remote device's SecurityOption not equal to local
constexpr int E_SYSTEM_API_ADAPTER_CALL_FAILED = (E_BASE + 91); // Adapter call failed
constexpr int E_NOT_NEED_DELETE_MSG = (E_BASE + 92); // not need delete msg, will be delete by sliding window receiver
constexpr int E_SLIDING_WINDOW_SENDER_ERR = (E_BASE + 93); // sliding window sender err
constexpr int E_SLIDING_WINDOW_RECEIVER_INVALID_MSG = (E_BASE + 94); // sliding window receiver invalid msg
constexpr int E_IGNOR_DATA = (E_BASE + 95); // ignore the data changed by other devices.
constexpr int E_FORBID_CACHEDB = (E_BASE + 96); // such after rekey can not check passwd due to file control.
// Num 150+ is reserved for schema related errno, since it may be added regularly
constexpr int E_JSON_PARSE_FAIL = (E_BASE + 150); // Parse json fail in grammatical level
constexpr int E_JSON_INSERT_PATH_EXIST = (E_BASE + 151); // Path already exist before insert
constexpr int E_JSON_INSERT_PATH_CONFLICT = (E_BASE + 152); // Nearest path ends with type not object
constexpr int E_JSON_DELETE_PATH_NOT_FOUND = (E_BASE + 153); // Path to delete not found
constexpr int E_SCHEMA_PARSE_FAIL = (E_BASE + 160); // Parse schema fail in content level
constexpr int E_SCHEMA_EQUAL_EXACTLY = (E_BASE + 161); // Two schemas are exactly the same
constexpr int E_SCHEMA_UNEQUAL_COMPATIBLE = (E_BASE + 162); // New schema contain different index
constexpr int E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE = (E_BASE + 163); // New schema contain more field(index may differ)
constexpr int E_SCHEMA_UNEQUAL_INCOMPATIBLE = (E_BASE + 164); // New schema contain more field or index
constexpr int E_SCHEMA_VIOLATE_VALUE = (E_BASE + 165); // New schema violate values already exist in dbFile
constexpr int E_FLATBUFFER_VERIFY_FAIL = (E_BASE + 170); // Verify flatbuffer content(schema or value) fail.
constexpr int E_VALUE_MATCH = (E_BASE + 180); // Value match schema(strict or compatible) without amend
constexpr int E_VALUE_MATCH_AMENDED = (E_BASE + 181); // Value match schema(strict or compatible) with amend
constexpr int E_VALUE_MISMATCH_FEILD_COUNT = (E_BASE + 182); // Value mismatch schema in field count
constexpr int E_VALUE_MISMATCH_FEILD_TYPE = (E_BASE + 183); // Value mismatch schema in field type
constexpr int E_VALUE_MISMATCH_CONSTRAINT = (E_BASE + 184); // Value mismatch schema in constraint
constexpr int E_VALUE_MISMATCH_OTHER_REASON = (E_BASE + 185); // Value mismatch schema in other reason
// Num 200+ is reserved for fixed value errno, which should not be changed between time
// Message with errorNo of Feedback-type is generated by CommunicatorAggregator without data part(No deserial if exist)
constexpr int E_FEEDBACK_UNKNOWN_MESSAGE = (E_BASE + 200); // Unknown message feedback from remote device
constexpr int E_FEEDBACK_COMMUNICATOR_NOT_FOUND = (E_BASE + 201); // Communicator not found feedback from remote device
} // namespace DistributedDB

#endif // DISTRIBUTEDDB_ERRNO_H

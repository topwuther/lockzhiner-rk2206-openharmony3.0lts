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

#include <uv.h>
#include <mutex>
#include "subscribe.h"

namespace OHOS {
namespace NotificationNapi {
const int SUBSRIBE_MAX_PARA = 3;
const int NO_DELETE_REASON = -1;
const std::string CONSUME = "onConsume";
const std::string CANCEL = "onCancel";
const std::string UPDATE = "onUpdate";
const std::string CONNECTED = "onConnect";
const std::string DIS_CONNECTED = "onDisconnect";
const std::string DIE = "onDestroy";
const std::string DISTURB_MODE_CHANGE = "onDisturbModeChange";

struct NotificationReceiveDataWorker {
    napi_env env = nullptr;
    napi_ref ref = nullptr;
    std::shared_ptr<OHOS::Notification::Notification> request;
    std::shared_ptr<NotificationSortingMap> sortingMap;
    int deleteReason = 0;
    int result = 0;
    int disturbMode = 0;
    SubscriberInstance *subscriber = nullptr;
};

napi_value SetSubscribeCallbackData(const napi_env &env,
    const std::shared_ptr<OHOS::Notification::Notification> &request,
    const std::shared_ptr<NotificationSortingMap> &sortingMap, int deleteReason, napi_value &result)
{
    ANS_LOGI("enter");
    if (request == nullptr) {
        ANS_LOGE("request is null");
        return Common::NapiGetboolean(env, false);
    }

    if (sortingMap == nullptr) {
        ANS_LOGE("sortingMap is null");
        return Common::NapiGetboolean(env, false);
    }

    // request: NotificationRequest
    napi_value requestResult = nullptr;
    napi_create_object(env, &requestResult);
    if (!Common::SetNotification(env, request.get(), requestResult)) {
        ANS_LOGE("SetNotification call failed");
        return Common::NapiGetboolean(env, false);
    }
    napi_set_named_property(env, result, "request", requestResult);

    // sortingMap?: NotificationSortingMap
    napi_value sortingMapResult = nullptr;
    napi_create_object(env, &sortingMapResult);
    if (!Common::SetNotificationSortingMap(env, sortingMap, sortingMapResult)) {
        ANS_LOGE("SetNotificationSortingMap call failed");
        return Common::NapiGetboolean(env, false);
    }
    napi_set_named_property(env, result, "sortingMap", sortingMapResult);

    // reason?: number
    if (deleteReason != NO_DELETE_REASON) {
        napi_value value = nullptr;
        int outReason = 0;
        if (!Common::ReasonCToJS(deleteReason, outReason)) {
            return Common::NapiGetboolean(env, false);
        }
        napi_create_int32(env, outReason, &value);
        napi_set_named_property(env, result, "reason", value);
    }

    // sound?: string
    napi_value soundResult = nullptr;
    std::string sound;
    if (request->EnableSound()) {
        sound = request->GetSound().ToString();
    }
    napi_create_string_utf8(env, sound.c_str(), NAPI_AUTO_LENGTH, &soundResult);
    napi_set_named_property(env, result, "sound", soundResult);

    // vibrationValues?: Array<number>
    napi_value arr = nullptr;
    napi_create_array(env, &arr);
    size_t count = 0;
    if (request->EnableVibrate()) {
        for (auto vec : request->GetVibrationStyle()) {
            napi_value nVibrationValue = nullptr;
            napi_create_int64(env, vec, &nVibrationValue);
            napi_set_element(env, arr, count, nVibrationValue);
            count++;
        }
    }
    napi_set_named_property(env, result, "vibrationValues", arr);

    return Common::NapiGetboolean(env, true);
}

SubscriberInstance::SubscriberInstance()
{}

SubscriberInstance::~SubscriberInstance()
{
    if (canceCallbackInfo_.ref != nullptr) {
        napi_delete_reference(canceCallbackInfo_.env, canceCallbackInfo_.ref);
    }
    if (consumeCallbackInfo_.ref != nullptr) {
        napi_delete_reference(consumeCallbackInfo_.env, consumeCallbackInfo_.ref);
    }
    if (updateCallbackInfo_.ref != nullptr) {
        napi_delete_reference(updateCallbackInfo_.env, updateCallbackInfo_.ref);
    }
    if (subscribeCallbackInfo_.ref != nullptr) {
        napi_delete_reference(subscribeCallbackInfo_.env, subscribeCallbackInfo_.ref);
    }
    if (unsubscribeCallbackInfo_.ref != nullptr) {
        napi_delete_reference(unsubscribeCallbackInfo_.env, unsubscribeCallbackInfo_.ref);
    }
    if (dieCallbackInfo_.ref != nullptr) {
        napi_delete_reference(dieCallbackInfo_.env, dieCallbackInfo_.ref);
    }
    if (disturbModeCallbackInfo_.ref != nullptr) {
        napi_delete_reference(disturbModeCallbackInfo_.env, disturbModeCallbackInfo_.ref);
    }
}

void SubscriberInstance::OnCanceled(const std::shared_ptr<OHOS::Notification::Notification> &request)
{}

void SubscriberInstance::OnCanceled(const std::shared_ptr<OHOS::Notification::Notification> &request,
    const std::shared_ptr<NotificationSortingMap> &sortingMap, int deleteReason)
{
    ANS_LOGI("enter");

    if (canceCallbackInfo_.ref == nullptr) {
        ANS_LOGI("cancel callback unset");
        return;
    }

    if (request == nullptr) {
        ANS_LOGE("request is null");
        return;
    }

    if (sortingMap == nullptr) {
        ANS_LOGE("sortingMap is null");
        return;
    }
    ANS_LOGI("OnCanceled NotificationId = %{public}d", request->GetNotificationRequest().GetNotificationId());
    ANS_LOGI("OnCanceled sortingMap size = %{public}zu", sortingMap->GetKey().size());
    ANS_LOGI("OnCanceled deleteReason = %{public}d", deleteReason);

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(canceCallbackInfo_.env, &loop);
    if (loop == nullptr) {
        ANS_LOGE("loop instance is nullptr");
        return;
    }

    NotificationReceiveDataWorker *dataWorker = new (std::nothrow) NotificationReceiveDataWorker();
    if (dataWorker == nullptr) {
        ANS_LOGE("new dataWorker failed");
        return;
    }

    dataWorker->request = request;
    dataWorker->sortingMap = sortingMap;
    dataWorker->deleteReason = deleteReason;
    dataWorker->env = canceCallbackInfo_.env;
    dataWorker->ref = canceCallbackInfo_.ref;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        ANS_LOGE("new work failed");
        return;
    }

    work->data = (void *)dataWorker;

    uv_queue_work(loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            ANS_LOGI("OnCanceled uv_work_t start");

            if (work == nullptr) {
                ANS_LOGE("work is null");
                return;
            }

            NotificationReceiveDataWorker *dataWorkerData = (NotificationReceiveDataWorker *)work->data;
            if (dataWorkerData == nullptr) {
                ANS_LOGE("dataWorkerData is null");
                return;
            }
            napi_value result = nullptr;
            napi_create_object(dataWorkerData->env, &result);
            int error = 0;
            if (!SetSubscribeCallbackData(dataWorkerData->env,
                dataWorkerData->request,
                dataWorkerData->sortingMap,
                dataWorkerData->deleteReason,
                result)) {
                result = Common::NapiGetNull(dataWorkerData->env);
                error = ERROR;
            } else {
                error = NO_ERROR;
            }
            Common::SetCallback(dataWorkerData->env, dataWorkerData->ref, error, result);

            delete dataWorkerData;
            dataWorkerData = nullptr;
            delete work;
            work = nullptr;
        });
}

void SubscriberInstance::OnConsumed(const std::shared_ptr<OHOS::Notification::Notification> &request)
{}

void SubscriberInstance::OnConsumed(const std::shared_ptr<OHOS::Notification::Notification> &request,
    const std::shared_ptr<NotificationSortingMap> &sortingMap)
{
    ANS_LOGI("enter");

    if (consumeCallbackInfo_.ref == nullptr) {
        ANS_LOGI("consume callback unset");
        return;
    }

    if (request == nullptr) {
        ANS_LOGE("request is null");
        return;
    }

    if (sortingMap == nullptr) {
        ANS_LOGE("sortingMap is null");
        return;
    }
    ANS_LOGI("OnConsumed NotificationId = %{public}d", request->GetNotificationRequest().GetNotificationId());
    ANS_LOGI("OnConsumed sortingMap size = %{public}zu", sortingMap->GetKey().size());

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(consumeCallbackInfo_.env, &loop);
    if (loop == nullptr) {
        ANS_LOGE("loop instance is nullptr");
        return;
    }

    NotificationReceiveDataWorker *dataWorker = new (std::nothrow) NotificationReceiveDataWorker();
    if (dataWorker == nullptr) {
        ANS_LOGE("new dataWorker failed");
        return;
    }

    dataWorker->request = request;
    dataWorker->sortingMap = sortingMap;
    dataWorker->env = consumeCallbackInfo_.env;
    dataWorker->ref = consumeCallbackInfo_.ref;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        ANS_LOGE("new work failed");
        return;
    }

    work->data = (void *)dataWorker;

    uv_queue_work(loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            ANS_LOGI("OnConsumed uv_work_t start");

            if (work == nullptr) {
                ANS_LOGE("work is null");
                return;
            }

            NotificationReceiveDataWorker *dataWorkerData = (NotificationReceiveDataWorker *)work->data;
            if (dataWorkerData == nullptr) {
                ANS_LOGE("dataWorkerData is null");
                return;
            }
            napi_value result = nullptr;
            napi_create_object(dataWorkerData->env, &result);
            int error = 0;
            if (!SetSubscribeCallbackData(dataWorkerData->env,
                dataWorkerData->request,
                dataWorkerData->sortingMap,
                NO_DELETE_REASON,
                result)) {
                result = Common::NapiGetNull(dataWorkerData->env);
                error = ERROR;
            } else {
                error = NO_ERROR;
            }
            Common::SetCallback(dataWorkerData->env, dataWorkerData->ref, error, result);

            delete dataWorkerData;
            dataWorkerData = nullptr;
            delete work;
            work = nullptr;
        });
}

void SubscriberInstance::OnUpdate(const std::shared_ptr<NotificationSortingMap> &sortingMap)
{
    ANS_LOGI("enter");

    if (updateCallbackInfo_.ref == nullptr) {
        ANS_LOGI("update callback unset");
        return;
    }

    if (sortingMap == nullptr) {
        ANS_LOGE("sortingMap is null");
        return;
    }
    ANS_LOGI("OnUpdate sortingMap size = %{public}zu", sortingMap->GetKey().size());

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(updateCallbackInfo_.env, &loop);
    if (loop == nullptr) {
        ANS_LOGE("loop instance is nullptr");
        return;
    }

    NotificationReceiveDataWorker *dataWorker = new (std::nothrow) NotificationReceiveDataWorker();
    if (dataWorker == nullptr) {
        ANS_LOGE("new dataWorker failed");
        return;
    }

    dataWorker->sortingMap = sortingMap;
    dataWorker->env = updateCallbackInfo_.env;
    dataWorker->ref = updateCallbackInfo_.ref;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        ANS_LOGE("new work failed");
        return;
    }

    work->data = (void *)dataWorker;

    uv_queue_work(loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            ANS_LOGI("OnUpdate uv_work_t start");

            if (work == nullptr) {
                ANS_LOGE("work is null");
                return;
            }

            NotificationReceiveDataWorker *dataWorkerData = (NotificationReceiveDataWorker *)work->data;
            if (dataWorkerData == nullptr) {
                ANS_LOGE("dataWorkerData is null");
                return;
            }
            napi_value result = nullptr;
            napi_create_object(dataWorkerData->env, &result);
            int error = 0;
            if (!Common::SetNotificationSortingMap(dataWorkerData->env, dataWorkerData->sortingMap, result)) {
                result = Common::NapiGetNull(dataWorkerData->env);
                error = ERROR;
            } else {
                error = NO_ERROR;
            }
            Common::SetCallback(dataWorkerData->env, dataWorkerData->ref, error, result);

            delete dataWorkerData;
            dataWorkerData = nullptr;
            delete work;
            work = nullptr;
        });
}

void SubscriberInstance::OnSubscribeResult(NotificationConstant::SubscribeResult result)
{
    ANS_LOGI("enter");

    if (subscribeCallbackInfo_.ref == nullptr) {
        ANS_LOGI("subscribe callback unset");
        return;
    }
    ANS_LOGI("OnSubscribeResult result = %{public}d", result);

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(subscribeCallbackInfo_.env, &loop);
    if (loop == nullptr) {
        ANS_LOGE("loop instance is nullptr");
        return;
    }

    NotificationReceiveDataWorker *dataWorker = new (std::nothrow) NotificationReceiveDataWorker();
    if (dataWorker == nullptr) {
        ANS_LOGE("new dataWorker failed");
        return;
    }

    dataWorker->result = (int)result;
    dataWorker->env = subscribeCallbackInfo_.env;
    dataWorker->ref = subscribeCallbackInfo_.ref;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        ANS_LOGE("new work failed");
        return;
    }

    work->data = (void *)dataWorker;

    uv_queue_work(loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            ANS_LOGI("OnSubscribeResult uv_work_t start");

            if (work == nullptr) {
                ANS_LOGE("work is null");
                return;
            }

            NotificationReceiveDataWorker *dataWorkerData = (NotificationReceiveDataWorker *)work->data;
            if (dataWorkerData == nullptr) {
                ANS_LOGE("dataWorkerData is null");
                return;
            }
            // result: number
            napi_value result = nullptr;
            napi_create_int32(dataWorkerData->env, dataWorkerData->result, &result);

            Common::SetCallback(dataWorkerData->env, dataWorkerData->ref, NO_ERROR, result);

            delete dataWorkerData;
            dataWorkerData = nullptr;
            delete work;
            work = nullptr;
        });
}

void SubscriberInstance::OnUnsubscribeResult(NotificationConstant::SubscribeResult result)
{
    ANS_LOGI("enter");

    if (unsubscribeCallbackInfo_.ref == nullptr) {
        ANS_LOGI("unsubscribe callback unset");
        return;
    }
    ANS_LOGI("OnUnsubscribeResult result = %{public}d", result);

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(unsubscribeCallbackInfo_.env, &loop);
    if (loop == nullptr) {
        ANS_LOGE("loop instance is nullptr");
        return;
    }

    NotificationReceiveDataWorker *dataWorker = new (std::nothrow) NotificationReceiveDataWorker();
    if (dataWorker == nullptr) {
        ANS_LOGE("new dataWorker failed");
        return;
    }

    dataWorker->result = (int)result;
    dataWorker->env = unsubscribeCallbackInfo_.env;
    dataWorker->ref = unsubscribeCallbackInfo_.ref;
    dataWorker->subscriber = this;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        ANS_LOGE("new work failed");
        return;
    }

    work->data = (void *)dataWorker;

    uv_queue_work(loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            ANS_LOGI("OnUnsubscribeResult uv_work_t start");

            if (work == nullptr) {
                ANS_LOGE("work is null");
                return;
            }

            NotificationReceiveDataWorker *dataWorkerData = (NotificationReceiveDataWorker *)work->data;
            if (dataWorkerData == nullptr) {
                ANS_LOGE("dataWorkerData is null");
                return;
            }
            // result: number
            napi_value result = nullptr;
            napi_create_int32(dataWorkerData->env, dataWorkerData->result, &result);

            Common::SetCallback(dataWorkerData->env, dataWorkerData->ref, NO_ERROR, result);

            DelSubscriberInstancesInfo(dataWorkerData->env, dataWorkerData->subscriber);

            delete dataWorkerData;
            dataWorkerData = nullptr;
            delete work;
            work = nullptr;
        });
}

void SubscriberInstance::OnDied()
{
    ANS_LOGI("enter");

    if (dieCallbackInfo_.ref == nullptr) {
        ANS_LOGE("die callback unset");
        return;
    }

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(dieCallbackInfo_.env, &loop);
    if (loop == nullptr) {
        ANS_LOGE("loop instance is nullptr");
        return;
    }

    NotificationReceiveDataWorker *dataWorker = new (std::nothrow) NotificationReceiveDataWorker();
    if (dataWorker == nullptr) {
        ANS_LOGE("new dataWorker failed");
        return;
    }

    dataWorker->env = dieCallbackInfo_.env;
    dataWorker->ref = dieCallbackInfo_.ref;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        ANS_LOGE("new work failed");
        return;
    }

    work->data = (void *)dataWorker;

    uv_queue_work(loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            ANS_LOGI("OnDied uv_work_t start");

            if (work == nullptr) {
                ANS_LOGE("work is null");
                return;
            }

            NotificationReceiveDataWorker *dataWorkerData = (NotificationReceiveDataWorker *)work->data;
            if (dataWorkerData == nullptr) {
                ANS_LOGE("dataWorkerData is null");
                return;
            }

            Common::SetCallback(
                dataWorkerData->env, dataWorkerData->ref, NO_ERROR, Common::NapiGetNull(dataWorkerData->env));

            delete dataWorkerData;
            dataWorkerData = nullptr;
            delete work;
            work = nullptr;
        });
}

void SubscriberInstance::OnDisturbModeChanged(int disturbMode)
{
    ANS_LOGI("enter");

    if (disturbModeCallbackInfo_.ref == nullptr) {
        ANS_LOGI("disturbModeChange callback unset");
        return;
    }
    ANS_LOGI("OnDisturbModeChanged disturbMode = %{public}d", disturbMode);

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(disturbModeCallbackInfo_.env, &loop);
    if (loop == nullptr) {
        ANS_LOGE("loop instance is nullptr");
        return;
    }

    NotificationReceiveDataWorker *dataWorker = new (std::nothrow) NotificationReceiveDataWorker();
    if (dataWorker == nullptr) {
        ANS_LOGE("new dataWorker failed");
        return;
    }

    dataWorker->disturbMode = disturbMode;
    dataWorker->env = disturbModeCallbackInfo_.env;
    dataWorker->ref = disturbModeCallbackInfo_.ref;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        ANS_LOGE("new work failed");
        return;
    }

    work->data = (void *)dataWorker;

    uv_queue_work(loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            ANS_LOGI("OnDisturbModeChanged uv_work_t start");

            if (work == nullptr) {
                ANS_LOGE("work is null");
                return;
            }

            NotificationReceiveDataWorker *dataWorkerData = (NotificationReceiveDataWorker *)work->data;
            if (dataWorkerData == nullptr) {
                ANS_LOGE("dataWorkerData is null");
                return;
            }
            // disturbMode: number
            napi_value result = nullptr;
            napi_create_int32(dataWorkerData->env, dataWorkerData->disturbMode, &result);

            Common::SetCallback(dataWorkerData->env, dataWorkerData->ref, NO_ERROR, result);

            delete dataWorkerData;
            dataWorkerData = nullptr;
            delete work;
            work = nullptr;
        });
}

void SubscriberInstance::SetCancelCallbackInfo(const napi_env &env, const napi_ref &ref)
{
    canceCallbackInfo_.env = env;
    canceCallbackInfo_.ref = ref;
}

void SubscriberInstance::SetConsumeCallbackInfo(const napi_env &env, const napi_ref &ref)
{
    consumeCallbackInfo_.env = env;
    consumeCallbackInfo_.ref = ref;
}

void SubscriberInstance::SetUpdateCallbackInfo(const napi_env &env, const napi_ref &ref)
{
    updateCallbackInfo_.env = env;
    updateCallbackInfo_.ref = ref;
}

void SubscriberInstance::SetSubscribeCallbackInfo(const napi_env &env, const napi_ref &ref)
{
    subscribeCallbackInfo_.env = env;
    subscribeCallbackInfo_.ref = ref;
}

void SubscriberInstance::SetUnsubscribeCallbackInfo(const napi_env &env, const napi_ref &ref)
{
    unsubscribeCallbackInfo_.env = env;
    unsubscribeCallbackInfo_.ref = ref;
}

void SubscriberInstance::SetDieCallbackInfo(const napi_env &env, const napi_ref &ref)
{
    dieCallbackInfo_.env = env;
    dieCallbackInfo_.ref = ref;
}

void SubscriberInstance::SetDisturbModeCallbackInfo(const napi_env &env, const napi_ref &ref)
{
    disturbModeCallbackInfo_.env = env;
    disturbModeCallbackInfo_.ref = ref;
}

void SubscriberInstance::SetCallbackInfo(const napi_env &env, const std::string &type, const napi_ref &ref)
{
    if (type == CONSUME) {
        SetConsumeCallbackInfo(env, ref);
    } else if (type == CANCEL) {
        SetCancelCallbackInfo(env, ref);
    } else if (type == UPDATE) {
        SetUpdateCallbackInfo(env, ref);
    } else if (type == CONNECTED) {
        SetSubscribeCallbackInfo(env, ref);
    } else if (type == DIS_CONNECTED) {
        SetUnsubscribeCallbackInfo(env, ref);
    } else if (type == DIE) {
        SetDieCallbackInfo(env, ref);
    } else if (type == DISTURB_MODE_CHANGE) {
        SetDisturbModeCallbackInfo(env, ref);
    } else {
        ANS_LOGW("type is error");
    }
}

struct AsyncCallbackInfoSubscribe {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    NotificationSubscribeInfo subscriberInfo;
    CallbackPromiseInfo info;
};

bool HasNotificationSubscriber(const napi_env &env, const napi_value &value, SubscriberInstancesInfo &subscriberInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto vec : subscriberInstances_) {
        napi_value callback = nullptr;
        napi_get_reference_value(env, vec.ref, &callback);
        bool isEquals = false;
        napi_strict_equals(env, value, callback, &isEquals);
        if (isEquals) {
            subscriberInfo = vec;
            return true;
        }
    }
    return false;
}

napi_value GetNotificationSubscriber(
    const napi_env &env, const napi_value &value, SubscriberInstancesInfo &subscriberInfo)
{
    ANS_LOGI("enter");
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    napi_ref result = nullptr;

    subscriberInfo.subscriber = new (std::nothrow) SubscriberInstance();
    if (subscriberInfo.subscriber == nullptr) {
        ANS_LOGE("subscriber is null");
        return nullptr;
    }

    napi_create_reference(env, value, 1, &subscriberInfo.ref);

    // onConsume?:(data: SubscribeCallbackData) => void
    NAPI_CALL(env, napi_has_named_property(env, value, "onConsume", &hasProperty));
    if (hasProperty) {
        napi_value nOnConsumed = nullptr;
        napi_get_named_property(env, value, "onConsume", &nOnConsumed);
        NAPI_CALL(env, napi_typeof(env, nOnConsumed, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, nOnConsumed, 1, &result);
        subscriberInfo.subscriber->SetCallbackInfo(env, CONSUME, result);
    }
    // onCancel?:(data: SubscribeCallbackData) => void
    NAPI_CALL(env, napi_has_named_property(env, value, "onCancel", &hasProperty));
    if (hasProperty) {
        napi_value nOnCanceled = nullptr;
        napi_get_named_property(env, value, "onCancel", &nOnCanceled);
        NAPI_CALL(env, napi_typeof(env, nOnCanceled, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, nOnCanceled, 1, &result);
        subscriberInfo.subscriber->SetCallbackInfo(env, CANCEL, result);
    }
    // onUpdate?:(data: NotificationSortingMap) => void
    NAPI_CALL(env, napi_has_named_property(env, value, "onUpdate", &hasProperty));
    if (hasProperty) {
        napi_value nOnUpdate = nullptr;
        napi_get_named_property(env, value, "onUpdate", &nOnUpdate);
        NAPI_CALL(env, napi_typeof(env, nOnUpdate, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, nOnUpdate, 1, &result);
        subscriberInfo.subscriber->SetCallbackInfo(env, UPDATE, result);
    }
    // onConnect?:() => void
    NAPI_CALL(env, napi_has_named_property(env, value, "onConnect", &hasProperty));
    if (hasProperty) {
        napi_value nOnConnected = nullptr;
        napi_get_named_property(env, value, "onConnect", &nOnConnected);
        NAPI_CALL(env, napi_typeof(env, nOnConnected, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, nOnConnected, 1, &result);
        subscriberInfo.subscriber->SetCallbackInfo(env, CONNECTED, result);
    }
    // onDisconnect?:() => void
    NAPI_CALL(env, napi_has_named_property(env, value, "onDisconnect", &hasProperty));
    if (hasProperty) {
        napi_value nOnDisConnect = nullptr;
        napi_get_named_property(env, value, "onDisconnect", &nOnDisConnect);
        NAPI_CALL(env, napi_typeof(env, nOnDisConnect, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, nOnDisConnect, 1, &result);
        subscriberInfo.subscriber->SetCallbackInfo(env, DIS_CONNECTED, result);
    }
    // onDestroy?:() => void
    NAPI_CALL(env, napi_has_named_property(env, value, "onDestroy", &hasProperty));
    if (hasProperty) {
        napi_value nOnDied = nullptr;
        napi_get_named_property(env, value, "onDestroy", &nOnDied);
        NAPI_CALL(env, napi_typeof(env, nOnDied, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, nOnDied, 1, &result);
        subscriberInfo.subscriber->SetCallbackInfo(env, DIE, result);
    }
    // onDisturbModeChange?:(mode: notification.DoNotDisturbMode) => void
    NAPI_CALL(env, napi_has_named_property(env, value, "onDisturbModeChange", &hasProperty));
    if (hasProperty) {
        napi_value nOnDisturbModeChanged = nullptr;
        napi_get_named_property(env, value, "onDisturbModeChange", &nOnDisturbModeChanged);
        NAPI_CALL(env, napi_typeof(env, nOnDisturbModeChanged, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, nOnDisturbModeChanged, 1, &result);
        subscriberInfo.subscriber->SetCallbackInfo(env, DISTURB_MODE_CHANGE, result);
    }

    return Common::NapiGetNull(env);
}

bool AddSubscriberInstancesInfo(const napi_env &env, const SubscriberInstancesInfo &subscriberInfo)
{
    ANS_LOGI("enter");
    if (subscriberInfo.ref == nullptr) {
        ANS_LOGE("subscriberInfo.ref is null");
        return false;
    }
    if (subscriberInfo.subscriber == nullptr) {
        ANS_LOGE("subscriberInfo.subscriber is null");
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    subscriberInstances_.emplace_back(subscriberInfo);

    return true;
}

bool DelSubscriberInstancesInfo(const napi_env &env, SubscriberInstance *subscriber)
{
    ANS_LOGI("enter");
    if (subscriber == nullptr) {
        ANS_LOGE("subscriber is null");
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = subscriberInstances_.begin(); it != subscriberInstances_.end(); ++it) {
        if ((*it).subscriber == subscriber) {
            if ((*it).ref != nullptr) {
                napi_delete_reference(env, (*it).ref);
            }
            delete (*it).subscriber;
            (*it).subscriber = nullptr;
            subscriberInstances_.erase(it);
            return true;
        }
    }
    return false;
}
napi_value ParseParameters(const napi_env &env, const napi_callback_info &info,
    NotificationSubscribeInfo &subscriberInfo, SubscriberInstance *&subscriber, napi_ref &callback)
{
    ANS_LOGI("enter");

    size_t argc = SUBSRIBE_MAX_PARA;
    napi_value argv[SUBSRIBE_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valuetype = napi_undefined;

    // argv[0]:subscriber
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type for arg0. NotificationSubscriber object expected.");

    SubscriberInstancesInfo subscriberInstancesInfo;
    if (!HasNotificationSubscriber(env, argv[0], subscriberInstancesInfo)) {
        if (GetNotificationSubscriber(env, argv[0], subscriberInstancesInfo) == nullptr) {
            ANS_LOGE("NotificationSubscriber parse failed");
            return nullptr;
        }
        if (!AddSubscriberInstancesInfo(env, subscriberInstancesInfo)) {
            ANS_LOGE("AddSubscriberInstancesInfo add failed");
            return nullptr;
        }
    }
    subscriber = subscriberInstancesInfo.subscriber;

    // argv[1]:callback or NotificationSubscribeInfo
    if (argc >= SUBSRIBE_MAX_PARA - 1) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env,
            (valuetype == napi_function) || (valuetype == napi_object),
            "Wrong argument type for arg1. Function or NotificationSubscribeInfo object expected.");
        if (valuetype == napi_function) {
            napi_create_reference(env, argv[1], 1, &callback);
        } else {
            if (Common::GetNotificationSubscriberInfo(env, argv[1], subscriberInfo) == nullptr) {
                ANS_LOGE("NotificationSubscribeInfo parse failed");
                return nullptr;
            }
        }
    }

    // argv[2]:callback
    if (argc >= SUBSRIBE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[SUBSRIBE_MAX_PARA - 1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[SUBSRIBE_MAX_PARA - 1], 1, &callback);
    }

    return Common::NapiGetNull(env);
}

napi_value Subscribe(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    napi_ref callback = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    NotificationSubscribeInfo subscriberInfo;
    if (ParseParameters(env, info, subscriberInfo, objectInfo, callback) == nullptr) {
        return Common::JSParaError(env, callback);
    }
    ANS_LOGI("Subscribe objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoSubscribe *asynccallbackinfo = new (std::nothrow) AsyncCallbackInfoSubscribe {
        .env = env, .asyncWork = nullptr, .objectInfo = objectInfo, .subscriberInfo = subscriberInfo
    };
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "subscribeNotification", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("Subscribe napi_create_async_work start");
            AsyncCallbackInfoSubscribe *asynccallbackinfo = (AsyncCallbackInfoSubscribe *)data;
            if (asynccallbackinfo->subscriberInfo.hasSubscribeInfo) {
                ANS_LOGI("Subscribe with NotificationSubscribeInfo");
                OHOS::Notification::NotificationSubscribeInfo subscribeInfo;
                subscribeInfo.AddAppNames(asynccallbackinfo->subscriberInfo.bundleNames);
                asynccallbackinfo->info.errorCode =
                    NotificationHelper::SubscribeNotification(*(asynccallbackinfo->objectInfo), subscribeInfo);
            } else {
                asynccallbackinfo->info.errorCode =
                    NotificationHelper::SubscribeNotification(*(asynccallbackinfo->objectInfo));
            }
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("Subscribe napi_create_async_work end");
            AsyncCallbackInfoSubscribe *asynccallbackinfo = (AsyncCallbackInfoSubscribe *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, Common::NapiGetNull(env));

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

}  // namespace NotificationNapi
}  // namespace OHOS
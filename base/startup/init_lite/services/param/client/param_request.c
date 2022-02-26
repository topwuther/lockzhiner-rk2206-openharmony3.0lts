/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "param_request.h"

#include <string.h>
#include <unistd.h>
#include <time.h>

#include "param_manager.h"
#include "uv.h"

#define LABEL "Client"
#define BUFFER_SIZE  200
#define ParamEntry(ptr, type, member)   (type *)((char *)(ptr) - offsetof(type, member))

static ParamWorkSpace g_paramWorkSpaceReadOnly = {ATOMIC_VAR_INIT(0), {}, {}, {}};

static void OnWrite(uv_write_t *req, int status)
{
    PARAM_LOGD("OnWrite status %d", status);
}

static void OnReceiveAlloc(uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf)
{
    // 这里需要按实际回复大小申请内存，不需要大内存
    buf->base = (char *)malloc(sizeof(ResponseMsg));
    PARAM_CHECK(buf->base != NULL, return, "OnReceiveAlloc malloc failed");
    buf->len = sizeof(ResponseMsg);
    PARAM_LOGD("OnReceiveAlloc handle %p %zu", handle, suggestedSize);
}

static void OnReceiveResponse(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
{
    RequestNode *req = ParamEntry(handle, RequestNode, handle);
    PARAM_LOGD("OnReceiveResponse %p", handle);
    if (nread <= 0 || buf == NULL || handle == NULL || buf->base == NULL) {
        if (buf != NULL && buf->base != NULL) {
            free(buf->base);
        }
        if (handle != NULL) {
            uv_close((uv_handle_t*)handle, NULL);
            uv_stop(req->loop);
        }
        return;
    }
    ResponseMsg *response = (ResponseMsg *)(buf->base);
    PARAM_CHECK(response != NULL, return, "The response is null");
    PARAM_LOGD("OnReceiveResponse %p cmd %d result: %d", handle, response->type, response->result);
    switch (response->type) {
        case SET_PARAM:
            req->result = response->result;
            break;
        default:
            PARAM_LOGE("not supported the command: %d", response->type);
            break;
    }
    PARAM_LOGD("Close handle %p", handle);
    free(buf->base);
    uv_close((uv_handle_t*)handle, NULL);
    uv_stop(req->loop);
}

static void OnConnection(uv_connect_t *connect, int status)
{
    PARAM_CHECK(status >= 0, return, "Failed to conntect status %s", uv_strerror(status));
    RequestNode *request = ParamEntry(connect, RequestNode, connect);
    PARAM_LOGD("Connect to server handle %p", &(request->handle));
    uv_buf_t buf = uv_buf_init((char*)&request->msg, request->msg.contentSize + sizeof(request->msg));
    int ret = uv_write2(&request->wr, (uv_stream_t*)&(request->handle), &buf, 1,
        (uv_stream_t*)&(request->handle), OnWrite);
    PARAM_CHECK(ret >= 0, return, "Failed to uv_write2 porperty");

    // read result
    ret = uv_read_start((uv_stream_t*)&(request->handle), OnReceiveAlloc, OnReceiveResponse);
    PARAM_CHECK(ret >= 0, return, "Failed to uv_read_start response");
}

static int StartRequest(int cmd, RequestNode *request)
{
    PARAM_CHECK(request != NULL, return -1, "Invalid request");
    request->result = -1;
    request->msg.type = cmd;
    request->loop = uv_loop_new();
    PARAM_CHECK(request->loop != NULL, return -1, "StartRequest uv_loop_new failed");
    uv_pipe_init(request->loop, &request->handle, 1);
    uv_pipe_connect(&request->connect, &request->handle, PIPE_NAME, OnConnection);
    uv_run(request->loop, UV_RUN_DEFAULT);
    uv_loop_delete(request->loop);
    int result = request->result;
    free(request);
    return result;
}

int SystemSetParameter(const char *name, const char *value)
{
    PARAM_CHECK(name != NULL && value != NULL, return -1, "Invalid param");
    int ret = CheckParamName(name, 0);
    PARAM_CHECK(ret == 0, return ret, "Illegal param name");
    const u_int32_t len = 2;
    PARAM_LOGD("StartRequest %s", name);
    u_int32_t msgSize = sizeof(RequestMsg) + strlen(name) + strlen(value) + len;
    if (strlen(value) >= PARAM_VALUE_LEN_MAX) {
        return -1;
    }
    RequestNode *request = (RequestNode *)malloc(sizeof(RequestNode) + msgSize);
    PARAM_CHECK(request != NULL, return -1, "Failed to malloc for connect");

    memset_s(request, sizeof(RequestNode), 0, sizeof(RequestNode));
    // 带字符串结束符
    int contentSize = BuildParamContent(request->msg.content, msgSize - sizeof(RequestMsg), name, value);
    PARAM_CHECK(contentSize > 0, free(request); return -1, "Failed to copy porperty");
    request->msg.contentSize = contentSize;
    return StartRequest(SET_PARAM, request);
}

int SystemGetParameter(const char *name, char *value, unsigned int *len)
{
    PARAM_CHECK(name != NULL && len != NULL, return -1, "The name or value is null");
    InitParamWorkSpace(&g_paramWorkSpaceReadOnly, 1, NULL);

    ParamHandle handle = 0;
    int ret = ReadParamWithCheck(&g_paramWorkSpaceReadOnly, name, &handle);
    PARAM_CHECK(ret == 0, return ret, "Can not get param for %s", name);
    return ReadParamValue(&g_paramWorkSpaceReadOnly, handle, value, len);
}

int SystemGetParameterName(ParamHandle handle, char *name, unsigned int len)
{
    PARAM_CHECK(name != NULL && handle != 0, return -1, "The name is null");
    InitParamWorkSpace(&g_paramWorkSpaceReadOnly, 1, NULL);
    return ReadParamName(&g_paramWorkSpaceReadOnly, handle, name, len);
}

int SystemGetParameterValue(ParamHandle handle, char *value, unsigned int *len)
{
    PARAM_CHECK(len != NULL && handle != 0, return -1, "The value is null");
    InitParamWorkSpace(&g_paramWorkSpaceReadOnly, 1, NULL);
    return ReadParamValue(&g_paramWorkSpaceReadOnly, handle, value, len);
}

int SystemTraversalParameter(void (*traversalParameter)(ParamHandle handle, void* cookie), void* cookie)
{
    PARAM_CHECK(traversalParameter != NULL, return -1, "The param is null");
    InitParamWorkSpace(&g_paramWorkSpaceReadOnly, 1, NULL);
    return TraversalParam(&g_paramWorkSpaceReadOnly, traversalParameter, cookie);
}

const char *SystemDetectParamChange(ParamCache *cache,
    ParamEvaluatePtr evaluate, u_int32_t count, const char *parameters[][2])
{
    PARAM_CHECK(cache != NULL && evaluate != NULL && parameters != NULL, return NULL, "The param is null");
    return DetectParamChange(&g_paramWorkSpaceReadOnly, cache, evaluate, count, parameters);
}

/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#include "param_service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "init_param.h"
#include "init_utils.h"
#include "param_manager.h"
#include "param_request.h"
#include "sys_param.h"
#include "uv.h"

#define BUFFER_SIZE 256
#define LABEL "Server"

static char *g_initContext = "";
static ParamWorkSpace g_paramWorkSpace = {ATOMIC_VAR_INIT(0), {}, {}, {}};

void InitParamService()
{
    int ret = InitParamWorkSpace(&g_paramWorkSpace, 0, g_initContext);
    PARAM_CHECK(ret == 0, return, "Init parameter workspace fail");
}

int LoadDefaultParams(const char *fileName)
{
    u_int32_t flags = atomic_load_explicit(&g_paramWorkSpace.flags, memory_order_relaxed);
    if ((flags & WORKSPACE_FLAGS_INIT) != WORKSPACE_FLAGS_INIT) {
        return PARAM_CODE_NOT_INIT;
    }
    char *real = realpath(fileName, NULL);
    PARAM_CHECK(real != NULL, return -1, "Can not get real path %s", fileName);
    FILE *fp = fopen(real, "r");
    free(real);
    PARAM_CHECK(fp != NULL, return -1, "Open file %s fail", fileName);
    char buff[BUFFER_SIZE] = {0};
    SubStringInfo *info = malloc(sizeof(SubStringInfo) * (SUBSTR_INFO_LABEL + 1));
    PARAM_CHECK(info != NULL,
        fclose(fp);
        fp = NULL;
        return -1, "malloc failed");

    while (fgets(buff, BUFFER_SIZE, fp) != NULL) {
        int subStrNumber = GetSubStringInfo(buff, strlen(buff), '=',  info, SUBSTR_INFO_LABEL + 1);
        if (subStrNumber <= SUBSTR_INFO_LABEL) {
            continue;
        }

        if (strncmp(info[0].value, "ctl.", strlen("ctl.")) == 0) {
            PARAM_LOGE("Do not set ctl. parameters from init %s", info[0].value);
            continue;
        }
        if (strcmp(info[0].value, "selinux.restorecon_recursive") == 0) {
            PARAM_LOGE("Do not set selinux.restorecon_recursive from init %s", info[0].value);
            continue;
        }
        int ret = CheckParamName(info[0].value, 0);
        PARAM_CHECK(ret == 0, continue, "Illegal param name %s", info[0].value);

        ret = WriteParam(&g_paramWorkSpace.paramSpace, info[0].value, info[1].value);
        PARAM_CHECK(ret == 0, continue, "Failed to set param %d %s", ret, buff);
    }
    fclose(fp);
    free(info);
    PARAM_LOGI("LoadDefaultParams proterty success %s", fileName);
    return 0;
}

int LoadParamInfos(const char *fileName)
{
    u_int32_t flags = atomic_load_explicit(&g_paramWorkSpace.flags, memory_order_relaxed);
    if ((flags & WORKSPACE_FLAGS_INIT) != WORKSPACE_FLAGS_INIT) {
        return PARAM_CODE_NOT_INIT;
    }
    char *real = realpath(fileName, NULL);
    PARAM_CHECK(real != NULL, return -1, "Can not get real path %s", fileName);
    FILE *fp = fopen(real, "r");
    free(real);
    PARAM_CHECK(fp != NULL, return -1, "Open file %s fail", fileName);
    SubStringInfo *info = malloc(sizeof(SubStringInfo) * SUBSTR_INFO_MAX);
    PARAM_CHECK(info != NULL,
        fclose(fp);
        fp = NULL;
        return -1, "Load parameter malloc failed.");
    char buff[BUFFER_SIZE] = {0};
    int infoCount = 0;
    while (fgets(buff, BUFFER_SIZE, fp) != NULL) {
        int subStrNumber = GetSubStringInfo(buff, strlen(buff), ' ',  info, SUBSTR_INFO_MAX);
        if (subStrNumber <= 0) {
            continue;
        }
        int ret = WriteParamInfo(&g_paramWorkSpace, info, subStrNumber);
        PARAM_CHECK(ret == 0, continue, "Failed to write param info %d %s", ret, buff);
        infoCount++;
    }
    fclose(fp);
    free(info);
    PARAM_LOGI("Load parameter info %d success %s", infoCount, fileName);
    return 0;
}

static int ProcessParamSet(const RequestMsg *msg)
{
    PARAM_CHECK(msg != NULL, return PARAM_CODE_INVALID_PARAM, "Failed to check param");

    SubStringInfo info[3];
    int ret = GetSubStringInfo(msg->content, msg->contentSize, '=', info,
        ARRAY_LENGTH(info));
    PARAM_CHECK(ret >= 2, return ret, "Failed to get name from content %s", msg->content);

    PARAM_LOGD("ProcessParamSet name %s value: %s", info[0].value, info[1].value);
    ret = WriteParamWithCheck(&g_paramWorkSpace, &msg->securitylabel, info[0].value, info[1].value);
    PARAM_CHECK(ret == 0, return ret, "Failed to set param %d name %s %s", ret, info[0].value, info[1].value);
    ret = WritePersistParam(info[0].value, info[1].value);
    PARAM_CHECK(ret == 0, return ret, "Failed to set param");
    // notify event to process trigger
    PostTrigger(EVENT_PROPERTY, msg->content, msg->contentSize);
    return 0;
}

static void OnClose(uv_handle_t *handle)
{
    free(handle);
}

static void OnReceiveAlloc(uv_handle_t *handle, size_t suggestedSize, uv_buf_t* buf)
{
    // 这里需要按实际消息的大小申请内存，取最大消息的长度
    const unsigned int tmp = 2;
    buf->len = sizeof(RequestMsg) + BUFFER_SIZE * tmp;
    buf->base = (char *)malloc(buf->len);
}

static void OnWriteResponse(uv_write_t *req, int status)
{
    // 发送成功，释放请求内存
    PARAM_LOGD("OnWriteResponse status %d", status);
    ResponseNode *node = (ResponseNode*)req;
    free(node);
}

static void SendResponse(const uv_stream_t *handle, RequestType type, int result, const char *content, int size)
{
    int ret = 0;
    // 申请整块内存，用于回复数据和写请求
    PARAM_CHECK(size >= 0, return, "Invalid content size %d", size);
    ResponseNode *response = (ResponseNode *)malloc(sizeof(ResponseNode) + size);
    PARAM_CHECK(response != NULL, return, "Failed to alloc memory for response");
    response->msg.type = type;
    response->msg.contentSize = size;
    response->msg.result = result;
    if (content != NULL && size != 0) {
        ret = memcpy_s(response->msg.content, size, content, size);
        PARAM_CHECK(ret == 0,
            free(response);
            response = NULL;
            return, "Failed to copy content");
    }
    uv_buf_t buf = uv_buf_init((char *)&response->msg, sizeof(response->msg) + size);
    ret = uv_write2(&response->writer, handle, &buf, 1, handle, OnWriteResponse);
    PARAM_CHECK(ret >= 0,
        free(response);
        response = NULL;
        return, "Failed to uv_write2 ret %s", uv_strerror(ret));
}

static void OnReceiveRequest(uv_stream_t *handle, ssize_t nread, uv_buf_t *buf)
{
    if (nread <= 0 || buf == NULL || buf->base == NULL) {
        uv_close((uv_handle_t*)handle, OnClose);
        if (buf != NULL && buf->base != NULL) {
            free(buf->base);
            buf->base = NULL;
        }
        return;
    }
    RequestMsg *msg = (RequestMsg *)buf->base;
    switch (msg->type) {
        case SET_PARAM: {
            int ret = ProcessParamSet(msg);
            SendResponse(handle, SET_PARAM, ret, NULL, 0);
            break;
        }
        default:
            PARAM_LOGE("not supported the command: %d", msg->type);
            break;
    }
    free(buf->base);
    buf->base = NULL;
    uv_close((uv_handle_t*)handle, OnClose);
}

static void OnConnection(uv_stream_t *server, int status)
{
    PARAM_CHECK(status >= 0, return, "Error status %d", status);
    PARAM_CHECK(server != NULL, return, "Error server");
    uv_pipe_t *stream = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
    PARAM_CHECK(stream != NULL, return, "Failed to alloc stream");

    int ret = uv_pipe_init(uv_default_loop(), (uv_pipe_t*)stream, 1);
    PARAM_CHECK(ret == 0, free(stream); return, "Failed to uv_pipe_init %d", ret);

    stream->data = server;
    ret = uv_accept(server, (uv_stream_t *)stream);
    PARAM_CHECK(ret == 0, uv_close((uv_handle_t*)stream, NULL); free(stream);
        return, "Failed to uv_accept %d", ret);

    ret = uv_read_start((uv_stream_t *)stream, OnReceiveAlloc, OnReceiveRequest);
    PARAM_CHECK(ret == 0, uv_close((uv_handle_t*)stream, NULL); free(stream);
        return, "Failed to uv_read_start %d", ret);
}

void StopParamService()
{
    uv_fs_t req;
    uv_fs_unlink(uv_default_loop(), &req, PIPE_NAME, NULL);
    CloseParamWorkSpace(&g_paramWorkSpace);
    ClosePersistParamWorkSpace();
    uv_stop(uv_default_loop());
    PARAM_LOGI("StopParamService.");
}

int StartParamService()
{
    PARAM_LOGI("StartParamService.");
    uv_fs_t req;
    uv_fs_unlink(uv_default_loop(), &req, PIPE_NAME, NULL);

    uv_pipe_t pipeServer;
    int ret = uv_pipe_init(uv_default_loop(), &pipeServer, 0);
    PARAM_CHECK(ret == 0, return ret, "Failed to uv_pipe_init %d", ret);
    ret = uv_pipe_bind(&pipeServer, PIPE_NAME);
    PARAM_CHECK(ret == 0, return ret, "Failed to uv_pipe_bind %d %s", ret, uv_err_name(ret));
    ret = chmod(PIPE_NAME, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    PARAM_CHECK(ret == 0, return ret, "Failed to chmod %s, err %d. ", PIPE_NAME, errno);
    ret = uv_listen((uv_stream_t*)&pipeServer, SOMAXCONN, OnConnection);
    PARAM_CHECK(ret == 0, return ret, "Failed to uv_listen %d %s", ret, uv_err_name(ret));
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    PARAM_LOGI("Start service exit.");
    return 0;
}

int SystemWriteParam(const char *name, const char *value)
{
    PARAM_CHECK(name != NULL && value != NULL, return -1, "The name is null");
    PARAM_LOGI("SystemWriteParam name %s value: %s", name, value);
    int ret = WriteParamWithCheck(&g_paramWorkSpace, &g_paramWorkSpace.label, name, value);
    PARAM_CHECK(ret == 0, return ret, "Failed to set param %s", name);
    ret = WritePersistParam(name, value);
    PARAM_CHECK(ret == 0, return ret, "Failed to set persist param %s", name);

    // notify event to process trigger
    PostParamTrigger(name, value);
    return ret;
}

int SystemReadParam(const char *name, char *value, unsigned int *len)
{
    PARAM_CHECK(name != NULL && len != NULL, return -1, "The name is null");
    ParamHandle handle = 0;
    int ret = ReadParamWithCheck(&g_paramWorkSpace, name, &handle);
    if (ret == 0) {
        ret = ReadParamValue(&g_paramWorkSpace, handle, value, len);
    }
    return ret;
}

ParamWorkSpace *GetParamWorkSpace()
{
    return &g_paramWorkSpace;
}

int LoadPersistParams()
{
    return RefreshPersistParams(&g_paramWorkSpace, g_initContext);
}

int SystemTraversalParam(void (*traversalParameter)(ParamHandle handle, void* cookie), void* cookie)
{
    PARAM_CHECK(traversalParameter != NULL, return -1, "The param is null");
    return TraversalParam(&g_paramWorkSpace, traversalParameter, cookie);
}

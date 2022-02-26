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
#include "audio_proxy_manager.h"
#include <servmgr_hdi.h>
#include "audio_proxy_common.h"
#include "audio_proxy_internal.h"

#define HDI_SERVER_NAME  "audio_hdi_service"
#define CONFIG_FRAME_SIZE      (1024 * 2 * 1)
#define FRAME_SIZE              1024
#define CONFIG_FRAME_COUNT     ((8000 * 2 * 1 + (CONFIG_FRAME_SIZE - 1)) / CONFIG_FRAME_SIZE)

int32_t AudioProxyManagerGetAllAdapters(struct AudioProxyManager *manager,
                                        struct AudioAdapterDescriptor **descs,
                                        int *size)
{
    LOG_FUN_INFO();
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    int32_t ret;
    if (manager == NULL || manager->remote == NULL || descs == NULL || size == NULL) {
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessSBuf(&data, &reply) < 0) {
        return HDF_FAILURE;
    }
    ret = AudioAdaptersForUser(descs, size);
    if (ret < 0) {
        AudioProxyBufReplyRecycle(data, reply);
        LOG_FUN_ERR("AudioAdaptersForUser FAIL!");
        return ret;
    }
    ret = AudioProxyDispatchCall(manager->remote, AUDIO_HDI_MGR_GET_ALL_ADAPTER, data, reply);
    if (ret != HDF_SUCCESS) {
        AudioProxyBufReplyRecycle(data, reply);
        LOG_FUN_ERR("Failed to send service call!");
        return ret;
    }
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}

int32_t AudioProxyManagerLoadAdapter(struct AudioProxyManager *manager, const struct AudioAdapterDescriptor *desc,
                                     struct AudioAdapter **adapter)
{
    LOG_FUN_INFO();
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;

    if (manager == NULL || manager->remote == NULL || desc == NULL ||
        desc->adapterName == NULL || desc->ports == NULL || adapter == NULL) {
        return HDF_FAILURE;
    }
    if (AudioAdapterExist(desc->adapterName)) {
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessSBuf(&data, &reply) < 0) {
        return HDF_FAILURE;
    }
    // adapterName
    if (!HdfSbufWriteString(data, desc->adapterName)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    // dir
    uint32_t tempDir = (uint32_t)desc->ports->dir;
    if (!HdfSbufWriteUint32(data, tempDir)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    struct AudioHwAdapter *hwAdapter = (struct AudioHwAdapter *)calloc(1, sizeof(struct AudioHwAdapter));
    if (hwAdapter == NULL) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret;
    ret = AudioProxyDispatchCall(manager->remote, AUDIO_HDI_MGR_LOAD_ADAPTER, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("Failed to send service call!!");
        AudioProxyBufReplyRecycle(data, reply);
        AudioMemFree((void **)&hwAdapter);
        return ret;
    }
    /* check return result from server first */
    hwAdapter->common.InitAllPorts = AudioProxyAdapterInitAllPorts;
    hwAdapter->common.CreateRender = AudioProxyAdapterCreateRender;
    hwAdapter->common.DestroyRender = AudioProxyAdapterDestroyRender;
    hwAdapter->common.CreateCapture = AudioProxyAdapterCreateCapture;
    hwAdapter->common.DestroyCapture = AudioProxyAdapterDestroyCapture;
    hwAdapter->common.GetPortCapability = AudioProxyAdapterGetPortCapability;
    hwAdapter->common.SetPassthroughMode = AudioProxyAdapterSetPassthroughMode;
    hwAdapter->common.GetPassthroughMode = AudioProxyAdapterGetPassthroughMode;
    hwAdapter->adapterDescriptor = *desc;
    hwAdapter->proxyRemoteHandle = manager->remote; // get dispatch Server
    *adapter = &hwAdapter->common;
    AudioProxyBufReplyRecycle(data, reply);
    LOG_FUN_INFO();
    return HDF_SUCCESS;
}

void AudioProxyManagerUnloadAdapter(struct AudioProxyManager *manager, struct AudioAdapter *adapter)
{
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    const char *adapterName = NULL;
    int32_t i = 0;
    int32_t portNum;
    struct AudioHwAdapter *hwAdapter = (struct AudioHwAdapter *)adapter;
    if (manager == NULL || manager->remote == NULL || adapter == NULL) {
        return;
    }
    if (hwAdapter->portCapabilitys != NULL) {
        portNum = hwAdapter->adapterDescriptor.portNum;
        while (i < portNum) {
            if (&hwAdapter->portCapabilitys[i] != NULL) {
                AudioMemFree((void **)&hwAdapter->portCapabilitys[i].capability.subPorts);
            }
            i++;
        }
        AudioMemFree((void **)&hwAdapter->portCapabilitys);
    }
    if (AudioProxyPreprocessSBuf(&data, &reply) == HDF_SUCCESS) {
        adapterName = hwAdapter->adapterDescriptor.adapterName;
        if (HdfSbufWriteString(data, adapterName)) {
            int32_t ret = AudioProxyDispatchCall(manager->remote, AUDIO_HDI_MGR_UNLOAD_ADAPTER, data, reply);
            if (ret < 0) {
                LOG_FUN_ERR("Send Server fail!");
            }
        }
        AudioProxyBufReplyRecycle(data, reply);
    }
    AudioMemFree((void **)&adapter);
    return;
}

static void ProxyMgrConstruct(struct AudioProxyManager *proxyMgr)
{
    proxyMgr->GetAllAdapters = AudioProxyManagerGetAllAdapters;
    proxyMgr->LoadAdapter = AudioProxyManagerLoadAdapter;
    proxyMgr->UnloadAdapter = AudioProxyManagerUnloadAdapter;
}

struct AudioProxyManager *GetAudioProxyManagerFuncs(void)
{
    LOG_FUN_INFO();
    struct HDIServiceManager *serviceMgr = HDIServiceManagerGet();
    if (serviceMgr == NULL) {
        LOG_FUN_ERR("HDIServiceManagerGet failed!");
        return NULL;
    }
    struct HdfRemoteService *remote = serviceMgr->GetService(serviceMgr, HDI_SERVER_NAME);
    if (remote == NULL) {
        LOG_FUN_ERR("Remote GetService failed!");
        HDIServiceManagerRelease(serviceMgr);
        return NULL;
    }
    HDIServiceManagerRelease(serviceMgr);
    struct AudioProxyManager *proxyDevMgr = OsalMemAlloc(sizeof(struct AudioProxyManager));
    if (proxyDevMgr == NULL) {
        LOG_FUN_ERR("malloc failed!");
        HdfRemoteServiceRecycle(remote);
        return NULL;
    }
    ProxyMgrConstruct(proxyDevMgr);
    proxyDevMgr->remote = remote;
    return proxyDevMgr;
}


/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "coap_discover.h"
#include "coap_adapter.h"
#include "coap_socket.h"
#include "json_payload.h"
#include "nstackx_common.h"
#include "nstackx_device.h"
#include "nstackx_error.h"
#include "os_adapter.h"

#if defined(__LITEOS_M__) || defined(__LITEOS_RISCV__)
#include "lwip/sockets.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "wifi_device.h"
#include "wifi_hotspot_config.h"
#include <cmsis_os.h>
#else
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#if defined(__LITEOS_A__)
#include "common/wpa_ctrl.h"
#endif

#include <securec.h>

#define WIFI_QUEUE_SIZE  5
#define MIN_STACK_SIZE 0x8000
typedef struct {
    WIFI_PROC_FUNC     handler;
    AddressEvent       state;
} AddressEventHandler;

int g_wifiQueueId = -1;
int g_queryIpFlag = -1;
unsigned int g_wifiTaskStart = 0;
WIFI_PROC_FUNC g_wifiCallback = NULL;

#define COAP_DEFAULT_PRIO 11
#define TEN_MS  (10 * 1000)
#define HUNDRED_MS  (100 * 1000)
typedef struct CoapRequest {
    const char *remoteUrl;
    char *data;
    size_t dataLength;
    const char *remoteIp;
} CoapRequest;


unsigned int g_terminalFlag = 0;

#if defined(__LITEOS_M__) || defined(__LITEOS_RISCV__)
#define WLAN_NAME "wlan0"
static WifiEvent g_coapEventHandler = {0};
static void CoapConnectionChangedHandler(int state, WifiLinkedInfo* info);
osThreadId_t g_coapTaskId = NULL;
osThreadId_t g_msgQueTaskId = NULL;
#else
#define WLAN "wlan0"
#define ETH  "eth0"
intptr_t g_coapTaskId = -1;
intptr_t g_msgQueTaskId = -1;
intptr_t g_queryIpTaskId = -1;
unsigned int g_updateIpFlag = 0;
typedef void *(*Runnable)(void *argv);
typedef struct ThreadAttr ThreadAttr;
struct ThreadAttr {
    const char *name;
    uint32_t stackSize;
    uint8_t priority;
    uint8_t reserved1;
    uint16_t reserved2;
};

int CreateThread(Runnable run, void *argv, const ThreadAttr *attr, unsigned int *threadId)
{
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setstacksize(&threadAttr, (attr->stackSize | MIN_STACK_SIZE));
    struct sched_param sched = {attr->priority};
    pthread_attr_setschedparam(&threadAttr, &sched);
    int errCode = pthread_create((pthread_t *)threadId, &threadAttr, run, argv);
    return errCode;
}
#endif

static int CoapSendRequest(const CoapRequest *coapRequest)
{
    if (coapRequest == NULL || coapRequest->remoteUrl == NULL) {
        return NSTACKX_EFAILED;
    }

    struct sockaddr_in sockAddr = {0};
    if (coapRequest->remoteIp == NULL) {
        return NSTACKX_EFAILED;
    }

    sockAddr.sin_addr.s_addr = inet_addr(coapRequest->remoteIp);
    sockAddr.sin_port = htons(COAP_DEFAULT_PORT);
    sockAddr.sin_family = AF_INET;

    int ret = CoapCreateUdpClient(&sockAddr);
    if (ret != NSTACKX_EOK) {
        return NSTACKX_EFAILED;
    }
    SocketInfo socket = {0};
    socket.cliendFd = GetCoapClientSocket();
    socket.dstAddr = sockAddr;
    if (CoapSocketSend(&socket, (uint8_t *)coapRequest->data, coapRequest->dataLength) == -1) {
        SOFTBUS_PRINT("[DISCOVERY]reponse coap failed.\r\n");
        return NSTACKX_EFAILED;
    }
    return NSTACKX_EOK;
}

static int CoapResponseService(const COAP_Packet *pkt, const char* remoteUrl, const char* remoteIp)
{
    int ret;
    CoapRequest coapRequest;
    (void)memset_s(&coapRequest, sizeof(coapRequest), 0, sizeof(coapRequest));
    coapRequest.remoteUrl = remoteUrl;
    coapRequest.remoteIp = remoteIp;
    char *payload = PrepareServiceDiscover();
    if (payload == NULL) {
        return NSTACKX_EFAILED;
    }

    COAP_ReadWriteBuffer sndPktBuff = {0};
    sndPktBuff.readWriteBuf = calloc(1, COAP_MAX_PDU_SIZE);
    if (sndPktBuff.readWriteBuf == NULL) {
        free(payload);
        return NSTACKX_EFAILED;
    }
    sndPktBuff.size = COAP_MAX_PDU_SIZE;
    sndPktBuff.len = 0;

    ret = BuildSendPkt(pkt, remoteIp, payload, &sndPktBuff);
    free(payload);
    if (ret != DISCOVERY_ERR_SUCCESS) {
        free(sndPktBuff.readWriteBuf);
        sndPktBuff.readWriteBuf = NULL;
        return ret;
    }
    coapRequest.data = sndPktBuff.readWriteBuf;
    coapRequest.dataLength = sndPktBuff.len;
    ret = CoapSendRequest(&coapRequest);
    free(sndPktBuff.readWriteBuf);
    sndPktBuff.readWriteBuf = NULL;

    return ret;
}

int GetServiceDiscoverInfo(const uint8_t *buf, size_t size, DeviceInfo *deviceInfo, char **remoteUrlPtr)
{
    uint8_t *newBuf = NULL;
    if (size <= 0) {
        return NSTACKX_EFAILED;
    }

    if (buf[size - 1] != '\0') {
        newBuf = (uint8_t *)calloc(1, size + 1);
        if (newBuf == NULL) {
            return NSTACKX_ENOMEM;
        }
        if (memcpy_s(newBuf, size + 1, buf, size) != EOK) {
            goto L_COAP_ERR;
        }
        buf = newBuf;
    }

    if (ParseServiceDiscover(buf, deviceInfo, remoteUrlPtr) != NSTACKX_EOK) {
        goto L_COAP_ERR;
    }

    if (newBuf != NULL) {
        free(newBuf);
    }

    return NSTACKX_EOK;
L_COAP_ERR:
    if (newBuf != NULL) {
        free(newBuf);
    }
    return NSTACKX_EFAILED;
}

void PostServiceDiscover(const COAP_Packet *pkt)
{
    char *remoteUrl = NULL;
    DeviceInfo deviceInfo;

    if (pkt == NULL) {
        return;
    }

    (void)memset_s(&deviceInfo, sizeof(deviceInfo), 0, sizeof(deviceInfo));
    if (GetServiceDiscoverInfo(pkt->payload.buffer, pkt->payload.len, &deviceInfo, &remoteUrl) != NSTACKX_EOK) {
        return;
    }

    char wifiIpAddr[NSTACKX_MAX_IP_STRING_LEN];
    (void)memset_s(wifiIpAddr, sizeof(wifiIpAddr), 0, sizeof(wifiIpAddr));
    (void)inet_ntop(AF_INET, &deviceInfo.netChannelInfo.wifiApInfo.ip, wifiIpAddr, sizeof(wifiIpAddr));

    if (remoteUrl != NULL) {
        CoapResponseService(pkt, remoteUrl, wifiIpAddr);
        free(remoteUrl);
    }
}

static void HandleReadEvent(int fd)
{
    int socketFd = fd;
    unsigned char *recvBuffer = calloc(1, COAP_MAX_PDU_SIZE + 1);
    if (recvBuffer == NULL) {
        return;
    }
    ssize_t nRead;
    nRead = CoapSocketRecv(socketFd, recvBuffer, COAP_MAX_PDU_SIZE);
    if ((nRead == 0) || (nRead < 0 && errno != EAGAIN &&
        errno != EWOULDBLOCK && errno != EINTR)) {
        free(recvBuffer);
        return;
    }

    COAP_Packet decodePacket;
    (void)memset_s(&decodePacket, sizeof(COAP_Packet), 0, sizeof(COAP_Packet));
    decodePacket.protocol = COAP_UDP;
    COAP_SoftBusDecode(&decodePacket, recvBuffer, nRead);
    PostServiceDiscover(&decodePacket);
    free(recvBuffer);
}

#define TIME_MICRO_SEC 10000
static void CoapReadHandle(unsigned int uwParam1, unsigned int uwParam2, unsigned int uwParam3, unsigned int uwParam4)
{
    (void)uwParam1;
    (void)uwParam2;
    (void)uwParam3;
    (void)uwParam4;
    int ret;
    fd_set readSet;
    int serverFd = GetCoapServerSocket();
    SOFTBUS_PRINT("[DISCOVERY] CoapReadHandle coin select begin\n");
    while (g_terminalFlag) {
        FD_ZERO(&readSet);
        FD_SET(serverFd, &readSet);
        ret = select(serverFd + 1, &readSet, NULL, NULL, NULL);
        if (ret > 0) {
            if (FD_ISSET(serverFd, &readSet)) {
                HandleReadEvent(serverFd);
            }
        } else {
            SOFTBUS_PRINT("[DISCOVERY]ret:%d,error:%d\n", ret, errno);
        }
    }
    SOFTBUS_PRINT("[DISCOVERY] CoapReadHandle exit\n");
}

void RegisterWifiCallback(WIFI_PROC_FUNC callback)
{
    g_wifiCallback = callback;
}

void CoapHandleWifiEvent(unsigned int para)
{
    if (g_wifiCallback != NULL) {
        g_wifiCallback(para);
    }
}

void CoapWriteMsgQueue(int state)
{
    SOFTBUS_PRINT("[DISCOVERY] CoapWriteMsgQueue\n");
    AddressEventHandler handler;
    handler.handler = CoapHandleWifiEvent;
    handler.state = state;
    /* while a new event coming, it must stop the previous loop */
    g_queryIpFlag = 0;
    (void)WriteMsgQue(g_wifiQueueId, &handler, sizeof(AddressEventHandler));
}

void CoapWifiEventThread(unsigned int uwParam1, unsigned int uwParam2, unsigned int uwParam3, unsigned int uwParam4)
{
    (void)uwParam1;
    (void)uwParam2;
    (void)uwParam3;
    (void)uwParam4;

    AddressEventHandler handle;
    unsigned int readSize;
    unsigned int ret;
    readSize = sizeof(AddressEventHandler);
    g_wifiTaskStart = 1;
    while (g_wifiTaskStart) {
        ret = ReadMsgQue(g_wifiQueueId, &handle, &readSize);
        if ((ret == 0) && (readSize == sizeof(AddressEventHandler))) {
            if (handle.handler == NULL) {
                continue;
            }
            handle.handler(handle.state);
        }
    }
}

int CoapDeinitWifiEvent(void)
{
    unsigned int ret;
    g_wifiTaskStart = 0;
#if defined(__LITEOS_M__) || defined(__LITEOS_RISCV__)
    if (g_msgQueTaskId != NULL) {
        ret = osThreadTerminate(g_msgQueTaskId);
        if (ret != 0) {
            return -1;
        }
        g_msgQueTaskId = NULL;
    }
#else
    if (g_msgQueTaskId != -1) {
        ret = pthread_join((pthread_t)g_msgQueTaskId, NULL);
        if (ret != 0) {
            SOFTBUS_PRINT("[DISCOVERY] pthread_join fail\n");
            return -1;
        }
        g_msgQueTaskId = -1;
    }
#endif

    if (g_wifiQueueId != -1) {
        ret = DeleteMsgQue(g_wifiQueueId);
        if (ret != 0) {
            return -1;
        }
        g_wifiQueueId = -1;
    }
#if defined(__LITEOS_M__) || defined(__LITEOS_RISCV__)
    WifiErrorCode error = UnRegisterWifiEvent(&g_coapEventHandler);
    if (error != WIFI_SUCCESS) {
        return -1;
    }
    g_coapEventHandler.OnWifiConnectionChanged = NULL;
#endif
    return NSTACKX_EOK;
}

int CreateMsgQueThread(void)
{
#if defined(__LITEOS_M__) || defined(__LITEOS_RISCV__)
    if (g_msgQueTaskId != NULL) {
        return NSTACKX_EOK;
    }

    osThreadAttr_t attr;
    attr.name = "wifi_event";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    attr.priority = osPriorityNormal4; // COAP_DEFAULT_PRIO -> cmsis prio

    g_msgQueTaskId = osThreadNew((osThreadFunc_t)CoapWifiEventThread, NULL, &attr);
    if (NULL == g_msgQueTaskId) {
        SOFTBUS_PRINT("[DISCOVERY]Task Create fail\n");
        return NSTACKX_EOK;
    }
#else
    int ret;

    if (g_msgQueTaskId != -1) {
        return NSTACKX_EOK;
    }

    ThreadAttr attr = {"wifi_event", 0x800, 20, 0, 0};
    ret = CreateThread((Runnable)CoapWifiEventThread, NULL, &attr, (unsigned int*)&g_msgQueTaskId);
    if (ret != 0) {
        SOFTBUS_PRINT("[DISCOVERY]Task Create fail\n");
        return ret;
    }
#endif
    return NSTACKX_EOK;
}

int CoapInitWifiEvent(void)
{
    SOFTBUS_PRINT("[DISCOVERY] CoapInitWifiEvent\n");
    unsigned int ret;
    if (g_wifiQueueId == -1) {
        ret = CreateMsgQue("/wifiQue",
            WIFI_QUEUE_SIZE, (unsigned int*)&g_wifiQueueId,
            0, sizeof(AddressEventHandler));
        if (ret != 0) {
            SOFTBUS_PRINT("[DISCOVERY]CreateMsgQue fail\n");
            (void)CoapDeinitWifiEvent();
            return ret;
        }

#if defined(__LITEOS_M__) || defined(__LITEOS_RISCV__)
        g_coapEventHandler.OnWifiConnectionChanged = CoapConnectionChangedHandler;
        WifiErrorCode error = RegisterWifiEvent(&g_coapEventHandler);
        if (error != WIFI_SUCCESS) {
            SOFTBUS_PRINT("[DISCOVERY]RegisterWifiEvent fail, error:%d\n", error);
            (void)CoapDeinitWifiEvent();
            g_wifiQueueId = -1;
            return error;
        }
#endif
    }
    return NSTACKX_EOK;
}

#if defined(__LITEOS_M__) || defined(__LITEOS_RISCV__)
static void CoapConnectionChangedHandler(int state, WifiLinkedInfo* info)
{
    (void)info;
    CoapWriteMsgQueue(state);
}

void CoapGetWifiIp(char *ip, int length)
{
    struct netif *netif = NULL;
    int ret;

    netif = netif_find(WLAN_NAME);
    if (netif == NULL) {
        return;
    }

    ip4_addr_t ipAddr;
    ip4_addr_t netMask;
    ip4_addr_t gateWay;

    ret = netifapi_netif_get_addr(netif, &ipAddr, &netMask, &gateWay);
    if (ret == 0) {
        inet_ntop(AF_INET, &ipAddr, ip, INET_ADDRSTRLEN);
    }
    (void)length;
    return;
}
#else
typedef enum {
    STATUS_UP,
    STATUS_DOWN
} InterfaceStatus;

typedef struct {
    char ip[NSTACKX_MAX_IP_STRING_LEN];
    InterfaceStatus status;
    int flag;
} InterfaceInfo;

void GetInterfaceInfo(int fd, const char* interfaceName, int length, InterfaceInfo *info)
{
    if (fd < 0 || info == NULL || interfaceName == NULL) {
        return;
    }
    struct ifreq ifr;
    memset_s(&ifr, sizeof(struct ifreq), 0, sizeof(struct ifreq));
    int ret = strncpy_s(ifr.ifr_name, sizeof(ifr.ifr_name), interfaceName, length);
    if (ret != EOK) {
        SOFTBUS_PRINT("[DISCOVERY] CoapGetWifiIp ifr.ifr_name cpy fail\n");
        return;
    }

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        return;
    }

    if ((unsigned short int)ifr.ifr_flags & IFF_UP) {
        info->status = STATUS_UP;
    } else {
        info->status = STATUS_DOWN;
        info->ip[0] = '\0';
        info->flag = 1;
        return;
    }
    memset_s(&ifr, sizeof(struct ifreq), 0, sizeof(struct ifreq));
    ret = strncpy_s(ifr.ifr_name, sizeof(ifr.ifr_name), interfaceName, length);
    if (ret != EOK) {
        SOFTBUS_PRINT("[DISCOVERY] CoapGetWifiIp ifr.ifr_name cpy fail\n");
        return;
    }

    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        return;
    }
    ret = strcpy_s(info->ip, sizeof(info->ip),
        inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));
    if (ret != EOK) {
        SOFTBUS_PRINT("[DISCOVERY] CoapGetWifiIp cpy fail\n");
    }

    info->flag = 1;
    return;
}

void CoapGetWifiIp(char *ip, int length)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return;
    }
    InterfaceInfo info = {0};
    GetInterfaceInfo(fd, ETH, strlen(ETH) + 1, &info);
    if (!info.flag) {
        memset_s(&info, sizeof(info), 0, sizeof(info));
        GetInterfaceInfo(fd, WLAN, strlen(WLAN) + 1, &info);
    }

    if (!info.flag) {
        close(fd);
        return;
    }

    if (strcpy_s(ip, length, info.ip) != EOK) {
        SOFTBUS_PRINT("[DISCOVERY]copy ip to dst failed.\n");
    }

    close(fd);
}

#if defined(__LITEOS_A__)
static int CheckEthInfoAndNotify(InterfaceInfo *prev, const InterfaceInfo *new)
{
    if (prev == NULL || new == NULL) {
        return NSTACKX_EFAILED;
    }
    if (prev->flag == 0 || new->flag == 0) {
        prev->status = new->status;
        if (memcpy_s(prev->ip, sizeof(prev->ip), new->ip, sizeof(new->ip))) {
            SOFTBUS_PRINT("[DISCOVERY]fail copy ip to prev.\n");
        }
        return NSTACKX_EFAILED;
    }

    if (prev->status != new->status) {
        if (new->status == STATUS_DOWN) {
            SOFTBUS_PRINT("Eth disconnected.\r\n");
            CoapWriteMsgQueue(CLEAR_IP_EVENT);
        } else {
            SOFTBUS_PRINT("Eth connected.\r\n");
            CoapWriteMsgQueue(UPDATE_IP_EVENT);
        }
        prev->status = new->status;
        if (memcpy_s(prev->ip, sizeof(prev->ip), new->ip, sizeof(new->ip))) {
            SOFTBUS_PRINT("[DISCOVERY]fail copy ip to prev.\n");
        }
    }
    return NSTACKX_EOK;
}

static int SendCtrlCommand(const char *cmd, struct wpa_ctrl *ctrlConn, char *reply, size_t *replyLen)
{
    size_t len = *replyLen - 1;
    wpa_ctrl_request(ctrlConn, cmd, strlen(cmd), reply, &len, 0);
    if (len != 0 && strncmp(reply, "FAIL", strlen("FAIL"))) {
        *replyLen = len;
        return 0;
    }
    SOFTBUS_PRINT("send ctrl request [%s] failed.", cmd);
    return -1;
}

#define WPA_CONNECTED "wpa_state=COMPLETED"
#define WPA_DISCONNECTED "wpa_state=SCANNING"
#define REPLY_LENGTH 512
void CheckWlanInfoAndNotify(int *prev, int first)
{
    struct wpa_ctrl *ctrlConn = wpa_ctrl_open(WLAN);
    char reply[REPLY_LENGTH] = {0};
    if (ctrlConn == NULL) {
        SOFTBUS_PRINT("open wpa control interface failed.");
        return;
    }
    char *cmd = "STATUS";
    size_t len = REPLY_LENGTH;
    int ret = SendCtrlCommand(cmd, ctrlConn, reply, &len);
    if (ret != 0) {
        SOFTBUS_PRINT("reply:%s\n.", reply);
        wpa_ctrl_close(ctrlConn);
        return;
    }

    int status;
    if (strstr(reply, WPA_CONNECTED)) {
        status = UPDATE_IP_EVENT;
    } else if (strstr(reply, WPA_DISCONNECTED)) {
        status = CLEAR_IP_EVENT;
    } else {
        wpa_ctrl_close(ctrlConn);
        return;
    }

    if (first) {
        *prev = status;
        wpa_ctrl_close(ctrlConn);
        return;
    }

    if (*prev != status) {
        CoapWriteMsgQueue(status);
    }
    *prev = status;
    wpa_ctrl_close(ctrlConn);
    return;
}

void CoapQueryIpHandle(unsigned int uwParam1, unsigned int uwParam2, unsigned int uwParam3, unsigned int uwParam4)
{
    (void)uwParam1;
    (void)uwParam2;
    (void)uwParam3;
    (void)uwParam4;
    g_updateIpFlag = 1;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    InterfaceInfo ethInfo;
    InterfaceInfo prevEthInfo;
    memset_s(&prevEthInfo, sizeof(prevEthInfo), 0, sizeof(prevEthInfo));
    GetInterfaceInfo(fd, ETH, strlen(ETH) + 1, &prevEthInfo);
    int updateFlag;
    int prevFlag;
    CheckWlanInfoAndNotify(&prevFlag, 1);
    while (g_updateIpFlag) {
        usleep(HUNDRED_MS);
        memset_s(&ethInfo, sizeof(ethInfo), 0, sizeof(ethInfo));
        GetInterfaceInfo(fd, ETH, strlen(ETH) + 1, &ethInfo);
        updateFlag = CheckEthInfoAndNotify(&prevEthInfo, &ethInfo);
        if (updateFlag != -1) {
            continue;
        }
        CheckWlanInfoAndNotify(&prevFlag, 0);
    }
    close(fd);
}

int CreateQueryIpThread(void)
{
    if (g_queryIpTaskId != -1) {
        return NSTACKX_EOK;
    }
    ThreadAttr attr = {"queryIp_task", 0x800, 20, 0, 0};
    int error = CreateThread((Runnable)CoapQueryIpHandle, NULL, &attr, (unsigned int*)&g_queryIpTaskId);
    if (error != 0) {
        SOFTBUS_PRINT("[DISCOVERY] create task fail\n");
        return NSTACKX_EFAILED;
    }
    return NSTACKX_EOK;
}

void ExitQueryIpThread(void)
{
    g_updateIpFlag = 0;
    if (g_queryIpTaskId != -1) {
        int ret = pthread_join((pthread_t)g_queryIpTaskId, NULL);
        if (ret != 0) {
            SOFTBUS_PRINT("[DISCOVERY] ExitQueryIpThread pthread_join fail\n");
            return;
        }
    }
}
#endif
#endif

int CreateCoapListenThread(void)
{
    g_terminalFlag = 1;

#if defined(__LITEOS_M__) || defined(__LITEOS_RISCV__)
    if (g_coapTaskId != NULL) {
        return NSTACKX_EOK;
    }

    osThreadAttr_t attr;
    attr.name = "coap_listen_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    attr.priority = osPriorityNormal4; // COAP_DEFAULT_PRIO -> cmsis prio

    g_coapTaskId = osThreadNew((osThreadFunc_t)CoapReadHandle, NULL, &attr);
    if (g_coapTaskId == NULL) {
        g_terminalFlag = 0;
        SOFTBUS_PRINT("[DISCOVERY] create task fail\n");
        return NSTACKX_EFAILED;
    }
#else
    if (g_coapTaskId != -1) {
        return NSTACKX_EOK;
    }

    ThreadAttr attr = {"coap_listen_task", 0x800, 20, 0, 0};
    int error = CreateThread((Runnable)CoapReadHandle, NULL, &attr, (unsigned int *)&g_coapTaskId);
    if (error != 0) {
        g_terminalFlag = 0;
        SOFTBUS_PRINT("[DISCOVERY] create task fail\n");
        return NSTACKX_EFAILED;
    }
#endif
    return NSTACKX_EOK;
}

void ExitCoapListenThread(void)
{
    g_terminalFlag = 0;
#if defined(__LITEOS_M__) || defined(__LITEOS_RISCV__)
    if (g_coapTaskId != NULL) {
        if (osThreadTerminate(g_coapTaskId) != 0) {
            SOFTBUS_PRINT("[DISCOVERY] ExitCoapListenThread pthread_join fail\n");
            return;
        }
    }
#else
    if (g_coapTaskId != -1) {
        int ret = pthread_join((pthread_t)g_coapTaskId, NULL);
        if (ret != 0) {
            SOFTBUS_PRINT("[DISCOVERY] ExitCoapListenThread pthread_join fail\n");
            return;
        }
    }
#endif
}

int CoapInitDiscovery(void)
{
    int ret = CoapInitSocket();
    if (ret != NSTACKX_EOK) {
        SOFTBUS_PRINT("[DISCOVERY] Init socket fail\n");
        return ret;
    }

    ret = CoapInitWifiEvent();
    if (ret != NSTACKX_EOK) {
        SOFTBUS_PRINT("[DISCOVERY] Init wifi event fail\n");
        return ret;
    }
#if defined(__LITEOS_A__)
    ret = CreateQueryIpThread();
    if (ret != NSTACKX_EOK) {
        SOFTBUS_PRINT("[DISCOVERY] Init query Ip fail\n");
        return ret;
    }
#endif
    if (CreateMsgQueThread() != NSTACKX_EOK) {
        return NSTACKX_EFAILED;
    }
    return CreateCoapListenThread();
}

int CoapDeinitDiscovery(void)
{
    ExitCoapListenThread();
    int rtn = CoapDeinitWifiEvent();
    if (rtn != NSTACKX_EOK) {
        return NSTACKX_EFAILED;
    }
#if defined(__LITEOS_A__)
    ExitQueryIpThread();
#endif
    CoapDeinitSocket();
    return NSTACKX_EOK;
}

#define GET_IP_TIMES 300
#define GET_IP_INFINITE (-1)
#define IP_MAX_LEN 15
#define IP_MIN_LEN 7
int CheckIpIsValid(const char *ip, int length)
{
    if (ip == NULL || length < IP_MIN_LEN || length > IP_MAX_LEN) {
        return -1;
    }

    if (strcmp(ip, "0.0.0.0") == 0) {
        return -1;
    }
    int pos = length - 1;
    if (ip[pos] == '1' && ip[pos - 1] == '.') {
        return -1;
    }
    return 0;
}
void CoapGetIp(char *ip, int length, int finite)
{
    if (ip == NULL || length != NSTACKX_MAX_IP_STRING_LEN) {
        return;
    }

    g_queryIpFlag = 1;
    int count = finite ? GET_IP_TIMES : GET_IP_INFINITE;
    while (g_queryIpFlag) {
        CoapGetWifiIp(ip, length);
        if (CheckIpIsValid(ip, strlen(ip)) == 0) {
            break;
        }

        if (count == 0) {
            break;
        }
        count--;
        usleep(TEN_MS);
    }
    return;
}

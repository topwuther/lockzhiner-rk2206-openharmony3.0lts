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

#include "wpa_work.h"
#include <sys/prctl.h>

#define WPA_IFACE_NAME "wlan0"
#define WIFI_AUTH_FAILED_REASON_STR "WRONG_KEY"
#define WIFI_AUTH_FAILED_REASON_CODE "reason=2"
#define WPA_CTRL_REQUEST_OK "OK"
#define WPA_CTRL_REQUEST_FAIL "FAIL"

#define SAMPLE_INFO(format, args...) \
    do { \
        fprintf(stderr, "\033[1;32m WpaCliSample(%s:%d):\t\033[0m" format, __func__, __LINE__, ##args); \
        printf("\n"); \
    } while (0)

#define SAMPLE_ERROR(format, args...) \
    do { \
        fprintf(stderr, "\033[1;31m WpaCliSample(%s:%d):\t\033[0m" format, __func__, __LINE__, ##args); \
        printf("\n"); \
    } while (0)


static struct wpa_ctrl *g_monitorConn = NULL;
static pthread_t g_threadId = 0;
pthread_mutex_t g_mutex;
static char g_mySsidD[40][40] = {0};
static int g_ssidCount = 0;

static char g_useSsidD[40][40] = {0};
static int g_useSsidCount = 0;

static struct wpa_ctrl *g_ctrlConn = NULL;
static pthread_t g_wpaThreadId = 0;
static pthread_t g_scanThreadId = 0;
static int g_scanAvailable = 0;
static int g_Connect = 0;

static void DumpString(const char *buf, int len, const char *tag)
{
    SAMPLE_INFO("%s dump start.", tag);
    for (int i = 0; i < len; i++) {
        printf("%c", buf[i]);
    }
    printf("\n");

    printf("\n");
    SAMPLE_INFO("%s dump end.", tag);
}

static int StrMatch(const char *a, const char *b)
{
    return strncmp(a, b, strlen(b)) == 0;
}

static int SendCtrlCommand(const char *cmd, char *reply, size_t *replyLen)
{
    int i = 0;
    int reTime = 5;
    while (i++ < reTime) { // check wpa init success
        g_ctrlConn = wpa_ctrl_open(WPA_IFACE_NAME);
        if (g_ctrlConn > 0) {
            break;
        }
        sleep(1);
    }
    if (g_ctrlConn == NULL) {
        printf("%s:%d [ERROR] control connect handle is null\n", __FUNCTION__, __LINE__);
        return -1;
    }
    size_t len = *replyLen - 1;
    printf("SendCtrlCommand cmd=%s.", cmd);
    wpa_ctrl_request(g_ctrlConn, cmd, strlen(cmd), reply, &len, 0);
    DumpString(reply, len, "SendCtrlCommand raw return");
    wpa_ctrl_close(g_ctrlConn);
    g_ctrlConn = NULL;
    if (len != 0 && !StrMatch(reply, WPA_CTRL_REQUEST_FAIL)) {
        *replyLen = len;
        return 0;
    }
    SAMPLE_ERROR("send ctrl request [%s] failed.", cmd);
    return -1;
}

static void ProcessScanResult(const char *buf, int len)
{
    int myUse1 = 1;
    int myUse2 = 2;
    int myUse5 = 5;

    int err = memset_s(g_mySsidD, sizeof(g_mySsidD), 0, sizeof(g_mySsidD));
    g_ssidCount = 0;
    if (err != EOK) {
        return;
    }
    for (int i = 0; i < len; i++) {
        if (buf[i] == 'E' && buf[i + myUse1] == 'S' && buf[i + myUse2] == 'S') {
            int j;
            printf("[ERROR]get the ESS i->%d\n", i);
            i = i + myUse5;
            for (j = 0; i < len && buf[i] != '\n'; i++) {
                g_mySsidD[g_ssidCount][j] = buf[i];
                j++;
            }

            g_ssidCount++;
        }
    }
}

void ResetSSIDBuff(void)
{
    g_useSsidCount = 0;
}

int GetIdNum(void)
{
    return g_useSsidCount;
}

int GetCurrentConnInfo(char *ssid, int len)
{
    int offset = 5;
    char connInfo[2048] = {0};
    size_t infoLen = sizeof(connInfo);
    if (g_Connect == 0) {
        printf("[LOG]GetCurrentConnInfo g_Connect -> 0 \n");
        return -1;
    }

    int ret = SendCtrlCommand("STATUS", connInfo, &infoLen);
    if (ret == -1) {
        printf("[ERROR]GetCurrentConnInfo Command(STATUS) Error \n");
        return -1;
    }
    DumpString(connInfo, infoLen, "connInfo");
    printf("[LOG]end the DumpStrint\n");
    char *pos = strstr(connInfo, "ssid=");
    if (pos == NULL) {
        printf("[ERROR]strstr(ssid) is null");
        return -1;
    }
    if (offset >= strlen(pos)) {
        return -1;
    }
    pos += offset;
    pos = strstr(pos, "ssid=");
    if (pos == NULL) {
        printf("[ERROR]secound strstr(ssid) is null");
        return -1;
    }
    char *end = strchr(pos, '\n');
    if (end == NULL) {
        printf("[ERROR]secound strstr(end) is null");
        return -1;
    }
    int ssidLen = end - pos - offset;
    if (len < ssidLen) {
        return -1;
    }
    int i = 0;
    int myOffset = 5;
    for (pos += myOffset; pos < end; pos++, i++) {
        ssid[i] = *pos;
    }
    return 0;
}

void ExitWpaScan(void)
{
    int ret;
    if (g_scanThreadId != 0) {
        ret = pthread_cancel(g_scanThreadId);
        printf("[INFO]pthread_cancel(g_scanThreadId) ret -> %d \n", ret);
        g_scanThreadId = 0;
    }
    if (g_wpaThreadId != 0) {
        ret = pthread_cancel(g_wpaThreadId);
        printf("[INFO]pthread_cancel(g_wpaThreadId) ret -> %d \n", ret);
        g_wpaThreadId = 0;
    }
}

void ExitWpa(void)
{
    int ret;
    if (g_monitorConn != NULL) {
        wpa_ctrl_close(g_monitorConn);
        printf("[INFO]wpa_ctrl_close(g_monitorConn).\n");
        g_monitorConn = NULL;
    }
    char result[100] = {0};
    size_t len = sizeof(result);
    printf("[INFO]ExitWpa TERMINATE begin.\n");
    SendCtrlCommand("TERMINATE", result, &len);
    printf("[INFO]ExitWpa TERMINATE end.\n");
    sleep(1);
    DeinitWifiService();
    if (g_threadId != 0) {
        ret = pthread_cancel(g_threadId);
        if (ret != 0) {
            printf("[ERROR]pthread_cancel(g_threadId) ret -> %d \n", ret);
        }
    }

    ret = pthread_mutex_destroy(&g_mutex);
    if (ret != 0) {
        printf("[ERROR]pthread_mutex_destroy ret -> %d \n", ret);
    }
}

int GetAndResetScanStat(void)
{
    int ret = g_scanAvailable;
    g_scanAvailable = 0;
    return ret;
}

char *GetSsid(const int ssidNum)
{
    return g_useSsidD[ssidNum];
}

void LockWifiData()
{
    pthread_mutex_lock(&g_mutex);
}

void UnLockWifiData()
{
    pthread_mutex_unlock(&g_mutex);
}

static void CheckSsid(void)
{
    int i, ret, err;
    int chrckSsid = 0x5C;

    err = memset_s(g_useSsidD, sizeof(g_useSsidD), 0, sizeof(g_useSsidD));
    if (err != EOK) {
        printf("[ERROR]memset_s g_useSsidD failed, err = %d\n", err);
        return;
    }
    g_useSsidCount = 0;
    for (i = 0; i < g_ssidCount; i++) {
        int j = i + 1;
        for (; j < g_ssidCount; j++) {
            ret = strcmp(g_mySsidD[i], g_useSsidD[j]);
            if (ret == 0) {
                break;
            }
        }
        if ((g_mySsidD[i][0] == chrckSsid) || (g_mySsidD[i][0] == 0)) {
            continue;
        }
        if (j == g_ssidCount) {
            err = strcpy_s(g_useSsidD[g_useSsidCount], sizeof(g_useSsidD[g_useSsidCount]), g_mySsidD[i]);
            if (err != EOK) {
                continue;
            }
            g_useSsidCount++;
        }
    }
}

static void WifiEventHandler(char *rawEvent, int len)
{
    char *pos = rawEvent;
    if (*pos == '<') {
        pos = strchr(pos, '>');
        if (pos) {
            pos++;
        } else {
            pos = rawEvent;
        }
    }
    if (StrMatch(pos, WPA_EVENT_CONNECTED)) {
        SAMPLE_INFO("WIFI_EVENT_CONNECTED");
        g_Connect = 1;
        return;
    }
    if (StrMatch(pos, WPA_EVENT_SCAN_RESULTS)) {
        pthread_mutex_lock(&g_mutex);
        SAMPLE_INFO("WIFI_EVENT_SCAN_DONE");
        char scanResult[4096] = {0};
        size_t scanLen = sizeof(scanResult);
        SendCtrlCommand("SCAN_RESULTS", scanResult, &scanLen);

        ProcessScanResult(scanResult, scanLen);
        CheckSsid();
        g_scanAvailable = 1;
        pthread_mutex_unlock(&g_mutex);
        return;
    }
    if (StrMatch(pos, WPA_EVENT_TEMP_DISABLED) && strstr(pos, WIFI_AUTH_FAILED_REASON_STR)) {
        SAMPLE_INFO("WIFI_EVENT_WRONG_KEY");
        return;
    }
    if (StrMatch(pos, WPA_EVENT_DISCONNECTED) && !strstr(pos, WIFI_AUTH_FAILED_REASON_CODE)) {
        SAMPLE_INFO("WIFI_EVENT_DISCONNECTED");
        return;
    }
}

static void CliRecvPending(void)
{
    while (wpa_ctrl_pending(g_monitorConn)) {
        char buf[4096];
        size_t len = sizeof(buf) - 1;
        if (wpa_ctrl_recv(g_monitorConn, buf, &len) == 0) {
            buf[len] = '\0';
            SAMPLE_INFO("event received %s", buf);
            WifiEventHandler(buf, len);
        } else {
            SAMPLE_INFO("could not read pending message.");
            break;
        }
    }
}

static void* MonitorTask(void *args)
{
    (void)args;
    int fd, ret;
    fd_set rfd;
    while (1) {
        fd = wpa_ctrl_get_fd(g_monitorConn);
        FD_ZERO(&rfd);
        FD_SET(fd, &rfd);
        ret = select(fd + 1, &rfd, NULL, NULL, NULL);
        if (ret <= 0) {
            SAMPLE_INFO("select failed ret = %d\n", ret);
            break;
        }
        CliRecvPending();
        sleep(1);
    }
    return NULL;
}

static void TestNetworkConfig(const char *gSsid, const char *gPassWord)
{
    char networkId[20] = {0};
    size_t networkIdLen = sizeof(networkId);
    int ret = SendCtrlCommand("DISCONNECT", networkId, &networkIdLen);
    ret += SendCtrlCommand("ADD_NETWORK", networkId, &networkIdLen);
    if (ret != 0) {
        SAMPLE_ERROR("add network failed.");
        return;
    }
    SAMPLE_INFO("add network success, network id [%.*s]", networkIdLen, networkId);
    char reply[100] = {0};
    size_t replyLen = sizeof(reply);
    char cmd[200] = {0};
    int err = sprintf_s(cmd, sizeof(cmd), "SET_NETWORK %.*s ssid \"%s\"", networkIdLen, networkId, gSsid);
    if (err < 0) {
        printf("[ERROR]sprintf_s failed, err = %d\n", err);
        return;
    }
    ret += SendCtrlCommand(cmd, reply, &replyLen);
    replyLen = sizeof(reply);
    err = sprintf_s(cmd, sizeof(cmd), "SET_NETWORK %.*s psk \"%s\"", networkIdLen, networkId, gPassWord);
    if (err < 0) {
        printf("[ERROR]sprintf_s failed, err = %d\n", err);
        return;
    }
    ret += SendCtrlCommand(cmd, reply, &replyLen);
    replyLen = sizeof(reply);
    err = sprintf_s(cmd, sizeof(cmd), "ENABLE_NETWORK %.*s", networkIdLen, networkId);
    if (err < 0) {
        printf("[ERROR]sprintf_s failed, err = %d\n", err);
        return;
    }
    ret += SendCtrlCommand(cmd, reply, &replyLen);
    replyLen = sizeof(reply);
    ret += SendCtrlCommand("RECONNECT", reply, &replyLen);
    replyLen = sizeof(reply);
    if (ret == 0) {
        SAMPLE_INFO("network config success.");
        return;
    }
    err = sprintf_s(cmd, sizeof(cmd), "REMOVE_NETWORK %.*s", networkIdLen, networkId);
    if (err < 0) {
        printf("[ERROR]sprintf_s failed, err = %d\n", err);
        return;
    }
    SendCtrlCommand(cmd, reply, &replyLen);
    SAMPLE_ERROR("network config failed remove network [%.*s].", networkIdLen, networkId);
}

int InitControlInterface()
{
    int i = 0;
    int ret;
    int reTime = 5;
    while (i++ < reTime) { // create control interface for event monitor
        g_monitorConn = wpa_ctrl_open(WPA_IFACE_NAME);
        if (g_ctrlConn > 0) {
            break;
        }
        sleep(1);
    }
    if (!g_monitorConn) {
        SAMPLE_ERROR("open wpa control interface failed.");
        return -1;
    }
    ret = wpa_ctrl_attach(g_monitorConn);
    printf("[INFO]wpa_ctrl_attach return %d.\n", ret);
    if (ret == 0) { // start monitor
        ret = pthread_create(&g_wpaThreadId, NULL, MonitorTask, NULL); // create thread for read event
        if (ret != 0) {
            printf("[ERROR]thread error %s\n", strerror(ret));
            return -1;
        }
        return 0;
    }
    return -1;
}

void* WpaScanThread(void *args)
{
    int mySleep = 2;
    int ret = 0;
    sleep(mySleep);
    if (g_ctrlConn == NULL) {
        ret = InitControlInterface();
        printf("%s:%d [INFO] InitControlInterface return %d.\n", __FUNCTION__, __LINE__, ret);
        if (ret == -1) {
            return NULL;
        }
    }
    char reply[100] = {0};
    size_t replyLen = sizeof(reply);
    g_scanAvailable = 0;
    SendCtrlCommand("SCAN", reply, &replyLen);
    return NULL;
}

void WpaScanReconnect(const char *gSsid, const char *gPassWord, const int hiddenSwitch)
{
    if (HIDDEN_OPEN == hiddenSwitch) {
        TestNetworkConfig(gSsid, gPassWord);
    } else {
        int ret = pthread_create(&g_scanThreadId, NULL, WpaScanThread, NULL); // create thread for read event
        if (ret != 0) {
            printf("[ERROR]thread error %s\n", strerror(ret));
            return;
        }
    }
}

static void *ThreadMain()
{
    prctl(PR_SET_NAME, "WPA_THREAD");
    int i = 0;
    int myfor = 5;
    char *arg[20] = {0};
    arg[i] = (char *)"wpa_supplicant";
    arg[++i] = (char *)"-i";
    arg[++i] = (char *)"wlan0";
    arg[++i] = (char *)"-c";
    arg[++i] = (char *)"/storage/app/run/com.huawei.setting/setting/assets/setting/resources/base/element/wpa_supplicant.conf";

    for (i = 0; i < myfor; i++) {
        printf("[LOG]arg[%d]->%s \n", i, arg[i]);
    }
    wpa_main(myfor, arg);
    return NULL;
}

void WpaClientStart(void)
{
    static int runStatus = 0;
    if (runStatus == 0) {
        int ret = pthread_create(&g_threadId, NULL, ThreadMain, NULL);
        if (ret != 0) {
            printf("[ERROR]thread error %s\n", strerror(ret));
            return;
        }
        ret = pthread_mutex_init(&g_mutex, NULL);
        if (ret != 0) {
            printf("[ERROR]pthread_mutex_init error %s\n", strerror(ret));
            return;
        }
        runStatus = 1;
    }
}

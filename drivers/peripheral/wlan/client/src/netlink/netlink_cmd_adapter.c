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

#include <dirent.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/nl80211.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/msg.h>
#include <netlink/socket.h>
#include <net/if.h>
#include <poll.h>
#include <pthread.h>
#include <securec.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "hilog/log.h"

#define VENDOR_ID 0x001A11

#define LISTEN_FD_NUMS 2

#define EVENT_SOCKET_INDEX 0
#define CTRL_SOCKET_INDEX 1

#define CTRL_SOCKET_WRITE_SIDE 0
#define CTRL_SOCKET_READ_SIDE 1

// vendor subcmd
#define WIFI_SUBCMD_SET_COUNTRY_CODE 0x100E

// vendor attr
enum AndrWifiAttr {
    ANDR_WIFI_ATTRIBUTE_NUM_FEATURE_SET,
    ANDR_WIFI_ATTRIBUTE_FEATURE_SET,
    ANDR_WIFI_ATTRIBUTE_RANDOM_MAC_OUI,
    ANDR_WIFI_ATTRIBUTE_NODFS_SET,
    WIFI_ATTRIBUTE_COUNTRY
};

typedef int32_t (*RespHandler)(struct nl_msg *msg, void *data);

struct WifiHalInfo {
    struct nl_sock *cmdSock;
    int familyId;

    // thread controller info
    pthread_t thread;
    int ctrlSocks[2];
    int running;
};

static struct WifiHalInfo g_wifiHalInfo;

static struct nl_sock *OpenNetlinkSocket()
{
    struct nl_sock *sock = NULL;

    sock = nl_socket_alloc();
    if (sock == NULL) {
        HILOG_ERROR(LOG_DOMAIN, "%s: fail to alloc socket", __FUNCTION__);
        return NULL;
    }

    if (nl_connect(sock, NETLINK_GENERIC) != 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: fail to connect socket", __FUNCTION__);
        nl_socket_free(sock);
        return NULL;
    }
    return sock;
}

static void CloseNetlinkSocket(struct nl_sock *sock)
{
    if (sock != NULL) {
        nl_socket_free(sock);
    }
}

static void *EventThread(void *para)
{
    struct pollfd pollFds[LISTEN_FD_NUMS];
    int32_t rc;

    (void)para;

    memset_s(pollFds, sizeof(pollFds), 0, sizeof(pollFds));
    pollFds[EVENT_SOCKET_INDEX].fd = nl_socket_get_fd(g_wifiHalInfo.cmdSock);
    pollFds[EVENT_SOCKET_INDEX].events = POLLIN | POLLERR;
    rc = socketpair(AF_UNIX, SOCK_STREAM, 0, g_wifiHalInfo.ctrlSocks);
    if (rc != 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: fail socketpair", __FUNCTION__);
        return NULL;
    }
    pollFds[CTRL_SOCKET_INDEX].fd = g_wifiHalInfo.ctrlSocks[CTRL_SOCKET_READ_SIDE];
    pollFds[CTRL_SOCKET_INDEX].events = POLLIN | POLLERR;

    g_wifiHalInfo.running = 1;
    while (1) {
        rc = TEMP_FAILURE_RETRY(poll(pollFds, 2, -1));
        if (rc < 0) {
            HILOG_ERROR(LOG_DOMAIN, "%s: fail poll", __FUNCTION__);
            break;
        } else if (pollFds[EVENT_SOCKET_INDEX].revents & POLLERR) {
            HILOG_ERROR(LOG_DOMAIN, "%s: event socket get POLLERR event", __FUNCTION__);
            break;
        } else if (pollFds[EVENT_SOCKET_INDEX].revents & POLLIN) {
            if (HandleEvent(handle) != RET_CODE_SUCCESS) {
                break;
            }
        } else if (pollFds[CTRL_SOCKET_INDEX].revents & POLLERR) {
            HILOG_ERROR(LOG_DOMAIN, "%s: ctrl socket get POLLERR event", __FUNCTION__);
            break;
        } else if (pollFds[CTRL_SOCKET_INDEX].revents & POLLIN) {
            HILOG_ERROR(LOG_DOMAIN, buf, 0, sizeof(buf));
            ssize_t result2 = TEMP_FAILURE_RETRY(read(pfd[1].fd, buf, sizeof(buf)));
            HILOG_ERROR(LOG_DOMAIN, "%s: Read after POLL returned %zd, error no = %d (%s)",
                __FUNCTION__, result2, errno, strerror(errno));
            if (HandleCtrlEvent() != RET_CODE_SUCCESS) {
                break;
            }
        }
    }

    g_wifiHalInfo.running = 0;
    return NULL;
}

static int32_t SetupEventThread()
{
    int32_t rc;

    rc = pthread_create(&g_wifiHalInfo.thread, NULL, EventThread, NULL);
    if (rc != 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: failed create event thread", __FUNCTION__);
        return RET_CODE_FAILURE;
    }
    return RET_CODE_SUCCESS;
}

static int32_t ConnectCmdSocket()
{
    g_wifiHalInfo.cmdSock = OpenNetlinkSocket();
    if (g_wifiHalInfo.cmdSock == NULL) {
        HILOG_ERROR(LOG_DOMAIN, "%s: fail to open cmd socket", __FUNCTION__);
        return RET_CODE_FAILURE;
    }
    nl_socket_disable_seq_check(g_wifiHalInfo.cmdSock);
    // send find familyId result to Controller
    g_wifiHalInfo.familyId = genl_ctrl_resolve(g_wifiHalInfo.cmdSock,
        NL80211_GENL_NAME);
    if (g_wifiHalInfo.familyId < 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: fail to resolve family", __FUNCTION__);
        CloseNetlinkSocket(g_wifiHalInfo.cmdSock);
        g_wifiHalInfo.cmdSock = NULL;
        return RET_CODE_FAILURE;
    }
    HILOG_INFO(LOG_DOMAIN, "%s: family id: %d", __FUNCTION__, g_wifiHalInfo.familyId);
    return RET_CODE_SUCCESS;
}

static void DisconnectCmdSocket()
{
    CloseNetlinkSocket(g_wifiHalInfo.cmdSock);
    g_wifiHalInfo.cmdSock = NULL;
}

int32_t WifiDriverClientInit(void)
{
    if (g_wifiHalInfo.cmdSock != NULL) {
        HILOG_ERROR(LOG_DOMAIN, "%s: already create cmd socket", __FUNCTION__);
        return RET_CODE_MISUSE;
    }
    return ConnectCmdSocket();
}

int32_t WifiDriverClientDeinit(void)
{
    if (g_wifiHalInfo.cmdSock == NULL) {
        HILOG_ERROR(LOG_DOMAIN, "%s: cmd socket not inited", __FUNCTION__);
        return RET_CODE_MISUSE;
    }
    DisconnectCmdSocket();
    return RET_CODE_SUCCESS;
}

int32_t WifiMsgRegisterEventListener(struct HdfDevEventlistener *listener)
{
    if (listener == NULL) {
        HILOG_ERROR(LOG_DOMAIN, "%s: listener must not null", __FUNCTION__);
        return RET_CODE_INVALID_PARAM;
    }
    if (g_wifiHalInfo.cmdSock == NULL) {
        HILOG_ERROR(LOG_DOMAIN, "%s: not inited", __FUNCTION__);
        return RET_CODE_MISUSE;
    }
    if (g_wifiHalInfo.running > 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: thread already started", __FUNCTION__);
        return RET_CODE_MISUSE;
    }

    return SetupEventThread();
}

static int32_t CmdSocketErrorHandler(struct sockaddr_nl *nla,
    struct nlmsgerr *err, void *arg)
{
    int *ret = (int *)arg;

    (void)nla;
    *ret = err->error;
    return NL_SKIP;
}

static int32_t CmdSocketFinishHandler(struct nl_msg *msg, void *arg)
{
    int *ret = (int *)arg;

    (void)msg;
    *ret = 0;
    return NL_SKIP;
}

static int32_t CmdSocketAckHandler(struct nl_msg *msg, void *arg)
{
    int *err = (int *)arg;

    (void)msg;
    *err = 0;
    return NL_STOP;
}

static int32_t ParserSupportIfType(struct nl_msg *msg, void *arg)
{
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    uint8_t *mode = (uint8_t *)arg;
    int32_t ret, i;
    struct nlattr *nl_mode = NULL;
    int32_t type;

    ret = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
        genlmsg_attrlen(gnlh, 0), NULL);
    if (ret != 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nla_parse failed", __FUNCTION__);
        return NL_SKIP;
    }

    if (tb[NL80211_ATTR_SUPPORTED_IFTYPES] != NULL) {
        nla_for_each_nested(nl_mode, tb[NL80211_ATTR_SUPPORTED_IFTYPES], i) {
            type = nla_type(nl_mode);
            if (type > WIFI_IFTYPE_UNSPECIFIED && type < WIFI_IFTYPE_MAX) {
                mode[type] = 1;
                HILOG_INFO(LOG_DOMAIN, "%s: mode: %d", __FUNCTION__, type);
            }
        }
    }
    return NL_SKIP;
}

static int32_t ParserIsSupportCombo(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *nl_combi;
    struct nlattr *tb_comb[NUM_NL80211_IFACE_COMB];
    uint8_t *isSupportCombo = (uint8_t *)arg;
    int32_t ret, i;
    static struct nla_policy
    iface_combination_policy[NUM_NL80211_IFACE_COMB] = {
        [NL80211_IFACE_COMB_LIMITS] = { .type = NLA_NESTED },
        [NL80211_IFACE_COMB_MAXNUM] = { .type = NLA_U32 },
        [NL80211_IFACE_COMB_NUM_CHANNELS] = { .type = NLA_U32 },
    };

    // parse all enum nl80211_attrs type
    ret = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
        genlmsg_attrlen(gnlh, 0), NULL);
    if (ret != 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nla_parse tb failed", __FUNCTION__);
        return NL_SKIP;
    }

    if (tb[NL80211_ATTR_INTERFACE_COMBINATIONS] != NULL) {
        nla_for_each_nested(nl_combi, tb[NL80211_ATTR_INTERFACE_COMBINATIONS], i) {
            // parse all enum nl80211_if_combination_attrs type
            ret = nla_parse_nested(tb_comb, MAX_NL80211_IFACE_COMB,
                nl_combi, iface_combination_policy);
            if (ret != 0) {
                HILOG_ERROR(LOG_DOMAIN, "%s: nla_parse_nested nl_combi failed", __FUNCTION__);
                return NL_SKIP;
            }
            if (!tb_comb[NL80211_IFACE_COMB_LIMITS] ||
                !tb_comb[NL80211_IFACE_COMB_MAXNUM] ||
                !tb_comb[NL80211_IFACE_COMB_NUM_CHANNELS]) {
                    *isSupportCombo = 0;
            } else {
                    *isSupportCombo = 1;
            }
        }
    }
    HILOG_INFO(LOG_DOMAIN, "%s: isSupportCombo is %d", __FUNCTION__, *isSupportCombo);
    return NL_SKIP;
}

static int32_t ParserSupportComboInfo(struct nl_msg *msg, void *arg)
{
    (void)arg;
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *nl_combi, *nl_limit, *nl_mode;
    struct nlattr *tb_comb[NUM_NL80211_IFACE_COMB];
    struct nlattr *tb_limit[NUM_NL80211_IFACE_LIMIT];
    // uint64_t *comboInfo = (uint64_t *)arg;
    int32_t ret, i, j, k, type;
    static struct nla_policy
    iface_combination_policy[NUM_NL80211_IFACE_COMB] = {
        [NL80211_IFACE_COMB_LIMITS] = { .type = NLA_NESTED },
        [NL80211_IFACE_COMB_MAXNUM] = { .type = NLA_U32 },
        [NL80211_IFACE_COMB_NUM_CHANNELS] = { .type = NLA_U32 },
    },
    iface_limit_policy[NUM_NL80211_IFACE_LIMIT] = {
        [NL80211_IFACE_LIMIT_TYPES] = { .type = NLA_NESTED },
        [NL80211_IFACE_LIMIT_MAX] = { .type = NLA_U32 },
    };

    ret = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
        genlmsg_attrlen(gnlh, 0), NULL);
    if (ret != 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nla_parse tb failed", __FUNCTION__);
        return NL_SKIP;
    }

    if (tb[NL80211_ATTR_INTERFACE_COMBINATIONS] != NULL) {
        // get each ieee80211_iface_combination
        nla_for_each_nested(nl_combi, tb[NL80211_ATTR_INTERFACE_COMBINATIONS], i) {
            ret = nla_parse_nested(tb_comb, MAX_NL80211_IFACE_COMB,
                nl_combi, iface_combination_policy);
            if (ret != 0) {
                HILOG_ERROR(LOG_DOMAIN, "%s: nla_parse_nested nl_combi failed", __FUNCTION__);
                return NL_SKIP;
            }
            if (!tb_comb[NL80211_IFACE_COMB_LIMITS] ||
                !tb_comb[NL80211_IFACE_COMB_MAXNUM] ||
                !tb_comb[NL80211_IFACE_COMB_NUM_CHANNELS]) {
                return RET_CODE_NOT_SUPPORT;
            }
            // parse each ieee80211_iface_limit
            nla_for_each_nested(nl_limit, tb_comb[NL80211_IFACE_COMB_LIMITS], j) {
                ret = nla_parse_nested(tb_limit, MAX_NL80211_IFACE_LIMIT,
                    nl_limit, iface_limit_policy);
                if (ret || !tb_limit[NL80211_IFACE_LIMIT_TYPES])
                    return RET_CODE_NOT_SUPPORT; /* broken combination */
                // parse each ieee80211_iface_limit's types
                nla_for_each_nested(nl_mode, tb_limit[NL80211_IFACE_LIMIT_TYPES], k) {
                    type = nla_type(nl_mode);
                    if (type > WIFI_IFTYPE_UNSPECIFIED && type < WIFI_IFTYPE_MAX) {
                        HILOG_INFO(LOG_DOMAIN, "%s: mode: %d", __FUNCTION__, type);
                    }
                }
                HILOG_INFO(LOG_DOMAIN, "%s: has parse a tb_limit", __FUNCTION__);
            }
        }
    }
    return NL_SKIP;
}

struct PrivDevMac {
    uint8_t *mac;
    uint8_t len;
};

static int32_t ParserDevMac(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct PrivDevMac *info = (struct PrivDevMac *)arg;
    uint8_t *getmac = NULL;

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
        genlmsg_attrlen(gnlh, 0), NULL);
    if (tb[NL80211_ATTR_MAC]) {
        getmac = nla_data(tb[NL80211_ATTR_MAC]);
    }
    HILOG_ERROR(LOG_DOMAIN, "%s: has parse a tb_mac[%2x:%2x:%2x:%2x:%2x:%2x]", __FUNCTION__,
        getmac[0], getmac[1], getmac[2], getmac[3], getmac[4], getmac[5]);
    memcpy_s(info->mac, info->len, getmac, info->len);
    return NL_SKIP;
}

static int32_t ParserValidFreq(struct nl_msg *msg, void *arg)
{
    struct FreqInfoResult *result = (struct FreqInfoResult *)arg;
    struct genlmsghdr *gnlh;
    struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
    struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];
    struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];
    struct nlattr *nl_band, *nl_freq;
    int i, j;
    int32_t freq;
    static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
        [NL80211_FREQUENCY_ATTR_FREQ] = { .type = NLA_U32 },
        [NL80211_FREQUENCY_ATTR_MAX_TX_POWER] = { .type = NLA_U32 },
    };

    gnlh = nlmsg_data(nlmsg_hdr(msg));
    nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
        genlmsg_attrlen(gnlh, 0), NULL);
    if (!tb_msg[NL80211_ATTR_WIPHY_BANDS]) {
        HILOG_ERROR(LOG_DOMAIN, "%s: no wiphy bands", __FUNCTION__);
        return NL_SKIP;
    }
    HILOG_INFO(LOG_DOMAIN, "%s: parse freq 000000000000000000", __FUNCTION__);
    // get each ieee80211_supported_band
    nla_for_each_nested(nl_band, tb_msg[NL80211_ATTR_WIPHY_BANDS], i) {
        nla_parse(tb_band, NL80211_BAND_ATTR_MAX, nla_data(nl_band),
            nla_len(nl_band), NULL);
        if (tb_band[NL80211_BAND_ATTR_FREQS] == NULL)
            continue;
        // get each ieee80211_channel
        nla_for_each_nested(nl_freq, tb_band[NL80211_BAND_ATTR_FREQS], j) {
            nla_parse(tb_freq, NL80211_FREQUENCY_ATTR_MAX,
                nla_data(nl_freq), nla_len(nl_freq), freq_policy);
            // get center freq
            if (!tb_freq[NL80211_FREQUENCY_ATTR_FREQ] && tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER])
                continue;
            freq = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_FREQ]);
            switch (result->band) {
                case NL80211_BAND_2GHZ:
                    if (freq > 2400 && freq < 2500) {
                        result->freqs[result->nums] = freq;
                        result->txPower[result->nums] = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER]);
                        result->nums++;
                    }
                    break;
                case NL80211_BAND_5GHZ:
                    if (freq > 5100 && freq < 5900) {
                        result->freqs[result->nums] = freq;
                        result->nums++;
                    }
                    break;
                default:
                    break;
            }
        }
    }
    return NL_SKIP;
}

static int32_t SendCmdSync(struct nl_msg *msg,
    RespHandler handler, void *data)
{
    int32_t rc;
    int32_t error;
    struct nl_cb *cb = NULL;

    if (g_wifiHalInfo.cmdSock == NULL) {
        HILOG_ERROR(LOG_DOMAIN, "%s: command socket not inited", __FUNCTION__);
        return RET_CODE_MISUSE;
    }

    rc = nl_send_auto(g_wifiHalInfo.cmdSock, msg); // seq num auto add
    if (rc < 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nl_send_auto failed", __FUNCTION__);
        return RET_CODE_FAILURE;
    }

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (cb == NULL) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nl_cb_alloc failed", __FUNCTION__);
        return RET_CODE_NOMEM;
    }
    nl_cb_err(cb, NL_CB_CUSTOM, CmdSocketErrorHandler, &error);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, CmdSocketFinishHandler, &error);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, CmdSocketAckHandler, &error);
    if (handler != NULL) {
        nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, handler, data);
    }
    /* wait for reply */
    error = 1;
    while (error > 0) {
        rc = nl_recvmsgs(g_wifiHalInfo.cmdSock, cb);
        if (rc < 0) {
            HILOG_ERROR(LOG_DOMAIN, "%s: nl_recvmsgs failed: %d", __FUNCTION__, rc);
        }
    }
    nl_cb_put(cb);
    return ((rc == 0) ? RET_CODE_SUCCESS : RET_CODE_FAILURE);
}

static bool IsWifiIface(const char *name)
{
    if (strncmp(name, "wlan", 4) != 0 && strncmp(name, "p2p", 3) != 0 &&
        strncmp(name, "nan", 3) != 0) {
        /* not a wifi interface; ignore it */
        return false;
    } else {
        return true;
    }
}

static int32_t GetAllIfaceInfo(struct NetworkInfoResult *infoResult)
{
    struct dirent *de;

    DIR *d = opendir("/sys/class/net");
    if (d == 0) {
        return RET_CODE_FAILURE;
    }
    infoResult->nums = 0;
    while ((de = readdir(d))) {
        if (de->d_name[0] == '.') {
            continue;
        }
        if (IsWifiIface(de->d_name)) {
            strncpy_s(infoResult->infos[infoResult->nums].name, IFNAMSIZ,
                de->d_name, sizeof(de->d_name));
            infoResult->nums++;
        }
    }
    closedir(d);
    if (infoResult->nums == 0)
        return RET_CODE_NOT_AVAILABLE;
    return RET_CODE_SUCCESS;
}

int32_t GetUsableNetworkInfo(struct NetworkInfoResult *result)
{
    int32_t ret;
    uint32_t i;
    uint32_t ifaceId;
    struct nl_msg *msg = NULL;

    ret = GetAllIfaceInfo(result);
    if (ret != RET_CODE_SUCCESS) {
        HILOG_ERROR(LOG_DOMAIN, "%s: GetAllIfaceInfo failed", __FUNCTION__);
        return ret;
    }

    HILOG_INFO(LOG_DOMAIN, "%s: wifi iface num %d", __FUNCTION__, result->nums);
    for (i = 0; i < result->nums; ++i) {
        // NL80211_CMD_GET_WIPHY
        msg = nlmsg_alloc();
        ifaceId = if_nametoindex(result->infos[i].name);
        if (msg == NULL || ifaceId == 0) {
            HILOG_ERROR(LOG_DOMAIN, "%s: nlmsg alloc or get iface id(%d) failed",
                __FUNCTION__, ifaceId);
            return RET_CODE_NOMEM;
        }
        genlmsg_put(msg, 0, 0, g_wifiHalInfo.familyId, 0, NLM_F_DUMP,
            NL80211_CMD_GET_WIPHY, 0);
        nla_put_flag(msg, NL80211_ATTR_SPLIT_WIPHY_DUMP);
        nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifaceId);
        memset_s(result->infos[i].supportMode, sizeof(result->infos[i].supportMode),
            0, sizeof(result->infos[i].supportMode));
        HILOG_INFO(LOG_DOMAIN, "%s: get networinfo of %s, %d", __FUNCTION__, result->infos[i].name, ifaceId);
        ret = SendCmdSync(msg, ParserSupportIfType, &result->infos[i].supportMode);
        if (ret != RET_CODE_SUCCESS) {
            HILOG_ERROR(LOG_DOMAIN, "%s: send cmd failed", __FUNCTION__);
            nlmsg_free(msg);
            return RET_CODE_FAILURE;
        }
        nlmsg_free(msg);
    }
    return RET_CODE_SUCCESS;
}

int32_t IsSupportCombo(uint8_t *isSupportCombo)
{
    uint32_t ifaceId;
    struct nl_msg *msg = NULL;
    struct NetworkInfoResult networkInfo;
    int32_t ret;

    ret = GetUsableNetworkInfo(&networkInfo);
    if (ret != RET_CODE_SUCCESS) {
        HILOG_ERROR(LOG_DOMAIN, "%s: get network info failed", __FUNCTION__);
        return ret;
    }

    msg = nlmsg_alloc();
    ifaceId = if_nametoindex(networkInfo.infos[0].name);
    if (msg == NULL || ifaceId == 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nlmsg alloc or get iface id(%d) failed",
            __FUNCTION__, ifaceId);
        return RET_CODE_NOMEM;
    }
    genlmsg_put(msg, 0, 0, g_wifiHalInfo.familyId, 0, NLM_F_DUMP,
        NL80211_CMD_GET_WIPHY, 0);
    nla_put_flag(msg, NL80211_ATTR_SPLIT_WIPHY_DUMP);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifaceId);
    ret = SendCmdSync(msg, ParserIsSupportCombo, isSupportCombo);
    if (ret != RET_CODE_SUCCESS) {
        HILOG_ERROR(LOG_DOMAIN, "%s: send cmd failed", __FUNCTION__);
        nlmsg_free(msg);
        return RET_CODE_FAILURE;
    }
    nlmsg_free(msg);

    return RET_CODE_SUCCESS;
}

int32_t GetComboInfo(uint64_t *comboInfo, uint32_t size)
{
    (void)size;
    uint32_t ifaceId;
    struct nl_msg *msg = NULL;
    struct NetworkInfoResult networkInfo;
    int32_t ret;

    ret = GetUsableNetworkInfo(&networkInfo);
    if (ret != RET_CODE_SUCCESS) {
        HILOG_ERROR(LOG_DOMAIN, "%s: get network info failed", __FUNCTION__);
        return ret;
    }
    msg = nlmsg_alloc();
    ifaceId = if_nametoindex(networkInfo.infos[0].name);
    if (msg == NULL || ifaceId == 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nlmsg alloc or get iface id(%d) failed",
            __FUNCTION__, ifaceId);
        return RET_CODE_NOMEM;
    }
    genlmsg_put(msg, 0, 0, g_wifiHalInfo.familyId, 0, NLM_F_DUMP,
        NL80211_CMD_GET_WIPHY, 0);
    nla_put_flag(msg, NL80211_ATTR_SPLIT_WIPHY_DUMP);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifaceId);
    ret = SendCmdSync(msg, ParserSupportComboInfo, comboInfo);
    if (ret != RET_CODE_SUCCESS) {
        HILOG_ERROR(LOG_DOMAIN, "%s: send cmd failed", __FUNCTION__);
        nlmsg_free(msg);
        return RET_CODE_FAILURE;
    }
    nlmsg_free(msg);
    return RET_CODE_SUCCESS;
}

int32_t SetMacAddr(const char *ifName, unsigned char *mac, uint8_t len)
{
    int32_t fd;
    int32_t ret;
    struct ifreq req;

    fd = socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: open socket failed", __FUNCTION__);
        return RET_CODE_FAILURE;
    }
    strncpy_s(req.ifr_name, IFNAMSIZ, ifName, sizeof(ifName));
    req.ifr_addr.sa_family = ARPHRD_ETHER;
    memcpy_s(req.ifr_hwaddr.sa_data, len, mac, len);
    ret = ioctl(fd, SIOCSIFHWADDR, &req);
    if (ret != 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: ioctl failed", __FUNCTION__);
        ret = RET_CODE_FAILURE;
    }
    close(fd);
    return ret;
}

struct ethtool_perm_addr {
    uint32_t    cmd; /* ETHTOOL_GPERMADDR */
    uint32_t    size;
    uint8_t     data[0];
};
static int GetFactoryMac()
{
    int fd, ret;
    struct ifreq req;
    struct ethtool_perm_addr *epaddr;

    fd = socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: open socket failed, fd=%d", __FUNCTION__, fd);
        return RET_CODE_FAILURE;
    }
    epaddr = (struct ethtool_perm_addr *) malloc(sizeof(struct ethtool_perm_addr) + 6);
    epaddr->cmd = 0x00000020;   // 0x00000020 ETHTOOL_GPERMADDR
    epaddr->size = ETH_ADDR_LEN;
    req.ifr_data = epaddr;
    req.ifr_addr.sa_family = AF_INET;
    ret = ioctl(fd, SIOCETHTOOL, &req);
    if (ret != 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: ioctl failed, ret=%d", __FUNCTION__, ret);
        return RET_CODE_FAILURE;
    }
    HILOG_INFO(LOG_DOMAIN, "%s: get factory wlan0 mac is %2x:%2x:%2x:%2x:%2x:%2x", __FUNCTION__,
        epaddr->data[0], epaddr->data[1], epaddr->data[2], epaddr->data[3], epaddr->data[4], epaddr->data[5]);
    close(fd);
    if (epaddr != NULL) {
        free(epaddr);
    }
    return ret;
}

int32_t GetDevMacAddr(const char *ifName,
    int32_t type, uint8_t *mac, uint8_t len)
{
    (void)type;
    int fd, ret;
    struct ifreq req;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: open socket failed", __FUNCTION__);
        return RET_CODE_FAILURE;
    }
    req.ifr_addr.sa_family = AF_INET;
    strncpy_s(req.ifr_name, IFNAMSIZ, ifName, sizeof(ifName));
    ret = ioctl(fd, SIOCGIFHWADDR, &req);
    if (ret != 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: ioctl failed", __FUNCTION__);
        return RET_CODE_FAILURE;
    }
    memcpy_s(mac, len, (unsigned char *)req.ifr_hwaddr.sa_data, len);
    close(fd);
    ret = GetFactoryMac();
    return ret;
}

int32_t GetValidFreqByBand(const char *ifName, int32_t band,
    struct FreqInfoResult *result)
{
    uint32_t ifaceId;
    struct nl_msg *msg = NULL;
    int32_t ret;

    msg = nlmsg_alloc();
    ifaceId = if_nametoindex(ifName);
    if (msg == NULL || ifaceId == 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nlmsg alloc or get iface id(%d) failed",
            __FUNCTION__, ifaceId);
        return RET_CODE_NOMEM;
    }
    genlmsg_put(msg, 0, 0, g_wifiHalInfo.familyId, 0, NLM_F_DUMP,
        NL80211_CMD_GET_WIPHY, 0);
    nla_put_flag(msg, NL80211_ATTR_SPLIT_WIPHY_DUMP);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifaceId);
    memset_s(result->freqs, sizeof(result->freqs), 0, sizeof(result->freqs));
    result->nums = 0;
    result->band = band;
    ret = SendCmdSync(msg, ParserValidFreq, result);
    if (ret != RET_CODE_SUCCESS) {
        HILOG_ERROR(LOG_DOMAIN, "%s: send cmd failed", __FUNCTION__);
        nlmsg_free(msg);
        return RET_CODE_FAILURE;
    }
    nlmsg_free(msg);
    return RET_CODE_SUCCESS;
}

struct WifiPrivCmd {
    int32_t verify;
    int32_t cmd;
    int32_t power;
};

int32_t SetTxPower(const char *ifName, int32_t power)
{
    uint32_t ifaceId;
    struct nl_msg *msg = NULL;
    int32_t ret;

    msg = nlmsg_alloc();
    ifaceId = if_nametoindex(ifName);
    if (msg == NULL || ifaceId == 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nlmsg alloc or get iface id(%d) failed",
            __func__, ifaceId);
        return RET_CODE_NOMEM;
    }
    genlmsg_put(msg, 0, 0, g_wifiHalInfo.familyId, 0, 0,
        NL80211_CMD_SET_WIPHY, 0);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifaceId);
    nla_put_u32(msg, NL80211_ATTR_WIPHY_TX_POWER_SETTING, NL80211_TX_POWER_LIMITED);
    nla_put_u32(msg, NL80211_ATTR_WIPHY_TX_POWER_LEVEL, 100 * power);
    ret = SendCmdSync(msg, NULL, NULL);
    if (ret != RET_CODE_SUCCESS) {
        HILOG_ERROR(LOG_DOMAIN, "%s: send cmd failed", __func__);
        nlmsg_free(msg);
        return RET_CODE_FAILURE;
    }
    nlmsg_free(msg);
    HILOG_INFO(LOG_DOMAIN, "%s: send end success", __func__);
    return RET_CODE_SUCCESS;
}

int32_t GetAssociatedStas(const char *ifName,
    struct AssocStaInfoResult *result)
{
    (void)ifName;
    (void)result;
    return RET_CODE_SUCCESS;
}

int32_t WifiSetCountryCode(const char *ifName, const char *code, uint32_t len)
{
    uint32_t ifaceId = if_nametoindex(ifName);
    struct nl_msg *msg = nlmsg_alloc();
    struct nlattr *data = NULL;
    int32_t ret;

    if (ifaceId == 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: if_nametoindex failed", __FUNCTION__);
        return RET_CODE_FAILURE;
    }
    if (msg == NULL) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nlmsg alloc failed", __FUNCTION__);
        return RET_CODE_NOMEM;
    }
    genlmsg_put(msg, 0, 0, g_wifiHalInfo.familyId, 0, 0,
        NL80211_CMD_VENDOR, 0);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifaceId);
    nla_put_u32(msg, NL80211_ATTR_VENDOR_ID, VENDOR_ID);
    nla_put_u32(msg, NL80211_ATTR_VENDOR_SUBCMD, WIFI_SUBCMD_SET_COUNTRY_CODE);
    data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
    if (data == NULL) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nla_nest_start failed", __FUNCTION__);
        nlmsg_free(msg);
        return RET_CODE_FAILURE;
    }
    nla_put(msg, WIFI_ATTRIBUTE_COUNTRY, len, code);
    nla_nest_end(msg, data);

    ret = SendCmdSync(msg, NULL, NULL);
    if (ret != RET_CODE_SUCCESS) {
        HILOG_ERROR(LOG_DOMAIN, "%s: send cmd failed", __FUNCTION__);
    }
    nlmsg_free(msg);
    return ret;
}

int32_t SetScanMacAddr(const char *ifName, uint8_t *scanMac, uint8_t len)
{
    int32_t ret;
    uint32_t ifaceId = if_nametoindex(ifName);
    struct nl_msg *msg = nlmsg_alloc();
    struct nlattr *data = NULL;

    if (ifaceId == 0) {
        HILOG_ERROR(LOG_DOMAIN, "%s: if_nametoindex failed", __func__);
        return RET_CODE_FAILURE;
    }
    if (msg == NULL) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nlmsg alloc failed", __func__);
        return RET_CODE_NOMEM;
    }
    genlmsg_put(msg, 0, 0, g_wifiHalInfo.familyId, 0, 0,
        NL80211_CMD_VENDOR, 0);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifaceId);
    nla_put_u32(msg, NL80211_ATTR_VENDOR_ID, VENDOR_ID);
    nla_put_u32(msg, NL80211_ATTR_VENDOR_SUBCMD, WIFI_SUBCMD_SET_RANDOM_MAC_OUI);
    data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
    if (data == NULL) {
        HILOG_ERROR(LOG_DOMAIN, "%s: nla_nest_start failed", __func__);
        nlmsg_free(msg);
        return RET_CODE_FAILURE;
    }
    nla_put(msg, ANDR_WIFI_ATTRIBUTE_RANDOM_MAC_OUI, len, scanMac);
    nla_nest_end(msg, data);
    ret = SendCmdSync(msg, NULL, NULL);
    if (ret != RET_CODE_SUCCESS) {
        HILOG_ERROR(LOG_DOMAIN, "%s: send cmd failed", __func__);
    }
    nlmsg_free(msg);
    return ret;
}

int32_t AcquireChipId(const char *ifName, uint8_t *chipId)
{
    (void)ifName;
    (void)chipId;
    return RET_CODE_SUCCESS;
}

int32_t GetIfNamesByChipId(const uint8_t chipId, char **ifNames, uint32_t *num)
{
    (void)chipId;
    (void)ifNames;
    (void)num;
    return RET_CODE_SUCCESS;
}

int32_t SetResetDriver(const uint8_t chipId)
{
    (void)chipId;
    return RET_CODE_SUCCESS;
}
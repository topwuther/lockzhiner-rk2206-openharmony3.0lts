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

#ifndef OHOS_HTTPREQ_H
#define OHOS_HTTPREQ_H

#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

constexpr int BUFSIZE = 41000;
constexpr int URLSIZE = 1024;
constexpr int INVALID_SOCKET = -1;
constexpr int HTTP_HEADER_LENGTH = 7;
constexpr int HTTPS_HEADER_LENGTH = 8;
constexpr int DEFAULT_PORT = 80;
constexpr int SEND_HTTP_DELAY_TIME = 6;
constexpr int MAX_IP_STRING_LENGTH = 64;

namespace OHOS {
namespace Wifi {
class HttpRequest {
public:
    /**
     * @Description : Constructs a new Http Request object.
     *
     */
    HttpRequest();

    /**
     * @Description : Destroy the Http Request object.
     *
     */
    ~HttpRequest();

    /**
     * @Description : HttpGet request.
     *
     * @param strUrl - HTTP request URL.[in]
     * @param strResponse - HTTP request response.[in]
     * @return int
     */
    int HttpGet(const std::string &strUrl, std::string &strResponse);

    /**
     * @Description : HttpPost request
     *
     * @param strUrl - HTTP request URL.[in]
     * @param strData - Data sent in a POST request.[in]
     * @param strResponse - HTTP request response.[in]
     * @return int
     */
    int HttpPost(const std::string &strUrl, const std::string &strData, std::string &strResponse);

private:
    /**
     * @Description : Executes HTTP requests, GET or POST.
     *
     * @param strMethod - The Http request type.[in]
     * @param strUrl - HTTP request URL.[in]
     * @param strData - Data sent in a POST request.[in]
     * @param strResponse - HTTP request response.[in]
     * @return int
     */
    int HttpRequestExec(
        const std::string &strMethod, const std::string &strUrl, const std::string &strData, std::string &strResponse);

    /**
     * @Description : Constructing an HTTP Message Header
     *
     * @param strMethod - The Http request type.[in]
     * @param strData - Data sent in a POST request.[in]
     */
    void HttpHeadCreate(const std::string &strMethod, const std::string &strData);

    /**
     * @Description : Send an HTTP request and receive a response.
     *
     * @param iSockFd - a sign.[in]
     * @return int
     */
    int HttpDataTransmit(const int &iSockFd);

    /**
     * @Description : Http connection processing function.
     *
     * @param strResponse - HTTP request response.[in]
     * @return int
     */
    int HttpConnect(std::string &strResponse);

    /**
     * @Description : Obtain the port number from the HTTP request URL object
     *
     * @param strUrl - HTTP request URL.[in]
     * @return none
     */
    void GetPortFromUrl();

    /**
     * @Description : Obtain the IP address from the HTTP request URL.
     *
     * @param strUrl - HTTP request URL.[in]
     * @return int
     */
    int GetIPFromUrl();

    /**
     * @Description : Obtain the host address, website address, or IP address in
     * dotted decimal notation from the HTTP request URL.
     *
     * @param strUrl - HTTP request URL.[in]
     * @return int
     */
    int GetHostAddrFromUrl(const std::string &strUrl);

    /**
     * @Description : Check whether SocketFd is writable and unreadable.
     *
     * @param iSockFd - a sign.[in]
     * @return int
     */
    int SocketFdCheck(const int &iSockFd) const;

private:
    int mISocketFd;
    int iPort;
    std::string strHost;
    std::string strIp;
    std::string strRes;
    std::string strParam;
    std::string httpHead;
};
}  // namespace Wifi
}  // namespace OHOS
#endif
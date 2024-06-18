#include "ntpclient.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ntpclient.h"
#include <stdio.h>

#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/stats.h"
#include "lwip/inet_chksum.h"

#define SERVER_IP "120.25.115.20"
#define NTP_TIMESTAMP_DELTA 2208988800ull
#define close lwip_close

uint32_t GetNTPTime()
{
    int sockfd, n; 
    int portno = 123;
    char *server = SERVER_IP; // NTP server host-name.
    ntp_packet packet = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    packet.li_vn_mode = 0x1b;
    struct sockaddr_in serv_addr; // Server address data structure.
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Create a UDP socket.
    if (sockfd < 0)
        return ERR_OPEN_SOCKET;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(server);
    serv_addr.sin_port = htons(portno);
    while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        close(sockfd);
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    }
    n = send(sockfd, (char *)&packet, sizeof(ntp_packet),0);

    if (n < 0)
        return ERR_WRITE_TO_SOCKET;
    n = recv(sockfd, (char *)&packet, sizeof(ntp_packet),0);
    if (n < 0)
        return ERR_READ_FROM_SOCKET;
    n = close(sockfd);
    if (n < 0)
        return ERR_CLOSE_SOCKET;
    packet.txTm_s = ntohl(packet.txTm_s);
    time_t txTm = (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA);
    return (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA);
}
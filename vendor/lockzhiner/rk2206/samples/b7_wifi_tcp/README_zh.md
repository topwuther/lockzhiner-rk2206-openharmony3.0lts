# 小凌派-RK2206开发板基础外设开发——wifi-tcp通信

本示例将演示如何在小凌派-RK2206开发板上使用wifi进行tcp通信

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## WiFi ssid 和密码设置

修改文件`device/rockchip/rk2206/sdk_liteos/board/src/config_network.c` 中的`TaskConfigWifiModeEntry()`函数修改`set_wifi_config_route_ssid()`和`set_wifi_config_route_passwd()`的参数（即分别为WiFi名称和WiFi密码），让设备连接到与pc同一网络。

```c
static void TaskConfigWifiModeEntry()
{
    // 如果需要修改Wifi的SSID和密码，可以在此启用下述接口
    set_wifi_config_route_ssid(printf,   "凌智电子");     // 路由的WiFi名称
    set_wifi_config_route_passwd(printf, "88888888");    // 路由器WiFi密码
    ......
}
```

确认main文件```device/rockchip/rk2206/sdk_liteos/board/main.c``` wifi功能已开启

```c
ExternalTaskConfigNetwork();
```


### 查看小凌派获取到的IP地址和网关

图如下所示：
![wifi ip地址](/vendor/lockzhiner/rk2206/docs/figures/wifi_tcp/2022-05-09_14-17-53wifi_ip.png)

### 确认pc与小凌派在同一局域网，查看PC的IP地址和网关
在控制台输入 ```ipconfig```
图如下所示：
![pc ip地址](/vendor/lockzhiner/rk2206/docs/figures/wifi_tcp/2022-05-09_14-25-56pc_ip.png)
网关都是```192.168.2.1```表示在同一局域网


修改wifi_tcp例程中 服务地址及端口号后重新编译烧录程序
```c
#define  OC_SERVER_IP   "192.168.2.49"   //pc IP地址 此地址为上一步查询到的地址每个人的地址可能不一样需要根据自己的修改
#define  SERVER_PORT     6666            //端口号
```
pc上打开两个网络调试工具一个客户端一个服务端，并设置IP地址和端口号
![pc 网络调试工具](/vendor/lockzhiner/rk2206/docs/figures/wifi_tcp/2022-05-09_14-41-34网络调试工具.png)

查看log 等待小凌派的tcp客户端和服务端任务启动
![tcp start](/vendor/lockzhiner/rk2206/docs/figures/wifi_tcp/2022-05-09_14-45-59tcp_start.png)


当小凌派tcp客户端和服务端启动后， 再用网络调试助手点击启动客户端和服务端
![网络调试工具 start](/vendor/lockzhiner/rk2206/docs/figures/wifi_tcp/2022-05-09_14-55-05pc_tool_start.png)

然后就可以通过网络工具与小凌派通信了
![网络调试工具 tcp_msg](/vendor/lockzhiner/rk2206/docs/figures/wifi_tcp/2022-05-09_15-00-15tcp_msg.png)


## 程序设计

### API分析

#### 头文件

```c
#include "lz_hardware.h"
#include "config_network.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/stats.h"
#include "lwip/inet_chksum.h"
```

#### socket()

```c
int socket(int domain, int type, int protocol)；
```

**描述：**

创建套接字
**参数：**

| 名字          | 描述           |
| :------------ | :------------- |
| domai         |协议类型，一般为AF_INET      |
| type          | socket类型 |
| protocol      | 用来指定socket所使用的传输协议编号，通常设为0即可 |

**返回值：**

返回大于0为成功，反之为失败


#### bind()

```c
int bind(int sockfd, struct sockaddr *my_addr, int addrlen)；
```

**描述：**

绑定地址端口号

**参数：**

| 名字          | 描述           |
| :------------ | :------------- |
| sockfd        | socket描述符         |
| my_addr       | 是一个指向包含有本机ip地址和端口号等信息的sockaddr类型的指针 |
| addrlen       | 常被设为sizeof(struct sockaddr)  |

**返回值：**

返回大于0为成功，反之为失败


#### connect()

```c
int connect(int sockfd, struct sockaddr *serv_addr, int addrlen)；
```

**描述：**

连接。

**参数：**

| 名字           | 描述                      |
| :------------ | :------------------------ |
| sockfd        | 目的服务器的socket描述符       |
| serv_addr     | 包含目的机器ip地址和端口号的指针 |
| addrlen       | sizeof(struct sockaddr)     |

**返回值：**

返回大于0为成功，反之为失败

#### listen()

```c
int listen(int sockfd, int backlog)；
```

**描述：**

监听

**参数：**

| 名字          | 描述                      |
| :------------ | :------------------------ |
| sockfd        | socket()系统调用返回的socket描述符 |
| backlog       | 指定在请求队列中的最大请求数，进入的连接请求将在队列中等待accept|

**返回值：**

返回等于0为成功，反之为失败


#### accept()

```c
int accept(int sockfd, void *addr, int addrlen);
```

**描述：**

接受的一个连接

**参数：**

| 名字           | 描述                      |
| :------------ | :------------------------ |
| sockfd        | 是被监听的socket描述符 |
| addr          | 通常是一个指向sockaddr_in变量的指针，该变量用来存放提出连接请求服务的主机的信息|
| addrlen       | sizeof(struct sockaddr_in) |

**返回值：**

如果没有错误产生，则accept()返回一个描述所接受包的SOCKET类型的值


#### send()

```c
int send(int sockfd, const void *msg, int len, int flags);
```

**描述：**

发送消息

**参数：**

| 名字          | 描述                      |
| :------------ | :------------------------ |
| sockfd        | 用来传输数据的socket描述符 |
| msg           | 要发送数据的指针  |
| len           | 要发送的数据长度（字节）|
| flags         | 0|

**返回值：**

发送成功返回发送字节数，失败返回值小于0


#### recv()

```c
int recv(int sockfd, void *buf, int len, unsigned int flags)；
```

**描述：**

接收消息

**参数：**

| 名字          | 描述                      |
| :------------ | :------------------------ |
| sockfd        | 接收数据的socket描述符 |
| buf           | 存放数据的缓冲区  |
| len           | 缓冲的长度（字节）|
| flags         | 0|

**返回值：**

接收成功返回大于0，失败返回值小于0


#### lwip_close()

```c
int lwip_close(int sockfd)；
```

**描述：**

关闭套接字

**参数：**

| 名字          | 描述                      |
| :------------ | :------------------------ |
| sockfd        | 要关闭的套接字 |

**返回值：**

发送成功返回发送字节数，失败返回值小于0



### 主要代码分析

创建客户端任务 socket-->connect-->send-->recv-->lwip_close
```c
int wifi_client(void* arg)
{
    int client_fd, ret;
    struct sockaddr_in serv_addr;
    
    while(1)
    {
        client_fd = socket(AF_INET, SOCK_STREAM, 0);//AF_INET:IPV4;SOCK_STREAM:TCP
        if (client_fd < 0)
        {
            printf("create socket fail!\n");
            return -1;
        }

        /*设置调用close(socket)后,仍可继续重用该socket。调用close(socket)一般不会立即关闭socket，而经历TIME_WAIT的过程。*/
        int flag = 1;
        ret = setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
        if (ret != 0) {
            printf("[CommInitTcpServer]setsockopt fail, ret[%d]!\n", ret);
        }
        
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(OC_SERVER_IP);
        serv_addr.sin_port = htons(SERVER_PORT);
        printf("[tcp client] connect:%d<%s:%d>\n",client_fd, inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
        
        
        tcp_client_msg_handle(client_fd, (struct sockaddr*)&serv_addr);
        
        LOS_Msleep(1000);
    }
}
```


连接服务端，并发送消息和接收消息
```c
void tcp_client_msg_handle(int fd, struct sockaddr* dst)
{
    int cnt = 0, count = 0;
    while (connect(fd, dst, len) < 0)
    {
        printf("connect server failed...%d\n", ++count);
        lwip_close(fd);
        LOS_Msleep(5000);
        fd = socket(AF_INET, SOCK_STREAM, 0); //AF_INET:IPV4;SOCK_STREAM:TCP
    }

    while (1)
    {
        char buf[BUFF_LEN];
        sprintf(buf, "TCP TEST cilent send:%d", ++cnt);
        count = send(fd, buf, strlen(buf), 0);                      //发送数据给server
        // count = lwip_write(fd, buf, strlen(buf));                   //发送数据给server
        printf("------------------------------------------------------------\n");
        printf("[tcp client] send:%s\n", buf);
        printf("[tcp client] client sendto msg to server %d,waitting server respond msg!!!\n", count);
        memset(buf, 0, BUFF_LEN);
        count = recv(fd, buf, BUFF_LEN, 0);       //接收来自server的信息
        // count = lwip_read(fd, buf, BUFF_LEN);     //接收来自server的信息
        if(count == -1)
        {
            printf("[tcp client] recieve data fail!\n");
            LOS_Msleep(3000);
            break;
        }
        printf("[tcp client] rev:%s\n", buf);
    }
    lwip_close(fd);
}
```


创建服务端任务 socket-->bind-->listen-->accept-->recv-->send-->close
```c
int wifi_server(void* arg)
{
    int server_fd, ret;

    while(1)
    {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);         //AF_INET:IPV4;SOCK_STREAM:TCP
        // server_fd = lwip_socket(AF_INET, SOCK_STREAM, 0); //AF_INET:IPV4;SOCK_STREAM:TCP
        if (server_fd < 0)
        {
            printf("create socket fail!\n");
            return -1;
        }

        /*设置调用close(socket)后,仍可继续重用该socket。调用close(socket)一般不会立即关闭socket，而经历TIME_WAIT的过程。*/
        int flag = 1;
        ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
        if (ret != 0) {
            printf("[CommInitTcpServer]setsockopt fail, ret[%d]!\n", ret);
        }
        
        struct sockaddr_in serv_addr = {0};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP地址，需要进行网络序转换，INADDR_ANY：本地地址
        // serv_addr.sin_addr.s_addr = inet_addr(OC_SERVER_IP); //IP地址，需要进行网络序转换，INADDR_ANY：本地地址
        serv_addr.sin_port = htons(SERVER_PORT);       //端口号，需要网络序转换
        /* 绑定服务器地址结构 */
        ret = bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        // ret = lwip_bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (ret < 0)
        {
            printf("socket bind fail!\n");
            lwip_close(server_fd);
            return -1;
        }
        /* 监听socket 此处不阻塞 */
        ret = listen(server_fd, 64);
        // ret = lwip_listen(server_fd, 64);
        if(ret != 0)
        {
            printf("socket listen fail!\n");
            lwip_close(server_fd);
            return -1;
        }
        printf("[tcp server] listen:%d<%s:%d>\n",server_fd, inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
        tcp_server_msg_handle(server_fd);   //处理接收到的数据
        LOS_Msleep(1000);
    }
}
```

连接客户端，并发送消息和接收消息
```c
void tcp_server_msg_handle(int fd)
{
    char buf[BUFF_LEN];  //接收缓冲区
    socklen_t client_addr_len;
    int cnt = 0, count;
    int client_fd;
    struct sockaddr_in client_addr = {0};
    
    printf("waitting for client connect...\n");
    /* 监听socket 此处会阻塞 */
    client_fd = accept(fd, (struct sockaddr*)&client_addr, &client_addr_len);
    // client_fd = lwip_accept(fd, (struct sockaddr*)&client_addr, &client_addr_len);
    printf("[tcp server] accept <%s:%d>\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    while (1)
    {
        memset(buf, 0, BUFF_LEN);
        printf("-------------------------------------------------------\n");
        printf("[tcp server] waitting client msg\n");
        count = recv(client_fd, buf, BUFF_LEN, 0);       //read是阻塞函数，没有数据就一直阻塞
        // count = lwip_read(client_fd, buf, BUFF_LEN);  //read是阻塞函数，没有数据就一直阻塞
        if (count == -1)
        {
            printf("[tcp server] recieve data fail!\n");
            LOS_Msleep(3000);
            break;
        }
        printf("[tcp server] rev client msg:%s\n", buf);
        memset(buf, 0, BUFF_LEN);
        sprintf(buf, "I have recieved %d bytes data! recieved cnt:%d", count, ++cnt);
        printf("[tcp server] send msg:%s\n", buf);
        send(client_fd, buf, strlen(buf), 0);        //发送信息给client
        // lwip_write(client_fd, buf, strlen(buf));  //发送信息给client
    }
    lwip_close(client_fd);
    lwip_close(fd);
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor\lockzhiner\rk2206\sample` 路径下 BUILD.gn 文件，指定 `wifi_tcp_example` 参与编译。

```r
"./b7_wifi_tcp:wifi_tcp_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-lwifi_tcp_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -lwifi_tcp_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，串口0显示如下：


```c
[wifi_api:D]netif setup ...
[coonfig_network:D]rknetwork EnableWifi done

[coonfig_network:D]rknetwork SetWifiScan after g_wificonfig.bssid:

[wifi_api_internal:D]Connect to (SSID=凌智电子)
[wifi_api_internal:D]derive psk ...
[wifi_api_internal:D]derive psk done
[wifi_api_internal:D]recovery process ...
[wifi_api_internal:D]AP BSSID (30:5f:77:80:80:b0)
[coonfig_network:I]ConnectTo (凌智电子) done
[coonfig_network:D]rknetwork IP (192.168.2.50)
[coonfig_network:D]network GW (192.168.2.1)
[coonfig_network:D]network NETMASK (255.255.255.0)
[WIFI_DEVICE:E]l o num:0 127.0.0.1
[WIFI_DEVICE:E]w l num:1 192.168.2.50
[coonfig_network:D]set network GW
[coonfig_network:D]network GW (192.168.2.1)
[coonfig_network:D]network NETMASK (255.255.255.0)
[tcp:D]rknetwork IP (192.168.2.50)
[tcp:D]network GW (192.168.2.1)
[tcp:D]network NETMASK (255.255.255.0)
[WIFI_DEVICE:E]l o num:0 127.0.0.1
[WIFI_DEVICE:E]w l num:1 192.168.2.50
[tcp:D]set network GW
[tcp:D]network GW (192.168.2.1)
[tcp:D]network NETMASK (255.255.255.0)
[tcp client] connect:50<192.168.2.49:6666>
[tcp server] listen:51<0.0.0.0:6666>
waitting for client connect...
------------------------------------------------------------
[tcp client] send:TCP TEST cilent send:1
[tcp client] client sendto msg to server 22,waitting server respond msg!!!
[tcp client] rev:
------------------------------------------------------------
[tcp client] send:TCP TEST cilent send:2
[tcp client] client sendto msg to server 22,waitting server respond msg!!!
[tcp client] recieve data fail!
[tcp client] connect:50<192.168.2.49:6666>
connect server failed...1
connect server failed...2
[tcp server] accept <0.0.0.0:0>
-------------------------------------------------------
[tcp server] waitting client msg
------------------------------------------------------------
[tcp client] send:TCP TEST cilent send:1
[tcp client] client sendto msg to server 22,waitting server respond msg!!!
[tcp client] rev:凌智电子服务端
------------------------------------------------------------
[tcp client] send:TCP TEST cilent send:2
[tcp client] client sendto msg to server 22,waitting server respond msg!!!
[tcp server] rev client msg:凌智电子客户端
[tcp server] send msg:I have recieved 14 bytes data! recieved cnt:1
-------------------------------------------------------
[tcp server] waitting client msg
[tcp client] rev:凌智电子服务端
------------------------------------------------------------
[tcp client] send:TCP TEST cilent send:3
[tcp client] client sendto msg to server 22,waitting server respond msg!!!
[tcp server] rev client msg:凌智电子客户端
[tcp server] send msg:I have recieved 14 bytes data! recieved cnt:2
-------------------------------------------------------
[tcp server] waitting client msg
[tcp server] rev client msg:凌智电子客户端
[tcp server] send msg:I have recieved 14 bytes data! recieved cnt:3
-------------------------------------------------------
[tcp server] waitting client msg
```


当log中出现以下信息表示小凌派的tcp客户端和服务端任务已启动
```c
[tcp client] connect:50<192.168.2.49:6666>
[tcp server] listen:51<0.0.0.0:6666>
```
此时用网络调试助手查看并收发消息。
![网络调试工具 tcp_msg](/vendor/lockzhiner/rk2206/docs/figures/wifi_tcp/2022-05-09_15-00-15tcp_msg.png)

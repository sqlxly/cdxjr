/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author      Notes
 *
 */
/*

#include <rtthread.h>
#include <sys/socket.h> /* ??BSD socket,????socket.h??? */
#include <netdb.h>
#include <string.h>
#include <finsh.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>

#define BUFSZ       (1024)
static const char send_data[] = "This is TCP Server from RT-Thread."; /* ??????? */

static void socket1_thread(void *paramete)
{   
	  uint32_t recive_len=0;
		char *recv_data; /* ???????,????????????????? */
    socklen_t sin_size;
    int sock, connected, bytes_received;
    struct sockaddr_in server_addr, client_addr;
    rt_bool_t stop = RT_FALSE; /* ???? */
    int ret;
    recv_data = rt_malloc(BUFSZ + 1); /* ?????????? */
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }
    /* ??socket????,????????,??SOCK_STREAM?TCP?socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* ????????? */
        rt_kprintf("Socket error\n");
        /* ?????????? */
        rt_free(recv_data);
        return;
    }
    /* ???????? */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000); /* ???????? */
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    /* ??socket?????? */
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* ???? */
        rt_kprintf("Unable to bind\n");
        /* ?????????? */
        rt_free(recv_data);
        return;
    }
    /* ?socket????? */
    if (listen(sock, 5) == -1)
    {
        rt_kprintf("Listen error\n");
        /* release recv buffer */
        rt_free(recv_data);
        return;
    }
    rt_kprintf("\nTCPServer Waiting for client on port 5000...\n");
    while (stop != RT_TRUE)
    {
        sin_size = sizeof(struct sockaddr_in);
        /* ?????????socket???,??????????? */
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        /* ?????????socket */
        if (connected < 0)
        {
            rt_kprintf("accept connection failed! errno = %d\n", errno);
            continue;
        }
        /* ?????client_addr??????????? */
        rt_kprintf("I got a connection from (%s , %d)\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        /* ???????? */
        while (1)
        {
            /* ?????connected socket */
            ret = send(connected, send_data, strlen(send_data), 0);
            if (ret < 0)
            {
                /* ????,?????? */
                closesocket(connected);
                rt_kprintf("\nsend error,close the socket.\r\n");
                break;
            }
            else if (ret == 0)
            {
                /* ??send??????0????? */
                rt_kprintf("\n Send warning,send function return 0.\r\n");
            }
            /* ?connected socket?????,??buffer?1024??,?????????1024????? */
            bytes_received = recv(connected, recv_data, BUFSZ, 0);
            if (bytes_received < 0)
            {
                /* ????,????connected socket */
                closesocket(connected);
                break;
            }
            else if (bytes_received == 0)
            {
                /* ??recv??????0????? */
                rt_kprintf("\nReceived warning,recv function return 0.\r\n");
                closesocket(connected);
                break;
            }
            /* ??????,????? */
            recv_data[bytes_received] = '\0';
            if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0)
            {
                /* ???????q?Q,?????? */
                closesocket(connected);
                break;
            }
            else if (strcmp(recv_data, "exit") == 0)
            {
                /* ??????exit,???????? */
                closesocket(connected);
                stop = RT_TRUE;
                break;
            }
            else
            {
							recive_len+= bytes_received;
                /* ???????????? */
              rt_kprintf("re1=%d\n", recive_len);
            }
        }
    }
    /* ???? */
    closesocket(sock);
    /* ?????? */
    rt_free(recv_data);
    return ;

}
static void socket2_thread(void *paramete)
{   
	  uint32_t recive_len=0;
		char *recv_data; /* ???????,????????????????? */
    socklen_t sin_size;
    int sock, connected, bytes_received;
    struct sockaddr_in server_addr, client_addr;
    rt_bool_t stop = RT_FALSE; /* ???? */
    int ret;
    recv_data = rt_malloc(BUFSZ + 1); /* ?????????? */
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }
    /* ??socket????,????????,??SOCK_STREAM?TCP?socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* ????????? */
        rt_kprintf("Socket error\n");
        /* ?????????? */
        rt_free(recv_data);
        return;
    }
    /* ???????? */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5001); /* ???????? */
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    /* ??socket?????? */
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* ???? */
        rt_kprintf("Unable to bind\n");
        /* ?????????? */
        rt_free(recv_data);
        return;
    }
    /* ?socket????? */
    if (listen(sock, 5) == -1)
    {
        rt_kprintf("Listen error\n");
        /* release recv buffer */
        rt_free(recv_data);
        return;
    }
    rt_kprintf("\nTCPServer Waiting for client on port 5001...\n");
    while (stop != RT_TRUE)
    {
        sin_size = sizeof(struct sockaddr_in);
        /* ?????????socket???,??????????? */
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        /* ?????????socket */
        if (connected < 0)
        {
            rt_kprintf("accept connection failed! errno = %d\n", errno);
            continue;
        }
        /* ?????client_addr??????????? */
        rt_kprintf("I got a connection from (%s , %d)\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        /* ???????? */
        while (1)
        {
            /* ?????connected socket */
            ret = send(connected, send_data, strlen(send_data), 0);
            if (ret < 0)
            {
                /* ????,?????? */
                closesocket(connected);
                rt_kprintf("\nsend error,close the socket.\r\n");
                break;
            }
            else if (ret == 0)
            {
                /* ??send??????0????? */
                rt_kprintf("\n Send warning,send function return 0.\r\n");
            }
            /* ?connected socket?????,??buffer?1024??,?????????1024????? */
            bytes_received = recv(connected, recv_data, BUFSZ, 0);
            if (bytes_received < 0)
            {
                /* ????,????connected socket */
                closesocket(connected);
                break;
            }
            else if (bytes_received == 0)
            {
                /* ??recv??????0????? */
                rt_kprintf("\nReceived warning,recv function return 0.\r\n");
                closesocket(connected);
                break;
            }
            /* ??????,????? */
            recv_data[bytes_received] = '\0';
            if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0)
            {
                /* ???????q?Q,?????? */
                closesocket(connected);
                break;
            }
            else if (strcmp(recv_data, "exit") == 0)
            {
                /* ??????exit,???????? */
                closesocket(connected);
                stop = RT_TRUE;
                break;
            }
            else
            {
							recive_len+= bytes_received;
                /* ???????????? */
              rt_kprintf("re2=%d\n", recive_len);
            }
        }
    }
    /* ???? */
    closesocket(sock);
    /* ?????? */
    rt_free(recv_data);
    return ;

}
static void socket3_thread(void *paramete)
{   
	  uint32_t recive_len=0;
		char *recv_data; /* ???????,????????????????? */
    socklen_t sin_size;
    int sock, connected, bytes_received;
    struct sockaddr_in server_addr, client_addr;
    rt_bool_t stop = RT_FALSE; /* ???? */
    int ret;
    recv_data = rt_malloc(BUFSZ + 1); /* ?????????? */
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }
    /* ??socket????,????????,??SOCK_STREAM?TCP?socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* ????????? */
        rt_kprintf("Socket error\n");
        /* ?????????? */
        rt_free(recv_data);
        return;
    }
    /* ???????? */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5002); /* ???????? */
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    /* ??socket?????? */
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* ???? */
        rt_kprintf("Unable to bind\n");
        /* ?????????? */
        rt_free(recv_data);
        return;
    }
    /* ?socket????? */
    if (listen(sock, 5) == -1)
    {
        rt_kprintf("Listen error\n");
        /* release recv buffer */
        rt_free(recv_data);
        return;
    }
    rt_kprintf("\nTCPServer Waiting for client on port 5002...\n");
    while (stop != RT_TRUE)
    {
        sin_size = sizeof(struct sockaddr_in);
        /* ?????????socket???,??????????? */
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        /* ?????????socket */
        if (connected < 0)
        {
            rt_kprintf("accept connection failed! errno = %d\n", errno);
            continue;
        }
        /* ?????client_addr??????????? */
        rt_kprintf("I got a connection from (%s , %d)\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        /* ???????? */
        while (1)
        {
            /* ?????connected socket */
            ret = send(connected, send_data, strlen(send_data), 0);
            if (ret < 0)
            {
                /* ????,?????? */
                closesocket(connected);
                rt_kprintf("\nsend error,close the socket.\r\n");
                break;
            }
            else if (ret == 0)
            {
                /* ??send??????0????? */
                rt_kprintf("\n Send warning,send function return 0.\r\n");
            }
            /* ?connected socket?????,??buffer?1024??,?????????1024????? */
            bytes_received = recv(connected, recv_data, BUFSZ, 0);
            if (bytes_received < 0)
            {
                /* ????,????connected socket */
                closesocket(connected);
                break;
            }
            else if (bytes_received == 0)
            {
                /* ??recv??????0????? */
                rt_kprintf("\nReceived warning,recv function return 0.\r\n");
                closesocket(connected);
                break;
            }
            /* ??????,????? */
            recv_data[bytes_received] = '\0';
            if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0)
            {
                /* ???????q?Q,?????? */
                closesocket(connected);
                break;
            }
            else if (strcmp(recv_data, "exit") == 0)
            {
                /* ??????exit,???????? */
                closesocket(connected);
                stop = RT_TRUE;
                break;
            }
            else
            {
							recive_len+= bytes_received;
                /* ???????????? */
              rt_kprintf("re3=%d\n", recive_len);
            }
        }
    }
    /* ???? */
    closesocket(sock);
    /* ?????? */
    rt_free(recv_data);
    return ;

}
static void socket4_thread(void *paramete)
{   
	  uint32_t recive_len=0;
		char *recv_data; /* ???????,????????????????? */
    socklen_t sin_size;
    int sock, connected, bytes_received;
    struct sockaddr_in server_addr, client_addr;
    rt_bool_t stop = RT_FALSE; /* ???? */
    int ret;
    recv_data = rt_malloc(BUFSZ + 1); /* ?????????? */
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }
    /* ??socket????,????????,??SOCK_STREAM?TCP?socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* ????????? */
        rt_kprintf("Socket error\n");
        /* ?????????? */
        rt_free(recv_data);
        return;
    }
    /* ???????? */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5003); /* ???????? */
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    /* ??socket?????? */
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* ???? */
        rt_kprintf("Unable to bind\n");
        /* ?????????? */
        rt_free(recv_data);
        return;
    }
    /* ?socket????? */
    if (listen(sock, 5) == -1)
    {
        rt_kprintf("Listen error\n");
        /* release recv buffer */
        rt_free(recv_data);
        return;
    }
    rt_kprintf("\nTCPServer Waiting for client on port 5003...\n");
    while (stop != RT_TRUE)
    {
        sin_size = sizeof(struct sockaddr_in);
        /* ?????????socket???,??????????? */
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        /* ?????????socket */
        if (connected < 0)
        {
            rt_kprintf("accept connection failed! errno = %d\n", errno);
            continue;
        }
        /* ?????client_addr??????????? */
        rt_kprintf("I got a connection from (%s , %d)\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        /* ???????? */
        while (1)
        {
            /* ?????connected socket */
            ret = send(connected, send_data, strlen(send_data), 0);
            if (ret < 0)
            {
                /* ????,?????? */
                closesocket(connected);
                rt_kprintf("\nsend error,close the socket.\r\n");
                break;
            }
            else if (ret == 0)
            {
                /* ??send??????0????? */
                rt_kprintf("\n Send warning,send function return 0.\r\n");
            }
            /* ?connected socket?????,??buffer?1024??,?????????1024????? */
            bytes_received = recv(connected, recv_data, BUFSZ, 0);
            if (bytes_received < 0)
            {
                /* ????,????connected socket */
                closesocket(connected);
                break;
            }
            else if (bytes_received == 0)
            {
                /* ??recv??????0????? */
                rt_kprintf("\nReceived warning,recv function return 0.\r\n");
                closesocket(connected);
                break;
            }
            /* ??????,????? */
            recv_data[bytes_received] = '\0';
            if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0)
            {
                /* ???????q?Q,?????? */
                closesocket(connected);
                break;
            }
            else if (strcmp(recv_data, "exit") == 0)
            {
                /* ??????exit,???????? */
                closesocket(connected);
                stop = RT_TRUE;
                break;
            }
            else
            {
							recive_len+= bytes_received;
                /* ???????????? */
              rt_kprintf("re4=%d\n", recive_len);
            }
        }
    }
    /* ???? */
    closesocket(sock);
    /* ?????? */
    rt_free(recv_data);
    return ;

}
static void socket5_thread(void *paramete)
{   
	  uint32_t recive_len=0;
		char *recv_data; /* ???????,????????????????? */
    socklen_t sin_size;
    int sock, connected, bytes_received;
    struct sockaddr_in server_addr, client_addr;
    rt_bool_t stop = RT_FALSE; /* ???? */
    int ret;
    recv_data = rt_malloc(BUFSZ + 1); /* ?????????? */
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }
    /* ??socket????,????????,??SOCK_STREAM?TCP?socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* ????????? */
        rt_kprintf("Socket error\n");
        /* ?????????? */
        rt_free(recv_data);
        return;
    }
    /* ???????? */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5004); /* ???????? */
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    /* ??socket?????? */
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* ???? */
        rt_kprintf("Unable to bind\n");
        /* ?????????? */
        rt_free(recv_data);
        return;
    }
    /* ?socket????? */
    if (listen(sock, 5) == -1)
    {
        rt_kprintf("Listen error\n");
        /* release recv buffer */
        rt_free(recv_data);
        return;
    }
    rt_kprintf("\nTCPServer Waiting for client on port 5004...\n");
    while (stop != RT_TRUE)
    {
        sin_size = sizeof(struct sockaddr_in);
        /* ?????????socket???,??????????? */
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        /* ?????????socket */
        if (connected < 0)
        {
            rt_kprintf("accept connection failed! errno = %d\n", errno);
            continue;
        }
        /* ?????client_addr??????????? */
        rt_kprintf("I got a connection from (%s , %d)\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        /* ???????? */
        while (1)
        {
            /* ?????connected socket */
            ret = send(connected, send_data, strlen(send_data), 0);
            if (ret < 0)
            {
                /* ????,?????? */
                closesocket(connected);
                rt_kprintf("\nsend error,close the socket.\r\n");
                break;
            }
            else if (ret == 0)
            {
                /* ??send??????0????? */
                rt_kprintf("\n Send warning,send function return 0.\r\n");
            }
            /* ?connected socket?????,??buffer?1024??,?????????1024????? */
            bytes_received = recv(connected, recv_data, BUFSZ, 0);
            if (bytes_received < 0)
            {
                /* ????,????connected socket */
                closesocket(connected);
                break;
            }
            else if (bytes_received == 0)
            {
                /* ??recv??????0????? */
                rt_kprintf("\nReceived warning,recv function return 0.\r\n");
                closesocket(connected);
                break;
            }
            /* ??????,????? */
            recv_data[bytes_received] = '\0';
            if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0)
            {
                /* ???????q?Q,?????? */
                closesocket(connected);
                break;
            }
            else if (strcmp(recv_data, "exit") == 0)
            {
                /* ??????exit,???????? */
                closesocket(connected);
                stop = RT_TRUE;
                break;
            }
            else
            {
							recive_len+= bytes_received;
                /* ???????????? */
              rt_kprintf("re5=%d\n", recive_len);
            }
        }
    }
    /* ???? */
    closesocket(sock);
    /* ?????? */
    rt_free(recv_data);
    return ;

}
static void tcpserv1(int argc, char **argv)
{
    rt_thread_t tid1 = RT_NULL;


    tid1 = rt_thread_create("md_s_poll", socket1_thread, argv[1], 4096, 10, 10);
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);
    }
    else
    {
       rt_kprintf("socket error\n");
    }	
}

static void tcpserv2(int argc, char **argv)
{
    rt_thread_t tid1 = RT_NULL;


    tid1 = rt_thread_create("md_s_poll", socket2_thread, argv[1], 4096, 10, 10);
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);
    }
    else
    {
       rt_kprintf("socket error\n");
    }	
}
static void tcpserv3(int argc, char **argv)
{
    rt_thread_t tid1 = RT_NULL;


    tid1 = rt_thread_create("md_s_poll", socket3_thread, argv[1], 4096, 10, 10);
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);
    }
    else
    {
       rt_kprintf("socket error\n");
    }	
}
static void tcpserv4(int argc, char **argv)
{
    rt_thread_t tid1 = RT_NULL;


    tid1 = rt_thread_create("md_s_poll", socket4_thread, argv[1], 4096, 10, 10);
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);
    }
    else
    {
       rt_kprintf("socket error\n");
    }	
}
static void tcpserv5(int argc, char **argv)
{
    rt_thread_t tid1 = RT_NULL;


    tid1 = rt_thread_create("md_s_poll", socket5_thread, argv[1], 4096, 10, 10);
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);
    }
    else
    {
       rt_kprintf("socket error\n");
    }	
}


MSH_CMD_EXPORT(tcpserv1, a tcp server sample);
MSH_CMD_EXPORT(tcpserv2, a tcp server sample);
MSH_CMD_EXPORT(tcpserv3, a tcp server sample);
MSH_CMD_EXPORT(tcpserv4, a tcp server sample);
MSH_CMD_EXPORT(tcpserv5, a tcp server sample);
#ifndef PROJECT_H
#define PROJECT_H 
#include <stdio.h>
#include <linux/input.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/sysmacros.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 

 ssize_t Read(int fd, void *buf, size_t count)
{
    int total = 0;
    while(count > 0)
    {
    	int m = read(fd, buf+total, count);

	if(m == -1)
	{
		perror("read失败");
		exit(0);
	}

	count -= m;
	total += m;
    }

    return total;
}

ssize_t Write(int fd, const void *buf, size_t count)
{
    int total = 0;
    while(count > 0)
    {
    	int m = write(fd, buf+total, count);

	if(m == -1)
	{
		perror("write失败");
		exit(0);
	}

	count -= m;
	total += m;
    }
    return total;
}

int Socket(int domain, int type, int protocol)
{
    int sockfd = socket(domain, type, protocol);
    if(sockfd == -1)
    {
        perror("创建UDP套接字失败");
        exit(0);
    }

    return sockfd;
}


int Bind(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen)
{
    if(bind(sockfd, addr, addrlen) == -1)
    {
        perror("绑定地址失败");
        exit(0);
    }
    return 0;
}


int Connect(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen)
{
    int ret = connect(sockfd, addr, addrlen);
    if(ret != 0)
    {
        perror("连接失败");
        exit(0);
    }
    return ret;
}


int Listen(int sockfd, int backlog)
{
    int ret = listen(sockfd, backlog);
    if(ret != 0)
    {
        perror("设置监听失败");
        exit(0);
    }
    return ret;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int connfd = accept(sockfd, addr, addrlen);
    if(connfd == -1)
    {
        perror("接收连接失败");
        exit(0);
    }
    return connfd;
} 

#endif


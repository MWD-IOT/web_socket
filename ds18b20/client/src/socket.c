#include <time.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "socket.h"
#include "my_sqlite3.h"
#include "logger.h"

/* 初始化socket */
/* ********************************************* 
 * * 函数名  ：socket_init
 * * 函数介绍：初始化网络socket，创建必要的数据结构
 * * 输入参数：
 * * @socket_t *sock：服务器网络socket结构体
 * * @char *hostname： 服务器主机名/域名/ip
 * * @int port： 服务器端口
 * * 输出参数：无
 * * 返回值  ：0：成功，其他：失败
 * * ********************************************/
int socket_init(socket_t *sock, char *hostname, int port)
{
	if ( (NULL == sock) || (NULL == hostname) || (port < 0) )
	{
		return -1;
		LOGGER_ERROR("NULL pointer");
	}
	memset(sock, 0, sizeof(sock));

	strncpy(sock->host, hostname, sizeof(sock->host));/* 服务器主机名/域名/ip */
	sock->port = port;	/* 服务器端口 */
	LOGGER_DEBUG("socket_init successfully");
	return 0;
}

/* 连接socket */
/* ********************************************* 
 * * 函数名  ：socket_connect
 * * 函数介绍：连接服务器socket
 * * 输入参数：
 * * @socket_t *sock：服务器网络socket结构体
 * * 输出参数：无
 * * 返回值  ：0：成功，其他：失败
 * * ********************************************/
int socket_connect(socket_t *sock)
{
	int					conn_fd = -1;	/* 连接标志 */
	int					status;			/* 错误码 */
	struct addrinfo 	hints;			/* socket通用结构体 */
	memset(&hints, 0, sizeof(hints));	/* 将结构体置零 */
	struct addrinfo  	*res;			/* 临时存放解析结果 */
	struct addrinfo 	*ip_buf;		/* 存放解析出来的所有结果 */
	hints.ai_family = AF_UNSPEC;		/* ipv4或ipv6 */
	hints.ai_socktype = SOCK_STREAM;	/* tcp */

	if ( NULL == sock )
	{
		LOGGER_ERROR("NULL pointer");
		goto cleanup;
	}

	if ( (status = getaddrinfo(sock->host, NULL, &hints, &res)) != 0 )
	{	/* 域名解析失败 */
		LOGGER_ERROR("getaddrinfo [%s] failure: %s", sock->host, gai_strerror(status));
		goto cleanup;
	}
	for (ip_buf = res; ip_buf != NULL; ip_buf = ip_buf->ai_next)
	{	/* 域名解析成功，遍历解析后的ip地址并尝试连接，直到成功或全部连接失败 */
		if ( ip_buf->ai_family == AF_INET )
		{	/* 如果解析出来是ipv4地址 */
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)ip_buf->ai_addr;	
			ipv4->sin_port = htons(sock->port);
			//LOGGER_DEBUG("getaddrinfo [%s->%s] successfully\n", sock->host, inet_ntop(ip_buf->ai_family, &(ipv4->sin_addr), sock->host, sizeof(sock->host)));
		}
		else if ( ip_buf->ai_family == AF_INET6 )
		{	/* 如果解析出来是ipv6地址 */
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ip_buf->ai_addr;	
			ipv6->sin6_port = htons(sock->port);
			//LOGGER_DEBUG("getaddrinfo [%s->%s] successfully\n", sock->host, inet_ntop(ip_buf->ai_family, &(ipv6->sin6_addr), sock->host, sizeof(sock->host)));
		}
		if ( (sock->fd = socket(ip_buf->ai_family, SOCK_STREAM, 0)) < 0 )
		{	/* 创建对应ip地址类型的socket */
			LOGGER_ERROR("socket failure: %s", strerror(errno));
			goto cleanup;
		}
		LOGGER_DEBUG("socket [%d] successfully", sock->fd);

		/* 尝试连接socket */
		conn_fd = connect(sock->fd, (struct sockaddr *)ip_buf->ai_addr, sizeof(struct sockaddr));
		if ( 0 == conn_fd )
		{	/* 连接成功 */
			sock->connected = 0;
			LOGGER_DEBUG("connect to [%s:%d] successfully", sock->host, sock->port);
			freeaddrinfo(res);
			return 0;
		}
	}/* for */
	sock->connected = -1;	/* 连接失败 */
	socket_close(sock);		/* 关闭socket */
	LOGGER_ERROR("connect to [%s:%d] failure: %s", sock->host, sock->port, strerror(errno));
cleanup:
	freeaddrinfo(res);
	return -1;
}/* socket_connect */


/* 判断和设置socket连接标志 */
/* ********************************************* 
 * * 函数名  ：socket_diag
 * * 函数介绍：设置服务器socket结构体的成员值
 * * 输入参数：
 * * @socket_t *sock：服务器网络socket结构体
 * * 输出参数：无
 * * 返回值  ：0：成功，其他：失败
 * * ********************************************/
int socket_diag(socket_t *sock)
{
	int 		error;
	socklen_t 	len = sizeof(error);

	if ( NULL == sock )
	{
		return -1;
		LOGGER_ERROR("NULL pointer");
	}

	/* 判断连接情况 */
	getsockopt(sock->fd, SOL_SOCKET, SO_ERROR, &error, &len);
	if ( (0 == sock->connected) && (0 == error) )
	{
		sock->connected = 0;
		LOGGER_DEBUG("diag: connected\n");
		return 0;
	}
	else
	{
		sock->connected = -1;
		LOGGER_DEBUG("diag：connect failure\n");
		return -1;
	}
}

/* 写入（发送）数据 */
/* ********************************************* 
 * * 函数名  ：socket_write
 * * 函数介绍：发送数据给服务器
 * * 输入参数：
 * * @socket_t *sock：服务器网络socket结构体
 * * @char *data：要发送的数据
 * * @int bytes：数据大小
 * * 输出参数：无
 * * 返回值  ：0：成功，其他：失败
 * * ********************************************/
int socket_write(socket_t *sock, char *data, int bytes)
{
	int			wv = 0;

	if ( (NULL == sock) || (NULL == data) )
	{
		return -1;
		LOGGER_ERROR("NULL pointer");
	}

	wv = write(sock->fd, data, bytes);
	if ( wv <= 0 )
	{
		LOGGER_ERROR("write failure: %s", strerror(errno));
		return -1;
	}
	return 0;
}

/* 读取（接收）数据 */
/* ********************************************* 
 * * 函数名  ：socket_read
 * * 函数介绍：读取数据
 * * 输入参数：
 * * @socket_t *sock：服务器网络socket结构体
 * * @char *data：接收到的数据
 * * @int bytes：数据大小
 * * 输出参数：无
 * * 返回值  ：0：成功，其他：失败
 * * ********************************************/
int socket_read(socket_t *sock, char *data, int bytes)
{
	int			rv = 0;

	if ( (NULL == sock) || (NULL == data) )
	{
		return -1;
		LOGGER_ERROR("NULL pointer");
	}

	rv = read(sock->fd, data, bytes);
	if ( rv <= 0 )
	{
		LOGGER_ERROR("read failure: %s", strerror(errno));
		return -1;
	}
	return 0;
}

/* 关闭socket */
/* ********************************************* 
 * * 函数名  ：socket_close
 * * 函数介绍：关闭socket
 * * 输入参数：
 * * @socket_t *sock：服务器网络socket结构体
 * * 输出参数：无
 * * 返回值  ：0：成功，其他：失败
 * * ********************************************/
int socket_close(socket_t *sock)
{
	if ( NULL == sock )
	{
		return -1;
		LOGGER_ERROR("NULL pointer");
	}
	close(sock->fd);
	sock->fd = -1;
	return 0;
}
















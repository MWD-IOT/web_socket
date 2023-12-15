#ifndef SOCKET_H
#define SOCKET_H


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

#include "get_temp.h"
#include "get_time.h"
#include "socket.h"
#include "my_sqlite3.h"
#include "logger.h"
#include "pack_data.h"

/*   socket 结构体 */
typedef struct socket_s
{
	int          fd;         /*   socket文件描述符 */
	char         host[128];  /*   服务器主机名/域名/ip */
	int          port;       /*   服务器端口号 */
	int          connected;  /*   socket连接标志 */
} socket_t;


/* 函数声明 */
/*Function declaration*/
int domain_to_ip(char *domain, char **ip);					/* 域名解析 */	
int socket_init(socket_t *sock, char *hostname, int port);	/* 初始化socket */
int socket_connect(socket_t *sock);							/* 连接socket */
int socket_diag(socket_t *sock);							/* 判断和设置socket连接状态 */
int socket_write(socket_t *sock, char *data, int bytes);	/* 写入（发送）数据 */
int socket_read(socket_t *sock, char *data, int bytes);		/* 读取（接收）数据 */
int socket_close(socket_t *sock);							/* 关闭socket */





#endif

































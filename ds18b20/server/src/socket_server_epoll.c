/*********************************************************************************
 *      Copyright:  (C) 2023 MWD
 *                  All rights reserved.
 *
 *       Filename:  socket_server_epoll.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2023年04月02日)
 *         Author:  MoWeida <2594041017@qq.com>
 *      ChangeLog:  1, Release initial version on "2023年04月02日 12时30分38秒"
 *                 
 ********************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/resource.h>

#include "my_sqlite3.h"
#include "logger.h"
#include "pack_data.h"

#define LOG_LEVEL 		LOG_ERROR	/* 日志级别 */
#define MAX_EVENTS 		512			/* 最大监听事件数 */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

int print_usage(char *progname);
int set_socket_rlimit(void); 
int socket_server_init(char *listen_ip, int listen_port);

int main(int argc, char **argv)
{
	int 				 listen_fd = -1;								/* socket描述符 */
	int					 conn_fd = -1;									/* accept函数返回值 */
	int 				 serv_port = 0;									/* 监听端口号 */
	int 				 daemon_run = 0;								/* 后台运行标志 */
	char 				*progname = NULL;								/* 文件名 */
	char 				 opt = 0;										/* 参数解析函数getopt_long返回值 */
	int 				 rv = -1;										/* 函数返回值 */
	char 				 buf[128];										/* 接收数据缓冲区 */
	int 				 epoll_rv = -1;									/* epoll函数返回值 */
	struct epoll_event 	 event;											/* epoll事件 */
	struct epoll_event 	 event_array[MAX_EVENTS];						/* epoll事件数组 */
	int 				 events;										/* epoll事件数 */
	packet_t			*pack = (packet_t *)malloc(sizeof(packet_t));	/* 打包函数结构体 */
	char  				*log_filename = "server.log";					/* 日志文件名 */
	struct option 		 long_options[] =								/* 解析的参数结构体 */
	{
		{"daemon", no_argument, NULL, 'b'},
		{"port", required_argument, NULL, 'p'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	progname = basename(argv[0]);	/* 文件名 */

	/*  Parser the command line parameters */
	while ( (opt = getopt_long(argc, argv, "bp:h", long_options, NULL)) != -1 )
	{
		switch (opt)
		{
			case 'b':
				daemon_run=1;					/* 是否后台运行 */
				break;
			case 'p':
				serv_port = atoi(optarg);		/* 监听的端口号 */
				break;
			case 'h': 
				print_usage(progname);			/* 说明信息 */
				return EXIT_SUCCESS;
			default:
				break;
		}
	}

	if( !serv_port )
	{	/* 判断输入的端口号参数是否为空 */
		print_usage(progname);
		return -1;
	}

	logger_init(log_filename, LOG_LEVEL);	/* 初始化日志：设置日志文件名和记录日志级别 */

	set_socket_rlimit(); /*  设置最大连接数 */

	if( (listen_fd = socket_server_init(NULL, serv_port)) < 0 )
	{	/* 初始化socketi，监听指定的端口号 */
		printf("[%s: ERROR: %s server listen on port %d failure]\n",   __FILE__, argv[0], serv_port);
		return -2;
	}
	printf("[%s: %s server start to listen on port %d]\n",   __FILE__, argv[0], serv_port);

	if( daemon_run )
	{	/* 判断是否后台运行 */
		daemon(0, 0);
	}

	if( (epoll_rv = epoll_create(MAX_EVENTS)) < 0 )
	{	/* 创建epoll监听队列 */
		printf("[%s: epoll_create() failure: %s]\n",   __FILE__, strerror(errno));
		return -3;
	}
	event.events = EPOLLIN|EPOLLET;	/*  */
	event.events = EPOLLIN;			/*  */
	event.data.fd = listen_fd;		/*  */

	if( epoll_ctl(epoll_rv, EPOLL_CTL_ADD, listen_fd, &event) < 0)
	{	/*  */
		printf("[%s: epoll add listen socket failure: %s]\n",   __FILE__, strerror(errno));
		return -4;
	}

	for ( ; ; )
	{	/*  */
		events = epoll_wait(epoll_rv, event_array, MAX_EVENTS, -1);
		if(events < 0)
		{
			printf("[%s: epoll failure: %s]\n",   __FILE__, strerror(errno));
			break;
		}
		else if(events == 0)
		{
			printf("[%s: epoll get timeout]\n",   __FILE__);
			continue;
		}

		/* rv>0 is the active events count */
		int i = 0;
		for(i=0; i<events; i++)
		{
			if ( (event_array[i].events&EPOLLERR) || (event_array[i].events&EPOLLHUP) )
			{	
				printf("[%s: epoll_wait get error on fd[%d]: %s]\n",   __FILE__, event_array[i].data.fd, strerror(errno));
				epoll_ctl(epoll_rv, EPOLL_CTL_DEL, event_array[i].data.fd, NULL);
				close(event_array[i].data.fd);
			}
			if( event_array[i].data.fd == listen_fd )
			{	/* 有事件就绪 */
				if( (conn_fd = accept(listen_fd, (struct sockaddr *)NULL, NULL)) < 0)
				{	/* 接受连接失败 */
					printf("[%s: accept new client failure: %s]\n",   __FILE__, strerror(errno));
					continue;
				}
				event.data.fd = conn_fd;
				//event.events = EPOLLIN|EPOLLET;
				event.events = EPOLLIN;
				if( epoll_ctl(epoll_rv, EPOLL_CTL_ADD, conn_fd, &event) < 0 )
				{
					printf("[%s: epoll add client socket failure: %s]\n",   __FILE__, strerror(errno));
					close(event_array[i].data.fd);
					continue;
				}	
				printf("[%s: epoll add new client socket[%d] ok.]\n",   __FILE__, conn_fd);
			}
			else /*  already connected client socket get data incoming */
			{	/* 已连接的socket，读数据 */
				if( (rv = read(event_array[i].data.fd, buf, sizeof(buf))) <= 0)
				{	/* 读失败 */
					printf("[%s: socket[%d] read failure or get disconncet]\n",   __FILE__, event_array[i].data.fd);
					epoll_ctl(epoll_rv, EPOLL_CTL_DEL, event_array[i].data.fd, NULL);
					close(event_array[i].data.fd);
					continue;
				}
				else
				{
					printf("%s\n", buf);
					unpack_data(buf, pack);	/* 将收到的数据解包 */
					db_insert(pack);		/* 将解包的数据插入数据库 */
				}

			}	

		} /*  for(i=0; i<rv; i++) */

	} /*  while(1) */

CleanUp:
	close(listen_fd);
	logger_destroy();
	free(pack);
	pack = NULL;
	return 0;
}

int print_usage(char *progname)
{
	printf("Usage: %s [OPTION]...\n", progname);
	printf(" %s is a socket server program, which used to verify client and echo back string from it\n",progname);
	printf("\nMandatory arguments to long options are mandatory for short options too:\n");
	printf(" -b[daemon ] set program running on background\n");
	printf(" -p[port ] Socket server port address\n");
	printf(" -h[help ] Display this help information\n");
	printf("\nExample: %s -b -p 8900\n", progname);
	return 0;

}

int socket_server_init(char *listen_ip, int listen_port)
{
	struct sockaddr_in servaddr;
	int rv = 0;
	int on = 1;
	int listen_fd;
	if ( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Use socket() to create a TCP socket failure: %s\n", strerror(errno));
		return -1;
	}
	/*  Set socket port reuseable, fix 'Address already in use' bug when socket server restart */
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(listen_port);
	if( !listen_ip ) /*  Listen all the local IP address */
	{
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else /*  listen the specified IP address */
	{
		if  (inet_pton(AF_INET, listen_ip, &servaddr.sin_addr) <= 0 )
		{
			printf("inet_pton() set listen IP address failure.\n");
			rv = -2;
			goto CleanUp;
		}
	}
	if( bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 )
	{
		printf("Use bind() to bind the TCP socket failure: %s\n", strerror(errno));
		rv = -3;
		goto CleanUp;
	}
	if( listen(listen_fd, 64) < 0)
	{
		printf("Use bind() to bind the TCP socket failure: %s\n", strerror(errno));
		rv = -4;
		goto CleanUp;
	}
CleanUp:
	if( rv<0 )
		close(listen_fd);
	else
		rv = listen_fd;
	return rv;
}

/*  Set open file description count to max */
int set_socket_rlimit(void)
{
	struct rlimit limit = {0};
	getrlimit(RLIMIT_NOFILE, &limit );
	limit.rlim_cur = limit.rlim_max;
	setrlimit(RLIMIT_NOFILE, &limit );
	printf("set socket open fd max count to %ld\n", limit.rlim_max);
	return 0;
}




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

#define LOG_LEVEL LOG_ERROR

int print_usage(char *progname);

/* 主函数 */
/*main function*/
/************************************************ 
 * 函数名  ： main
 * 函数介绍：采集温度并上报服务器，断线插入数据库
 * 输入参数：
 * @argv[1]：-i [服务器IP地址]或-d [服务器域名]
 * @argv[2]：-p [服务器端口]
 * @argv[3]：-s [产品序列号]
 * @argv[4]：-t [采集时间间隔]
 * @srgv[5]：-h [打印使用说明信息]
 * 输出参数：无
 * 返回值  ：0：成功，其他：失败
 * **********************************************/
int main(int argc, char **argv)
{
	char				*hostname = NULL;							//服务器ip/域名/主机名
	int			 		 port = 0;									//服务器端口号
	char				*device_id = "001";							//设备id：默认001
	int			 	 	 interval_sec = 1800;						//采集时间间隔：秒，默认1800秒，0.5小时
	char				*db_name = "ds18b20.db";					//数据库名字：默认ds18b20.db
	char				*log_filename = "client.log";				//日志文件名：默认client.log
	int					 daemon_flag = 0;							//后台运行标志：默认0：非后台运行
	int					 sample_flag = 0;							//采集标志
	time_t				 pretime = 0;								//启动时间
	time_t				 nowtime = 0;								//当前时间
	char			 	 data[128] = {0};							//发送给服务器的最终数据包
	char			 	 query_data[128] = {0};						//数据缓冲区
	socket_t			 sock;										//socket结构体
	packet_t			 pack;										//数据打包结构体
	int			 		 ch = -1;									//命令行选项/参数解析函数返回值，用于switch()
	struct option		 opts[] =							 
	{
		{"ipaddr", required_argument, NULL, 'i'},
		{"port", required_argument, NULL, 'p'},
		{"serial", required_argument, NULL, 's'},
		{"interval", required_argument, NULL, 't'},
		{"dbname", required_argument, NULL, 'D'},
		{"logname", required_argument, NULL, 'L'},
		{"daemon", no_argument, NULL, 'd'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	/*命令行参数解析 */
	while( (ch = getopt_long(argc, argv, "i:p:s:t:D:L:dh", opts, NULL)) != -1 )
	{
		switch( ch )
		{
			case 'i':
				hostname = optarg; 			/* 服务器ip/主机名/域名 */
				break;
			case 'p':
				port = atoi(optarg); 		/* 服务器端口号 */
				break;
			case 's':
				device_id = optarg; 		/* 设备id */
				break;
			case 't':
				interval_sec = atoi(optarg);/* 采样时间间隔 */
				break;
			case 'D':
				db_name = optarg;			/* 数据库名字 */
				break;
			case 'L':
				log_filename = optarg;		/* 日志文件名 */
				break;
			case 'd':
				daemon_flag = 1; 			/* 后台运行标志 */
				break;
			case 'h': 
				print_usage(argv[0]); 		/* 帮助信息 */
				return 0;
			default:
				print_usage(argv[0]); 		/* 帮助信息 */
				return -1;
		}
	}

	if( (!hostname) || (port < 0) )				/* 判断输入参数是否合法 */
	{											
		print_usage(argv[0]);					/* 打印帮助信息 */
		return 0;
	}
	printf("socket[%s:%d]\n", hostname, port);

	if ( 1 == daemon_flag )						/* 判断是否要后台运行 */
	{											
		daemon(1, 0);							/* 后台运行 */
	}
	
	logger_init(log_filename, LOG_LEVEL);		/* 设置日志文件名和日志记录级别 */
	
	socket_init(&sock, hostname, port);			/* 初始化socket */

	if ( 0 < db_init(db_name) )
	{											/* 初始化数据库 */
		return -1;
	}

	printf("running...\n");
	pretime = 0;
	while( 1 )
	{
		sample_flag = 0;												/* 未采集 */
		time(&nowtime);													/* 获取当前时间戳 */
		if ( (nowtime - pretime) >= interval_sec )
		{																/* 判断是否到采集时间 */
			snprintf(pack.dev_id, sizeof(pack.dev_id), device_id);	/* 设备id */
			get_time(pack.dev_time);									/* 获取当前日期时间 */
			get_temp(&pack.dev_temp);								/* 采集温度 */

			sample_flag = 1;											/* 已采集标志 */
			pretime = nowtime;											/* 重新计时 */
		}

		socket_diag(&sock);				/* 判断连接状态 */

		/* 1.连接失败的情况 */
		if ( sock.connected < 0 )		/* 连接失败 */
		{
			socket_connect(&sock);		/* 尝试重连 */
		}
		if ( sock.connected < 0 )		/* 重连失败 */
		{
			if ( 1 == sample_flag )		/* 如果已采集数据 */
			{							
				db_insert(&pack);		/*  将采集数据data插入数据库中 */ 
				memset(data, 0, sizeof(data));
				pack_data(&pack, data, sizeof(data)); 				/* 将采集的数据从pack打包到data */
				LOGGER_DEBUG("%s\n", data);
				printf("%s\n", data);
			}
			continue;
		}

		/* 2.连接成功的情况 */
		if ( 1 == sample_flag )	
		{														/* 如果已采集数据 */
			memset(data, 0, sizeof(data));
			pack_data(&pack, data, sizeof(data)); 				/* 将采集的数据从pack打包到data */
			LOGGER_DEBUG("%s\n", data);
			printf("%s\n", data);
			if ( socket_write(&sock, data, sizeof(data)) < 0 )	/* 发送数据 */
			{													
				db_insert(&pack);								/* 发送失败，将数据插入数据库 */
				socket_close(&sock);							/* 关闭socket */
				continue;
			}
		}

		if ( 0 == db_query(&pack) )								/* 查询数据库是否有数据 */ 
		{														
			memset(data, 0, sizeof(data));
			pack_data(&pack, data, sizeof(data));				/* 将查询到的数据pack打包到data中 */
			if ( socket_write(&sock, data, sizeof(data)) < 0 )	/* 发送数据data */
			{													
				socket_close(&sock);							/* 关闭socket */
				continue;
			}
			db_delete();										/* 删除数据库中已发送的数据 */
		}

	}/* while(1) */


	socket_close(&sock);										/* 关闭socket */

	db_close();													/* 关闭数据库 */

	/* 关闭日志 */
	logger_destroy();

	return 0;

}/* main */



/* 使用说明 */
/* ********************************************* 
 * * * 函数名  ：print_usage
 * * * 函数介绍：打印执行该程序时需要输入的参数
 * * * 输入参数：
 * * * @argv[0]：文件名
 * * * 输出参数：无
 * * * 返回值  ：0：成功，其他：失败
 * * * ********************************************/
int print_usage(char *progname)
{
	if ( NULL == progname )
	{
		return -1;
		LOGGER_ERROR("NULL pointer");
	}
	printf("%s usage: \n", progname);
	printf("-i(--ipaddr): sepcify server IP address.\n");
	printf("-p(--port): sepcify server port.\n");
	printf("-s(--serial): sepcify device serial number.\n");
	printf("-t(--time):sepcify sampling interval.\n");
	printf("-h(--help): print help informatiom.\n");
	printf("\n");
	return 0;
}



























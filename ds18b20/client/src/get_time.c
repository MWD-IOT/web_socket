#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "logger.h"

/* **********************************************
 * 函数名 ：		get_time
 * 函数功能 ：		获取系统当前日期、时间
 * 输入参数 ：		无
 * 输出参数 ：
 * @char *getttime：	返回采集获取到的日期时间数据
 * 返回值 ：		0：成功，其他：失败
 * **********************************************/
int get_time(char *getttime)
{
	if ( NULL == getttime )
	{
		LOGGER_ERROR("Invalid input argument");
		return -1;
	}

	time_t			 sec = 0;
	struct tm		 ptm;	
	struct tm		*rv = NULL;	
	
	time(&sec);				

	rv = localtime_r(&sec, &ptm);

	if ( NULL == &ptm )
	{
		LOGGER_ERROR("NULL pointer: &resulte");
		return -1;
	}

	strftime(getttime, 32, "%Y-%m-%d %H:%M:%S", &ptm);
	LOGGER_DEBUG("get_time successfully: %s", getttime);
	
	return 0;
}





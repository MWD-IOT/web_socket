/*********************************************************************************
 *      Copyright:  (C) 2023 LXC
 *                  All rights reserved.
 *
 *       Filename: logger.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(08/05/23)
 *         Author:  Lin XinCheng <1481155734@qq.com>
 *      ChangeLog:  1, Release initial version on "08/05/23 16:48:22"
 *                 
 ********************************************************************************/



#include "logger.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

static logger_t *logger;

/* 初始化日志 */
int logger_init(const char *filename, log_level_t level) 
{
	if ( NULL == filename ) 
	{
		LOGGER_ERROR("NULL pointer");
		return -1;
	}
	logger = (logger_t *)malloc(sizeof(logger_t));
	// 打开日志文件
	logger->fp = fopen(filename, "a");
	// 判断打开日志文件是否成功
	if ( NULL == logger->fp )
	{
		LOGGER_ERROR("open/create log file [%s] failure: %s", filename, strerror(errno));
		return -2;
	}

	// 设置日志级别
	logger->level = level;

	return 0;
}

/* 销毁日志对象 */
int logger_destroy(void) 
{
	if (logger != NULL) 
	{
		fclose(logger->fp);
		free(logger);
		logger = NULL;
	}
	return 0;
}

/* 写入日志 */
int logger_write(log_level_t level, const char *code_name, const char *func_name, int code_line, const char *format, ...) 
{
	if ( (NULL == code_name) || (NULL == func_name) ) 
	{
		LOGGER_ERROR("NULL pointer");
		return -1;
	}
	// 获取当前时间
	time_t now = time(NULL);
	struct tm *tm = localtime(&now);
	char time_str[20];
	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm);
	// 判断日志级别是否满足输出条件
	if (level < logger->level) 
	{
		//LOGGER_ERROR("log level error");
		return -1;
	}

	// 输入日志信息
	fprintf(logger->fp, "[%s]  [%s:%s:%d]: ", time_str, code_name, func_name, code_line);
	   
	switch (level) 
	{
		case LOG_DEBUG:
			fprintf(logger->fp, "[DEBUG]: ");
			break;
		case LOG_INFO:
			fprintf(logger->fp, "[INFO]: ");
			break;
		case LOG_WARN:
			fprintf(logger->fp, "[WARN]: ");
			break;
		case LOG_ERROR:
			fprintf(logger->fp, "[ERROR]: ");
			break;
	}
	
	va_list args;
	char	buf[1024] = {0}; /* 每条日志的最大字节数 */
	va_start(args, format);
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
	fprintf(logger->fp, "%s\n", buf);

	fflush(logger->fp);

	return 0;
}
































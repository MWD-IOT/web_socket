/********************************************************************************
 *      Copyright:  (C) 2023 MWD
 *                  All rights reserved.
 *
 *       Filename:  logger.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(08/05/23)
 *         Author:  Mo weida <2594041017@qq.com>
 *      ChangeLog:  1, Release initial version on "08/05/23 16:49:13"
 *                 
 ********************************************************************************/



#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>

//日志级别
typedef enum 
{
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR
} log_level_t;

// 日志结构�?
typedef struct 
{
	FILE *fp; // 日志文件指针
	log_level_t level; // 日志级别
} logger_t;

/* 初�?�化日志 */
int logger_init(const char *filename, log_level_t level);

/* 关闭日志 */
int logger_destroy(void);

#if 1
/* 输入日志信息 */
int logger_write(log_level_t level, const char *code_name, const char *func_name, int code_line, const char *format, ...);
#define LOGGER_DEBUG(format, args...) logger_write(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, format, ##args);
#define LOGGER_INFO(format, args...) logger_write(LOG_INFO, __FILE__, __FUNCTION__, __LINE__, format, ##args);
#define LOGGER_WARN(format, args...) logger_write(LOG_WARN, __FILE__, __FUNCTION__, __LINE__, format, ##args);
#define LOGGER_ERROR(format, args...) logger_write(LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, format, ##args);
#endif










































/* 创建日志对象 */
//int logger_create(logger_t *logger, const char *filename, log_level_t level);

/* 销毁日志�?�象 */
//int logger_destroy(logger_t *logger);

#if 0
/* 输出日志信息 */
int logger_write(log_level_t level, const char *code_name, const char *func_name, int code_line, const char *format, va_list args);
int logger_log(log_level_t level, const char *code_name, const char *func_name, int code_line, const char *format, ...);
#define LOGGER_DEBUG(format, args...) logger_log(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, format, ##args);
#define LOGGER_INFO(format, args...) logger_log(LOG_INFO, __FILE__, __FUNCTION__, __LINE__, format, ##args);
#define LOGGER_WARN(format, args...) logger_log(LOG_WARN, __FILE__, __FUNCTION__, __LINE__, format, ##args);
#define LOGGER_ERROR(format, args...) logger_log(LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, format, ##args);
#endif

#if 0
// 输出日志信息
int logger_log(logger_t *logger, log_level_t level, const char *code_name, const char *func_name, int code_line, const char *format, ...);
#define LOGGER_DEBUG(logger, format, args...) logger_log(logger, LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, format, ##args);
#define LOGGER_INFO(logger, format, args...) logger_log(logger, LOG_INFO, __FILE__, __FUNCTION__, __LINE__, format, ##args);
#define LOGGER_WARN(logger, format, args...) logger_log(logger, LOG_WARN, __FILE__, __FUNCTION__, __LINE__, format, ##args);
#define LOGGER_ERROR(logger, format, args...) logger_log(logger, LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, format, ##args);
#endif




#endif /* #ifndef */





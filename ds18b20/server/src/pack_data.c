/*********************************************************************************
 *      Copyright:  (C) 2023 LXC
 *                  All rights reserved.
 *
 *       Filename:  packet.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(10/05/23)
 *         Author:  Mo weida <2594041017@qq.com>
 *      ChangeLog:  1, Release initial version on "10/05/23 08:11:39"
 *                 
 ********************************************************************************/



#include <stdio.h>
#include <string.h>

#include "pack_data.h"
#include "logger.h"

int pack_data(packet_t *pack, char *data, int bytes)
{
	if ( NULL == pack )
	{
		LOGGER_ERROR("pointer [pack] is NULL");
		return -1;
	}
	memset(data, 0, bytes);
	snprintf(data, bytes, "id:%s|time:%s|temp:%f", pack->dev_id, pack->dev_time, pack->dev_temp);
	LOGGER_DEBUG("pack_data successfully");
	return 0;
}

int unpack_data(char *data, packet_t *pack)
{
	if ( NULL == data )
	{
		LOGGER_ERROR("pointer [data] if NULL");
		return -1;
	}
	sscanf(data, "id:%[^|]time:%[^|]temp:%f", pack->dev_id, pack->dev_time, &pack->dev_temp);
	//printf("id:%s\n", pack->dev_id);
	//printf("time:%s\n", pack->dev_time);
	//printf("temp:%f\n", pack->dev_temp);
	LOGGER_DEBUG("unpack_data successfully");
	return 0;
}














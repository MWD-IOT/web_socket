/********************************************************************************
 *      Copyright:  (C) 2023 LXC
 *                  All rights reserved.
 *
 *       Filename:  packet.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(07/05/23)
 *         Author:  Lin XinCheng <1481155734@qq.com>
 *      ChangeLog:  1, Release initial version on "07/05/23 19:14:01"
 *                 
 ********************************************************************************/



#ifndef _PACK_DATA__H_
#define _PACK_DATA__H_

typedef struct pack
{
	char 	dev_id[32];
	char 	dev_time[32];
	float	dev_temp;
} packet_t;

int pack_data(packet_t *pack, char *data, int bytes);
int unpack_data(char *data, packet_t *pack);

#endif




/********************************************************************************
 *      Copyright:  (C) 2023 LXC
 *                  All rights reserved.
 *
 *       Filename:  my_sqlite3.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(07/05/23)
 *         Author:  Lin XinCheng <2594041017@qq.com>
 *      ChangeLog:  1, Release initial version on "07/05/23 19:19:15"
 *                 
 ********************************************************************************/



#ifndef _MY_SQLITE__H_
#define _MY_SQLITE__H_

#include "pack_data.h"

int db_init(void);
int db_insert(packet_t *pack);
int db_query(packet_t *pack);
int db_delete(void);
int	db_close();

#endif




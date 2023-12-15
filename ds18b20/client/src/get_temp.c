#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "logger.h"

/*get temperature function*/
/* *************************************
 * 函数名 ：	get_temp
 * 函数介绍 ：	从ds18b20文件获取温度
 * 输入参数 ：	无
 * 输出参数 ：
 * @char temp：返回获取到的温度
 * *************************************/
int get_temp(float *temp)
{
	if( NULL == temp )
	{
		LOGGER_ERROR("Invalid input argument");
		return -1;
	}
	char		 	 dev_path[128] = "/sys/bus/w1/devices/";	//The path of the devices
	DIR				*dev_pathfd = NULL;							//The pathfd of the devices
	struct dirent	*dev_dir = NULL;							//The directory of the devices
	char		 	 chip_name[128] = {0};						//The chip name of the devices
	char		 	 dev_name[128] = "w1_slave";				//The filename of the devices
	int		 		 fd = -1;									//The open file desciptor
	char		 	 buf[128] = {0};							//The temperature buffer
	char			*data = NULL;								//The temperature output parameter to main()
	int 		 	 found_flag = 0;							//The falg of whether found the chip or not
	int		 		 n = 0;										//The count of date that read from the file

	/*Open the path of the devices*/
	if( NULL == (dev_pathfd = opendir(dev_path)) )
	{
		LOGGER_ERROR("open dev_path failure: %s", strerror(errno));
		return -1;
	}
	LOGGER_DEBUG("Open dev_path: \"%s\" successfully.\n", dev_path);

	/*Read the directory of the devices*/
	if( NULL == (dev_dir = readdir(dev_pathfd)) )
	{
		LOGGER_ERROR("Read dev_dir failure: %s", strerror(errno));
		return -2;	
	}
	while( NULL != (dev_dir = readdir(dev_pathfd)) )
	{
		/*Search for the chip_name of the devices*/
		if( strstr(dev_dir->d_name, "28-") )
		{
			strncpy(chip_name, dev_dir->d_name, sizeof(chip_name));
			found_flag = 1;
		}	
	}
	if( !found_flag )
	{
		LOGGER_ERROR("read dev_dir failure: %s", strerror(errno));
		return -3;	
	}

	/*Generate the full path of the devices and the chip*/
	strncat(dev_path, chip_name, (sizeof(dev_path) - strlen(dev_path)));
	strncat(dev_path, "/w1_slave", (sizeof(dev_path) - strlen(dev_path)));
	LOGGER_DEBUG("The full path is: \"%s\"\n", dev_path);

	closedir(dev_pathfd);

	/*Open the full path and the file of the devices*/
	if( (fd = open(dev_path, O_RDONLY)) < 0 )
	{
		LOGGER_ERROR("open full path and file failure: %s", strerror(errno));
		return -4;
	}

	/*Read the file of the devices*/
	if( (n = read(fd, buf, sizeof(buf))) < 0 )
	{
		LOGGER_ERROR("read the file failure: %s", strerror(errno));
		return -5;
	}

	/*Get temperature from the file*/
	if( NULL == (data = strstr(buf, "t=")) )
	{
		LOGGER_ERROR("get temperature failure: %s", strerror(errno));
		return -6;
	}
	data += 2;
	*temp = atof(data)/1000;
	LOGGER_DEBUG("get_temp successfully: %f", *temp);

	/*Close the file fd*/
	close(fd);

	return 0;	
}




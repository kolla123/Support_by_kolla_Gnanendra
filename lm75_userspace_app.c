/*
*File name :  Dex_Temp_App.c                                              
*Author    :  Gnanendra.K.
*Copyright Dexcel Electronics Design Pvt Ltd 
*Description: This code is for reading Temperature in celsius from lm75 sensor.. Through SYSfs
*Class: User space Application 
*
*/

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

float temp_data();

int main(void)
{
	temp_data();

        return(0);
}

float temp_data()
{
	int temp_fd = 0 ,ret = 0;
	char temp_buf[10] ={0};
	char read_buf[100] ={0};                                                          
	float temperature = 0;
	int addr = 0x48;
	temp_fd = open("/dev/i2c-0", O_RDWR);
	if(temp_fd < 0)
	{
		printf("device open fails :%d\n",temp_fd);
		return -1;
	}
	printf("device opened successfully\n");

	//ret = ioctl(temp_fd, I2C_SLAVE, addr);
	ret = ioctl(temp_fd, I2C_SLAVE_FORCE, addr);
	if(ret < 0)
	{
	   printf("error: ioctl() failed :%d\n",ret);
	   return -2;
	}

	memset(temp_buf,0,sizeof(temp_buf));
	
	while(1)
	{
		sleep(1);
		ret = read(temp_fd,temp_buf, sizeof(temp_buf));
		if(ret < 0)
		{
			printf("Error..! Can't read any data..\n");
			return(-3);
		}
		printf("temp_buf =%d\n",temp_buf[0]);
		uint8_t msb, lsb;
		msb = temp_buf[0] & 0x00FF; // extract msb byte
		lsb = (temp_buf[0] & 0xFF00) >> 8; // extract lsb byte
		int8_t temperature0 = (int8_t) msb;
		int8_t temperature1 = (lsb & 0x80) >> 7; // is either zero or one
		float temperature = temperature0 + 0.5 * temperature1;
		printf("temperature: %0.2f\n", temperature);

	}
	close (temp_fd);
	return (temperature);
}

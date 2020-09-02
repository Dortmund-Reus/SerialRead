#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "openDev.h"
#include "kk.h"
#include "file.h"

struct STime		stcTime;
struct SAcc 		stcAcc;
struct SGyro 		stcGyro;
struct SAngle 		stcAngle;
struct SMag 		stcMag;
struct SDStatus 	stcDStatus;
struct SPress 		stcPress;
struct SLonLat 		stcLonLat;
struct SGPSV 		stcGPSV;

void CopeSerialData(unsigned char ucData, FILE * fp)
{
	static unsigned char ucRxBuffer[250];
	static unsigned char ucRxCnt = 0;	
	
//	printf("%02d ", ucData);
	ucRxBuffer[ucRxCnt++]=ucData;
	if (ucRxBuffer[0]!=0x55) 
	{
		ucRxCnt=0;
		return;
	}
	if (ucRxCnt<11) {return;}//都是11
	else
	{
//		printf("hello world! %02d %02d\n", ucRxBuffer[0], ucRxBuffer[1]);
		switch(ucRxBuffer[1])
		{
			//只要8个字节数据
			case 0x50:	
				memcpy(&stcTime,&ucRxBuffer[2],8);
				fprintf(fp, "Time:20%hhu-%hhu-%hhu %hhu:%hhu:%f\n", stcTime.ucYear, stcTime.ucMonth, stcTime.ucDay, stcTime.ucHour, stcTime.ucMinute, (float)stcTime.ucSecond+(float)stcTime.usMiliSecond/1000);
				break;
			case 0x51:	
				memcpy(&stcAcc,&ucRxBuffer[2],8);
				fprintf(fp, "Acc:%f %f %f\n", (float)stcAcc.a[0]/32768*16, (float)stcAcc.a[1]/32768*16, (float)stcAcc.a[2]/32768*16);
				break;
			case 0x52:	
				memcpy(&stcGyro,&ucRxBuffer[2],8);
				fprintf(fp, "Gyro:%f %f %f\n", (float)stcGyro.w[0]/32768*2000, (float)stcGyro.w[1]/32768*2000, (float)stcGyro.w[2]/32768*2000);
				break;
			case 0x53:	
				memcpy(&stcAngle,&ucRxBuffer[2],8);
				fprintf(fp, "Angle:%f %f %f\n", (float)stcAngle.Angle[0]/32768*180, (float)stcAngle.Angle[1]/32768*180, (float)stcAngle.Angle[2]/32768*180);
				break;
			case 0x54:	
				memcpy(&stcMag,&ucRxBuffer[2],8);
				fprintf(fp, "Mag:%d %d %d\n", stcMag.h[0], stcMag.h[1], stcMag.h[2]);
				break;
			case 0x55:	
				memcpy(&stcDStatus,&ucRxBuffer[2],8);
				fprintf(fp, "DStatus:%d %d %d %d\n", stcDStatus.sDStatus[0], stcDStatus.sDStatus[1], stcDStatus.sDStatus[2], stcDStatus.sDStatus[3]);
				break;
			case 0x56:	
				memcpy(&stcPress,&ucRxBuffer[2],8);
				fprintf(fp, "Pressure:%ld %f\n", stcPress.lPressure, (float)stcPress.lAltitude/100);
				break;
			case 0x57:	
				memcpy(&stcLonLat,&ucRxBuffer[2],8);
				fprintf(fp, "Longitude: %ldDeg %fm Lattitude: %ldDeg %fm\n", stcLonLat.lLon/10000000, (double)(stcLonLat.lLon % 10000000)/1e5,
						(stcLonLat.lLat/10000000), (double)(stcLonLat.lLat % 10000000)/1e5);
				break;
			case 0x58:	
				memcpy(&stcGPSV,&ucRxBuffer[2],8);
				fprintf(fp, "GPSHeight: %fm GPSYaw: %fDeg GPSV: %fkm/h\n", (float)stcGPSV.sGPSHeight/10, (float)stcGPSV.sGPSYaw/10, (float)stcGPSV.lGPSVelocity/1000);
				break;
			default:
				fprintf(fp, "\n");
				break;
		}
		ucRxCnt=0;
	}
}

int main()
{
//	fprintf(fp, "hello world!\n");
//	fprintf(fp, "hello world!\n");
    int fd;
    int nread;
//	char str[60];
//	sprintf(str, "helloworld");
//	printf("%s\n", str);
//	sprintf(str, "helloworld");
//	printf("%s\n", str);
    serial_parse phandle;
	memset(phandle.buff, 0, MAX_BUFF_SIZE);
    phandle.rxbuffsize = 0;

    char *dev_name = "/dev/ttyUSB0";//根据实际情况选择串口

    while(1) 
    {  
        fd = OpenDev(dev_name); //打开串口 

        if(fd>0)
        {
            set_speed(fd,115200); //设置波特率
            printf("set speed success!\n");
        }     
        else  
        { 
            printf("Can't Open Serial Port!\n"); 
            sleep(1);  //休眠1s
            continue; 
        } 
        break;
    }

    if(set_Parity(fd,8,1,'N')==FALSE) //设置校验位 
    {
        printf("Set Parity Error\n"); 
        exit(1);
    }
    else
    {
        printf("Set Parity Success!\n"); 
    }

	//printf("length of a.log = %ld\n", get_file_size("logs/a.log"));

	int count = 0;
	char filename[20];
    while(1) 
    { 
		sprintf(filename, "logs/log_%d.txt", count);
		long length = get_file_size(filename);
		if(length >= FILE_MAX_SIZE)
		{
			printf("文件 %s 大小超出！\n", filename);
			count = (++count) % 10;
			sprintf(filename, "logs/log_%d.txt", count);
			//如果下一个也满了，就把下一个清零
			if(get_file_size(filename) >= FILE_MAX_SIZE)
				remove(filename);
		}
		FILE * fp = fopen(filename, "a+");
//		printf("loop\n");
        usleep(10000);  //休眠1ms
//		memset(phandle.buff, 0, phandle.rxbuffsize);
        nread = read(fd , phandle.buff + phandle.rxbuffsize, MAX_BUFF_SIZE - phandle.rxbuffsize);
//		printf("nread = %d\n", nread);
        phandle.rxbuffsize += nread;
        phandle.buff[phandle.rxbuffsize] = '\0';
		/*
		处理 phandle.buff
		*/
		for(size_t i = 0; i < phandle.rxbuffsize; ++i)
		{
			CopeSerialData(phandle.buff[i], fp);
		}
		phandle.rxbuffsize = 0;
		fclose(fp);
	}
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_modelname(FILE *fp)
{
	int j = 0;
	char buffer[128];
	char *bf = buffer;

	fp = fopen("/proc/cpuinfo", "r");

	if(fp == NULL)
		printf("Error to open\n");
	
	for(int i=0;i<5;i++)
		fgets(buffer, sizeof(buffer), fp);

	while(buffer[j++] != ':')
		bf++;

	printf("Model Name	: %s", ++bf);
	fclose(fp);
}

void parse_vendorid(FILE *fp)
{
	int j  = 0;
	char buffer[128];
	char *bf = buffer;

	fp = fopen("/proc/cpuinfo", "r");

	if(fp == NULL)
		printf("Error to open\n");
	
	for(int i=0;i<2;i++)
		fgets(buffer, sizeof(buffer), fp);

	while(buffer[j++] != ':')
		bf++;

	printf("Vendor ID	: %s", ++bf);
	fclose(fp);
}

void parse_cpumhz(FILE *fp)
{
	int j  = 0;
	char buffer[128];
	char *bf = buffer;

	fp = fopen("/proc/cpuinfo", "r");

	if(fp == NULL)
		printf("Error to open\n");
	
	for(int i=0;i<7;i++)
		fgets(buffer, sizeof(buffer), fp);

	while(buffer[j++] != ':')
		bf++;

	printf("CPU MHz		: %s", ++bf);
	fclose(fp);
}

void parse_L1dcachesize(FILE *fp)
{
	int j = 0;
	char buffer[128];

	fp = fopen("/sys/devices/system/cpu/cpu0/cache/index0/size", "r");

	if(fp == NULL)
		printf("Error to open\n");

	fgets(buffer, sizeof(buffer), fp);

	printf("L1d cache size	:  %s", buffer);
	fclose(fp);
}

void parse_L1icachesize(FILE *fp)
{
	int j = 0;
	char buffer[128];

	fp = fopen("/sys/devices/system/cpu/cpu0/cache/index1/size", "r");

	if(fp == NULL)
		printf("Error to open\n");

	fgets(buffer, sizeof(buffer), fp);

	printf("L1i cache size	:  %s", buffer);
	fclose(fp);
}

void parse_L2cachesize(FILE *fp)
{
	int j = 0;
	char buffer[128];

	fp = fopen("/sys/devices/system/cpu/cpu0/cache/index2/size", "r");

	if(fp == NULL)
		printf("Error to open\n");

	fgets(buffer, sizeof(buffer), fp);

	printf("L2  cache size	:  %s", buffer);
	fclose(fp);
}

int main(void)
{
	FILE *fp;

	parse_vendorid(fp);
	parse_modelname(fp);
	parse_cpumhz(fp);
	parse_L1dcachesize(fp);
	parse_L1icachesize(fp);
	parse_L2cachesize(fp);

	return 0;
}

/*************************************************************************
	> File Name: backRunTest.c
	> Author: Ce Qi
	> Mail: 
	> Created Time: Sun 09 Jun 2015 06:03:27 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main()
{
	FILE *fp1;
	int i = 0;
	
	fp1 = fopen("back_1.txt","w");
	for(i=0;i<10;i++)
	{
	   	//printf("%d\n",i);
		fprintf(fp1,"%d\n",i);
		sleep(2);
	}
}

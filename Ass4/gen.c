#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
int main()
{
	int n=2;
	while(n--)
	{
		int N=10000,priority=10,slptime=1;
		float slpprob=0.3;
		
		char arg[100];
		memset(arg,0,100);
		sprintf(arg,"xterm -hold -e ./process %d %d %f %d",N,priority,slpprob,slptime);
		printf("here\n");
		if(!fork())
		{
			system(arg);
			return;
		}
		sleep(1);
	}
	n=2;
	while(n--)
	{
		int N=4000,priority=5,slptime=3;
		float slpprob=0.7;
		char arg[100];
		memset(arg,0,100);
		sprintf(arg,"xterm -hold -e ./process %d %d %f %d",N,priority,slpprob,slptime);
		printf("here\n");
		if(!fork())
		{
			system(arg);
			return;
		}
		sleep(1);
	}
}
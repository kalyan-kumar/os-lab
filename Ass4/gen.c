#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
int main()
{
	int spid=72, n=2;
	while(n--)
	{
		int N=100,priority=10,slptime=1;
		float slpprob=0.3;
		
		char arg[100];
		memset(arg,0,100);
		sprintf(arg,"xterm -hold -e ./process %d %d %f %d %d",N,priority,slpprob,slptime,spid);
		spid++;
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
		int N=40,priority=5,slptime=3;
		float slpprob=0.7;
		char arg[100];
		memset(arg,0,100);
		sprintf(arg,"xterm -hold -e ./process %d %d %f %d %d",N,priority,slpprob,slptime,spid);
		spid++;
		printf("here\n");
		if(!fork())
		{
			system(arg);
			return;
		}
		sleep(1);
	}
}
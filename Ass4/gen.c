#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

int main()
{
	int n=2;
	//printf("Enter the number of process\n");
	//scanf("%d",&n);
	while(n--)
	{
		int N=10000,priority=10,slptime=1;
		float slpprob=0.3;
		
		char arg[100];
		memset(arg,0,100);
		
		
		sprintf(arg,"xterm -hold -e ./process %d %d %f %d",N,priority,slpprob,slptime);
		printf("here\n");
		// execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c", "arg", (void*)NULL);
// 			if ( (execlp("xterm","xterm","-hold","-e",arg, "127.0.0.1", (char *) 0)) < 0)  {
// 				  printf("Failed to Start the Echo Client. Exiting application.");
// 				  return 1;
// }	
		if(fork())
			sleep(1);
		else
		{
			system(arg);
			return;
		}

		

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
		if(fork())
			sleep(1);
		else
		{
			system(arg);
			return;
		}
		
		

	}
}
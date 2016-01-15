/*
Kalyan Kumar  - 13CS10023
Nitesh Sekhar - 13CS10033
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[])
{
	int A[100010], size, search, b, e, x, y, i, j;
	FILE *p;
	if(argc!=3)
	{
		printf("Run in './<executable> <file-name> <number-to-search>' format.\n");
		return 0;
	}
	p = fopen(argv[1], "r");
	for(size = 0;fscanf(p, "%d ", &A[size]) != EOF; size++){}

	pid_t pid1, pid2, pid3 = getpid();
	search = atoi(argv[2]);
	b = 0;
	e = size-1;
	while(1)
	{
		if(e<b)
			exit(0);
		if((e-b+1)<=5)
		{
			for(i=b;i<=e;i++)
			{
				if(A[i]==search)
				{
					if(i!=0)
						exit(i);
					else
						exit(size);
				}
			}
			exit(0);
		}
		x = fork();
		if(x)
		{
			y = fork();
			if(!y)
				e = (b+e)/2;
			else
			{
				pid1 = wait(&i);
				kill(pid1, SIGKILL);
				pid2 = wait(&j);
				kill(pid2, SIGKILL);	
				break;
			}
		}
		else
			b = ((b+e)/2)+1;
	}
	if(pid3==getpid())
	{	
		if(i==0 && j==0 )
			printf("Not Found :'(\n");
		else
		{	
			if(WEXITSTATUS(i)==size||WEXITSTATUS(j)==size)
				printf("Found it at 0 :D!!\n");
			else
			{
				if(i==0)
					printf("Found it at %d :D!!\n",WEXITSTATUS(j));
				else
					printf("Found it at %d :D!!\n",WEXITSTATUS(i));
			}	
		}
	}
	else
	{
		if(i==0 && j==0)
			exit(0);
		else
		{
			if(i==0)
				exit(WEXITSTATUS(j));
			else
				exit(WEXITSTATUS(i));
		}
	}	
	return 0;
}
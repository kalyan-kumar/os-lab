#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

int readFromPipe(int file)
{
    int tmp;
    read(file, &tmp, sizeof(tmp));
    return tmp;
}

void writeToPipe(int file, int x)
{
    write(file, &x, sizeof(x));
}

int main(int argc, char *argv[])
{
	printf("Created node\n");
	int i, j, n=atoi(argv[argc-1]), num=(argc-3)/2, val=atoi(argv[1]), *pipes, start, end, *list, flags, avg;
	pipes = (int *)malloc(2*num*sizeof(int));
	list = (int *)malloc(n*sizeof(int));
	for(i=0;i<num;i++)
	{
		pipes[2*i] = atoi(argv[2*i+2]);
		pipes[(2*i)+1] = atoi(argv[2*i+3]);
		flags = fcntl(pipes[2*i], F_GETFL, 0);
		fcntl(pipes[2*i], F_SETFL, flags | O_NONBLOCK);
	}
	for(i=0,list[0]=val,start=-1,end=0;i<n;i++)
	{
		for(;start!=end;start++)
		{
			for(j=0;j<num;j++)
				writeToPipe(pipes[(2*j)+1], list[start+1]);
		}
		for(j=0;j<num;j++)
		{
			while((list[++end]=readFromPipe(pipes[2*j]))!=-1);
			if(list[end-1]==-1)
				end--;
		}
	}
	for(i=0,avg=0;i<n;i++)
		avg += list[i];
	avg = avg/n;
	return avg;
}
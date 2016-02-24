#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct my_msgbuf {
	long mtype;
	int pid;
	int prior;
};

void heapify(struct my_msgbuf *A, int n, int pos)
{
	int i;
	for(i=pos;i<n;)
	{
		if(2*i<n)
	}
}

int main()
{

}
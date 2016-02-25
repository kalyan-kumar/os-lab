#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define TIME_QUANTUM 1000

int preempt;

struct my_msgbuf {
	long mtype;
	int pid;
	int prior;
};

void heapify(struct my_msgbuf *A, int n, int pos)
{
	int i, x;
	struct my_msgbuf tmp;
	for(i=pos;i<n;)
	{
		x = -1;
		if(((2*i)+1)<n)
		{
			if(A[(2*i)+1].prior > A[2*i].prior)
				x = (2*i)+1;
			else
				x = 2*i;
		}
		else if(2*i < n)
			x = 2*i;
		if(x != -1 && A[x].prior > A[i].prior)
		{
			tmp = A[x];
			A[x] = A[i];
			A[i] = tmp;
			i = x;
		}
		else 
			i = n;
	}
}

void makeHeap(struct my_msgbuf *A, int n)
{
	int i;
	for(i=n/2;i>=0;i--)
		heapify(A, n, i);
}

void changeIt(int sig)
{
	preempt = 1;
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("Incorrect syntax. Enter <executable-path> <P-RR/RR>\n");
		exit(1);
	}

	int msqid, i, x, running, suspending, tq = TIME_QUANTUM;
	key_t key;
	pid_t pid=getpid(), sched_pid;
	struct my_msgbuf buf;

	buf.pid = pid;
	buf.prior = priority;

	if ((key = ftok("sched.c", 'H')) == -1) {
		perror("ftok");
		exit(1);
	}
	if ((msqid = msgget(key, 0644 )) == -1) {
		perror("msgget");
		exit(1);
	}


	struct my_msgbuf A[100000];

	while(1)
	{
		signal(SIGUSR1, changeIt);
		if(argv[1]=="P-RR")
			running = 0;			// Get the process here
		else
			running = 0;			// Get the process here
		preempt = 0;
		for(i=0;i<tq;i++)
		{
			if(preempt==1)
			{
				// Add code to remove the process here
				suspending = running;
				break;
			}
		}
		if(i==tq)
			kill(A[suspending].pid, SIGUSR1);
	}
}
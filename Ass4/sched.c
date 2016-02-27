#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define TIME_QUANTUM1 1000
#define TIME_QUANTUM2 2000

#define PROC_LIMIT 10000

int preempt;

struct my_msgbuf {
	long mtype;
	int pid;
	int prior;
};

void interchange(struct my_msgbuf *a, struct my_msgbuf *b)
{
	struct my_msgbuf tmp;
	tmp.mtype = a->mtype;
	tmp.pid = a->pid;
	tmp.prior = a->prior;
	a->mtype = b->mtype;
	a->pid = b->pid;
	a->prior = b->prior;
	b->mtype = tmp.mtype;
	b->pid = tmp.pid;
	b->prior = tmp.prior;
}

void heapify(struct my_msgbuf *A, int n, int pos)
{
	int i, x;
	for(i=pos;i<n;)
	{
		x = -1;
		if(((2*i)+1)<n)
		{
			if(A[(2*i)+1].prior < A[2*i].prior)
				x = (2*i)+1;
			else
				x = 2*i;
		}
		else if(2*i < n)
			x = 2*i;
		if(x != -1 && A[x].prior < A[i].prior)
		{
			interchange(&A[i], &A[x]);
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

	int msqid, i, x, running, tq = 0, qu_size=0, start=0, end=0;
	key_t key;
	pid_t pid=getpid(), sched_pid;
	struct my_msgbuf buf;

	if((key = ftok("gen.c", 'A')) == -1)
	{
		perror("ftok");
		exit(1);
	}
	if((msqid = msgget(key, 0644|IPC_CREAT )) == -1)
	{
		perror("msgget");
		exit(1);
	}
	struct my_msgbuf A[PROC_LIMIT];

	while(1)
	{
		signal(SIGUSR1, changeIt);
		signal(SIGUSR2, changeIt);

		while(msgrcv(msqid, &buf, sizeof (struct my_msgbuf), 5, IPC_NOWAIT) != -1)
		{
			if(buf.prior!=0)
			{
				 if(!strcmp(argv[1],"P-RR"))
				{	
					printf("lellol\n");
					interchange(&A[++qu_size], &buf);
					makeHeap(A, qu_size);
				}
				else
				{
					interchange(&A[end++], &buf);
				}
				buf.mtype = 1;
				buf.pid = getpid();
				buf.prior = 0;
				if(msgsnd(msqid, &buf, sizeof(struct my_msgbuf), 0) == -1)
				{
					perror("msgsnd");
					exit(1);
				}
			}
		}

		if(!strcmp(argv[1],"P-RR"))
		{
			if(qu_size==0)
				continue;
			running = 1;
		}
		else
		{
			if(start==end)
				continue;
			running = start;
		}
		preempt = 0;
		if(A[running].prior==5)
			tq=TIME_QUANTUM2;
		else
			tq=TIME_QUANTUM1;
		kill(A[running].pid, SIGUSR1);
		nanosleep(1000);
		printf("%d is running now\n", A[running].pid );
		//getchar();

		
		for(i=0;i<tq;i++)
		{
			if(preempt==1)
			{
				if(!strcmp(argv[1], "P-RR"))
				{
					interchange(&A[running], &A[qu_size]);
					qu_size--;
					heapify(A, qu_size, 1);
				}
				else  
				{
					start++;
					start %= PROC_LIMIT;
				}
				break;
			}
		}
		if(i==tq)
		{	
			kill(A[running].pid, SIGUSR2);
			nanosleep(1);
			if(!strcmp(argv[1], "P-RR"))
			{
				interchange(&A[running], &A[qu_size]);
				makeHeap(A, qu_size);
			}
			else
			{
				interchange(&A[start], &A[end]);
				start++;
				start %= PROC_LIMIT;
				end++;
				end %= PROC_LIMIT;
			}
		}
	}
}
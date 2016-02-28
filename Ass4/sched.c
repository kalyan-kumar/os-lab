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
	int pseudo;
	int pid;
	int prior;
};

void interchange(struct my_msgbuf *a, struct my_msgbuf *b)
{
	a->mtype = b->mtype;
	a->pseudo = b->pseudo;
	a->pid = b->pid;
	a->prior = b->prior;
}

void heapify(struct my_msgbuf *A, int n, int pos)
{
	struct my_msgbuf tmp;
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
			interchange(&tmp, &A[x]);
			interchange(&A[x], &A[i]);
			interchange(&A[i], &tmp);
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
	struct my_msgbuf buf, tmp;

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
		if(!strcmp(argv[1], "P-RR"))
		{
			for(i=1;i<=qu_size;i++)
				printf("%d ", A[i].pid);
			printf("\n");
		}
		else
		{
			for(i=start;i<end;i++)
				printf("%d ", A[i].pid);
			printf("\n");
		}
		signal(SIGUSR1, changeIt);
		signal(SIGUSR2, changeIt);

		while(msgrcv(msqid, &buf, sizeof (struct my_msgbuf), 5, IPC_NOWAIT) != -1)
		{
			if(buf.prior!=0)
			{
				if(!strcmp(argv[1],"P-RR"))
				{	
					interchange(&A[++qu_size], &buf);
					makeHeap(A, qu_size);
				}
				else
				{
					interchange(&A[end++], &buf);
				}
				buf.mtype = buf.pseudo;
				buf.pseudo = 0;
				buf.pid = getpid();
				buf.prior = 0;
				if(msgsnd(msqid, &buf, sizeof(struct my_msgbuf), 0) == -1)
				{
					perror("msgsnd");
					exit(1);
				}
			}
		}
		sleep(1);
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
		printf("Runnin process: %d\n", A[running].pid);
		for(i=0;i<tq;i++)
		{
			usleep(50);
			if(preempt==1)
			{
				printf("preempted\n");
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
			signal(SIGUSR1, changeIt);
			signal(SIGUSR2, changeIt);
			kill(A[running].pid, SIGUSR2);
			if(!strcmp(argv[1], "P-RR"))
			{
				interchange(&tmp, &A[running]);
				interchange(&A[running], &A[qu_size]);
				interchange(&A[qu_size], &tmp);
				makeHeap(A, qu_size);
			}
			else
			{
				interchange(&tmp, &A[start]);
				interchange(&A[start], &A[end]);
				interchange(&A[end], &tmp);
				start++;
				start %= PROC_LIMIT;
				end++;
				end %= PROC_LIMIT;
			}
		}
	}
}
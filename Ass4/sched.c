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

volatile int preempt;
int no_p=0,reached=0;
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
void avg()
{
    FILE *fp =fopen("result.txt" , "r");
    int i;
    char buff[2000];
    double id, avgrsp=0, avgwt=0, avgtat=0;
    for(i = 0; i<no_p;i++)
    {
        double tid, tavgrsp, tavgwt, tavgtat;
        fscanf(fp,"%lf %lf %lf %lf", &tid, &tavgrsp, &tavgwt, &tavgtat);

        avgrsp = tavgrsp + avgrsp;
        avgwt = tavgwt + avgwt;
        avgtat = tavgtat + avgtat;
    }
    fclose(fp);
    fp =fopen("result.txt" , "a");
    fprintf(fp, "%f %f %f\n",avgrsp/no_p , avgwt/no_p, avgtat/no_p);
    fclose(fp);
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
void stopit(int sig)
{
	preempt=1;
	reached++;

}
int cmp(const void *a, const void *b) {
	return ((struct my_msgbuf *) a)->prior < ((struct my_msgbuf *) b)->priority;
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
		signal(SIGUSR2, stopit);
		if(no_p==reached && reached!=0)
			break;
		while(msgrcv(msqid, &buf, sizeof (struct my_msgbuf), -10, IPC_NOWAIT) != -1)
		{
			if(buf.prior!=0)
			{
<<<<<<< HEAD
				if(buf.mtype==5)
					no_p++;
				 if(!strcmp(argv[1],"P-RR"))
=======
				if(!strcmp(argv[1],"P-RR"))
>>>>>>> f80c7d02d05fcc329b0bca5cf12eb26b474d1eb1
				{	
					interchange(&A[++qu_size], &buf);
					qsort(A, qu_size, sizeof(struct my_msgbuf), cmp);
					//makeHeap(A, qu_size);
					//usleep(50);
				}
				
				else
				{
					interchange(&A[end++], &buf);
				}
				printf("process %d\n", buf.pseudo);
				buf.mtype = buf.pseudo;
				buf.pseudo = 0;
				buf.pid = getpid();
				buf.prior = 0;
				if(msgsnd(msqid, &buf, sizeof(struct my_msgbuf), 0) == -1)
				{
					perror("msgsnd");
					exit(1);
				}
				
				nanosleep(1);
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
		usleep(50);
		kill(A[running].pid, SIGUSR1);
<<<<<<< HEAD
		printf("%d is running now\n", A[running].pid );
		
		printf("\n");
		//getchar();

		
=======
		printf("Runnin process: %d\n", A[running].pid);
>>>>>>> f80c7d02d05fcc329b0bca5cf12eb26b474d1eb1
		for(i=0;i<tq;i++)
		{
			usleep(50);
			if(preempt==1)
			{
				printf("preempted\n");
				if(!strcmp(argv[1], "P-RR"))
				{
					interchange(&A[running], &A[qu_size]);
					qsort(A, --qu_size, sizeof(struct my_msgbuf), cmp);
					//qu_size--;
					//heapify(A, qu_size, 1);
				}
				else  
				{
					start++;
					start %= PROC_LIMIT;
				}
				break;
			}
		}
		preempt=0;
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
	avg();

}
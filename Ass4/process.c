#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct my_msgbuf {
	long mtype;
	int pid;
	int prior;
};

void notify(int sig)
{

}

void suspend(int sig)
{
	pause();
}

int main(int argc, char *argv[])
{
	printf("Started\n");
	srand(time(NULL));
	if(argc < 5)
	{
		printf("Pass arguments correctly\n");
		exit(0);
	}
	int noi, priority, slp_time;
	float slp_prob, slp_now;
	noi = atoi(argv[1]);
	priority = atoi(argv[2]);
	slp_prob = atof(argv[3]);
	slp_time = atoi(argv[4]);
	
	int msqid, i, x;
	key_t key;
	pid_t pid=getpid(), sched_pid;
	struct my_msgbuf buf;

	if ((key = ftok("sched.c", 'H')) == -1) {
		perror("ftok");
		exit(1);
	}
	if ((msqid = msgget(key, 0644 )) == -1) {
		perror("msgget");
		exit(1);
	}

	signal(SIGUSR1, notify);
	signal(SIGUSR2, suspend);

	buf.mtype = 1;
	buf.pid = pid;
	buf.prior = priority;

	if(msgsnd(msqid, &buf, sizeof(struct my_msgbuf), 0) == -1)
	{
		perror("msgsnd");
		exit(1);
	}
	if(msgrcv(msqid, &buf, sizeof (struct my_msgbuf), getppid(), 0) == -1)
	{
		perror("msgrcv");
		exit(1);
	}
	sched_pid = buf.pid;

	for(i=0;i<noi;i++)
	{
		printf("PID:%d, iteration:%d", getpid(), i+1);
		slp_now = (rand()%101)/100;
		if(slp_now < slp_prob)
		{
			kill(sched_pid, SIGUSR1);
			printf("PID:%d Going for IO", pid);
			sleep(slp_time);
			if (msgsnd(msqid, &buf, sizeof(struct my_msgbuf), 0) == -1)
				perror("msgsnd");
			printf("PID:%d Back from IO", pid);
			buf.mtype = 1;
			buf.pid = pid;
			buf.prior = priority;
			if(msgsnd(msqid, &buf, sizeof(struct my_msgbuf), 0) == -1)
			{
				perror("msgsnd");
				exit(1);
			}
			if(msgrcv(msqid, &buf, sizeof (struct my_msgbuf), getppid(), 0) == -1)
			{
				perror("msgrcv");
				exit(1);
			}
			sched_pid = buf.pid;
			suspend(1);
		}
	}
	kill(sched_pid, SIGUSR2);
	return 0;
}
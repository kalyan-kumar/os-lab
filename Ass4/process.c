#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct my_msgbuf {
	long mtype;
	int pseudo;
	int pid;
	int prior;
};

void notify(int sig)
{
	printf("Received signal notify\n");
}

void suspend(int sig)
{
	printf("Received signal suspend\n");
	pause();
	printf("Woke up from pause\n");
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
	int noi, priority, slp_time, spid;
	float slp_prob, slp_now;
	noi = atoi(argv[1]);
	priority = atoi(argv[2]);
	slp_prob = atof(argv[3]);
	slp_time = atoi(argv[4]);
	spid = atoi(argv[5]);
	
	int msqid, i, x;
	key_t key;
	pid_t pid=getpid(), sched_pid;
	struct my_msgbuf buf;

	if ((key = ftok("gen.c", 'A')) == -1) {
		perror("ftok");
		exit(1);
	}
	if ((msqid = msgget(key, 0644 )) == -1) {
		perror("msgget");
		exit(1);
	}

	signal(SIGUSR1, notify);
	signal(SIGUSR2, suspend);

	buf.mtype = 5;
	buf.pseudo = spid;
	buf.pid = pid;
	buf.prior = priority;

	if(msgsnd(msqid, &buf, sizeof(struct my_msgbuf), 0) == -1)
	{
		perror("msgsnd");
		exit(1);
	}
	if(msgrcv(msqid, &buf, sizeof (struct my_msgbuf), spid, 0) ==-1)
	{
		if(errno!=EINTR)
		{
			perror("msgsrcv");
			exit(1);
		}
	}
	printf("Finally\n");
	pause();
	sched_pid = buf.pid;

	printf("%f\n",slp_prob);
	for(i=0;i<noi;i++)
	{
		printf("PID:%d, iteration:%d\n", getpid(), i+1);
		slp_now = (rand()%101)/100.0;
		if(slp_now < slp_prob)
		{
			kill(sched_pid, SIGUSR1);
			printf("PID %d Going to IO\n", pid);
			signal(SIGUSR2,SIG_IGN);
			signal(SIGUSR1,SIG_IGN);
			sleep(slp_time);

			printf("PID:%d Back from IO\n", pid);
			buf.mtype = 5;
			buf.pseudo = spid;
			buf.pid = pid;
			buf.prior = priority;
			if(msgsnd(msqid, &buf, sizeof(struct my_msgbuf), 0) == -1)
			{
				perror("msgsnd");
				exit(1);
			}
			printf("Sent message\n");
			if(msgrcv(msqid, &buf, sizeof (struct my_msgbuf), spid, 0) == -1)
			{
				if(errno!=EINTR)
				{
					perror("msgsrcv");
					exit(1);
				}
			}
			printf("Received message\n");
			sched_pid = buf.pid;
			signal(SIGUSR1, notify);
			signal(SIGUSR2, suspend);
			pause();
		}
	}
	kill(sched_pid, SIGUSR2);
	return 0;
}
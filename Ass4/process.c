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
volatile int pause_=0;
void notify(int sig)
{
	if (sig== SIGUSR1) {
		// fprintf(stderr, "process paused\n");
		pause_ = 0;
	}}

void suspend(int sig)
{
	if (sig== SIGUSR2) {
		// fprintf(stderr, "process paused\n");
		pause_ = 1;
	}
	//pause();
	//printf("Woke up from pause\n");
}

int main(int argc, char *argv[])
{
	signal(SIGUSR1, notify);
	signal(SIGUSR2, suspend);
	struct timespec tstart={0,0}, tend={0,0},twaitb={0,0},twaita={0,0},tresp={0,0};
    clock_gettime(CLOCK_MONOTONIC, &tstart);
//for(i=0;i<10000;i++);
	srand(time(NULL));
	if(argc < 6)
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
	printf("Started process %d \n",spid);
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

	if(msgrcv(msqid, &buf, sizeof (struct my_msgbuf), spid, 0) ==-1)
	{
		if(errno!=EINTR)
		{
			perror("msgsrcv");
			exit(1);
		}
	}
	printf("Finally\n");

	
	clock_gettime(CLOCK_MONOTONIC, &twaitb);

	suspend(SIGUSR2);
	clock_gettime(CLOCK_MONOTONIC, &tresp);
	double wait_=((double)tresp.tv_sec + 1.0e-9*tresp.tv_nsec)-((double)twaitb.tv_sec + 1.0e-9*twaitb.tv_nsec);
	double resp=((double)tresp.tv_sec + 1.0e-9*tresp.tv_nsec)-((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);

	sched_pid = buf.pid;

	printf("%f\n",slp_prob);
	for(i=0;i<noi;i++)
	{
		if(pause_==1)
		{	

			clock_gettime(CLOCK_MONOTONIC, &twaitb);

			pause_=0;
			pause();
			clock_gettime(CLOCK_MONOTONIC, &twaita);
			wait_+=((double)twaita.tv_sec + 1.0e-9*twaita.tv_nsec)-((double)twaitb.tv_sec + 1.0e-9*twaitb.tv_nsec);

		}
		printf("PID:%d, iteration:%d\n", getpid(), i+1);
		slp_now = (rand()%101)/100.0;
		if(slp_now < slp_prob)
		{
			kill(sched_pid, SIGUSR1);
			printf("PID %d Going to IO\n", pid);
			signal(SIGUSR1,SIG_IGN);
			signal(SIGUSR2,SIG_IGN);

			sleep(slp_time);

			printf("PID:%d Back from IO\n", pid);
			buf.mtype = 7;
			buf.pseudo = spid;
			buf.pid = pid;
			buf.prior = priority;
			if(msgsnd(msqid, &buf, sizeof(struct my_msgbuf), 0) == -1)
			{
				perror("msgsnd");
				exit(1);
			}
			printf("Sent message\n");
			signal(SIGUSR1, notify);
			signal(SIGUSR2, suspend);
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
	clock_gettime(CLOCK_MONOTONIC, &tend);
	double stotal;
	double totaltime=stotal=((double)tend.tv_sec + 1.0e-9*tend.tv_nsec)-((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
	double extra=totaltime-(int)totaltime;
	printf("Response took about %.5f seconds\n",resp);

    printf("Total took about %d %d %.5f time\n",((int)totaltime/3600),(((int)totaltime%3600)/60),(((int)totaltime%3600)%60)+extra);
    totaltime=wait_;
    extra=totaltime-(int)totaltime;
    printf("waited for %d: %d : %.5f time \n",((int)totaltime/3600),(((int)totaltime%3600)/60),(((int)totaltime%3600)%60)+extra );
    FILE * fp = fopen("result.txt" , "a");
    fprintf(fp, "%d \t%lf\t%lf\t%lf\n", pid, resp, wait_, stotal);
    fclose(fp);
	return 0;
}
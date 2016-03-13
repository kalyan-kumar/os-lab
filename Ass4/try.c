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
	int pid;
	int prior;
};



int main(int argc, char *argv[])
{
	printf("Started\n");
	time_t rawtime;
  struct tm * timeinfo,*notinfo;
  int i;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  
//	kill(sched_pid, SIGUSR2);
  	  notinfo = localtime ( &rawtime );
  	printf("%d\n", (notinfo->tm_hour) );
  	 struct timespec tstart={0,0}, tend={0,0};
    clock_gettime(CLOCK_MONOTONIC, &tstart);
   for(i=0;i<10000;i++);
   	sleep(66);
    clock_gettime(CLOCK_MONOTONIC, &tend);
    double totaltime=((double)tend.tv_sec + 1.0e-9*tend.tv_nsec)-((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
    double extra=totaltime-(int)totaltime;
    printf("took about %d %d %.5f seconds\n",((int)totaltime/3600),(((int)totaltime%3600)/60),(((int)totaltime%3600)%60)+extra);
    

	return 0;
}
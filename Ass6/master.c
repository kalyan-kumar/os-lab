#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#define SHM_SIZE 1024
#define NUM_OF_ATM 20

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

void makeAtm()
{
	char arg[100];
	sprintf(arg,"xterm -hold -e ./atm");
	system(arg);
	exit(1);
}

int main(int argc, char *argv[])
{
	key_t key;
	int shmid, i, msgqs[NUM_OF_ATM];
	char *data;
	pid_t atms[NUM_OF_ATM];
	FILE *fp = fopen("locator.txt", "w");

	if((key = ftok("master.c", 125))==-1)
	{
		perror("ftok");
		exit(1);
	}
	printf("%d\n", key);
	if((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT))==-1)
	{
		perror("shmget");
		exit(1);
	}
	data = shmat(shmid, (void *)0, 0);
	if(data == (char *)(-1))
	{
		perror("shmat");
		exit(1);
	}

	for(i=0;i<NUM_OF_ATM;i++)
	{
		atms[i] = fork();
		if(atms[i] < 0)
		{
			perror("fork");
			exit(1);
		}
		else if(atms[i] == 0)
			makeAtm();
		else
		{
			if((key = ftok("master.c", i+1))==-1)
			{
				perror("ftok");
				exit(1);
			}
			if((msgqs[i] = msgget(key, 0644 | IPC_CREAT)) == -1)
			{
		        perror("msgget");
		        exit(1);
		    }
		   	fprintf(fp, "ATM%d\t\t%d\t\t0\n", atms[i], key);
		}
	}
	fclose(fp);





	if(shmdt(data) == -1)
	{
        perror("shmdt");
        exit(1);
    }
    if(shmctl(shmid, IPC_RMID, NULL) == -1)
    {
    	perror("shmctl");
    	exit(1);
    }
    return 0;
}
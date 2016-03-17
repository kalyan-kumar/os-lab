#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#define SHM_SIZE 1024
#define NUM_OF_ATM 20

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

int initsem(key_t key, int nsems)  /* key from ftok() */
{
    int i;
    union semun arg;
    struct semid_ds buf;
    struct sembuf sb;
    int semid;

    semid = semget(key, nsems, IPC_CREAT | IPC_EXCL | 0666);

    if (semid >= 0) /* we got it first */
    {
    	printf("Semaphore created first\n");
        sb.sem_op = 1;
        sb.sem_flg = 0;
        arg.val = 1;
        for(sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num++)
        { 
            /* do a semop() to "free" the semaphores. */
            /* this sets the sem_otime field, as needed below. */
            if (semop(semid, &sb, 1) == -1)
            {
                int e = errno;
                semctl(semid, 0, IPC_RMID); /* clean up */
                errno = e;
                return -1; /* error, check errno */
            }
        }
    }
    else if (errno == EEXIST) /* someone else got it first */
    {
        int ready = 0;
        semid = semget(key, nsems, 0); /* get the id */
        if (semid < 0) return semid; /* error, check errno */
        /* wait for other process to initialize the semaphore: */
        arg.buf = &buf;
        for(i = 0; i < MAX_RETRIES && !ready; i++)
        {
            semctl(semid, nsems-1, IPC_STAT, arg);
            if (arg.buf->sem_otime != 0)
                ready = 1;
            else
                sleep(1);
        }
        if (!ready)
        {
            errno = ETIME;
            return -1;
        }
    }
    else
        return semid; /* error, check errno */
    return semid;
}

void makeAtm(int i)
{
	char arg[100];
	sprintf(arg,"./atm %d", i);
	system(arg);
	exit(1);
}

int main(int argc, char *argv[])
{
	key_t key;
	int shmid, sid, i, msgqs[NUM_OF_ATM];
	char *data;
	pid_t atms[NUM_OF_ATM];
	FILE *fp = fopen("locator.txt", "w");

	if((key = ftok("master.c", 125))==-1)
	{
		perror("ftok");
		exit(1);
	}
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

	if((key = ftok("master.c", 126))==-1)
	{
		perror("ftok");
		exit(1);
	}
	if ((sid = initsem(key, 6)) == -1)
    {
        perror("initsem");
        exit(1);
    }

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

	for(i=0;i<NUM_OF_ATM;i++)
	{
		atms[i] = fork();
		if(atms[i] < 0)
		{
			perror("fork");
			exit(1);
		}
		else if(atms[i] == 0)
			makeAtm(i);
		else
		{
			if((key = ftok("master.c", i+1))==-1)
			{
				perror("ftok");
				exit(1);
			}
			
		   	fprintf(fp, "ATM%d\t\t%d\t\t0\t\t%d\n", atms[i], key);
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
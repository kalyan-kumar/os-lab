#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>

#define MAX_RETRIES 10
#define MAX_TRAINS 10000
#define NORTH 0
#define WEST 1
#define SOUTH 2
#define EAST 3
#define TRACK 4
#define MATRIX 5

int sid, **A;
FILE *fp1, *fp2;
char trains[MAX_TRAINS];
pid_t tr_pr[MAX_TRAINS];

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
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

void readMatrix(int total)
{
    FILE *fp = fopen("matrix.txt", "r");
    char buf[10000];
    fgets(buf, 10000, fp);
    fgets(buf, 10000, fp);
    int i, j, k;
    for(i=0;i<total;i++)
    {
        fgets(buf, 10000, fp);
        for(k=0,j=0;k<strlen(buf);k++)
        {
            if(buf[k]=='0')
            {
                A[i][j] = 0;
                j++;
            }
            else if(buf[k]=='1')
            {
                A[i][j] = 1;
                j++;
            }
            else if(buf[k]=='2')
            {
                A[i][j] = 2;
                j++;
            }
        }
        fgets(buf, 10000, fp);
    }
    fclose(fp);
}

void createTrain(int ind, int n)
{
	char arg[100];
	sprintf(arg,"xterm -hold -e ./train %c %d %d", trains[ind], ind, n);
	tr_pr[ind] = fork();
	if(tr_pr[ind] < 0)
	{
		perror("Fork error");
		exit(1);
	}
	else if (tr_pr[ind] == 0)
	{
		system(arg);
		exit(1);
	}
}

int checkDeadlock(int n)
{
	struct sembuf sb;
    sb.sem_flg = SEM_UNDO;
    sb.sem_num = MATRIX;
    sb.sem_op = -1;
    if(semop(sid, &sb, 1) == -1)
    {
        perror("semop");
        exit(1);
    }
    readMatrix(n);
    sb.sem_op = 1;
    if(semop(sid, &sb, 1) == -1)
    {
        perror("semop");
        exit(1);
    }
    int i, j, f[4];
    for(i=0;i<4;i++)
    	f[i] = 0;
    for(i=0;i<n;i++)
    {
    	for(j=0;j<4;j++)
    	{
    		if(A[i][j]==2 && A[i][(j+1)%4]==1)
    			f[j] = 1;
    	}
    }
    if(f[0]==1 && f[1]==1 && f[2]==1 && f[3]==1)
    	return 1;
    else
    	return 0;
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		perror("Run executable as <executable-path> <probability>\n");
		exit(1);
	}
	srand(time(NULL));
	key_t key;
	if ((key = ftok("manager.c", 'J')) == -1)
	{
        perror("ftok");
        exit(1);
    }
    if ((sid = initsem(key, 6)) == -1)
    {
        perror("initsem");
        exit(1);
    }
    int i, n, j;
    float cur, p;
    p = atof(argv[1]);
    fp1 = fopen("sequence.txt", "r");
    fp2 = fopen("matrix.txt", "w");

    fgets(trains, MAX_TRAINS, fp1);
    n = strlen(trains);
    fprintf(fp2, "\t\t\t\t\tSemaphores\n\t\tNorth\tWest\tSouth\tEast\n");
    for(i=0;i<n;i++)
    	fprintf(fp2, "\t\t0\t0\t0\t0\n\n");
    fclose(fp1);
    fclose(fp2);

    A = (int **)malloc(n*sizeof(int *));
    for(i=0;i<n;i++)
    	A[i] = (int *)malloc(4*sizeof(int));
	
	for(i=0;i<n;)
    {
    	cur = (rand()%101)/100;
    	if(cur > p)
    	{
    		createTrain(i, n);
    		i++;
    	}
    	else
    	{
    		if(checkDeadlock(n)==1)
    		{
    			printf("............................\nSystem Deadlocked\n");
    			for(j=0;j<i;j++)
    				kill(tr_pr[j], SIGKILL);
    			exit(1);
    		}
    	}
    }
    while(1)
    {
    	sleep(1);
    	if(checkDeadlock(n)==1)
		{
			printf("............................\nSystem Deadlocked\n");
			for(j=0;j<i;j++)
				kill(tr_pr[j], SIGKILL);
			exit(1);
		}
    }
    return 0;
}
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

#define SHM_SIZE 1024
#define NUM_OF_ATM 20
#define MAX_RETRIES 5
#define ENTER 1
#define WITHDRAW 2
#define DEPOSIT 3
#define VIEW 4
#define LEAVE 5

struct clidet
{
	int acc_num;
	int balance;
	time_t timestamp;
};

struct mas_msgbuf {
    long mtype;
    int cli_pid;
    int present;
};

struct transaction
{
	int acc_num;
	int money;
	int type;
	time_t timestamp;
};

int sid, msgqid, num_clients, quit;
struct clidet client_details[1000];

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

void addclient(int acc_num)
{
	struct clidet temp;
	temp.acc_num=acc_num;
	temp.balance=rand()%100000+2000;
	temp.timestamp=time(NULL);
}

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

void createIPC()
{
	key_t key;
	if((key = ftok("master.c", 126))==-1)
	{
		perror("ftok");
		exit(1);
	}
	if((sid = initsem(key, NUM_OF_ATM)) == -1)
    {
        perror("initsem");
        exit(1);
    }
	if((key = ftok("master.c", 127))==-1)
	{
		perror("ftok");
		exit(1);
	}
	if((msgqid = msgget(key, 0644 | IPC_CREAT)) == -1)
	{
        perror("msgget");
        exit(1);
    }
}

void makeAtm(int i)
{
	char *arg[3];
	arg[0]=(char*)malloc(100*sizeof(char));
	strcpy(arg[0],"./atm");
	arg[1]=(char*)malloc(10*sizeof(char));
	sprintf(arg[1],"%d",i);
	arg[2]=NULL;
	execvp(arg[0],arg);
	// sprintf(arg,[1]",%d", i);
	// system(arg);
	exit(1);
}
void globalConsistency(struct mas_msgbuf buf)
{
	int i, j, cur_mem, amount, k0, *ptr;
	void *datas;
	struct clidet *tempdata;
	struct transaction *temptime;	
	for(i=0,amount=0;i<NUM_OF_ATM;i++)
	{
		if((k0 = ftok("master.c", 2*i))==-1)
		{
			perror("ftok");
			exit(1);
		}
		if((cur_mem = shmget(k0, SHM_SIZE, 0644 ))==-1)
		{
			perror("shmget");
			exit(1);
		}
		datas=shmat(cur_mem, (void *)0, 0);
		ptr = (int *)datas;
		temptime = (struct transaction *)(datas+sizeof(int));
		for(j=0;j<(*ptr);j++)
		{
			if((temptime+j*sizeof(struct transaction))->acc_num == buf.cli_pid)
			{

				if((temptime+j*sizeof(struct transaction))->type==WITHDRAW)
					amount -= (temptime+j*sizeof(struct transaction))->money;
				else
					amount += (temptime+j*sizeof(struct transaction))->money;
				struct transaction temp;

				temp.money=(temptime+((*ptr)-1)*sizeof(struct transaction))->money;
				temp.acc_num=(temptime+((*ptr)-1)*sizeof(struct transaction))->acc_num;
				temp.type=(temptime+((*ptr)-1)*sizeof(struct transaction))->type;
				temp.timestamp=(temptime+((*ptr)-1)*sizeof(struct transaction))->timestamp;

				(temptime+((*ptr)-1)*sizeof(struct transaction))->money=(temptime+j*sizeof(struct transaction))->money;
				(temptime+((*ptr)-1)*sizeof(struct transaction))->acc_num=(temptime+j*sizeof(struct transaction))->acc_num;
				(temptime+((*ptr)-1)*sizeof(struct transaction))->type=(temptime+j*sizeof(struct transaction))->type;
				(temptime+((*ptr)-1)*sizeof(struct transaction))->timestamp=(temptime+j*sizeof(struct transaction))->timestamp;


				(temptime+j*sizeof(struct transaction))->money=temp.money;
				(temptime+j*sizeof(struct transaction))->acc_num=temp.acc_num;
				(temptime+j*sizeof(struct transaction))->type=temp.type;
				(temptime+j*sizeof(struct transaction))->timestamp=temp.timestamp;

				*ptr--;

			}
		}

		for(i=0;i<num_clients;i++)
		{
			if(client_details[i].acc_num==buf.cli_pid)
			{
				client_details[i].balance+=amount;
				client_details[i].timestamp=time(NULL);
			}
		}	
		//writetolocalshm
	}
	// void *data=shmat()
}

void sigHand(int sig)
{
	quit = 1;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	quit = 0;
	key_t k0, k1;
	int  i;
	pid_t atms[NUM_OF_ATM];
	struct mas_msgbuf buf;
	createIPC();
	num_clients = 0;
	FILE *fp = fopen("locator.txt", "w");
    fprintf(fp, "ATM ID\t\tmsg que key\t\tlock\t\tshm key\n" );
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
			if((k0 = ftok("master.c", 2*i))==-1)
			{
				perror("ftok");
				exit(1);
			}
			if((k1 = ftok("master.c", (2*i)+1))==-1)
			{
				perror("ftok");
				exit(1);
			}
		   	fprintf(fp, "%d\t\t\t%d\t\t\t0\t\t\t%d\n", i, k1, k0);
		}
	}
	fclose(fp);
	signal(SIGINT, sigHand);
	while(quit==0)
	{
		if(msgrcv(msgqid, &buf, sizeof(struct mas_msgbuf), 1, IPC_NOWAIT) != -1)
		{
			if(buf.present==1)
			{
				globalConsistency(buf);
				continue;
			}
			for(i=0;i<num_clients;i++)
			{
				if(client_details[i].acc_num != buf.cli_pid)
					continue;
				buf.present = 1;
				buf.mtype=buf.cli_pid;
				if(msgsnd(msgqid, &buf, sizeof(struct mas_msgbuf), 0) == -1)
				{
					perror("msgsnd");
					exit(1);
				}
				break;
			}
			if(i==num_clients)
			{
				buf.present = 0;
				buf.mtype=buf.cli_pid;
				if(msgsnd(msgqid, &buf, sizeof(struct mas_msgbuf), 0) == -1)
				{
					perror("msgsnd");
					exit(1);
				}
				addclient(buf.cli_pid);
			}
		}
	}
	for(i=0;i<NUM_OF_ATM;i++)
		kill(atms[i], SIGINT);
	if(msgctl(msgqid, IPC_RMID, NULL) == -1)
	{
        perror("msgctl");
        exit(1);
    }
    return 0;
}
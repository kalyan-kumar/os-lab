#include <time.h>
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

struct transaction
{
	int acc_num;
	int money;
	int type;
	time_t timestamp;
};

int msgqid, masqid, shmid,ind;
void *data;

struct mas_msgbuf {
	long mtype;
	int cli_pid;
	int present;
};

struct cli_msgbuf {
	long mtype;
	int cli_pid;
	int result;
	int money;
};

void createIPC()
{
	key_t key, k0, k1;
	if((key = ftok("master.c", 127))==-1)
	{
		perror("ftok");
		exit(1);
	}
	if((masqid = msgget(key, 0644)) == -1)
	{
		perror("msgget");
		exit(1);
	}
	if((k0 = ftok("master.c", 2*ind))==-1)
	{
		perror("ftok");
		exit(1);
	}
	if((shmid = shmget(k0, SHM_SIZE, 0644 | IPC_CREAT))==-1)
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
	int *A = (int *)data;
	(*A) = 0;
	A = (int *)(data+SHM_SIZE-sizeof(int));
	(*A) = 0;
	if((k1 = ftok("master.c", (2*ind)+1))==-1)
	{
		perror("ftok");
		exit(1);
	}
	if((msgqid = msgget(k1, 0644 | IPC_CREAT)) == -1)
	{
		perror("msgget");
		exit(1);
	}
}

int localConsistencyCheck(int accnt_num, int money)
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
			if((temptime+j*sizeof(struct transaction))->acc_num == accnt_num)
			{
				if((temptime+j*sizeof(struct transaction))->type==WITHDRAW)
					amount -= (temptime+j*sizeof(struct transaction))->money;
				else
					amount += (temptime+j*sizeof(struct transaction))->money;
			}
		}
	}
	ptr = (int *)(data+SHM_SIZE-sizeof(int));
	tempdata = (struct clidet *)(data+SHM_SIZE-sizeof(int));
	for(j=1;j<=(*ptr);j++)
	{
		if((tempdata-j*sizeof(struct clidet))->acc_num == accnt_num)
			break;
	}
	if(((tempdata-j*sizeof(struct clidet))->balance)+amount>=0)
		return 1;
	else
		return -1;
}

void enterRoutine(struct cli_msgbuf buf1)
{
	struct mas_msgbuf buf2;
	buf2.mtype = 2;					// Set mtype
	buf2.cli_pid = buf1.cli_pid;
	buf2.present=0;

	if(msgsnd(masqid, &buf2, sizeof(struct mas_msgbuf), 0) == -1)
	{
		perror("msgsnd");
		exit(1);
	}
	if(msgrcv(masqid, &buf2, sizeof(struct mas_msgbuf), buf1.cli_pid, 0) == -1)
	{
		perror("msgrcv");
		exit(1);
	}
	if(buf2.present==1)
	{
		buf1.result = 1;
		if(msgsnd(msgqid, &buf1, sizeof(struct cli_msgbuf), 0) == -1)
		{
			perror("msgsnd");
			exit(1);
		}

	}
	else
	{
		buf1.result = 0;
		if(msgsnd(msgqid, &buf1, sizeof(struct cli_msgbuf), 0) == -1)
		{
			perror("msgsnd");
			exit(1);
		}
	}
}

void withdrawRoutine(struct cli_msgbuf buf1)
{
	if(localConsistencyCheck(buf1.cli_pid, buf1.money)==-1)
	{
		buf1.result = 0;
		if(msgsnd(msgqid, &buf1, sizeof(struct cli_msgbuf), 0) == -1)
		{
			perror("msgsnd");
			exit(1);
		}
	}
	else
	{
		int *ptr = (int *)data;
		struct transaction *tempdata = (struct transaction *)(data+sizeof(int));
		(tempdata+(*ptr)*sizeof(struct transaction))->acc_num = buf1.cli_pid;
		(tempdata+(*ptr)*sizeof(struct transaction))->money = buf1.money;
		(tempdata+(*ptr)*sizeof(struct transaction))->type = WITHDRAW;
		(tempdata+(*ptr)*sizeof(struct transaction))->timestamp = time(NULL);
		(*ptr)++;
		buf1.result = 1;
		if(msgsnd(msgqid, &buf1, sizeof(struct cli_msgbuf), 0) == -1)
		{
			perror("msgsnd");
			exit(1);
		}
	}
}

void depositRoutine(struct cli_msgbuf buf1)
{
	int *ptr = (int *)data;
	struct transaction *tempdata = (struct transaction *)(data+sizeof(int));
	(tempdata+(*ptr)*sizeof(struct transaction))->acc_num = buf1.cli_pid;
		(tempdata+(*ptr)*sizeof(struct transaction))->money = buf1.money;
		(tempdata+(*ptr)*sizeof(struct transaction))->type = DEPOSIT;
		(tempdata+(*ptr)*sizeof(struct transaction))->timestamp = time(NULL);
	(*ptr)++;
	buf1.result = 1;
	if(msgsnd(msgqid, &buf1, sizeof(struct cli_msgbuf), 0) == -1)
	{
		perror("msgsnd");
		exit(1);
	}
}

void viewRoutine(struct cli_msgbuf buf1)
{
	struct mas_msgbuf buf2;
	buf2.mtype = 1;					// Set mtype
	buf2.cli_pid = buf1.cli_pid;
	buf2.present=1;
	if(msgsnd(masqid, &buf2, sizeof(struct mas_msgbuf), 0) == -1)
	{
		perror("msgsnd");
		exit(1);
	}
	if(msgrcv(masqid, &buf2, sizeof(struct mas_msgbuf), buf1.cli_pid, 0) == -1)
	{
		perror("msgrcv");
		exit(1);
	}
	if(buf2.present == 0)
		return ;
	// struct clidet *tempdata;
	// tempdata=(struct clidet *)data;
	int *ptr = (int *)(data+SHM_SIZE-sizeof(int));
	int j;
	struct clidet *tempdata = (struct clidet *)(data+SHM_SIZE-sizeof(int));
	for(j=1;j<=(*ptr);j++)
	{
		if((tempdata-j)->acc_num == buf1.cli_pid)
		{
			buf1.mtype=buf1.cli_pid;
				// buf1.cli_pid=pid;
			buf1.result=1;
			buf1.money=(tempdata-j)->balance;
			if(msgsnd(msgqid, &buf1, sizeof(struct cli_msgbuf), 0) == -1)
			{
				perror("msgsnd");
				exit(1);
			}
			break;
		}
	}
}

void waitForClient()
{
	struct cli_msgbuf buf1;
	if(msgrcv(msgqid, &buf1, sizeof(struct cli_msgbuf), 1, 0) == -1)
	{
		perror("msgrcv");
		exit(1);
	}
	switch(buf1.mtype)
	{
		case ENTER:
		enterRoutine(buf1);
		break;
		case WITHDRAW:
		withdrawRoutine(buf1);
		break;
		case DEPOSIT:
		depositRoutine(buf1);
		break;
		case VIEW:
		viewRoutine(buf1);
		break;
	}
}

int main(int argc, char *argv[])
{
	ind = atoi(argv[1]);
	createIPC(ind);
	while(1)
		waitForClient(ind);
	return 0;
}
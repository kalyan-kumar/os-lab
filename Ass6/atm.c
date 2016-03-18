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

int msgqid, masqid, shmid;
struct clidet *data;

struct mas_msgbuf {
    long mtype;
    int cli_pid;
    int present;
};

struct cli_msgbuf {
    long mtype;
    int cli_pid;
    int result;
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
	if(data == (struct clidet*)(-1))
	{
		perror("shmat");
		exit(1);
	}
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

int localConsistencyCheck()
{
	return -1;
}

void waitForClient()
{
	struct cli_msgbuf buf1;
	if(msgrcv(msgqid, &buf1, sizeof(struct cli_msgbuf), 1, 0) == -1)
	{
		perror("msgrcv");
		exit(1);
	}
	if(buf1.mtype == ENTER)
	{
		struct mas_msgbuf buf2;
		buf2.mtype = 1;					// Set mtype
		buf2.cli_pid = buf1.cli_pid;
		if(msgsnd(masqid, &buf2, sizeof(struct mas_msgbuf), 0) == -1)
		{
			perror("msgsnd");
			exit(1);
		}
		if(msgrcv(msgqid, &buf2, sizeof(struct mas_msgbuf), 1, 0) == -1)
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
				exit(1);1
			}
		}
	}
	else if(buf1.mtype == WITHDRAW)
	{
		if(localConsistencyCheck()==-1)
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
			// Update local address space here
			buf1.result = 1;
			if(msgsnd(msgqid, &buf1, sizeof(struct cli_msgbuf), 0) == -1)
			{
				perror("msgsnd");
				exit(1);
			}
		}
	}
	else if(buf1.mtype == DEPOST)
	{
		
	}
	else if(buf1.mtype == VIEW)
	{

	}
}

int main(int argc, char *argv[])
{
	int ind = atoi(argv[1]);
	createIPC(ind);
	while(1)
	{
		waitForClient();
	}

    return 0;
}
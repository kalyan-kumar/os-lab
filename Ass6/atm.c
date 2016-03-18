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
    char command[200];
    int cli_pid;
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

void waitForClient()
{
	struct cli_msgbuf buf1;
	if(msgrcv(msgqid, &buf1, sizeof(struct cli_msgbuf), 1, 0) == -1)
	{
		perror("msgrcv");
		exit(1);
	}
	if(strcmp(buf1.command, "ENTER")==0)
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
			buf1.mtype = 1;
			strcpy(buf1.command. "EX");
			if(msgsnd(msgqid, &buf1, sizeof(struct cli_msgbuf), 0) == -1)
			{
				perror("msgsnd");
				exit(1);
			}
		}
		else
		{
			buf1.mtype = 1;
			strcpy(buf1.command. "N-EX");
			if(msgsnd(msgqid, &buf1, sizeof(struct cli_msgbuf), 0) == -1)
			{
				perror("msgsnd");
				exit(1);
			}
		}
	}
	else if(strcmp(buf1.command, "VIEW")==0)
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
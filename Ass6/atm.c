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
struct transaction
{
	int acc_num;
	int money;
	int type;
	time_t timestamp;
};
int msgqid, masqid, shmid,ind;
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

int localConsistencyCheck(int money)
{
	int i,k0;
	for(i=0;i<20;i++)
	{
		if(i==ind)
			continue;
		struct clidet *tempdata;
		struct transaction *temptime;
		if((k0 = ftok("master.c", 2*i))==-1)
		{
		perror("ftok");
		exit(1);
		}
		if((shmid = shmget(k0, SHM_SIZE, 0644 ))==-1)
		{
			perror("shmget");
			exit(1);
		}
		void *datas=shmat(shmid,(void *)0,0);
		tempdata=(struct clidet *)datas;
		temptime=(struct transaction * )(datas+SHM_SIZE/2);
		int j;
		for(j=0;j<SHM_SIZE/2;j++)
		{
			if(tempdata[j].balance<0||tempdata[j].balance>102000)
				break;
			if(cur.timestamp>tempdata[j].timestamp)
				continue;
			if(cur.acc_num==tempdata[j].acc_num)
			{
				cur.balance=tempdata[j].balance;
				cur.timestamp=tempdata[j].timestamp;
			}
		}
		for(j=0;j<SHM_SIZE/2;j++)
		{
			if(cur.timestamp>temptime[j].timestamp)
				continue;
			//write timestamp
		}

	}
	if(cur.balance<money)
		return -1;
	cur.balance-=money;
	//make transaction;
}
int globalConsistencyCheck(int pid)
{
	struct mas_msgbuf buf2;
	buf2.mtype = 2;					// Set mtype
	buf2.cli_pid = pid;
	buf2.present=1;
	if(msgsnd(masqid, &buf2, sizeof(struct mas_msgbuf), 0) == -1)
		{
			perror("msgsnd");
			exit(1);
		}
	if(msgrcv(masqid, &buf2, sizeof(struct mas_msgbuf), pid, 0) == -1)
		{
			perror("msgrcv");
			exit(1);
		}
//sendtoclient and update yo shm	

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
		buf2.present=0;//using present as a type too
		if(msgsnd(masqid, &buf2, sizeof(struct mas_msgbuf), 0) == -1)
		{
			perror("msgsnd");
			exit(1);
		}
		if(msgrcv(masqid, &buf2, sizeof(struct mas_msgbuf), 1, 0) == -1)///was msgqid
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
	else if(buf1.mtype == WITHDRAW)
	{
		if(localConsistencyCheck(buf1.money)==-1)
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
	else if(buf1.mtype == DEPOSIT)
	{
		struct transaction temp;
		temp.acc_num=buf1.cli_pid;
		temp.money=buf1.money;
		temp.type=DEPOSIT;
		temp.timestamp=time(NULL);
		//addtoshm
	}
	else if(buf1.mtype == VIEW)
	{
		if(globalConsistencyCheck(buf1.cli_pid)==-1)
			return;

	}
}

int main(int argc, char *argv[])
{
	ind = atoi(argv[1]);
	createIPC(ind);
	while(1)
	{
		waitForClient(ind);
	}

    return 0;
}
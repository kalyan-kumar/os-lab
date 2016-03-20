#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>

#define NUM_OF_ATM 20
#define ENTER 1
#define WITHDRAW 2
#define DEPOSIT 3
#define VIEW 4
#define LEAVE 5

int keys[NUM_OF_ATM], msqid, semid, n;

struct cli_msgbuf {
    long mtype;
    int cli_pid;
    int result;
    int money;
};

void displayAtms()
{
	FILE *fp = fopen("locator.txt", "r");
	int i, id1, id2, id3;
	for(i=0;fscanf(fp, "%d\t\t\t%d\t\t\t%d\t\t\t%d", &id1, &keys[i], &id2, &id3)!=EOF;i++);
		//printf("Enter the ID of the ATM you want to access\n");
	fclose(fp);
	printf("Enter ATM number you want to access\n");
	return ;
}

void updateFile(int n)
{
	return ;
}

void createIPC()
{
	key_t key;
	scanf("%d", &n);
	if((msqid = msgget(keys[n], 0644)) == -1)
	{
        perror("msgget");
        exit(1);
    }
    if((key = ftok("master.c", 126))==-1)
	{
		perror("ftok");
		exit(1);
	}
	semid = semget(key, NUM_OF_ATM, 0);
    if(semid < 0)
    {
        perror("Master didnt create the ATMs yet\n");
        exit(1);
    }
	struct sembuf sb;
    sb.sem_flg = SEM_UNDO;
    sb.sem_num = n;
    sb.sem_op = -1;
    if(semop(semid, &sb, 1) == -1)
    {
        perror("semop");
        exit(1);
    }
    updateFile(n);
    return ;
}

void withdraw()
{
	int amount;
	printf("Enter the amount of money you want to withdraw\n");
	scanf("%d", &amount);
	struct cli_msgbuf buf;
	buf.mtype = WITHDRAW;
	buf.cli_pid = getpid();
	buf.money = amount;
	if(msgsnd(msqid, &buf, sizeof(struct cli_msgbuf), 0) == -1)
	{
		perror("msgsnd");
		exit(1);
	}
	if(msgrcv(msqid, &buf, sizeof(struct cli_msgbuf), 0, 0) == -1)
	{
		perror("msgrcv");
		exit(1);
	}
	if(buf.result==1)
		printf("Collect your money\n");
	else
		printf("Sorry, you do not have sufficient balance for the requested amount of money\n");
}

void deposit()
{
	int amount;
	printf("Enter the amount of money you want to deposit\n");
	scanf("%d", &amount);
	struct cli_msgbuf buf;
	buf.mtype = DEPOSIT;
	buf.cli_pid = getpid();
	buf.money = amount;
	buf.result=0;
	if(msgsnd(msqid, &buf, sizeof(struct cli_msgbuf), 0) == -1)
	{
		perror("msgsnd");
		exit(1);
	}
	if(msgrcv(msqid, &buf, sizeof(struct cli_msgbuf), 0, 0) == -1)
	{
		perror("msgrcv");
		exit(1);
	}
	if(buf.result==1)
		printf("Deposited\n");
	else
		printf("Sorry, there was an internal error\n");
}

void view()
{
	struct cli_msgbuf buf;
	buf.mtype = VIEW;
	buf.cli_pid = getpid();
	if(msgsnd(msqid, &buf, sizeof(struct cli_msgbuf), 0) == -1)
	{
		perror("msgsnd");
		exit(1);
	}
	if(msgrcv(msqid, &buf, sizeof(struct cli_msgbuf), getpid(), 0) == -1)
	{
		perror("msgrcv");
		exit(1);
	}
	if(buf.result==1)
		printf("Your account balance is - %d\n", buf.money);
	else
		printf("Sorry, there was an internal error\n");
}

int mainScreen()
{
	int n;
	printf("Enter -\n(1) for Withdraw\n(2) for Deposit\n(3) for View\n(4) to exit\n");
	scanf("%d", &n);
	switch(n)
	{
		case 1:
			withdraw();
			return 1;
		case 2:
			deposit();
			return 2;
		case 3:
			view();
			return 3;
		case 4:
			printf("Come back again\n");
			return -1;
	}
}

int main(int argc, char *argv[])
{
	while(1)

	{displayAtms();
	createIPC();
	
	struct cli_msgbuf buf;
	buf.mtype = ENTER;
	buf.cli_pid = getpid();
	if(msgsnd(msqid, &buf, sizeof(struct cli_msgbuf), 0) == -1)
	{
		perror("msgsnd");
		exit(1);
	}
	printf("sent\n");
	if(msgrcv(msqid, &buf, sizeof(struct cli_msgbuf), 0, 0) == -1)
	{
		perror("msgrcv");
		exit(1);
	}
	if(buf.result==0)
		printf("Create a new account\nYour account number - %d\n", getpid());
	else
		printf("Welcome!\nAccount number - %d\n", getpid());
	while(1)
	{
		if(mainScreen()==-1)
			break;
	}}
	return 0;
}
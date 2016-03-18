#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define NUM_OF_ATM 20

int keys[NUM_OF_ATM], msqid, semid, n;

struct cli_msgbuf {
    long mtype;
    char command[200];
    int cli_pid;
};

void displayAtms()
{
	FILE *fp = fopen("locator.txt", "r");
	int i, id1, id2, id3;
	for(i=0;fscanf(fp, "%d\t\t\t%d\t\t\t%d\t\t\t%d", &id1, &keys[i], &id2, &id3)!=EOF;i++)
		printf("Enter the ID of the ATM you want to access\n");
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

int main(int argc, char *argv[])
{
	displayAtms();
	createIPC();
	
	struct cli_msgbuf buf;
	buf.mtype = 1;					// Set mtype
	strcpy(buf.command, "ENTER");
	buf.cli_pid = getpid();
	
	if(msgsnd(msqid, &buf, sizeof(struct cli_msgbuf), 0) == -1)
	{
		perror("msgsnd");
		exit(1);
	}
	return 0;
}
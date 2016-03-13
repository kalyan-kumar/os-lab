#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>	
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>
#define BUF_SIZE 30000

struct msgbuf {
	long mtype;
	int mtext;
};



int graph[2][10];
int matrix[12][12];
int visit[12],parent[12];
pid_t pid[10];
int cycle;
void printcycle(int j,int i) 
{
	do {
		if(i<10)
			fprintf(stderr,"%d<--",pid[i]);
		else
			fprintf(stderr,"queue%d<--",i-10 );
		i=parent[i];
	} while (i != j );
	if (i < 10)
		printf("%d ", pid[i]);
	else
		printf("queue%d <-- ", i - 10);
}


int dfs (int i)
{
	int flag=0,cycle=0,j;
	visit[i]=1;
	for(j=0;j<12;j++)
	{
		if(matrix[i][j]==1)
		{
			if(visit[j]==0)
			{
				parent[j]=i;
				flag=dfs(j);
			}
			else if (visit[j]==1)
			{
				cycle=1;
				break;
			}
		}
	}
	visit[i]=2;
	if(cycle==1)
	{
		printcycle(j,i);
		return 1;
	}
	else
		return flag;
}

int findcycle()
{
	int i,flag=0;
	// rag();
	for(i=0;i<12;i++)
	{
		if(visit[i]==0){

			if(flag==1)
				break;
			parent[i]=-1;
			flag=dfs(i);
		}

	}
	return flag;


}

//0-mutex--1-empty--2-full (queue0) -------- 3---mutex--4-empty--5--full---6-file
void rag()
{
	int i,j;
	
	for(i=0;i<10;i++){

		for(j=0;j<10;j++)
			matrix[i][j]=0;
		for(;j<12;j++)
		{
			if(graph[j-10][i]==1)
				matrix[i][j]=1;
			else
				matrix[i][j]=0;
		}
	}
	for(;i<12;i++)	
	{
		for(j=0;j<10;j++)
		{
			if(graph[i-10][j]==2)
				matrix[i][j]=1;
			else
				matrix[i][j]=0;
		}
		for(;j<12;j++)
			matrix[i][j]=0;
	}
	// for(i=0;i<12;i++)
	// {
	// 	for(j=0;j<12;j++)
	// 		fprintf(stderr,"%d ", matrix[i][j]);
	// 	fprintf(stderr,"\n");
	// }

}
void makefile()
{
	FILE *fp;
	// fprintf(stderr,"%d\n",graph[0][0] );
	if((fp=fopen("matrix", "wb"))==NULL) {
		fprintf(stderr,"Cannot open file.\n");
	}
	if(fwrite(graph, sizeof(int), 20, fp) != 20) {
		fprintf(stderr,"File read error.");
	}
	fclose(fp);
}
void readfile()
{
	FILE *fp;
	// fprintf(stderr,"%d\n",graph[0][0] );
	if((fp=fopen("matrix", "rb"))==NULL) {
		fprintf(stderr,"Cannot open file.\n");
	}
	if(fread(graph, sizeof(int), 20, fp) != 20) {
		fprintf(stderr,"File read error.");
	}
	fclose(fp);
}
// void filewrite(int semid,int row,int column,int value)
// {
// 	//down(file)
// 	struct sembuf sop;

// 	sop.sem_num = 6;
// 	sop.sem_op  = -1;
// 	sop.sem_flg = 0;
// 	semop(semid, &sop, 1);
// 	FILE *fp;
// 	if((fp=fopen("matrix", "rb"))==NULL) {
// 		fprintf(stderr,"Cannot open file.\n");
// 	}
// 	if(fread(graph, sizeof(float), 20, fp) != 20) {
// 		if(feof(fp))
// 			fprintf(stderr,"Premature end of file.");
// 		else
// 			fprintf(stderr,"File read error.");
// 	}
// 	fclose(fp);  
// 	graph[row][column]=value;
// 	if((fp=fopen("matrix", "wb"))==NULL) {
// 		fprintf(stderr,"Cannot open file.\n");
// 	}
// 	if(fwrite(graph, sizeof(float), 20, fp) != 20) {
// 		fprintf(stderr,"File read error.");
// 	}
// 	//up(file)
// 	sop.sem_num = 6;
// 	sop.sem_op  = 1;
// 	sop.sem_flg = 0;
// 	semop(semid, &sop, 1);


// }
void nothing(int sig){}
int main(int argc, char const *argv[]) {
	signal(SIGTERM,nothing);
	int semid;
	int msqid[2];
	key_t key;
	int q_no;
	int num;
	int timeSleep;
	float p=0.2;
	struct sembuf sop;
	struct msgbuf buf;

	short values[8]={1,10,0,1,10,0,1,1};
	makefile();

	srand(time(NULL));

	if ((key = ftok("producer.c", 'G')) == -1) {
		perror("ftok");
		exit(1);
	}

	if ((semid = semget(key, 8, 0666|IPC_CREAT)) == -1) {
		perror("semget");
		exit(1);
	}
	semctl(semid, 0, SETALL, values);
	if ((key = ftok("producer.c", 'A')) == -1) {
		perror("ftok");
		exit(1);
	}

	if ((msqid[0] = msgget(key, 0666|IPC_CREAT)) == -1) {
		perror("msgget");
		exit(1);
	}
	if ((key = ftok("producer.c", 'B')) == -1) {
		perror("ftok");
		exit(1);
	}

	if ((msqid[1] = msgget(key, 0666|IPC_CREAT)) == -1) {
		perror("msgget");
		exit(1);
	}
	int i;
		// getchar();
	for(i=0;i<10;i++)
	{
		pid[i]=fork();
		if(!pid[i])
		{
			// fprintf(stderr,"here\n");
			char *arg[3];
			arg[0]=(char*)malloc(100*sizeof(char));
			arg[1]=(char*)malloc(10*sizeof(char));
			if(i%2)
			{
				strcpy(arg[0],"./producer");
				
			}
			else
			{
				strcpy(arg[0],"./consumer");
			}
			sprintf(arg[1],"%d",i);
			arg[2]=NULL;
			execvp(arg[0],arg);
		}
	}
	while(1){
		// fprintf(stderr,"here\n");
		sleep(2);
		readfile();
		rag();
		// cycle=0;
		if(findcycle())
		{
			printf("\nEnding now\n");
			int i,j;
			for(i=0;i<2;i++)
			{
				for(j=0;j<12;j++)
				{
					 kill(pid[j], SIGTERM);   
					printf("%d ",graph[i][j]);
				}
				printf("\n");
			}
			if(semctl(semid,0, IPC_RMID, NULL)==-1)
				perror("smctl");
			if (msgctl(msqid[0], IPC_RMID, NULL) == -1) 
				perror("msgctl");
			
			if (msgctl(msqid[1], IPC_RMID, NULL) == -1) 
				perror("msgctl");
			
			return;
		}
		
		// if(cycle){
		// 	for(i=0;i<12;i++)
		// 		printf("%d--\n", parent[i]);
		// 	return;
		// }
		for(i=0;i<12;i++)
		{
			visit[i]=parent[i]=0;
		}


	}
}	
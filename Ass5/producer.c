#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>
#define BUF_SIZE 30000
//0-mutex--1-empty--2-full (queue0) -------- 3---mutex--4-empty--5--full---6--file

struct msgbuf {
	long mtype;
	int mtext;
};
int graph[2][10];
int column;
//0-mutex--1-empty--2-full (queue0) -------- 3---mutex--4-empty--5--full---6-file
void writeresult(int semid,char* value)
{
	//down(file)
	struct sembuf sop;
	int i,j;
// 	for(j=0;j<2;j++){
// 	for(i=0;i<10;i++)
// 		printf("%d ",graph[j][i] );
// 	printf("\n");
// }
// printf("-------------------------------\n");
	sop.sem_num = 7;
	sop.sem_op  = -1;
	sop.sem_flg = 0;
	semop(semid, &sop, 1);
	FILE *fp;
	
	if((fp=fopen("result.txt", "a+"))==NULL) {
		printf("Cannot open file.\n");
	}
	// printf("%d\n", graph[0][0]);
	if(fwrite(value, sizeof(char), strlen(value), fp) ==0) {
		printf("File read error.");
	}
	//up(file)
	fclose(fp);
	sop.sem_num = 7;
	sop.sem_op  = 1;
	sop.sem_flg = 0;
	semop(semid, &sop, 1);
	

}
void filewrite(int semid,int row,int value)
{
	struct sembuf sop;
	//down(file)
	sop.sem_num = 6;
	sop.sem_op  = -1;
	sop.sem_flg = 0;
	semop(semid, &sop, 1);
	FILE *fp;
	if((fp=fopen("matrix", "rb"))==NULL) {
		printf("Cannot open file.\n");
	}
		// printf("%d\n", graph[0][0]);

	if(fread(graph, sizeof(int), 20, fp) != 20) {
		if(feof(fp))
			printf("Premature end of in %d file.",getpid());
		else
			printf("File read error.");
	}
		// printf("%d\n", graph[0][0]);

	fclose(fp);  
	graph[row][column]=value;

// printf("-------------------------------\n");
	if((fp=fopen("matrix", "wb"))==NULL) {
		printf("Cannot open file.\n");
	}
	// printf("%d\n", graph[0][0]);
	if(fwrite(graph, sizeof(int), 20, fp) != 20) {
		printf("File read error.");
	}
	fclose(fp);
	//up(file)
	sop.sem_num = 6;
	sop.sem_op  = 1;
	sop.sem_flg = 0;
	semop(semid, &sop, 1);


}

int main(int argc, char const *argv[]) {

	int semid;
	int msqid[2];
	key_t key;
	int q_no;
	int num;
	int timeSleep;
	struct sembuf sop;
	struct msgbuf buf;

	if(argc < 2)
	{
		printf("Pass arguments correctly1\n");
		exit(0);
	}
	    // printf("came to producer\n");

	column = atoi(argv[1]);
	srand(time(NULL)^(getpid()<<16));

	if ((key = ftok("producer.c", 'G')) == -1) {
		perror("ftok");
		exit(1);
	}

	if ((semid = semget(key, 0, 0666)) == -1) {
		perror("semget");
		exit(1);
	}

	if ((key = ftok("producer.c", 'A')) == -1) {
		perror("ftok");
		exit(1);
	}

	if ((msqid[0] = msgget(key, 0666)) == -1) {
		perror("msgget");
		exit(1);
	}

	if ((key = ftok("producer.c", 'B')) == -1) {
		perror("ftok");
		exit(1);
	}

	if ((msqid[1] = msgget(key, 0666)) == -1) {
		perror("msgget");
		exit(1);
	}

	while (1) {
		q_no = rand() % 2;
		// printf("%d is the queue\n", q_no);
		num  = rand() % 50 + 1;
		timeSleep = rand() %4;
		printf("PID %d Trying To Insert in  Queue %d \n", column, q_no);
		filewrite(semid,q_no,1);
		//down(empty q!)
		sop.sem_num = q_no*3+1;
		sop.sem_op  = -1;
		sop.sem_flg = IPC_NOWAIT;
		if(semop(semid, &sop, 1)==-1)
			continue;
		//down(mutex q!)
		sop.sem_num = q_no*3;
		sop.sem_op  = -1;
		sop.sem_flg = 0;
		semop(semid, &sop, 1);
		buf.mtype = 1;
		buf.mtext = num;
		filewrite(semid,q_no,2);
		if (msgsnd(msqid[q_no], &buf, sizeof(struct msgbuf), 0) == -1)
			perror("msgsnd");

		//up(mutex)
		// 	int i,j;
		// 	printf("%d\n",column );
		// 	for(j=0;j<2;j++){
		// 	for(i=0;i<10;i++)
		// 		printf("%d ",graph[j][i] );
		// 	printf("\n");
		// }
		
		// 	int i,j;
		// 	printf("%d has come out Successfully\n",column );
		// 	for(j=0;j<2;j++){
		// 	for(i=0;i<10;i++)
		// 		printf("%d ",graph[j][i] );
		// 	printf("\n");
		// }

		
		sop.sem_num = q_no*3;
		sop.sem_op  = 1;
		sop.sem_flg = 0;
		semop(semid, &sop, 1);
		//up(full)
		sop.sem_num = q_no*3+2;
		sop.sem_op  = 1;
		sop.sem_flg = 0;
		semop(semid, &sop, 1);
		char value[1000];
		sprintf(value,"PID %d  Successfully Inserted %d into Queue %d \n", getpid(),num, q_no);
		writeresult(semid,value); 
		filewrite(semid,q_no,0);


		

		sleep(timeSleep);
	}

	return 0;
}
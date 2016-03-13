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

struct msgbuf {
	long mtype;
	int mtext;
};



int graph[2][10];
int column;
//0-mutex--1-empty--2-full (queue0) -------- 3---mutex--4-empty--5--full---6-file--7-results
void writeresult(int semid,char* value)
{
	//down(file)
	struct sembuf sop;
	// int i,j;
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
	if(fwrite(value, sizeof(char), strlen(value), fp) !=strlen(value)) {
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
	//down(file)
	struct sembuf sop;
	int i,j;
// 	for(j=0;j<2;j++){
// 	for(i=0;i<10;i++)
// 		printf("%d ",graph[j][i] );
// 	printf("\n");
// }
// printf("-------------------------------\n");
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
			printf("Premature end of prucer %d file.", column);
		else
			printf("File read error.");
	}
		// printf("%d\n", graph[0][0]);

	fclose(fp);  
	graph[row][column]=value;

	if((fp=fopen("matrix", "wb"))==NULL) {
		printf("Cannot open file.\n");
	}
	// printf("%d\n", graph[0][0]);
	if(fwrite(graph, sizeof(int), 20, fp) != 20) {
		printf("File read error.");
	}
	//up(file)
	fclose(fp);
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
	float p=0.8;
	struct sembuf sop;
	struct msgbuf buf;
	if(argc < 2)
	{
		printf("Pass arguments correctly here\n");
		exit(0);
	}
	column = atoi(argv[1]);

	 // printf("came here\n");

		srand(time(NULL)^(getpid()<<16));
		rand();


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

	while(1)
	{
		q_no = rand() % 2;
		// printf("%d is the queue\n", q_no);

		float numberofqueues=(rand()%100)/100.0;
		if(numberofqueues<=p)
		{
			//down(full)
			printf("%d  trying to consume from queue %d \n",getpid(),q_no );
			filewrite(semid,q_no,1);

			sop.sem_num = q_no*3+2;
			sop.sem_op  = -1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);

			//down(mutex)
			sop.sem_num = q_no*3;
			sop.sem_op  = -1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
			filewrite(semid,q_no,2);
			if(msgrcv(msqid[q_no], &buf, sizeof (struct msgbuf), 0, 0) ==-1)
			{
				if(errno!=EINTR)
				{
					perror("msgsrcv");
					exit(1);
				}
			}
			char value[100];
			sprintf(value,"%d successfully consumed %d from queue %d\n",getpid(),buf.mtext,q_no );
			writeresult(semid,value);
			fprintf(stderr,"%s",value );
			//up(mutex)
			sop.sem_num = q_no*3;
			sop.sem_op  = 1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
			//up(full)
			sop.sem_num = q_no*3+1;
			sop.sem_op  = 1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
			filewrite(semid,q_no,0);


		}
		else
		{
			//down(full q)
			// q_no=0;
			filewrite(semid,q_no,1);
			printf("%d  trying to consume from queue %d \n",getpid(),q_no );

			sop.sem_num = q_no*3+2;
			sop.sem_op  = -1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
			//down(full q!)
			sop.sem_num = (1-q_no)*3+2;
			sop.sem_op  = -1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
			//down(mutex q)
			sop.sem_num = q_no*3;
			sop.sem_op  = -1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
			filewrite(semid,q_no,2);

			if(msgrcv(msqid[q_no], &buf, sizeof (struct msgbuf), 0, 0) ==-1)
			{
				if(errno!=EINTR)
				{
					perror("msgsrcv");
					exit(1);
				}
			}

			filewrite(semid,1-q_no,1);
			char value[100];
			sprintf(value,"%d consumed %d from queue %d\n", getpid(),buf.mtext,q_no);
			writeresult(semid,value);
			printf("%d  trying to consume from queue %d \n",getpid(),1-q_no );
			
			

			//down(mutex q!)
			sop.sem_num = (1-q_no)*3;
			sop.sem_op  = -1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
			filewrite(semid,1-q_no,2);
			
			if(msgrcv(msqid[1-q_no], &buf, sizeof (struct msgbuf), 0, 0) ==-1)
			{
				if(errno!=EINTR)
				{
					perror("msgsrcv");
					exit(1);
				}
			}
			//up(mutex q)
			memset(value,0,100);
			sprintf(value,"%d consumed %d from %d\n", getpid(),buf.mtext,1-q_no);
			writeresult(semid,value);
			int i,j;
		// 	printf("%d has wrtten in file\n",column );
		// 	for(j=0;j<2;j++){
		// 	for(i=0;i<10;i++)
		// 		printf("%d ",graph[j][i] );
		// 	printf("\n");
		// }
			sop.sem_num = q_no*3;
			sop.sem_op  = 1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
			//up(empty q)
			sop.sem_num = q_no*3+1;
			sop.sem_op  = 1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
			filewrite(semid,q_no,0);
			printf("%d successfully consumed from %d\n", getpid(),q_no);


			//up(mutex q!)
			
			sop.sem_num = (1-q_no)*3;
			sop.sem_op  = 1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
			//up(empty q!)
			sop.sem_num = (1-q_no)*3+1;
			sop.sem_op  = 1;
			sop.sem_flg = 0;
			semop(semid, &sop, 1);
			filewrite(semid,1-q_no,0);
			printf("%d successfully consumed from %d\n", getpid(),1-q_no);



		}

	}
}
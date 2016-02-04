/*
** spock.c -- reads from a message queue
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct my_msgbuf {
	long mtype;
	char mtext[2000];
	int pid;
};

int main(void)
{
	struct my_msgbuf buf;
	int msqid;
	key_t key;
	int pidarray[100];
	int clients=0;
	int i;
	for(i=0;i<100;i++)
		pidarray[i]=-1;
	if ((key = ftok("server.c", 'H')) == -1) {  
		perror("ftok");
		exit(1);
	}

	if ((msqid = msgget(key, 0644|IPC_CREAT)) == -1) { /* connect to the queue */
		perror("msgget");
		exit(1);
	}
	

	for(;;) { /* Spock never quits! */
	// printf("here\n");
		if (msgrcv(msqid, &buf, sizeof (struct my_msgbuf), 10, 1) == -1) {
			perror("msgrcv");
			exit(1);
		}
		printf("receiving--->%s\n",buf.mtext);
		if(strcmp(buf.mtext,"couple")==0)
		{
			int flag=0;
			for(i=0;i<clients;i++)
			{
				if(pidarray[i]==buf.pid)
					{
						
						flag=1;
					}
			}
			if(flag==0)		
			{	
				pidarray[clients++]=buf.pid;;
				printf("%d\n",buf.pid );
			}	
		}
		else if(strcmp(buf.mtext,"decouple")==0)
		{
			int flag=0;
			for(i=0;i<clients-1;i++)
			{
				if(pidarray[i]==buf.pid)
					{
						pidarray[i]=-1;
						flag=1;
					}
				if(flag)
				{
					pidarray[i]=pidarray[i+1];
				}	
			}
			clients--;

						
			printf("%d\n",buf.pid );
		}
		else 
		{
			printf("%d\n",clients );
			if(strcmp(buf.mtext,"\0"))	
			{
				int pi=buf.pid;
				for(i=0;i<clients;i++)
				{	
					// strcpy(buf.mtext,"lol");
					buf.mtype=pidarray[i];

					// printf("%d == %d\n",pi,pidarray[i] );
					if(pi==pidarray[i])
						continue;
					buf.pid=getpid();
					if (msgsnd(msqid, &buf, sizeof(struct my_msgbuf), 0) == -1) /* +1 for '\0' */
						perror("msgsnd");
				}
			}
		}	
		
	}

	return 0;
}

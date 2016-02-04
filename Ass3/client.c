/*
** kirk.c -- writes to a message queue
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <fcntl.h>

struct my_msgbuf {
	long mtype;
	char mtext[2000];
	int pid;
};

int redirect(int fd,int todup)
{
    int saved_state;
    saved_state = dup(fd);
    // close(fd);
    // int lel=dup(todup);
    dup2(todup,fd);
    return saved_state;
}


void restore(int saved_state,int fd)
{
    dup2(saved_state, fd);
    close(saved_state);    
}
// void execute(char **args, int siz)
// {
//     int status, ll, flag=0, BG=0;
//     pid_t pidc, pid1;
    
//     if(args[siz-1][strlen(args[siz-1])-1] == '&')
//     {
//         args[siz-1][strlen(args[siz-1])-1] = '\0';
//         BG = 1;
//     }
//     if((pidc = fork()) < 0)
//         perror("");
//     else if(pidc == 0)
//     {   
//         // printf("%s\n",args[0] );
//         if(execvp(args[0], args) < 0)
//             printf("Cannot execute");
//         else
//             printf("Successfully Executed\n");
//         exit(1);
//     }
//     else
//     {
//         if(BG==0)
//             pid1 = waitpid(pidc, &status, 0);
//     }
// }
int main(void)
{
	struct my_msgbuf buf;
	int msqid;
	key_t key;
	pid_t pid;
	int dir[2];
	pipe2(dir,O_NONBLOCK);
	pid=fork();
	
	if ((key = ftok("server.c", 'H')) == -1) {
			perror("ftok");
			exit(1);
		}

		if ((msqid = msgget(key, 0644 )) == -1) {
			perror("msgget");
			exit(1);
		}
		
	if(!pid)
	{
		for(;;) { /* Spock never quits! */
			if (msgrcv(msqid, &buf, sizeof (struct my_msgbuf), getppid(), 0) == -1) {
				perror("msgrcv");
				exit(1);
			}
			
			char  hist_loc[100];
			printf("%s\n",buf.mtext );
			getcwd(hist_loc, 100);
			char directory[200];
			memset(directory,0,200);
			int reads=read(dir[0],directory,200);
			if(reads>0)
				fprintf(stderr,"%s $ ",directory );
			else
				fprintf(stderr,"%s $ ",hist_loc );

			
   		 	
			if(strcmp(buf.mtext,"couple")==0)
			{
				//pidarray[clients++]=buf.pid;;
			}


		}
	}	
	else
	{
		close(dir[0]);
		char  hist_loc[100];
   		getcwd(hist_loc, 100);	
   		printf("%s $ ",hist_loc );
		buf.mtype = 10; /* we don't really care in this case */
		buf.pid=getpid();
		while(fgets(buf.mtext, sizeof buf.mtext, stdin) != NULL) 
		{
			int len = strlen(buf.mtext);
			char  hist_loc[100];
			
			

			/* ditch newline at end, if it exists */
			if (buf.mtext[len-1] == '\n') buf.mtext[len-1] = '\0';
			 
			if(strcmp(buf.mtext,"decouple")==0)
				return;
			else if(strcmp(buf.mtext,"couple")==0);

			else
			{
				int pipes[2];
				pipe2(pipes,O_NONBLOCK);
				// pipe2(err,O_NONBLOCK);
				// printf("lol\n");
				// printf("%d\n",STDERR_FILENO );
	            int saved_state_out=redirect(1,pipes[1]);
	            // int error_state=redirect(STDERR_FILENO,err[1]);

				char* tokens[100];
	            tokens[0] = (char*)malloc(1000*sizeof(char));
	            char temp[1000];
	            strcpy(temp,buf.mtext);
	            tokens[0] = strtok(temp, " ");
	            int i=1;
	           
	            while(tokens[i-1] != NULL)
	            {   
	                tokens[i] = (char *)malloc(100*sizeof(char));
	                tokens[i] = strtok(NULL, " ");
	                i++;
	            }
	            if(!strcmp(tokens[0],"history"))
	            {
	            	//history ka code copy paste
	            	fprintf(stderr, "not here");

	            }
	            else if(!strcmp(tokens[0],"cd"))
	            {

	            	if(tokens[1]!=NULL)
	                {
	                    int status=chdir(tokens[1]);
	                    if(status==-1)
	                        perror("cd ");
	                }
	                else
	                    chdir(getenv("HOME"));
	                char tempdir[200];
	                getcwd(tempdir, 200);
	                
          			write(dir[1], tempdir, 200);  /* Write data on pipe */

		        }    
				else
				{

						system(buf.mtext);
				}
				memset(tokens[0],'\0',100);
				close(pipes[1]);
				// close(err[1]);
				// fprintf(stderr, "%s\n", buf.mtext);

				int readit=read(pipes[0],tokens[0],2000);
				int flag=0;
				// fprintf(stderr, "%s\n",tokens[0] );
				if(readit>0)
				{
					if(strcmp(tokens[0],"\0"))
					{
						strcat(buf.mtext,"\n");
						strcat(buf.mtext,tokens[0]);
						flag=1;
						// int errd=read(err[0],tokens[0],2000);

						// if(errd>0)
						// {
						// 	if(strcmp(tokens[0],"\0"))
						// 	{
						// 		strcat(buf.mtext,tokens[0]);
						// 	}
						// }
					}
				}
				close(pipes[0]);
				// close(err[0]);
				restore(saved_state_out,1);
				// printf("%s\n",tokens[0] );
				// restore(error_state,STDERR_FILENO);
				if(flag)
					fprintf(stdout, "%s\n", tokens[0]);

				
			}
			if (msgsnd(msqid, &buf, sizeof(struct my_msgbuf), 0) == -1) /* +1 for '\0' */
				perror("msgsnd");
			
			getcwd(hist_loc, 100);	
   			fprintf(stderr,"%s $ ",hist_loc );
	
		}
		// close(dir[1]);

		if (msgctl(msqid, IPC_RMID, NULL) == -1) {
			perror("msgctl");
			exit(1);

		}
		
	}

	return 0;
}

/*
Garapaty Kalyan Kumar - 13CS10023
Nitesh Sekhar         - 13CS10033
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>           // Necessary for stat used before mkdir
#include <sys/types.h>          // Necessary for mkdir
#include <sys/wait.h>
#include <dirent.h>             // Necessary for directory streams
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>

#define HISTSIZE 100
#define HISTFILESIZE 1000

char *hist_list[HISTSIZE];
char *input;
int cmd_count, rev_sea;
int count=1;
int saved_stdin=0,inflag=0,outflag=0;
int saved_stdout=1;
int writefile,readfile;
int saved_state_pipe_in,saved_state_pipe_out;

int startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre), lenstr = strlen(str);
    if(lenstr < lenpre)
        return 0;
    if(strncmp(pre, str, lenpre)==0)
        return 1;
    return 0;
}

int redirect(int fd,int todup)
{
    int saved_state;
    saved_state = dup(fd);
    dup2(todup,fd);
    return saved_state;
}

void restore(int saved_state,int fd)
{
    dup2(saved_state, fd);
    close(saved_state);    
}

void execute(char **args, int siz)
{
    int status, ll, flag=0, BG=0;
    pid_t pidc, pid1;
    
    if(args[siz-1][strlen(args[siz-1])-1] == '&')
    {
        args[siz-1][strlen(args[siz-1])-1] = '\0';
        BG = 1;
    }
    if((pidc = fork()) < 0)
        perror("");
    else if(pidc == 0)
    {   
        execvp(args[0], args);
        exit(1);
    }
    else
    {
        if(BG==0)
            pid1 = waitpid(pidc, &status, 0);
    }
}
char* check()
{
    char* dest=(char*)malloc(100*sizeof(char));
		int flag=0;
   	if(strstr(input,">")!=NULL)
    {   
				dest=strchr(input  ,'>');
     		if(dest!=NULL)
     		{  
        		int length=strlen(dest);
            strncpy(dest,dest+1,length-1);
            dest[length-1]='\0';
            writefile = open(dest, O_RDWR|O_CREAT,0666);
            length=strlen(input)-strlen(dest);
            strncpy(dest,input,length);
            dest[length]='\0';
            saved_stdout=redirect(1,writefile);
            outflag=1;
            flag=1;
     		}   
		}
    if(strstr(input,"<")!=NULL)
    { 
    		char* sol=(char*)malloc(100*sizeof(char));
        if(flag==0)
        {
        		dest=strchr(input,'<');
          	strcpy(sol,input);
        }
        else
        {
        		strcpy(sol,dest);
            dest=strchr(dest,'<');
      	}
        if(dest!=NULL)
        {  
        		int length=strlen(dest);
            strncpy(dest,dest+1,length-1);
            if(flag==0)
            		dest[length-2]='\0';
            else
            		dest[length-1]='\0';
            readfile = open(dest, O_RDWR|O_CREAT,0666);
            if(flag==0)
            {
            		length=strlen(input)-strlen(dest);
            		strncpy(dest,input,length);
            		dest[length]='\0';
            }
            else
            {
              	length=strlen(sol)-strlen(dest);
                strncpy(dest,sol,length-1);
                dest[length-1]='\0';
            }
            saved_stdin=redirect(0,readfile);
            inflag=1;
        }   
    }
    return dest; 
}

int pipes(char *cmd, char command[100][400]){
    char *pch;
    int n = 0;
    pch = strcpy(command[n], strtok(cmd, "|\n"));
    while(pch != NULL)
    {
        n++;
        pch = strtok(NULL, "|\n");
        if(pch)
            strcpy(command[n], pch);
    }
    return n;
}

char* parser(int i,char* tokens[],int ind)
{
    char* dest=(char*)malloc(100*sizeof(char));
    if(1+ind>=i-1)
        return NULL;
    else if(i!=1)
    { 
        char* quote="\"";
        char* slash="\\";
        strcpy(dest,tokens[1+ind]);
        int len=strlen(tokens[1+ind]);

        if(strchr(quote,dest[0])!=NULL)
        {
            if(strchr(quote,dest[len-1])!=NULL)
            {
                strncpy(dest,dest+1,len-2);
                dest[len-2]='\0';
            }
            else
            {
                int index=2;
                for(index=2;index+ind<i-1;index++)
                {
                    if(strchr(quote,dest[len-1])!=NULL)
                        break;
                    char* space=" ";
                    strcat(dest,space);
                    strcat(dest,tokens[index+ind]);
                    len=strlen(dest);
                    count++;
                }
                len=strlen(dest);
                strncpy(dest,dest+1,len-2);
                dest[len-2]='\0';
            }
        }
        else if(strchr(slash,dest[len-1])!=NULL)
        {
            int index=2;
            for(index=2;index+ind<i-1;index++)
            {
                if(strchr(slash,dest[len-1])==NULL)
                    break;
                char* space=" ";
                dest[len-1]='\0';
                strcat(dest,space);
                strcat(dest,tokens[index+ind]);
                len=strlen(dest);
                count++;
            }
        }
    }
    return dest;   
}

void reverse_search(int sig)
{
    rev_sea = 1;
}

int main(int argc, char **argv, char **envp)
{
    rev_sea = 0;
    signal(SIGQUIT, reverse_search);
    cmd_count = 0;
    char cwd[1024], hist_loc[100];
    getcwd(hist_loc, 100);
    strcat(hist_loc, "/.nutshell_history");
    int trig, third_it1, third_it2;
    for(trig=0;trig<HISTSIZE;trig++)
        hist_list[trig] = (char *)malloc(100*sizeof(char));
    FILE *histfile = fopen(hist_loc, "r");  
    trig = 0;
    if(histfile)
    {
        while(fgets(hist_list[trig], 100, histfile)!=NULL)
        {
            hist_list[trig][strlen(hist_list[trig])-1] = '\0';
            trig++;
        }
        cmd_count = trig;
        fclose(histfile);
    }
    char command[100], write[100][400];
    while(1)
    {
        int count=0;
        int j=0;
        if(inflag)
        {
            restore(saved_stdin,0);
            close(readfile);
        }
        int i=1, len_sz, len_sz1;
        getcwd(cwd,sizeof(cwd));
        fprintf(stdout, "%s> ", cwd);
        fgets (command, 100, stdin);
        
        if(command[0]=='\n')
            continue;
        len_sz1 = strlen(command);
      	if(command[len_sz1-1]=='\n')
        		command[len_sz1-1] = '\0';
        if(rev_sea ==1)
        {
            for(trig=cmd_count-1;trig>=0;trig--)
            {
                if(startsWith(command, hist_list[trig]))
                {
                    strcpy(command, hist_list[trig]);
                  	fprintf(stdout, "%s\n", command);
                  	len_sz1 = strlen(command);
                    break;
                }
            }
            rev_sea = 0;
        }
       	int m=pipes(command,write);
       	int fd[m-1][2];
      	int x;
        for(third_it1=0;third_it1<m;third_it1++)
        {
            if(m-1!=0)
            {
                pipe(fd[j]);

                if(j==0){
                    saved_state_pipe_out = redirect(1,fd[j][1]);
                }
                if(j>0 && j<m-1){
                    x = redirect(fd[j-1][1],fd[j][1]);
                    close(x);
                }
                if(j==1){
                    saved_state_pipe_in = redirect(0,fd[j-1][0]); 
                }
                if(j > 1){
                    x = redirect(fd[j-2][0],fd[j-1][0]);
                    close(x);
                }
                if(j == m-1){
                    restore(saved_state_pipe_out,1);
                    close(fd[j-1][1]);
                }
            }	
            j++;
            if(third_it1==len_sz1)
                third_it1++;
            len_sz = strlen(write[third_it1]);
            input=(char*)malloc(100*sizeof(char));
            strcpy(input,write[third_it1]);
            if(write[third_it1][len_sz-1]=='\n')
                write[third_it1][--len_sz] = '\0';
            if(strchr(write[third_it1],'>')!=NULL || strchr(write[third_it1],'<')!=NULL)
                strcpy(write[third_it1],check());
            else
            {
                inflag=0;
                outflag=0;
            }
            hist_list[cmd_count%HISTSIZE] = (char *)malloc(len_sz*(sizeof(char)));
            strcpy(hist_list[cmd_count%HISTSIZE], write[third_it1]);
            cmd_count++;

            char* tokens[100];
            tokens[0] = (char*)malloc(100*sizeof(char));
            tokens[0] = strtok(write[third_it1], " ");
            while(tokens[i-1] != NULL)
            {   
                tokens[i] = (char *)malloc(100*sizeof(char));
                tokens[i] = strtok(NULL, " ");
                i++;
            }

            if(!strcmp("clear",tokens[0]))
            {
                printf("\e[2J\e[H");
            }
            else if(!strcmp("env", tokens[0]))
            {
                char **env;
                for(env=envp;*env!=0;env++)
                    fprintf(stdout, "%s\n", *env);
            }
            else if(!strcmp("cd",tokens[0]))
            {
                char* dest=parser(i,tokens,0);
                if(dest!=NULL)
                {
                    int status=chdir(dest);
                    if(status==-1)
                        perror("cd ");
                }
                else
                    chdir(getenv("HOME"));
            }
            else if(!strcmp("pwd",tokens[0]))
            {
                getcwd(cwd,sizeof(cwd));
                fprintf(stdout, "%s\n", cwd);
            }
            else if(!strcmp("mkdir", tokens[0]))
            {

                struct stat st = {0};
                int index=0;
                for(;index<i-1;)
                {
                    count=1;
                    char* dest=parser(i,tokens,index);
                    index=index+count;
                    if(dest)
                    {
                        if(mkdir(dest, 0700))
                            perror("Error ");
                    }
                }
            }
            else if(!strcmp("rmdir", tokens[0]))
            {
                int index=0;
                for(;index<i-1;)
                {
                    count=1;
                    char* dest=parser(i,tokens,index);
                    index=index+count;
                    if(dest)
                    {
                        if(rmdir(dest))
                            perror("Error ");
                    }
                }
            }
            else if(!strcmp("ls", tokens[0]))
            {
                DIR *d;
                struct dirent *dp;
                struct stat     statbuf;
                struct passwd  *pwd;
                struct group   *grp;
                struct tm      *tm;
                char            datestring[256];

                if(i==2)
                {
                    d = opendir(cwd);
                    if(d)
                    {
                        while((dp=readdir(d))!=NULL)
                        {
                            if(strcmp(dp->d_name,".")!=0 && strcmp(dp->d_name,"..")!=0)
                                fprintf(stdout, "%s\n", dp->d_name);
                        }
                    }
                    else
                        perror("");
                    closedir(d);
                }
                else
                {
                    d = opendir(cwd);
                    if(d)
                    {
                        while((dp=readdir(d))!=NULL)
                        {
                            /* Get entry's information. */
                            if (stat(dp->d_name, &statbuf) == -1)
                                continue;

                            /* Print out type, permissions, and number of links. */
                            printf( (S_ISDIR(statbuf.st_mode)) ? "d" : "-");
                            printf( (statbuf.st_mode & S_IRUSR) ? "r" : "-");
                            printf( (statbuf.st_mode & S_IWUSR) ? "w" : "-");
                            printf( (statbuf.st_mode & S_IXUSR) ? "x" : "-");
                            printf( (statbuf.st_mode & S_IRGRP) ? "r" : "-");
                            printf( (statbuf.st_mode & S_IWGRP) ? "w" : "-");
                            printf( (statbuf.st_mode & S_IXGRP) ? "x" : "-");
                            printf( (statbuf.st_mode & S_IROTH) ? "r" : "-");
                            printf( (statbuf.st_mode & S_IWOTH) ? "w" : "-");
                            printf( (statbuf.st_mode & S_IXOTH) ? "x" : "-");
                            printf("%4d", statbuf.st_nlink);

                            /* Print out owner's name if it is found using getpwuid(). */
                            if ((pwd = getpwuid(statbuf.st_uid)) != NULL)
                                printf(" %-8.8s", pwd->pw_name);
                            else
                                printf(" %-8d", statbuf.st_uid);

                            /* Print out group name if it is found using getgrgid(). */
                            if ((grp = getgrgid(statbuf.st_gid)) != NULL)
                                printf(" %-8.8s", grp->gr_name);
                            else
                                printf(" %-8d", statbuf.st_gid);

                            /* Print size of file. */
                            printf(" %9jd", (intmax_t)statbuf.st_size);

                            /* Get localized date string. */
                            tm = localtime(&statbuf.st_mtime);
                            strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);
                            printf(" %s %s\n", datestring, dp->d_name);
                        }
                    }
                    else
                        perror("");
                    closedir(d);
                }
            }
            else if(!strcmp("history", tokens[0]))
            {
                int tere;
                if(i!=2)
                {
                    char *pEnd;
                    int x = strtol(tokens[1], &pEnd, 10);
                    if(x>cmd_count || x > HISTSIZE)
                    {
                        if(cmd_count > HISTSIZE)
                            tere = cmd_count%HISTSIZE;
                        else
                            tere = 0;
                    }
                    else
                        tere = cmd_count-x;
                }
                else
                {
                    if(cmd_count > HISTSIZE)
                        tere = cmd_count%HISTSIZE;
                    else
                        tere = 0;
                }
                for(;tere<cmd_count;tere++)
                    fprintf(stdout, "%d %s\n", tere+1,hist_list[tere%HISTSIZE]);
            }
            else if(!strcmp("exit", tokens[0]))
            {
                FILE *histfile = fopen(hist_loc, "w");
                int tree;
                if(cmd_count > HISTSIZE)
                    tree = cmd_count%HISTSIZE;
                else
                    tree = 0;
                for(;tree<cmd_count;tree++)
                    fprintf(histfile, strcat(hist_list[tree%HISTSIZE],"\n"));
                fclose(histfile);
                return 0;
            }
            else
                execute(tokens, i-1);
            if(outflag)
            {
                restore(saved_stdout,1);
                close(writefile);
            }
        }
        if(j>1)
        {
        		restore(saved_state_pipe_in,0);
        		close(fd[j-2][0]);
        }
    }
}

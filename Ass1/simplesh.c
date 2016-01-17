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

#define HISTSIZE 100
#define HISTFILESIZE 1000

char *hist_list[HISTSIZE];
int cmd_count;
int count=1;

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
        if(execvp(args[0], args) < 0)
            printf("Cannot execute");
        else
            printf("Successfully Executed\n");
        exit(1);
    }
    else
    {
        if(BG==1)
        {

        }
        else
            pid1 = waitpid(pidc, &status, 0);
        kill(pid1, SIGKILL);
    }
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
                    strncpy(dest,dest+1,len-2);
                    dest[len-2]='\0';
                    count++;
                }
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
                len=strlen(dest);
                dest[len-1]='\0';
                strcat(dest,space);
                strcat(dest,tokens[index+ind]);
                count++;
            }
        }
                
    }
    return dest;   
}

int main(int argc, char **argv, char **envp)
{
    cmd_count = 0;
    char cwd[1024];
    char write[100];
    while(1)
    { 
        int i=1, len_sz;
        getcwd(cwd,sizeof(cwd));
        fprintf(stdout, "%s> ", cwd);
        
        fgets (write, 100, stdin);
        len_sz = strlen(write);
        if(write[len_sz-1]=='\n')
            write[--len_sz] = '\0';
        hist_list[cmd_count%HISTSIZE] = (char *)malloc(len_sz*(sizeof(char)));
        strcpy(hist_list[cmd_count%HISTSIZE], write);
        cmd_count++;
        
        char* tokens[100];
        tokens[0] = (char*)malloc(100*sizeof(char));
        tokens[0] = strtok(write, " ");

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
                    printf("%s\n", dest);  
                    if(mkdir(dest, 0700))
                        perror("Error");
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
                    printf("%s\n", dest);
                    if(rmdir(dest))
                        perror("Error");
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
                        fprintf(stdout, "%s\n", dp->d_name);
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
                fprintf(stdout, "%s\n", hist_list[tere%HISTSIZE]);
        }
        else if(!strcmp("exit", tokens[0]))
        {
            return 0;
        }
        else
        {
            execute(tokens, i-1);
        }
    }
}
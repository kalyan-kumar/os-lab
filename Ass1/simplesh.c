#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>           // Necessary for stat used before mkdir
#include <sys/types.h>          // Necessary for mkdir
#include <unistd>               // Necessary for rmdir


int getcw(char cwd[])
{
   if (getcwd(cwd, sizeof(cwd)) != 0)
      return 1;
   else
       perror("getcwd() error");
   return 0;
}

int main(int argc, char **argv, char **envp)
{
    char cwd[1024];
    char write[100];
    while(1)
    { 
        int i=1;
        getcwd(cwd,sizeof(cwd));
        fprintf(stdout, "%s >", cwd);
        fgets (write, 100, stdin);
        char* tokens[100];
        tokens[0]=(char*)malloc(100*sizeof(char));
        char *q=strtok(write,"\n");
        char *p =tokens[0]= strtok(q, " ");

        while(p != NULL)
        {
            tokens[i]=(char*)malloc(100*sizeof(char));
            p =tokens[i]=strtok(NULL, " ");
            i++;
        }
      
        if(!strcmp("cd",tokens[0]))
        {
            if(i!=1)
            { 
                if(!strcmp(tokens[1][0],"\""))
                {
                    int len=strlen(tokens[1]);
                    if(!strcmp(tokens[1][len-1],"\"")){}
                }
                int status=chdir(tokens[1]);
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
        else if(!strcmp("clear",tokens[0]))
        {
            printf("\e[2J\e[H");
        }
        else if(!strcmp("env", tokens[0]))
        {
            char **env;
            for(env=envp;*env!=0;env++)
                printf("%s\n", *env);
        }
        else if(!strcmp("mkdir", tokens[0]))
        {
            struct stat st = {0};
            if(stat(tokens[1], &st) == -1)
                mkdir(tokens[1], 0700);
            else
                printf("mkdir: cannot create directory ‘%s’: File exists\n", tokens[1]);
        }
        else if(!strcmp("rmdir", tokens[0]))
        {

        }
    }
}
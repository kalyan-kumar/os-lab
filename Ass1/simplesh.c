#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>           // Necessary for stat used before mkdir
#include <sys/types.h>          // Necessary for mkdir
#include <dirent.h>             // Necessary for directory streams
#include <unistd.h>
#include <time.h>

#define HISTSIZE 100
#define HISTFILESIZE 1000

char *hist_list[HISTSIZE];
int cmd_count;

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
            if(i!=1)
            { 
                char* dest=(char*)malloc(100*sizeof(char));
                char* quote="\"";
                char* slash="\\";
                strcpy(dest,tokens[1]);
                int len=strlen(tokens[1]);

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
                        for(index=2;index<i;index++)
                        {
                            char* space=" ";
                            strcat(dest,space);
                            strcat(dest,tokens[index]);
                            len=strlen(dest);
                            strncpy(dest,dest+1,len-2);
                            dest[len-2]='\0';
                            printf("%s\n",dest   );
                        }
                    }
                }
                else if(strchr(slash,dest[len-1])!=NULL)
                {
                    int index=2;
                    for(index=2;index<i;index++)
                    {
                        if(strchr(slash,dest[len-1])==NULL)
                            break;
                        char* space=" ";
                        strcat(dest,space);
                        strcat(dest,tokens[index]);
                        printf("%s\n",dest   );
                    }
                }
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
            if(mkdir(tokens[1], 0700))
                perror("Error");
            /*if(stat(tokens[1], &st) == -1)
                mkdir(tokens[1], 0700);
            else
                perror("mkdir: cannot create directory ‘%s’: File exists\n");*/
        }
        else if(!strcmp("rmdir", tokens[0]))
        {

        }
        else if(!strcmp("ls", tokens[0]))
        {
            DIR *d;
            struct dirent *dir;
            if(i==2)
            {
                d = opendir(cwd);
                if(d)
                {
                    while((dir=readdir(d))!=NULL)
                        fprintf(stdout, "%s\n", dir->d_name);
                    closedir(d);
                }
            }
            else
            {
                if(tokens[1][0] == '-')
                {
                    struct stat sb;
                    if(stat(cwd, &sb) == -1)
                        perror("stat");
                    else
                    {
                        printf("File type:                ");

                        switch (sb.st_mode & S_IFMT)
                        {
                            case S_IFBLK:  printf("block device\n");            break;
                            case S_IFCHR:  printf("character device\n");        break;
                            case S_IFDIR:  printf("directory\n");               break;
                            case S_IFIFO:  printf("FIFO/pipe\n");               break;
                            case S_IFLNK:  printf("symlink\n");                 break;
                            case S_IFREG:  printf("regular file\n");            break;
                            case S_IFSOCK: printf("socket\n");                  break;
                            default:       printf("unknown?\n");                break;
                        }

                        printf("I-node number:            %ld\n", (long) sb.st_ino);

                        printf("Mode:                     %lo (octal)\n", (unsigned long)sb.st_mode);

                        printf("Link count:               %ld\n", (long) sb.st_nlink);
                        printf("Ownership:                UID=%ld   GID=%ld\n", (long)sb.st_uid, (long)sb.st_gid);

                        printf("Preferred I/O block size: %ld bytes\n", (long)sb.st_blksize);
                        printf("File size:                %lld bytes\n", (long long)sb.st_size);
                        printf("Blocks allocated:         %lld\n", (long long)sb.st_blocks);

                        printf("Last status change:       %s", ctime(&sb.st_ctime));
                        printf("Last file access:         %s", ctime(&sb.st_atime));
                        printf("Last file modification:   %s", ctime(&sb.st_mtime));
                    }
                }
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

        }
    }
}
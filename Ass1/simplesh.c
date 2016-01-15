#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int getcw(char cwd[])
{
   if (getcwd(cwd, sizeof(cwd)) != 0)
   		return 1;
   else
       perror("getcwd() error");
   return 0;
}
int main()
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
   				if(strcmp(tokens[1][0],"\"")
   				int status=chdir(tokens[1]);
   				if(status==-1)
   					perror("cd ");

   			}
   			else
   				chdir(getenv("HOME"));
   		}
   		if(!strcmp("pwd",tokens[0]))
   		{
   			getcwd(cwd,sizeof(cwd));
   			fprintf(stdout, "%s\n", cwd);
   		}
    	if(!strcmp("clear",tokens[0]))
    	{
    		printf("\e[2J\e[H");



















    	}
  		
				
   		 


	// }
   		}


   

}
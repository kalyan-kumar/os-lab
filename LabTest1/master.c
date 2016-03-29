#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[])
{
	int n, m, i, j, k, l, u, v, status, **E, **pipes, *C;
	pid_t *pids;
	char **args;
	scanf("%d %d", &n, &m);
	E = (int **)malloc(n*sizeof(int *));
	C = (int *)malloc(n*sizeof(int));
	pids = (pid_t *)malloc(n*sizeof(pid_t));
	pipes = (int **)malloc(2*m*sizeof(int *));
	for(i=0;i<n;i++)
	{
		E[i] = (int *)malloc(n*sizeof(int));
		C[i] = 0;
		for(j=0;j<n;j++)
			E[i][j] = 0;
	}
	for(i=0,k=0;i<m;i++,k++)
	{
		pipes[i] = (int *)malloc(2*sizeof(int));
		scanf("%d %d", &u, &v);
		E[u-1][v-1] = k;
		E[v-1][u-1] = k;
		if(u!=v)
		{
			C[u-1]++;
			C[v-1]++;
		}
	}
	for(i=0,k=0;i<n;i++)
	{
		pids[i] = fork();
		if(pids[i] < 0)
		{
			perror("fork");
			exit(1);
		}
		else if(pids[i]==0)
		{
			args = (char **)malloc(((2*C[i])+3)*sizeof(char *));
			strcpy(args[0], "./node");
			sprintf(args[1], "%d", (1+(rand()%100)));
			for(j=0,l=2;j<i;j++)
			{
				if(E[i][j]!=0)
				{
					sprintf(args[l++], "%d", pipes[2*E[j][i]][0]);
					sprintf(args[l++], "%d", pipes[(2*E[j][i])+1][1]);
				}
			}
			for(j=i+1;j<n;j++)
			{
				if(E[i][j]!=0)
				{
					pipe(pipes[2*E[i][j]]);
					pipe(pipes[(2*E[j][i])+1]);
					sprintf(args[l++], "%d", pipes[(2*E[j][i])+1][0]);
					sprintf(args[l++], "%d", pipes[2*E[i][j]][1]);
				}
			}
			sprintf(args[l], "%d", n);
			execv("./node", args);
			exit(1);
		}
	}
	for(i=0;i<n;i++)
	{
		if(waitpid(pids[i], &status, 0) == -1)
		{
			perror("waitpid error");
			exit(1);
		}
		if(WIFEXITED(status))
		{
			const int es = WEXITSTATUS(status);
			printf("%d calculated %d\n", pids[i], es);
		}
	}
	return 0;
}
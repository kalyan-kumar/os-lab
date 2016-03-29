#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct param
{
	int k,i;	
}param;

int graph[100][100];
int n;
pthread_mutex_t mutex;

void *iterate (void *t)
{
	pthread_mutex_lock(&mutex);
	struct param *parameters=t;
	int k=parameters->k;
	int i=parameters->i;
	int j=0;
	for(j=0;j<n;j++)
	{
		if(graph[i][k]+graph[k][j]<graph[i][j])
		{
			graph[i][j]=graph[i][k]+graph[k][j];
		}
	}
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
}

void floyd()
{
	int i,j,k;
	pthread_t threads[n];
	pthread_attr_t attr;
	pthread_mutex_init(&mutex,NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	for(k=0;k<n;k++)
	{
		for(i=0;i<n;i++)
		{
			param *parameters=malloc(sizeof(param));
			parameters->i=i;
			parameters->k=k;
			pthread_create(&threads[i],&attr,iterate,(void *)parameters);
		}

		for(i=0;i<n;i++)
		{
			pthread_join(threads[i],NULL);
		}
	}
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex);
}

int main()
{
	int m, i, j, k, w;
	printf("enter n and m\n");
	scanf("%d %d",&n,&m);
	printf("Enter m edges in <v1> <v2> <weight> format\n");
	for(k=0;k<m;k++)
	{
		scanf("%d %d %d",&i,&j,&w);
		graph[i-1][j-1]=graph[j-1][i-1]=w;
	}
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			if(graph[i][j]==0)
				graph[i][j]=graph[j][i]=1000000000;
			if(i==j)
				graph[i][j]=0;
		}
	}

	floyd();
	for(i=0;i<n;i++)	
	{
		for(j=0;j<n;j++)
			printf("%d\t",graph[i][j]);
		printf("\n");
	}

}
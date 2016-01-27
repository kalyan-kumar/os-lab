#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int primes[100000], cur_gen=0, n, k, **pipes, flag[30001];
pid_t *pid, ppid;

int isPrime(int n)
{
    int i, r, flag=1;
    r = sqrt(n);
    for(i=2;i<=r;i+=2)
    {
        if(n%i==0)
        {
            flag = 0;
            break;
        }
        if(i==2)
            i = 1;
    }
    return flag;
}

int readFromPipe(int file)
{
    int tmp;
    read(file, &tmp, sizeof(tmp));
    return tmp;
}

void writeToPipe(int file, int x)
{
    write(file, &x, sizeof(x));
}

void sendTests(int x)
{
    int r, it;
    printf("Sending ");
    for(it=0;it<k;it++)
    {
        do
        {
            r = 1 + (rand()%30000);
        }while(flag[r]==1);
        flag[r] = 1;
        printf("%d ", r);
        writeToPipe(pipes[2*x][1], r);
    }
    printf("\n\n");
    writeToPipe(pipes[2*x][1], -1);
}

void availHandler(int sig, siginfo_t *siginfo, void *context)
{
    pid_t cur_pid = siginfo->si_pid;
    int x, fnd, it;
    for(x=0;x<k;x++)
    {
        if(pid[x]==cur_pid)
            break;
    }
    printf("Child %d is available.\n", x);
    fnd = readFromPipe(pipes[(2*x)+1][0]);
    while(fnd!=-1)
    {
        for(it=0;it<cur_gen;it++)
        {
            if(primes[it]==fnd)
                break;
        }
        if(it==cur_gen)
            primes[cur_gen++] = fnd;
        fnd = readFromPipe(pipes[(2*x)+1][0]);
    }
    if(cur_gen>=n)
    {
        for(x=0;x<k;x++)
            kill(pid[x], SIGKILL);
        printf("Generated primes are -\n");
        for(x=0;x<n;x++)
            printf("%d ", primes[x]);
        printf("\nExiting. Visit again. :)\n");
        exit(0);
    }
    else
    {
        printf("Generating numbers to be send to child number %d ...\n", x);
        sendTests(x);
    }
}

void busyHandler(int sig, siginfo_t *siginfo, void *context)
{
    pid_t cur_pid = siginfo->si_pid;
    int x;
    for(x=0;x<k;x++)
    {
        if(pid[x]==cur_pid)
            break;
    }

}

int main(int argc, char **argv)
{
    srand((time(NULL)));
    struct sigaction act1, act2;
    act1.sa_sigaction = &availHandler;
    act2.sa_sigaction = &busyHandler;
    act1.sa_flags = SA_SIGINFO;
    act2.sa_flags = SA_SIGINFO;
	int i, j,  pipes_num, test;
    ppid = getpid();
    for(i=0;i<30001;i++)
        flag[i] = 0;
    printf("Enter two integers n and k\n");
    scanf("%d %d",&n,&k);
    
    pipes_num = 2*k;
    pipes = (int **)malloc(pipes_num*sizeof(int *));
    for(i=0;i<pipes_num;i++)
        pipes[i] = (int *)malloc(2*sizeof(int));
    pid = (pid_t *)malloc(k*sizeof(pid_t));

    for(i=0;i<k;i++)
    {
        pipe(pipes[2*i]);
        pipe(pipes[(2*i)+1]);
        pid[i]= fork();
        if(pid[i] < 0)
        {
            fprintf(stderr, "Fork failed");
        	return 0;
        }
        else if(pid[i] > 0)   // Parent Process
        {
            close(pipes[2*i][0]);
            close(pipes[(2*i)+1][1]);            
    	}
        else  // Child Process
        {
            close(pipes[2*i][1]);
            close(pipes[(2*i)+1][0]);
            sleep(2);
            break;
        }
    }

    if(i==k)
    {
        while(1)
        {
            sigaction(SIGUSR1, &act1, NULL);
            sigaction(SIGUSR2, &act2, NULL);
        }
    }
    while(1)
    {
        writeToPipe(pipes[(2*i)+1][1], -1);
        kill(ppid, SIGUSR1);
        test = readFromPipe(pipes[2*i][0]);
        while(test!=-1)
        {
            if(isPrime(test))
                writeToPipe(pipes[(2*i)+1][1], test);
            test = readFromPipe(pipes[2*i][0]);
        }
        sleep(2);
        //kill(ppid, SIGUSR2);
    }
}
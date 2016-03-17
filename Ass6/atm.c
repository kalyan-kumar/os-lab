#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

int main(int argc, char *argv[])
{
	int ind = atoi(argv[1]);
	key_t k0, k1, k2;
	if((k0 = ftok("master.c", 3*ind))==-1)
	{
		perror("ftok");
		exit(1);
	}
	if((k1 = ftok("master.c", (3*ind)+1))==-1)
	{
		perror("ftok");
		exit(1);
	}
	if((k2 = ftok("master.c", (3*ind)+2))==-1)
	{
		perror("ftok");
		exit(1);
	}
	
	if((msgqs[i] = msgget(key, 0644 | IPC_CREAT)) == -1)
	{
        perror("msgget");
        exit(1);
    }
}
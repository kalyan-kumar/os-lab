#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct my_msgbuf {
    long mtype;
    int cli_pid;
};

int main(int argc, char *argv[])
{
	key_t key;
	if((key = ftok("master.c", x)) == -1) {
        perror("ftok");
        exit(1);
    }
    if ((msqid = msgget(key, 0644)) == -1) {
        perror("msgget");
        exit(1);
    }
	return 0;
}
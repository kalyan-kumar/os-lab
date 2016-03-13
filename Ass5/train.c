#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define NORTH 0
#define WEST 1
#define SOUTH 2
#define EAST 3
#define TRACK 4
#define MATRIX 5

int total, **A;

void readMatrix()
{
    FILE *fp = fopen("matrix.txt", "r");
    char buf[10000];
    fgets(buf, 10000, fp);
    fgets(buf, 10000, fp);
    int i, j, k;
    for(i=0;i<total;i++)
    {
        fgets(buf, 10000, fp);
        for(k=0,j=0;k<strlen(buf);k++)
        {
            if(buf[k]=='0')
            {
                A[i][j] = 0;
                j++;
            }
            else if(buf[k]=='1')
            {
                A[i][j] = 1;
                j++;
            }
            else if(buf[k]=='2')
            {
                A[i][j] = 2;
                j++;
            }
        }
        fgets(buf, 10000, fp);
    }
    fclose(fp);
}

void makeMatrix()
{
    int i;
    FILE *fp = fopen("matrix.txt", "w");
    fprintf(fp, "\t\t\t\t\tSemaphores\n\t\tNorth\tWest\tSouth\tEast\n");
    for(i=0;i<total;i++)
        fprintf(fp, "\t\t%d\t%d\t%d\t%d\n\n", A[i][0], A[i][1], A[i][2], A[i][3]);
    fclose(fp);
}

void writeToMatrix(int semid, int ind, int whichone, int state)
{
    if(whichone>3)
        return ;
    struct sembuf sb;
    sb.sem_flg = SEM_UNDO;
    sb.sem_num = MATRIX;
    sb.sem_op = -1;
    if(semop(semid, &sb, 1) == -1)
    {
        perror("semop");
        exit(1);
    }
    readMatrix();
    A[ind][whichone] = state;
    makeMatrix();
    sb.sem_op = 1;
    if(semop(semid, &sb, 1) == -1)
    {
        perror("semop");
        exit(1);
    }
}

void useRes(int semid, int whichone, int how, int ind)
{
    switch(whichone)
    {
        case NORTH:
            printf("Train <%d>: requests North-lock\n", getpid());
            break;
        case WEST:
            printf("Train <%d>: requests West-lock\n", getpid());
            break;
        case SOUTH:
            printf("Train <%d>: requests South-lock\n", getpid());
            break;
        case EAST:
            printf("Train <%d>: requests East-lock\n", getpid());
            break;
        case TRACK:
            printf("Train <%d>: requests Junction-lock\n", getpid());
            break;
    }
    struct sembuf sb;
    sb.sem_flg = SEM_UNDO;
    sb.sem_num = whichone;
    sb.sem_op = how;
    if(how==-1)
        writeToMatrix(semid, ind, whichone, 1);
    if(semop(semid, &sb, 1) == -1)
    {
        perror("semop");
        exit(1);
    }
    if(how==-1)
        writeToMatrix(semid, ind, whichone, 2);
    else
        writeToMatrix(semid, ind, whichone, 0);
    switch(whichone)
    {
        case NORTH:
            printf("Train <%d>: acquires North-lock\n", getpid());
            break;
        case WEST:
            printf("Train <%d>: acquires West-lock\n", getpid());
            break;
        case SOUTH:
            printf("Train <%d>: acquires South-lock\n", getpid());
            break;
        case EAST:
            printf("Train <%d>: acquires East-lock\n", getpid());
            break;
        case TRACK:
            printf("Train <%d>: acquires Junction-lock;", getpid());
            break;
    }
}

int main(int argc, char *argv[])
{
    if(argc < 4)
    {
        perror("Run executable as <executable-path> <probability>\n");
        exit(1);
    }
    printf("Train %d created\n", getpid());
    int semid;
    key_t key;
    if ((key = ftok("manager.c", 'J')) == -1)
    {
        perror("ftok");
        exit(1);
    }
    semid = semget(key, 6, 0);
    if (semid < 0)
    {
        perror("Manager didnt create the semaphores yet\n");
        exit(1);
    }

    int mine, right, ind = atoi(argv[2]);
    total = atoi(argv[3]);
    A = (int **)malloc(total*sizeof(int *));
    for(mine=0;mine<total;mine++)
        A[mine] = (int *)malloc(4*sizeof(int));

    if(!strcmp(argv[1], "N"))
    {
        mine = NORTH;
        right = WEST;
    }
    else if(!strcmp(argv[1], "W"))
    {
        mine = WEST;
        right = SOUTH;
    }
    else if(!strcmp(argv[1], "S"))
    {
        mine = SOUTH;
        right = EAST;
    }
    else if(!strcmp(argv[1], "E"))
    {
        mine = EAST;
        right = NORTH;
    }
    useRes(semid, mine, -1, ind);
    useRes(semid, right, -1, ind);
    useRes(semid, TRACK, -1, ind);
    printf("Passing Junction;\n");
    sleep(2);
    useRes(semid, TRACK, 1, ind);
    useRes(semid, right, 1, ind);
    useRes(semid, mine, 1, ind);
    printf("Bye Bye\n");
    return 0;
}
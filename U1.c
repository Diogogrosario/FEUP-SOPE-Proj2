#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <stdbool.h>

#define MAX_DESCRIPTORS 800

int numberOfFreeFileDescriptors = MAX_DESCRIPTORS;

struct timespec start;

typedef struct
{
    int nsecs;
    char *fifoname;
} flagsList;

flagsList flags;

typedef struct
{
    int i;
    pid_t pid;
    pthread_t tid;
    int dur;
    int pl;
} message;

int fd;

bool closed = false;

bool hasFifoName = false;

pthread_mutex_t printUMut = PTHREAD_MUTEX_INITIALIZER;

void printFlags(flagsList *flags)
{
    printf("nsecs: %d\n", flags->nsecs);
    printf("fifoname: %s\n", flags->fifoname);
}

void initFlags(flagsList *flags)
{
    flags->nsecs = 0;
}

void setFlags(int argc, char *argv[], flagsList *flags)
{
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-t"))
        {
            i++;
            for (int j = 0; j < strlen(argv[i]); ++j)
            {
                if (argv[i][j] > '9' || argv[i][j] < '0')
                {
                    printf("Invalid time!\n");
                    exit(1);
                }
            }
            flags->nsecs = atoi(argv[i]);
        }
        else
        {
            flags->fifoname = argv[i];
            hasFifoName = true;
        }
    }
}

message makeMessage(int i, int pl)
{
    message msg;
    msg.i = i;
    msg.pid = getpid();
    msg.tid = pthread_self();
    msg.dur = (rand() % 50) + 50;
    msg.pl = pl;
    return msg;
}

void printMessage(message *msg, char *op)
{
    pthread_mutex_lock(&printUMut);
    printf("%ld ; ", time(NULL));
    printf("%d ; ", msg->i);
    printf("%d ; ", msg->pid);
    printf("%ld ; ", msg->tid);
    printf("%d ; ", msg->dur);
    printf("%d ; ", msg->pl);
    printf("%s\n", op);
    pthread_mutex_unlock(&printUMut);
}

void *sendRequest(void *thread_no)
{
   
    pthread_detach(pthread_self());
    message msg = makeMessage(*(int *)thread_no, -1);

    char *publicFIFO = malloc(sizeof(char) * 100);
    sprintf(publicFIFO, "/tmp/%s", flags.fifoname);

    char *semName = malloc(sizeof(char) * 255);
    sprintf(semName, "sem.%d.%ld", msg.pid, msg.tid);
    sem_t *sem_id = sem_open(semName, O_CREAT, 0600, 0);

    if (sem_id == NULL)
    {
        char error[255]="";
        sprintf(error,"Error on sem_open for %s\n", semName);
        write(2, error,strlen(error));
        return NULL;
    }
    numberOfFreeFileDescriptors--;


    if ((fd = open(publicFIFO, O_WRONLY)) == -1)
    {
        numberOfFreeFileDescriptors++;
        sem_close(sem_id);
        printMessage(&msg, "CLOSD");
        closed = true;
        return NULL;
    }
    numberOfFreeFileDescriptors--;


    if (write(fd, &msg, sizeof(msg)) == -1)
    {
        numberOfFreeFileDescriptors++;
        sem_close(sem_id);
        perror("Error on writing client request to public FIFO\n");
        return NULL;
    }
    close(fd);
    numberOfFreeFileDescriptors++;



    printMessage(&msg, "IWANT");

    char *aux = malloc(sizeof(char) * 255);
    sprintf(aux, "/tmp/%d.%ld", msg.pid, msg.tid);

    if (mkfifo(aux, 0666) == -1)
    {
        numberOfFreeFileDescriptors++;
        sem_close(sem_id);
        perror("Error on making private FIFO\n");
        return NULL;
    }
    numberOfFreeFileDescriptors--;
    //UNLOCK SERVER CUZ FIFO IS CREATED

    int fdRequest;

    if (sem_post(sem_id) < 0)
    {
        fprintf(stderr, "Error on sem_post for %s\n", semName);
        sem_close(sem_id);

        return NULL;
    }
    sem_close(sem_id);
    numberOfFreeFileDescriptors++;

    if ((fdRequest = open(aux, O_RDONLY)) == -1)
    {
        perror(aux);
        return NULL;
    }

    message *toReceive = malloc(sizeof(message));

    if (read(fdRequest, toReceive, sizeof(message)) <= 0)
    {
        printMessage(&msg, "FAILD");
        numberOfFreeFileDescriptors++;
        close(fdRequest);
        return NULL;
    }

    if (toReceive->pl == -1)
    {
        printMessage(toReceive, "CLOSD");
        closed = true;
    }
    else
    {
        printMessage(toReceive, "IAMIN");
    }

    close(fdRequest);
    numberOfFreeFileDescriptors++;
    unlink(aux);

    
    sem_unlink(semName);

    free(semName);
    free(toReceive);
    free(publicFIFO);
    free(aux);


    return NULL;
}

int main(int argc, char *argv[])
{

    srand(time(NULL));

    pthread_t thread;

    initFlags(&flags);
    setFlags(argc, argv, &flags);
    if (flags.nsecs == 0 || !hasFifoName)
    {
        fprintf(stderr, "Correct usage: U1 -t n fifoName\n");
        pthread_exit(0);
    }

    int thread_no = 0;
    clock_gettime(CLOCK_REALTIME, &start);

    struct timespec timeNow;
    clock_gettime(CLOCK_REALTIME, &timeNow);
    while (timeNow.tv_sec - start.tv_sec < flags.nsecs && !closed)
    {
        if(numberOfFreeFileDescriptors > 3){
            if (pthread_create(&thread, NULL, sendRequest, &thread_no) == 0)
            {
                thread_no += 1;
                usleep((rand() % 5000)+3000); //time between requests
            }
            else
            {
                fprintf(stderr, "Error on creating thread %d on client\n", thread_no);
            }
        }
        clock_gettime(CLOCK_REALTIME, &timeNow);
    }

    close(fd);

    pthread_exit(0);
}